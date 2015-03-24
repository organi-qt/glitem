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

    GLTransformNode *root() { return m_root; }
    void setOpacity(float value) { m_state.setOpacity(value); }
    void setLightPos(const QVector3D &value) { m_state.setLightPos(value); }
    void setLightAmb(const QVector3D &value) { m_state.setLightAmb(value); }
    void setLightDif(const QVector3D &value) { m_state.setLightDif(value); }
    void setLightSpec(const QVector3D &value) { m_state.setLightSpec(value); }
    void setProjectionMatrix(const QMatrix4x4 &value) { m_state.setProjectionMatrix(value); }

public slots:
    void render();

private:
    GLTransformNode *m_root;
    GLShader::RenderState m_state;
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
