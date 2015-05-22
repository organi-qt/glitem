#include "glrender.h"
#include "glnode.h"

GLRender::GLRender(GLTransformNode *root, GLLoader *loader)
    : m_root(root), m_shaders{}
{
    initializeOpenGLFunctions();
    //printOpenGLInfo();

    const QMap<QString, GLLoader::Texture> &textures = loader->textures();
    QMapIterator<QString, GLLoader::Texture> it(textures);
    while (it.hasNext()) {
        it.next();
        QOpenGLTexture *texture = new QOpenGLTexture(it.value().image.mirrored());
        texture->setMinificationFilter(QOpenGLTexture::Nearest);
        texture->setMagnificationFilter(QOpenGLTexture::Linear);
        texture->setWrapMode(it.value().mode);
        m_textures[it.key()] = texture;
    }

    const QList<Light> &lights = loader->lights();
    // init lights
    for (int i = 0; i < lights.size(); i++) {
        RenderState::RSLight light;
        light.light = lights[i];
        m_state.lights.append(light);
    }

    // mark all states dirty
    m_state.setDirty();

    initNode(m_root, lights);

    for (int i = 0; i < GLShader::NUM_SHADERS; i++) {
        if (m_shaders[i]) {
            m_shaders[i]->initialize();
            m_shaders[i]->loadBuffer(m_root, loader->vertexBufferSize()[i],
                                     loader->indexBufferSize()[i]);
        }
    }
}

GLRender::~GLRender()
{
    if (m_root)
        delete m_root;

    QMapIterator<QString, QOpenGLTexture *> it(m_textures);
    while (it.hasNext()) {
        it.next();
        delete it.value();
    }
}

void GLRender::initNode(GLTransformNode *node, const QList<Light> &lights)
{
    for (int i = 0; i < node->renderChildCount(); i++) {
        GLPhongNode *pnode = static_cast<GLPhongNode *>(node->renderChildAtIndex(i));

        if (!pnode->diffuseTexturePath().isEmpty()) {
            QMap<QString, QOpenGLTexture *>::const_iterator it =
                    m_textures.find(pnode->diffuseTexturePath());
            Q_ASSERT(it != m_textures.end());
            pnode->setDiffuseTexture(it.value());
        }

        if (!pnode->specularTexturePath().isEmpty()) {
            QMap<QString, QOpenGLTexture *>::const_iterator it =
                    m_textures.find(pnode->specularTexturePath());
            Q_ASSERT(it != m_textures.end());
            pnode->setSpecularTexture(it.value());
        }

        if (!m_shaders[pnode->type()])
            m_shaders[pnode->type()] = new GLPhongShader(lights, pnode->type());
    }

    for (int i = 0; i < node->transformChildCount(); i++)
        initNode(node->transformChildAtIndex(i), lights);
}

void GLRender::updateLightFinalPos()
{
    for (int i = 0; i < m_state.lights.size(); i++) {
        Light *light = &m_state.lights[i].light;
        QVector3D pos;
        QMatrix4x4 &mat = light->node->modelviewMatrix();
        switch (light->type) {
        case Light::POINT:
            pos = mat * light->pos;
            break;
        case Light::SUN:
            {
                QVector3D origin = mat * QVector3D();
                QVector3D direction = mat * light->pos;
                pos = direction - origin;
                pos.normalize();
            }
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

    for (int i = 0; i < GLShader::NUM_SHADERS; i++)
        if (m_shaders[i])
            m_shaders[i]->render(m_root, &m_state);
    m_state.resetDirty();

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

