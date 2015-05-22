#include "glshader.h"
#include "glnode.h"
#include <QOpenGLContext>

#define GLES_FRAG_SHADER_HEADER \
    "#ifdef GL_ES\n" \
    "precision mediump float;\n" \
    "#endif\n"

GLShader::GLShader()
    : m_last_node(0),
      m_vertex_buffer(QOpenGLBuffer::VertexBuffer),
      m_index_buffer(QOpenGLBuffer::IndexBuffer)
{
}

void GLShader::initialize()
{
    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader());
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader());

    char const *const *attr = attributeNames();
    for (int i = 0; attr[i]; ++i) {
        if (*attr[i])
            m_program.bindAttributeLocation(attr[i], i);
    }

    if (!m_program.link()) {
        qWarning("GLShader: Shader compilation failed:");
        qWarning() << m_program.log();
        return;
    }

    m_program.bind();
    resolveUniforms();
    m_program.release();
}

const GLShader::AttributeSet &GLShader::defaultAttributes_NormalPoint3D()
{
    static Attribute data[] = {
        Attribute(0, GL_FLOAT, 3),
        Attribute(1, GL_FLOAT, 3)
    };

    static AttributeSet attrs = {
        2, sizeof(GLRenderNode::NormalPoint3D), data
    };
    return attrs;
}

const GLShader::AttributeSet &GLShader::defaultAttributes_TexturedNormalPoint3D()
{
    static Attribute data[] = {
        Attribute(0, GL_FLOAT, 3),
        Attribute(1, GL_FLOAT, 3),
        Attribute(2, GL_FLOAT, 2)
    };

    static AttributeSet attrs = {
        3, sizeof(GLRenderNode::TexturedNormalPoint3D), data
    };
    return attrs;
}

void GLShader::loadVertexBuffer(GLTransformNode *node)
{
    for (int i = 0; i < node->renderChildCount(); i++) {
        GLRenderNode *rnode = node->renderChildAtIndex(i);
        if (type() == rnode->type()) {
            m_vertex_buffer.write(rnode->vertexOffset(), rnode->vertexData(),
                                  rnode->vertexCount() * rnode->stride());
            rnode->freeVertexData();
        }
    }

    for (int i = 0; i < node->transformChildCount(); i++)
        loadVertexBuffer(node->transformChildAtIndex(i));
}

void GLShader::loadIndexBuffer(GLTransformNode *node)
{
    for (int i = 0; i < node->renderChildCount(); i++) {
        GLRenderNode *rnode = node->renderChildAtIndex(i);
        if (type() == rnode->type()) {
            m_index_buffer.write(rnode->indexOffset(), rnode->indexData(),
                                 rnode->indexCount() * sizeof(ushort));
            rnode->freeIndexData();
        }
    }

    for (int i = 0; i < node->transformChildCount(); i++)
        loadIndexBuffer(node->transformChildAtIndex(i));
}

void GLShader::loadBuffer(GLTransformNode *root, int vertex_buffer_size, int index_buffer_size)
{
    if (!vertex_buffer_size || !index_buffer_size)
        return;

    m_vertex_buffer.create();
    m_vertex_buffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vertex_buffer.bind();
    m_vertex_buffer.allocate(vertex_buffer_size);
    loadVertexBuffer(root);
    m_vertex_buffer.release();

    m_index_buffer.create();
    m_index_buffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_index_buffer.bind();
    m_index_buffer.allocate(index_buffer_size);
    loadIndexBuffer(root);
    m_index_buffer.release();
}

void GLShader::bind()
{
    m_program.bind();
    m_vertex_buffer.bind();

    int offset = 0;
    for (int i = 0; i < attributes().count; i++) {
        const Attribute *attr = attributes().attributes + i;
        m_program.enableAttributeArray(attr->position);
        m_program.setAttributeBuffer(attr->position, attr->type, offset,
                                     attr->tuple_size, attributes().stride);
        // now we only have float attributes
        offset += attr->tuple_size * sizeof(float);
    }

    m_index_buffer.bind();
}

void GLShader::release()
{
    m_index_buffer.release();

    for (int i = 0; i < attributes().count; i++) {
        const Attribute *attr = attributes().attributes + i;
        m_program.disableAttributeArray(attr->position);
    }

    m_program.release();
}

void GLShader::render(GLTransformNode *root, RenderState *state)
{
    if (!m_vertex_buffer.isCreated() || !m_index_buffer.isCreated())
        return;

    bind();
    updateRenderState(state);
    renderNode(root);
    release();
}

void GLShader::renderNode(GLTransformNode *node)
{
    updatePerTansformNode(node);
    for (int i = 0; i < node->renderChildCount(); i++) {
        GLRenderNode *rnode = node->renderChildAtIndex(i);
        if (type() == rnode->type()) {
            updatePerRenderNode(rnode);
            glDrawElements(GL_TRIANGLES, rnode->indexCount(), GL_UNSIGNED_SHORT,
                           (GLvoid *)rnode->indexOffset());
        }
    }

    for (int i = 0; i < node->transformChildCount(); i++)
        renderNode(node->transformChildAtIndex(i));
}

const int GLPhongShader::m_max_lights;

GLPhongShader::GLPhongShader(const QList<Light> &lights, ShaderType type)
    : GLShader(), m_lights(lights),
      m_num_lights(qMin(m_max_lights, m_lights.size())),
      m_type(type)
{
    Q_ASSERT(m_lights.size() > 0);
}

GLPhongShader::~GLPhongShader()
{
}

const QString &GLPhongShader::vertexShader() const
{
    static const QString pshader =
    "uniform highp mat4 modelview_matrix;\n"
    "uniform highp mat4 projection_matrix;\n"
    "uniform highp mat3 normal_matrix;\n"
    "attribute vec3 positionIn;\n"
    "attribute vec3 normalIn;\n"
    "varying vec3 normal;\n"
    "varying vec3 eyePosition;\n"
    "void main() {\n"
    "    vec4 eyeTemp = modelview_matrix * vec4(positionIn, 1);\n"
    "    eyePosition = eyeTemp.xyz;\n"
    "    normal = normal_matrix * normalIn;\n"
    "    gl_Position = projection_matrix * eyeTemp;\n"
    "}";

    static const QString tshader =
    "uniform highp mat4 modelview_matrix;\n"
    "uniform highp mat4 projection_matrix;\n"
    "uniform highp mat3 normal_matrix;\n"
    "attribute vec3 positionIn;\n"
    "attribute vec3 normalIn;\n"
    "attribute vec2 texcoordIn;\n"
    "varying vec3 normal;\n"
    "varying vec2 texcoord;\n"
    "varying vec3 eyePosition;\n"
    "void main() {\n"
    "    vec4 eyeTemp = modelview_matrix * vec4(positionIn, 1);\n"
    "    eyePosition = eyeTemp.xyz;\n"
    "    normal = normal_matrix * normalIn;\n"
    "    texcoord = texcoordIn;\n"
    "    gl_Position = projection_matrix * eyeTemp;\n"
    "}";

    if (m_type == PHONG)
        return pshader;
    else
        return tshader;
}

const QString &GLPhongShader::fragmentShader() const
{
    static QString shaders[4];

    if (shaders[m_type].isEmpty()) {
        QString lights_declare;
        QString lights_calc;
        for (int i = 0; i < m_num_lights; i++) {
            lights_declare += QString(
                "uniform lowp vec3 light%1_pos;\n"
                "uniform lowp vec3 light%1_amb;\n"
                "uniform lowp vec3 light%1_dif;\n"
                "uniform lowp vec3 light%1_spec;\n"
            ).arg(i);

            switch (m_lights[i].type) {
            case Light::POINT:
                lights_calc += QString(
                    "    L = normalize(light%1_pos - eyePosition);\n"
                ).arg(i);
                break;
            case Light::SUN:
                lights_calc += QString(
                    "    L = normalize(-light%1_pos);\n"
                ).arg(i);
                break;
            }

            lights_calc += QString(
                "    Rd = max(0.0, dot(L, N));\n"
                "    diffuse += Rd * light%1_dif;\n"
                "    R = reflect(-L, N);\n"
                "    Rs = pow(max(0.0, dot(V, R)), alpha);\n"
                "    specular += Rs * light%1_spec;\n"
                "    ambient += light%1_amb;\n"
            ).arg(i);
        }

        shaders[m_type] =
        GLES_FRAG_SHADER_HEADER
        "uniform lowp vec3 Ka;\n"
        "uniform lowp vec3 Kd;\n"
        "uniform lowp vec3 Ks;\n"
        "uniform lowp float alpha;\n"
        + lights_declare +
        "uniform lowp float opacity;\n"
        + (m_type == PHONG_DIFFUSE_TEXTURE || m_type == PHONG_DIFFUSE_SPECULAR_TEXTURE ?
        "uniform sampler2D diffuse_texture;\n" : "")
        + (m_type == PHONG_SPECULAR_TEXTURE || m_type == PHONG_DIFFUSE_SPECULAR_TEXTURE ?
        "uniform sampler2D specular_texture;\n" : "") +
        "varying vec3 normal;\n"
        "varying vec3 eyePosition;\n"
        + (m_type == PHONG ? "" :
        "varying vec2 texcoord;\n") +
        "void main() {\n"
        "    vec3 N = normalize(normal);\n"
        "    vec3 V = normalize(-eyePosition);\n"
        "    vec3 L, R;\n"
        "    float Rd, Rs;\n"
        "    vec3 ambient = vec3(0.0);\n"
        "    vec3 diffuse = vec3(0.0);\n"
        "    vec3 specular = vec3(0.0);\n"
        + lights_calc
        + (m_type == PHONG_DIFFUSE_TEXTURE || m_type == PHONG_DIFFUSE_SPECULAR_TEXTURE ?
        "    diffuse *= texture2D(diffuse_texture, texcoord).rgb;\n" : "")
        + (m_type == PHONG_SPECULAR_TEXTURE || m_type == PHONG_DIFFUSE_SPECULAR_TEXTURE ?
        "    specular *= texture2D(specular_texture, texcoord).rgb;\n" : "") +
        "    gl_FragColor = vec4(Ka * ambient + Kd * diffuse + Ks * specular, 1.0) * opacity;\n"
        "}";
    }

    return shaders[m_type];
}

char const *const *GLPhongShader::attributeNames() const {
    static char const *const pattr[] = { "positionIn", "normalIn", 0 };
    static char const *const tattr[] = { "positionIn", "normalIn", "texcoordIn", 0 };
    if (m_type == PHONG)
        return pattr;
    else
        return tattr;
}

void GLPhongShader::resolveUniforms() {
    m_id_modelview_matrix = program()->uniformLocation("modelview_matrix");
    if (m_id_modelview_matrix < 0) {
        qWarning("GLPhongShader does not implement 'uniform highp mat4 modelview_matrix;' in its shader");
    }

    m_id_projection_matrix = program()->uniformLocation("projection_matrix");
    if (m_id_projection_matrix < 0) {
        qWarning("GLPhongShader does not implement 'uniform highp mat4 projection_matrix;' in its shader");
    }

    m_id_normal_matrix = program()->uniformLocation("normal_matrix");
    if (m_id_normal_matrix < 0) {
        qWarning("GLPhongShader does not implement 'uniform highp mat3 normal_matrix;' in its shader");
    }

    for (int i = 0; i < m_num_lights; i++) {
        m_id_light_pos[i] = program()->uniformLocation(QString("light%1_pos").arg(i));
        if (m_id_light_pos[i] < 0) {
            qWarning("GLPhongShader does not implement 'uniform lowp vec3 light_pos' in its shader");
        }

        m_id_light_amb[i] = program()->uniformLocation(QString("light%1_amb").arg(i));
        if (m_id_light_amb[i] < 0) {
            qWarning("GLPhongShader does not implement 'uniform lowp vec3 light_amb' in its shader");
        }

        m_id_light_dif[i] = program()->uniformLocation(QString("light%1_dif").arg(i));
        if (m_id_light_dif[i] < 0) {
            qWarning("GLPhongShader does not implement 'uniform lowp vec3 light_dif' in its shader");
        }

        m_id_light_spec[i] = program()->uniformLocation(QString("light%1_spec").arg(i));
        if (m_id_light_spec[i] < 0) {
            qWarning("GLPhongShader does not implement 'uniform lowp vec3 light_spec' in its shader");
        }
    }

    m_id_opacity = program()->uniformLocation("opacity");
    if (m_id_opacity < 0) {
        qWarning("GLPhongShader does not implement 'uniform lowp float opacity' in its shader");
    }

    m_id_ka = program()->uniformLocation("Ka");
    if (m_id_ka < 0) {
        qWarning("GLPhongShader does not implement 'uniform lowp vec3 Ka' in its shader");
    }

    m_id_kd = program()->uniformLocation("Kd");
    if (m_id_kd < 0) {
        qWarning("GLPhongShader does not implement 'uniform lowp vec3 Kd' in its shader");
    }

    m_id_ks = program()->uniformLocation("Ks");
    if (m_id_ks < 0) {
        qWarning("GLPhongShader does not implement 'uniform lowp vec3 Ks' in its shader");
    }

    m_id_alpha = program()->uniformLocation("alpha");
    if (m_id_alpha < 0) {
        qWarning("GLPhongShader does not implement 'uniform lowp float alpha' in its shader");
    }

    int texture_slot = 0;
    if (m_type == PHONG_DIFFUSE_TEXTURE || m_type == PHONG_DIFFUSE_SPECULAR_TEXTURE) {
        m_id_diffuse_texture = program()->uniformLocation("diffuse_texture");
        if (m_id_diffuse_texture < 0) {
            qWarning("GLPhongShader does not implement 'uniform sampler2D diffuse_texture;' in its shader");
        }
        program()->setUniformValue(m_id_diffuse_texture, texture_slot++);
    }

    if (m_type == PHONG_SPECULAR_TEXTURE || m_type == PHONG_DIFFUSE_SPECULAR_TEXTURE) {
        m_id_specular_texture = program()->uniformLocation("specular_texture");
        if (m_id_specular_texture < 0) {
            qWarning("GLPhongShader does not implement 'uniform sampler2D specular_texture;' in its shader");
        }
        program()->setUniformValue(m_id_specular_texture, texture_slot);
    }
}

void GLPhongShader::updatePerRenderNode(GLRenderNode *n, GLRenderNode *o)
{
    GLPhongNode *pn = static_cast<GLPhongNode *>(n);
    GLPhongNode *po = static_cast<GLPhongNode *>(o);
    if (!po || po->ka() != pn->ka())
        program()->setUniformValue(m_id_ka, pn->ka());
    if (!po || po->kd() != pn->kd())
        program()->setUniformValue(m_id_kd, pn->kd());
    if (!po || po->ks() != pn->ks())
        program()->setUniformValue(m_id_ks, pn->ks());
    if (!po || po->alpha() != pn->alpha())
        program()->setUniformValue(m_id_alpha, pn->alpha());

    int texture_slot = 0;
    if ((m_type == PHONG_DIFFUSE_TEXTURE || m_type == PHONG_DIFFUSE_SPECULAR_TEXTURE) &&
        (!po || po->diffuseTexture() != pn->diffuseTexture()))
        pn->diffuseTexture()->bind(texture_slot++);
    if ((m_type == PHONG_SPECULAR_TEXTURE || m_type == PHONG_DIFFUSE_SPECULAR_TEXTURE) &&
        (!po || po->specularTexture() != pn->specularTexture()))
        pn->specularTexture()->bind(texture_slot);
}

void GLPhongShader::updatePerTansformNode(GLTransformNode *t)
{
    QMatrix4x4 &modelview = t->modelviewMatrix();
    program()->setUniformValue(m_id_modelview_matrix, modelview);
    program()->setUniformValue(m_id_normal_matrix, modelview.normalMatrix());
}

void GLPhongShader::updateRenderState(RenderState *s)
{
    if (s->projection_matrix_dirty)
        program()->setUniformValue(m_id_projection_matrix, s->projection_matrix);
    if (s->opacity_dirty)
        program()->setUniformValue(m_id_opacity, s->opacity);

    Q_ASSERT(m_num_lights <= s->lights.size());
    for (int i = 0; i < m_num_lights; i++) {
        if (s->lights[i].pos_dirty)
            program()->setUniformValue(m_id_light_pos[i], s->lights[i].final_pos);
        if (s->lights[i].amb_dirty)
            program()->setUniformValue(m_id_light_amb[i], s->lights[i].light.amb);
        if (s->lights[i].dif_dirty)
            program()->setUniformValue(m_id_light_dif[i], s->lights[i].light.dif);
        if (s->lights[i].spec_dirty)
            program()->setUniformValue(m_id_light_spec[i], s->lights[i].light.spec);
    }
}

void GLPhongShader::bind()
{
    GLPhongNode *last = static_cast<GLPhongNode *>(m_last_node);

    int texture_slot = 0;
    if ((m_type == PHONG_DIFFUSE_TEXTURE || m_type == PHONG_DIFFUSE_SPECULAR_TEXTURE) && last)
        last->diffuseTexture()->bind(texture_slot++);
    if ((m_type == PHONG_SPECULAR_TEXTURE || m_type == PHONG_DIFFUSE_SPECULAR_TEXTURE) && last)
        last->specularTexture()->bind(texture_slot);

    GLShader::bind();
}

void GLPhongShader::release()
{
    GLShader::release();

    GLPhongNode *last = static_cast<GLPhongNode *>(m_last_node);
    if ((m_type == PHONG_DIFFUSE_TEXTURE || m_type == PHONG_DIFFUSE_SPECULAR_TEXTURE) && last)
        last->diffuseTexture()->release();
    if ((m_type == PHONG_SPECULAR_TEXTURE || m_type == PHONG_DIFFUSE_SPECULAR_TEXTURE) && last)
        last->specularTexture()->release();
}

const GLShader::AttributeSet &GLPhongShader::attributes()
{
    if (m_type == PHONG)
        return defaultAttributes_NormalPoint3D();
    else
        return defaultAttributes_TexturedNormalPoint3D();
}
