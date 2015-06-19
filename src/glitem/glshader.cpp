#include "glshader.h"
#include "glnode.h"
#include "mesh.h"
#include "material.h"
#include "renderstate.h"
#include <QOpenGLContext>
#include <QOpenGLTexture>


#define GLES_FRAG_SHADER_HEADER \
    "#ifdef GL_ES\n" \
    "precision mediump float;\n" \
    "#endif\n"

GLShader::GLShader()
    : m_has_transparency(false), m_has_opaque(false),
      m_blend_mode(false), m_last_node(0)
{
}

void GLShader::initialize()
{
    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader());
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader());

    char const *const *attr = attributeNames();
    for (int i = 0; i < 3; ++i) {
        if (attr[i])
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

bool GLShader::setBlend(bool mode)
{
    if ((mode && !m_has_transparency) ||
        (!mode && !m_has_opaque))
        return false;

    m_blend_mode = mode;
    return true;
}

void GLShader::bind()
{
    m_program.bind();
}

void GLShader::release()
{
    m_program.release();
}

void GLShader::resolveUniforms()
{
    m_id_opacity = program()->uniformLocation("opacity");
    if (m_id_opacity < 0) {
        qWarning("GLShader does not implement 'uniform lowp float opacity' in its shader");
    }
}

void GLShader::updateRenderState(RenderState *s)
{
    m_global_opacity = s->opacity;
}

void GLShader::updatePerRenderNode(GLRenderNode *n, GLRenderNode *o)
{
    Material *pn = n->material();
    Material *po = o ? o->material() : 0;

    float opacity = m_global_opacity * pn->opacity();
    if (!po || opacity != m_opacity) {
        program()->setUniformValue(m_id_opacity, opacity);
        m_opacity = opacity;
    }
}

void GLShader::render(GLTransformNode *root, RenderState *state)
{
    bind();
    updateRenderState(state);
    renderNode(root);
    release();
}

void GLShader::renderNode(GLTransformNode *node)
{
    if (!node->visible())
        return;

    bool updated = false;
    foreach (GLRenderNode *rnode, node->renderChildren()) {
        if (rnode->material()->shader() == this &&
            rnode->material()->transparent() == m_blend_mode &&
            rnode->visible()) {
            if (!updated) {
                updatePerTansformNode(node);
                updated = true;
            }

            updatePerRenderNode(rnode);
            glDrawElements(GL_TRIANGLES, rnode->mesh()->index_count, GL_UNSIGNED_SHORT,
                           (GLvoid *)rnode->mesh()->index_offset);
        }
    }

    foreach (GLTransformNode *tnode, node->transformChildren()) {
        renderNode(tnode);
    }
}

GLBasicShader::GLBasicShader(bool has_texture)
    : GLShader(), m_has_texture(has_texture)
{
    m_attribute_activities[0] = true;
    m_attribute_activities[1] = false;
    m_attribute_activities[2] = m_has_texture;
}

QString GLBasicShader::vertexShader()
{
    return
    QString(m_has_texture ?
    "#define TEXTURED_VERTEX\n" : "") +
    "uniform highp mat4 combined_matrix;\n"
    "attribute vec3 positionIn;\n"
    "#ifdef TEXTURED_VERTEX\n"
    "attribute vec2 texcoordIn;\n"
    "varying vec2 texcoord;\n"
    "#endif\n"
    "void main() {\n"
    "#ifdef TEXTURED_VERTEX\n"
    "    texcoord = texcoordIn;\n"
    "#endif\n"
    "    gl_Position = combined_matrix * vec4(positionIn, 1);\n"
    "}";
}

QString GLBasicShader::fragmentShader()
{
    return
    GLES_FRAG_SHADER_HEADER
    + QString(m_has_texture ?
    "#define USE_MAP\n" : "") +
    "uniform lowp float opacity;\n"
    "#ifdef USE_MAP\n"
    "uniform sampler2D texture_map;\n"
    "varying vec2 texcoord;\n"
    "#endif\n"
    "void main() {\n"
    "#ifdef USE_MAP\n"
    "    gl_FragColor = texture2D(texture_map, texcoord) * opacity;\n"
    "#else\n"
    "    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0) * opacity;\n"
    "#endif\n"
    "}\n";
}

char const *const *GLBasicShader::attributeNames() const {
    static char const *const pattr[] = { "positionIn", 0, 0 };
    static char const *const tattr[] = { "positionIn", 0, "texcoordIn" };
    if (m_has_texture)
        return tattr;
    else
        return pattr;
}

void GLBasicShader::resolveUniforms() {
    GLShader::resolveUniforms();

    m_id_combined_matrix = program()->uniformLocation("combined_matrix");
    if (m_id_combined_matrix < 0) {
        qWarning("GLBasicShader does not implement 'uniform highp mat4 combined_matrix;' in its shader");
    }

    if (m_has_texture) {
        m_id_texture_map = program()->uniformLocation("texture_map");
        if (m_id_texture_map < 0) {
            qWarning("GLBasicShader does not implement 'uniform sampler2D texture_map;' in its shader");
        }
        program()->setUniformValue(m_id_texture_map, 0);
    }
}

void GLBasicShader::updatePerRenderNode(GLRenderNode *n, GLRenderNode *o)
{
    GLShader::updatePerRenderNode(n, o);

    BasicMaterial *pn = static_cast<BasicMaterial *>(n->material());
    BasicMaterial *po = o ? static_cast<BasicMaterial *>(o->material()) : 0;

    if (m_has_texture && (!po || po->texture() != pn->texture()))
        pn->texture()->bind(0);
}

void GLBasicShader::updatePerTansformNode(GLTransformNode *t)
{
    QMatrix4x4 &modelview = t->modelviewMatrix();
    program()->setUniformValue(m_id_combined_matrix, m_projection_matrix * modelview);
}

void GLBasicShader::updateRenderState(RenderState *s)
{
    GLShader::updateRenderState(s);

    if (s->projection_matrix_dirty)
        m_projection_matrix = s->projection_matrix;
}

void GLBasicShader::bind()
{
    BasicMaterial *last =
            m_last_node ? static_cast<BasicMaterial *>(m_last_node->material()) : 0;

    if (m_has_texture && last)
        last->texture()->bind(0);

    GLShader::bind();
}

void GLBasicShader::release()
{
    GLShader::release();

    BasicMaterial *last =
            m_last_node ? static_cast<BasicMaterial *>(m_last_node->material()) : 0;

    if (m_has_texture && last)
        last->texture()->release();
}

const int GLPhongShader::m_max_lights;

GLPhongShader::GLPhongShader(const QList<Light *> *lights, bool has_diffuse_texture,
                             bool has_specular_texture, bool has_env_map)
    : GLShader(), m_lights(lights),
      m_num_lights(qMin(m_max_lights, m_lights->size())),
      m_has_diffuse_texture(has_diffuse_texture),
      m_has_specular_texture(has_specular_texture),
      m_has_env_map(has_env_map)
{
    //Q_ASSERT(m_lights->size() > 0);
    m_attribute_activities[0] = true;
    m_attribute_activities[1] = true;
    m_attribute_activities[2] = m_has_diffuse_texture || m_has_diffuse_texture;
}

QString GLPhongShader::vertexShader()
{
    return
    QString(m_has_diffuse_texture || m_has_specular_texture ?
    "#define TEXTURED_VERTEX\n" : "") +
    "uniform highp mat4 modelview_matrix;\n"
    "uniform highp mat4 projection_matrix;\n"
    "uniform highp mat3 normal_matrix;\n"
    "attribute vec3 positionIn;\n"
    "attribute vec3 normalIn;\n"
    "varying vec3 normal;\n"
    "varying vec3 eyePosition;\n"
    "#ifdef TEXTURED_VERTEX\n"
    "attribute vec2 texcoordIn;\n"
    "varying vec2 texcoord;\n"
    "#endif\n"
    "void main() {\n"
    "    vec4 eyeTemp = modelview_matrix * vec4(positionIn, 1);\n"
    "    eyePosition = eyeTemp.xyz;\n"
    "    normal = normal_matrix * normalIn;\n"
    "#ifdef TEXTURED_VERTEX\n"
    "    texcoord = texcoordIn;\n"
    "#endif\n"
    "    gl_Position = projection_matrix * eyeTemp;\n"
    "}";
}

QString GLPhongShader::fragmentShader()
{
    QString lights_declare;
    QString lights_calc;
    for (int i = 0; i < m_num_lights; i++) {
        lights_declare += QString(
            "uniform lowp vec3 light%1_pos;\n"
            "uniform lowp vec3 light%1_dif;\n"
            "uniform lowp vec3 light%1_spec;\n"
        ).arg(i);

        switch (m_lights->at(i)->type) {
        case Light::POINT:
            lights_calc += QString(
                "    L = normalize(light%1_pos - eyePosition);\n"
            ).arg(i);
            break;
        case Light::DIRECTIONAL:
            lights_calc += QString(
                "    L = normalize(-light%1_pos);\n"
            ).arg(i);
            break;
        case Light::SPOT:
            break;
        }

        lights_calc += QString(
            "    Rd = max(0.0, dot(L, N));\n"
            "    diffuse += Rd * light%1_dif;\n"
            "    R = reflect(-L, N);\n"
            "    Rs = pow(max(0.0, dot(V, R)), alpha);\n"
            "    specular += Rs * light%1_spec;\n"
        ).arg(i);
    }

    return
    GLES_FRAG_SHADER_HEADER
    + QString(m_has_diffuse_texture ?
    "#define USE_MAP\n" : "")
    + QString(m_has_specular_texture ?
    "#define USE_SPECULAR_MAP\n" : "")
    + QString(m_has_env_map ?
    "#define USE_ENV_MAP\n" : "") +
    "uniform lowp vec3 Ka;\n"
    "uniform lowp vec3 Kd;\n"
    "uniform lowp vec3 Ks;\n"
    "uniform lowp float alpha;\n"
    "uniform lowp vec3 light_amb;\n"
    + lights_declare +
    "uniform lowp float opacity;\n"
    "#ifdef USE_MAP\n"
    "uniform sampler2D diffuse_texture;\n"
    "#endif\n"
    "#ifdef USE_SPECULAR_MAP\n"
    "uniform sampler2D specular_texture;\n"
    "#endif\n"
    "#ifdef USE_ENV_MAP\n"
    "uniform samplerCube env_map;\n"
    "uniform lowp float env_alpha;\n"
    "#endif\n"
    "varying vec3 normal;\n"
    "varying vec3 eyePosition;\n"
    "#if defined(USE_MAP) || defined(USE_SPECULAR_MAP)\n"
    "varying vec2 texcoord;\n"
    "#endif\n"
    "void main() {\n"
    "    vec3 N = normalize(normal);\n"
    "    vec3 V = normalize(-eyePosition);\n"
    "    vec3 L, R;\n"
    "    float Rd, Rs;\n"
    "    vec3 diffuse = vec3(0.0);\n"
    "    vec3 specular = vec3(0.0);\n"
    + lights_calc +
    "#ifdef USE_MAP\n"
    "    diffuse *= texture2D(diffuse_texture, texcoord).rgb;\n"
    "#endif\n"
    "#ifdef USE_SPECULAR_MAP\n"
    "    specular *= texture2D(specular_texture, texcoord).rgb;\n"
    "#endif\n"
    "    vec3 color = Ka * light_amb + Kd * diffuse + Ks * specular;\n"
    "#ifdef USE_ENV_MAP\n"
    "    vec3 ER = reflect(-V, N);\n"
    "    color = mix(color, textureCube(env_map, ER).rgb, env_alpha);\n"
    "#endif\n"
    "    gl_FragColor = vec4(color, 1.0) * opacity;\n"
    "}\n";
}

char const *const *GLPhongShader::attributeNames() const {
    static char const *const pattr[] = { "positionIn", "normalIn", 0 };
    static char const *const tattr[] = { "positionIn", "normalIn", "texcoordIn" };
    if (m_has_diffuse_texture || m_has_specular_texture)
        return tattr;
    else
        return pattr;
}

void GLPhongShader::resolveUniforms() {
    GLShader::resolveUniforms();

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

    m_id_light_amb = program()->uniformLocation("light_amb");
    if (m_id_light_amb < 0) {
        qWarning("GLPhongShader does not implement 'uniform lowp vec3 light_amb' in its shader");
    }

    for (int i = 0; i < m_num_lights; i++) {
        m_id_light_pos[i] = program()->uniformLocation(QString("light%1_pos").arg(i));
        if (m_id_light_pos[i] < 0) {
            qWarning("GLPhongShader does not implement 'uniform lowp vec3 light_pos' in its shader");
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
    if (m_has_env_map) {
        m_id_env_alpha = program()->uniformLocation("env_alpha");
        if (m_id_env_alpha < 0) {
            qWarning("GLPhongShader does not implement 'uniform lowp float env_alpha' in its shader");
        }

        m_id_env_map = program()->uniformLocation("env_map");
        if (m_id_env_map < 0) {
            qWarning("GLPhongShader does not implement 'uniform samplerCube env_map;' in its shader");
        }
        program()->setUniformValue(m_id_env_map, texture_slot++);
    }


    if (m_has_diffuse_texture) {
        m_id_diffuse_texture = program()->uniformLocation("diffuse_texture");
        if (m_id_diffuse_texture < 0) {
            qWarning("GLPhongShader does not implement 'uniform sampler2D diffuse_texture;' in its shader");
        }
        program()->setUniformValue(m_id_diffuse_texture, texture_slot++);
    }

    if (m_has_specular_texture) {
        m_id_specular_texture = program()->uniformLocation("specular_texture");
        if (m_id_specular_texture < 0) {
            qWarning("GLPhongShader does not implement 'uniform sampler2D specular_texture;' in its shader");
        }
        program()->setUniformValue(m_id_specular_texture, texture_slot);
    }
}

void GLPhongShader::updatePerRenderNode(GLRenderNode *n, GLRenderNode *o)
{
    GLShader::updatePerRenderNode(n, o);

    PhongMaterial *pn = static_cast<PhongMaterial *>(n->material());
    PhongMaterial *po = o ? static_cast<PhongMaterial *>(o->material()) : 0;

    if (!po || po->ka() != pn->ka())
        program()->setUniformValue(m_id_ka, pn->ka());
    if (!po || po->kd() != pn->kd())
        program()->setUniformValue(m_id_kd, pn->kd());
    if (!po || po->ks() != pn->ks())
        program()->setUniformValue(m_id_ks, pn->ks());
    if (!po || po->alpha() != pn->alpha())
        program()->setUniformValue(m_id_alpha, pn->alpha());

    if (m_has_env_map && (!po || po->env_alpha() != pn->env_alpha()))
        program()->setUniformValue(m_id_env_alpha, pn->env_alpha());

    int texture_slot = m_has_env_map ? 1 : 0;
    if (m_has_diffuse_texture && (!po || po->diffuseTexture() != pn->diffuseTexture()))
        pn->diffuseTexture()->bind(texture_slot++);
    if (m_has_specular_texture && (!po || po->specularTexture() != pn->specularTexture()))
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
    GLShader::updateRenderState(s);

    if (s->projection_matrix_dirty)
        program()->setUniformValue(m_id_projection_matrix, s->projection_matrix);
    if (s->light_amb_dirty)
        program()->setUniformValue(m_id_light_amb, s->light_amb);

    if (m_has_env_map && s->envmap)
        s->envmap->bind(0);

    Q_ASSERT(m_num_lights <= s->lights.size());
    for (int i = 0; i < m_num_lights; i++) {
        if (s->lights[i].final_pos_dirty)
            program()->setUniformValue(m_id_light_pos[i], s->lights[i].final_pos);
        if (s->lights[i].light->dif_dirty)
            program()->setUniformValue(m_id_light_dif[i], s->lights[i].light->dif);
        if (s->lights[i].light->spec_dirty)
            program()->setUniformValue(m_id_light_spec[i], s->lights[i].light->spec);
    }
}

void GLPhongShader::bind()
{
    PhongMaterial *last =
            m_last_node ? static_cast<PhongMaterial *>(m_last_node->material()) : 0;

    int texture_slot = m_has_env_map ? 1 : 0;
    if (m_has_diffuse_texture && last)
        last->diffuseTexture()->bind(texture_slot++);
    if (m_has_specular_texture && last)
        last->specularTexture()->bind(texture_slot);

    GLShader::bind();
}

void GLPhongShader::release()
{
    GLShader::release();

    PhongMaterial *last =
            m_last_node ? static_cast<PhongMaterial *>(m_last_node->material()) : 0;

    if (m_has_diffuse_texture && last)
        last->diffuseTexture()->release();
    if (m_has_specular_texture && last)
        last->specularTexture()->release();
}
