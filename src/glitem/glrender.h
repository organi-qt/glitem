#ifndef GLRENDER_H
#define GLRENDER_H

#include "glshader.h"
#include "glloader.h"
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QObject>

class GLShader;
class GLTransformNode;

class GLRender : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    GLRender(GLTransformNode *root, GLLoader *loader);
    ~GLRender();

    RenderState *state() { return &m_state; }
    GLTransformNode *root() { return m_root; }
    void updateLightFinalPos();
    void setViewport(const QRect &viewport);

public slots:
    void render();

private:
    GLLoader *m_loader;
    GLTransformNode *m_root;
    RenderState m_state;
    QRect m_viewport;
    GLShader *m_shaders[GLShader::NUM_SHADERS];
    QMap<QString, QOpenGLTexture *> m_textures;

    struct OpenGLState {
        bool depth_mask;
        bool depth_test;
        bool cull_face;
        bool blend;
    };
    OpenGLState m_opengl_state;

    QOpenGLBuffer m_vertex_buffer;
    QOpenGLBuffer m_index_buffer;

    void saveOpenGLState();
    void switchOpenGlState();
    void restoreOpenGLState();

    void initMaterials();
    void initPrimitives();
    void initNodes(GLTransformNode *node, const QList<Light> &lights);

    void renderNode(GLTransformNode *);

    void printOpenGLInfo();
};

#endif // GLRENDER_H
