#ifndef GLRENDER_H
#define GLRENDER_H

#include "glshader.h"
#include "glloader.h"
#include <QOpenGLFunctions>
#include <QObject>

class GLShader;
class GLTransformNode;

class GLRender : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    GLRender(GLTransformNode *root, const QRect &viewport, GLLoader *loader);
    ~GLRender();

    RenderState *state() { return &m_state; }
    GLTransformNode *root() { return m_root; }

public slots:
    void render();

private:
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

    void saveOpenGLState();
    void switchOpenGlState();
    void restoreOpenGLState();

    void initTexture(GLTransformNode *);
    void renderNode(GLTransformNode *);

    void printOpenGLInfo();
};

#endif // GLRENDER_H
