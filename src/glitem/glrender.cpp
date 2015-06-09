#include "glrender.h"
#include "glnode.h"
#include "glenvironment.h"
#include "material.h"


GLRender::GLRender(RenderParam *param)
    : m_root(param->root), m_shaders{}, m_envmap(0),
      m_num_vertex(param->num_vertex),
      m_vertex_buffer(QOpenGLBuffer::VertexBuffer),
      m_index_buffer(QOpenGLBuffer::IndexBuffer)
{
    initializeOpenGLFunctions();
    //printOpenGLInfo();

    // init lights
    m_state.lights.resize(param->lights->size());
    for (int i = 0; i < param->lights->size(); i++)
        m_state.lights[i].light = param->lights->at(i);

    EnvParam *env = param->env;
    if (env) {
        m_envmap = new QOpenGLTexture(QOpenGLTexture::TargetCubeMap);
        m_envmap->setSize(env->width, env->height);
        if (QOpenGLContext::currentContext()->isOpenGLES())
            m_envmap->setFormat(QOpenGLTexture::RGBFormat);
        else
            m_envmap->setFormat(QOpenGLTexture::RGB32F);
        m_envmap->setWrapMode(QOpenGLTexture::DirectionS, QOpenGLTexture::ClampToEdge);
        m_envmap->setWrapMode(QOpenGLTexture::DirectionT, QOpenGLTexture::ClampToEdge);
        //m_envmap->setWrapMode(QOpenGLTexture::DirectionR, QOpenGLTexture::ClampToEdge);
        m_envmap->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
        m_envmap->allocateStorage();

        QSize size(env->width, env->height);
        if (!initEnvTexture(QOpenGLTexture::CubeMapPositiveX, env->right, size) ||
            !initEnvTexture(QOpenGLTexture::CubeMapNegativeX, env->left, size) ||
            !initEnvTexture(QOpenGLTexture::CubeMapPositiveY, env->top, size) ||
            !initEnvTexture(QOpenGLTexture::CubeMapNegativeY, env->bottom, size) ||
            !initEnvTexture(QOpenGLTexture::CubeMapPositiveZ, env->back, size) ||
            !initEnvTexture(QOpenGLTexture::CubeMapNegativeZ, env->front, size)) {
            delete m_envmap;
            m_envmap = 0;
        }

        m_state.setEnvAlpha(env->alpha);
    }

    // mark all states dirty
    m_state.setDirty();

    // init primitives
    m_vertex_buffer.create();
    m_vertex_buffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vertex_buffer.bind();
    m_vertex_buffer.allocate(param->vertex->data(), param->vertex->size() * sizeof(float));
    m_vertex_buffer.release();

    m_index_buffer.create();
    m_index_buffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_index_buffer.bind();
    m_index_buffer.allocate(param->index->data(), param->index->size() * sizeof(ushort));
    m_index_buffer.release();

    foreach (Material *material, *param->materials) {
        material->init();

        GLShader::ShaderType type = material->type();
        if (!m_shaders[type])
            m_shaders[type] = new GLPhongShader(param->lights, type, m_envmap ? true : false);
    }

    for (int i = 0; i < GLShader::NUM_SHADERS; i++)
        if (m_shaders[i])
            m_shaders[i]->initialize();
}

GLRender::~GLRender()
{
    if (m_envmap)
        delete m_envmap;
}

bool GLRender::initEnvTexture(QOpenGLTexture::CubeMapFace face, QImage &image, QSize &size)
{
    if (image.isNull()) {
        static QByteArray data(size.width() * size.height() * 3, 0);
        m_envmap->setData(0, 0, face, QOpenGLTexture::RGB,
                          QOpenGLTexture::UInt8, data.constData());
    }
    else
        m_envmap->setData(0, 0, face, QOpenGLTexture::RGB,
                          QOpenGLTexture::UInt8, image.constBits());

    int err = glGetError();
    if (err) {
        qWarning() << "environment map texture upload fail: " << err;
        return false;
    }
    else
        return true;
}

void GLRender::updateLightFinalPos()
{
    for (int i = 0; i < m_state.lights.size(); i++) {
        Light *light = m_state.lights[i].light;
        QVector3D pos;
        QMatrix4x4 &mat = light->node->modelviewMatrix();
        switch (light->type) {
        case Light::POINT:
            pos = mat * light->pos;
            break;
        case Light::DIRECTIONAL:
            {
                QVector3D origin = mat * QVector3D();
                QVector3D direction = mat * light->pos;
                pos = direction - origin;
                pos.normalize();
            }
            break;
        case Light::SPOT:
            break;
        }

        m_state.setLightFinalPos(i, pos);
    }
}

void GLRender::setViewport(const QRect &viewport)
{
    if (m_viewport == viewport)
        return;
    m_viewport = viewport;

    float ratio = m_viewport.width();
    ratio /= m_viewport.height();

    QMatrix4x4 mat;
    mat.perspective(60, ratio, 0.1, 100.0);
    m_state.setProjectionMatrix(mat);
}

void GLRender::saveOpenGLState()
{
    GLboolean value;
    glGetBooleanv(GL_DEPTH_TEST, &value);
    if (value == GL_TRUE)
        m_opengl_state.depth_test = true;
    else
        m_opengl_state.depth_test = false;

    glGetBooleanv(GL_DEPTH_WRITEMASK, &value);
    if (value == GL_TRUE)
        m_opengl_state.depth_mask = true;
    else
        m_opengl_state.depth_mask = false;

    glGetBooleanv(GL_CULL_FACE, &value);
    if (value == GL_TRUE)
        m_opengl_state.cull_face = true;
    else
        m_opengl_state.cull_face = false;

    glGetBooleanv(GL_BLEND, &value);
    if (value == GL_TRUE)
        m_opengl_state.blend = true;
    else
        m_opengl_state.blend = false;
/*
    GLboolean cmask[4];
    glGetBooleanv(GL_COLOR_WRITEMASK, cmask);
    qDebug() << "cmask: r=" << cmask[0] << " g=" << cmask[1] << " b=" << cmask[2]
             << " a=" << cmask[3];
*/
    //glGetBooleanv(GL_MULTISAMPLE, &value);
    //qDebug() << "GL_MULTISAMPLE=" << value;
}

void GLRender::switchOpenGlState()
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

void GLRender::restoreOpenGLState()
{
    if (m_opengl_state.depth_test)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    if (m_opengl_state.depth_mask)
        glDepthMask(GL_TRUE);
    else
        glDepthMask(GL_FALSE);

    if (m_opengl_state.cull_face)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);

    if (m_opengl_state.blend)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
}

void GLRender::render()
{
    if (!m_state.visible)
        return;

    saveOpenGLState();
    switchOpenGlState();

    glViewport(m_viewport.x(), m_viewport.y(), m_viewport.width(), m_viewport.height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_envmap)
        m_envmap->bind(0);

    m_vertex_buffer.bind();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 6 * sizeof(float), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 6 * sizeof(float),
                          (void *)(3 * sizeof(float)));

    bool init_done = false;
    bool texture_enabled = false;
    for (int i = 0; i < GLShader::NUM_SHADERS; i++) {
        if (!m_shaders[i])
            continue;

        if (i && !texture_enabled) {
            if (init_done)
                m_vertex_buffer.bind();

            glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float),
                                  (void *)(m_num_vertex * 6 * sizeof(float)));
            glEnableVertexAttribArray(2);
            texture_enabled = true;

            if (init_done)
                m_index_buffer.bind();
        }

        if (!init_done) {
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            m_index_buffer.bind();
            init_done = true;
        }

        m_shaders[i]->render(m_root, &m_state);
    }
    m_state.resetDirty();

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    m_index_buffer.release();
    m_vertex_buffer.release();

    if (m_envmap)
        m_envmap->release();

    restoreOpenGLState();
}

void GLRender::printOpenGLInfo()
{
    QOpenGLContext *context = QOpenGLContext::currentContext();

    if (context->isOpenGLES()) {
        if (context->hasExtension("GL_OES_vertex_array_object"))
            qDebug() << "OGLES has GL_OES_vertex_array_object extension";
        else
            qDebug() << "OGLES has no GL_OES_vertex_array_object extension";
        qDebug() << "OGLES extensions: " << context->extensions();
    }
    else {
        if (context->hasExtension("GL_ARB_vertex_array_object"))
            qDebug() << "OGL has GL_ARB_vertex_array_object extension";
        else
            qDebug() << "OGL has no GL_ARB_vertex_array_object extension";
        qDebug() << "OGL extensions: " << context->extensions();
    }

    if (context->hasExtension("GL_KHR_debug"))
        qDebug() << "OGL has GL_KHR_debug extension";
    else
        qDebug() << "OGL has no GL_KHR_debug extension";

    qDebug() << "OGL surface format " << context->format();
}

