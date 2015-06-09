#ifndef GLRENDER_H
#define GLRENDER_H

#include "glshader.h"
#include "renderstate.h"
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QObject>

class GLShader;
class GLTransformNode;
class Material;
class EnvParam;

struct RenderParam {
    GLTransformNode *root;
    QVector<float> *vertex;
    QVector<ushort> *index;
    QList<Material *> *materials;
    QList<Light *> *lights;
    EnvParam *env;
    int num_vertex;
};

class GLRender : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    GLRender(RenderParam *param);
    ~GLRender();

    RenderState *state() { return &m_state; }
    GLTransformNode *root() { return m_root; }
    void updateLightFinalPos();
    void setViewport(const QRect &viewport);

public slots:
    void render();

private:
    GLTransformNode *m_root;
    RenderState m_state;
    QRect m_viewport;
    GLShader *m_shaders[GLShader::NUM_SHADERS];
    QOpenGLTexture *m_envmap;
    int m_num_vertex;

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

    bool initEnvTexture(QOpenGLTexture::CubeMapFace face, QImage &image, QSize &size);

    void renderNode(GLTransformNode *);

    void printOpenGLInfo();
};

#endif // GLRENDER_H
