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

GLPhongShader::GLPhongShader()
    : GLShader()
{

}

const char *GLPhongShader::vertexShader() const
{
    return
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
}

const char *GLPhongShader::fragmentShader() const
{
    return
    GLES_FRAG_SHADER_HEADER
    "uniform lowp vec3 Ka;\n"
    "uniform lowp vec3 Kd;\n"
    "uniform lowp vec3 Ks;\n"
    "uniform lowp float alpha;\n"
    "uniform lowp vec3 light_pos;\n"
    "uniform lowp vec3 light_amb;\n"
    "uniform lowp vec3 light_dif;\n"
    "uniform lowp vec3 light_spec;\n"
    "uniform lowp float opacity;\n"
    "varying vec3 normal;\n"
    "varying vec3 eyePosition;\n"
    "void main() {\n"
    "    vec3 N = normalize(normal);\n"
    "    vec3 L = normalize(light_pos - eyePosition);\n"
    "    vec3 V = normalize(-eyePosition);\n"
    "    float Rd = max(0.0, dot(L, N));\n"
    "    vec3 diffuse = Rd * Kd * light_dif;\n"
    "    vec3 R = reflect(-L, N);\n"
    "    float Rs = pow(max(0.0, dot(V, R)), alpha);\n"
    "    vec3 specular = Rs * Ks * light_spec;\n"
    "    vec3 ambient = Ka * light_amb;\n"
    "    gl_FragColor = vec4(ambient + diffuse + specular, 1.0) * opacity;\n"
    "}";
}

char const *const *GLPhongShader::attributeNames() const {
    static char const *const attr[] = { "positionIn", "normalIn", 0 };
    return attr;
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

    m_id_light_pos = program()->uniformLocation("light_pos");
    if (m_id_light_pos < 0) {
        qWarning("GLPhongShader does not implement 'uniform lowp vec3 light_pos' in its shader");
    }

    m_id_light_amb = program()->uniformLocation("light_amb");
    if (m_id_light_amb < 0) {
        qWarning("GLPhongShader does not implement 'uniform lowp vec3 light_amb' in its shader");
    }

    m_id_light_dif = program()->uniformLocation("light_dif");
    if (m_id_light_dif < 0) {
        qWarning("GLPhongShader does not implement 'uniform lowp vec3 light_dif' in its shader");
    }

    m_id_light_spec = program()->uniformLocation("light_spec");
    if (m_id_light_spec < 0) {
        qWarning("GLPhongShader does not implement 'uniform lowp vec3 light_spec' in its shader");
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
    if (s->light_pos_dirty)
        program()->setUniformValue(m_id_light_pos, s->light_pos);
    if (s->light_amb_dirty)
        program()->setUniformValue(m_id_light_amb, s->light_amb);
    if (s->light_dif_dirty)
        program()->setUniformValue(m_id_light_dif, s->light_dif);
    if (s->light_spec_dirty)
        program()->setUniformValue(m_id_light_spec, s->light_spec);
}

GLPhongDiffuseTextureShader::GLPhongDiffuseTextureShader()
    : GLPhongShader()
{

}

void GLPhongDiffuseTextureShader::resolveUniforms()
{
    GLPhongShader::resolveUniforms();

    m_id_diffuse_texture = program()->uniformLocation("diffuse_texture");
    if (m_id_diffuse_texture < 0) {
        qWarning("GLPhongDiffuseTextureShader does not implement 'uniform sampler2D diffuse_texture;' in its shader");
    }
    program()->setUniformValue(m_id_diffuse_texture, 0);
}

const char *GLPhongDiffuseTextureShader::vertexShader() const {
    return
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
}

const char *GLPhongDiffuseTextureShader::fragmentShader() const {
    return
    GLES_FRAG_SHADER_HEADER
    "uniform sampler2D diffuse_texture;\n"
    "uniform lowp vec3 Ka;\n"
    "uniform lowp vec3 Kd;\n"
    "uniform lowp vec3 Ks;\n"
    "uniform lowp float alpha;\n"
    "uniform lowp vec3 light_pos;\n"
    "uniform lowp vec3 light_amb;\n"
    "uniform lowp vec3 light_dif;\n"
    "uniform lowp vec3 light_spec;\n"
    "uniform lowp float opacity;\n"
    "varying vec3 normal;\n"
    "varying vec2 texcoord;\n"
    "varying vec3 eyePosition;\n"
    "void main() {\n"
    "    vec3 N = normalize(normal);\n"
    "    vec3 L = normalize(light_pos - eyePosition);\n"
    "    vec3 V = normalize(-eyePosition);\n"
    "    float Rd = max(0.0, dot(L, N));\n"
    "    vec3 Td = texture2D(diffuse_texture, texcoord).rgb;"
    "    vec3 diffuse = Rd * Kd * Td * light_dif;\n"
    "    vec3 R = reflect(-L, N);\n"
    "    float Rs = pow(max(0.0, dot(V, R)), alpha);\n"
    "    vec3 specular = Rs * Ks * light_spec;\n"
    "    vec3 ambient = Ka * light_amb;\n"
    "    gl_FragColor = vec4(diffuse + specular + ambient, 1.0) * opacity;\n"
    "}";
}

char const *const *GLPhongDiffuseTextureShader::attributeNames() const {
    static char const *const attr[] = { "positionIn", "normalIn", "texcoordIn", 0 };
    return attr;
}

void GLPhongDiffuseTextureShader::updatePerRenderNode(GLRenderNode *n, GLRenderNode *o)
{
    GLPhongShader::updatePerRenderNode(n, o);

    GLPhongDiffuseTextureNode *pn = static_cast<GLPhongDiffuseTextureNode *>(n);
    GLPhongDiffuseTextureNode *po = static_cast<GLPhongDiffuseTextureNode *>(o);
    if (!po || po->texture() != pn->texture())
        pn->texture()->bind(0);
}

void GLPhongDiffuseTextureShader::bind()
{
    GLPhongDiffuseTextureNode *last = static_cast<GLPhongDiffuseTextureNode *>(m_last_node);
    if (last)
        last->texture()->bind(0);

    GLShader::bind();
}

void GLPhongDiffuseTextureShader::release()
{
    GLShader::release();

    GLPhongDiffuseTextureNode *last = static_cast<GLPhongDiffuseTextureNode *>(m_last_node);
    if (last)
        last->texture()->release();
}



