#ifndef GLSHADER_H
#define GLSHADER_H

#include "gllight.h"
#include <QList>
#include <QOpenGLShaderProgram>

class GLRenderNode;
class GLTransformNode;

struct RenderState {
    QMatrix4x4 projection_matrix;
    float opacity;
    bool visible;

    struct RSLight {
        Light light;
        QVector3D final_pos;
        bool pos_dirty;
        bool amb_dirty;
        bool dif_dirty;
        bool spec_dirty;
    };
    QList<RSLight> lights;

    bool projection_matrix_dirty;
    bool opacity_dirty;

    void setProjectionMatrix(const QMatrix4x4 &value) {
        if (projection_matrix != value) {
            projection_matrix = value;
            projection_matrix_dirty = true;
        }
    }

    void setOpacity(float value) {
        if (opacity != value) {
            opacity = value;
            opacity_dirty = true;
        }
    }

    void setLightPos(int i, const QVector3D &value) {
        Q_ASSERT(i >= 0 && i < lights.size());

        lights[i].light.pos = value;
    }

    void setLightFinalPos(int i, const QVector3D &value) {
        Q_ASSERT(i >= 0 && i < lights.size());

        if (lights[i].final_pos != value) {
            lights[i].final_pos = value;
            lights[i].pos_dirty = true;
        }
    }

    void setLightAmb(int i, const QVector3D &value) {
        Q_ASSERT(i >= 0 && i < lights.size());

        if (lights[i].light.amb != value) {
            lights[i].light.amb = value;
            lights[i].amb_dirty = true;
        }
    }

    void setLightDif(int i, const QVector3D &value) {
        Q_ASSERT(i >= 0 && i < lights.size());

        if (lights[i].light.dif != value) {
            lights[i].light.dif = value;
            lights[i].dif_dirty = true;
        }
    }

    void setLightSpec(int i, const QVector3D &value) {
        Q_ASSERT(i >= 0 && i < lights.size());

        if (lights[i].light.spec != value) {
            lights[i].light.spec = value;
            lights[i].spec_dirty = true;
        }
    }

    void setDirty() {
        projection_matrix_dirty = true;
        opacity_dirty = true;
        for (int i = 0; i < lights.size(); i++) {
            lights[i].pos_dirty = true;
            lights[i].amb_dirty = true;
            lights[i].dif_dirty = true;
            lights[i].spec_dirty = true;
        }
    }

    void resetDirty() {
        projection_matrix_dirty = false;
        opacity_dirty = false;
        for (int i = 0; i < lights.size(); i++) {
            lights[i].pos_dirty = false;
            lights[i].amb_dirty = false;
            lights[i].dif_dirty = false;
            lights[i].spec_dirty = false;
        }
    }
};

class GLShader
{
public:
    enum ShaderType {
        PHONG = 0,
        PHONG_DIFFUSE_TEXTURE,
        PHONG_SPECULAR_TEXTURE,
        PHONG_DIFFUSE_SPECULAR_TEXTURE,
        NUM_SHADERS
    };

    GLShader();
    virtual ~GLShader() {}
    QOpenGLShaderProgram *program() { return &m_program; }

    void initialize();
    void render(GLTransformNode *root, RenderState *state);

protected:
    GLRenderNode *m_last_node;

    virtual ShaderType type() = 0;
    virtual void resolveUniforms() = 0;

    virtual void bind();
    virtual void release();

private:
    virtual const QString &vertexShader() const = 0;
    virtual const QString &fragmentShader() const = 0;
    virtual char const *const *attributeNames() const = 0;
    virtual void updatePerRenderNode(GLRenderNode *, GLRenderNode *) {}
    virtual void updatePerTansformNode(GLTransformNode *) {}
    virtual void updateRenderState(RenderState *) {}
    void updatePerRenderNode(GLRenderNode *n) {
        updatePerRenderNode(n, m_last_node);
        m_last_node = n;
    }
    void loadVertexBuffer(GLTransformNode *);
    void loadIndexBuffer(GLTransformNode *);

    void renderNode(GLTransformNode *);

    QOpenGLShaderProgram m_program;
};

class GLPhongShader : public GLShader
{
public:
    GLPhongShader(const QList<Light> &lights, ShaderType type);
    virtual ~GLPhongShader();

protected:
    const QList<Light> m_lights;
    const int m_num_lights;

    virtual void bind();
    virtual void release();

    virtual void resolveUniforms();
    virtual ShaderType type() { return m_type; }
    virtual void updatePerRenderNode(GLRenderNode *n, GLRenderNode *o);

private:
    static const int m_max_lights = 5;
    const ShaderType m_type;
    int m_id_opacity;
    int m_id_modelview_matrix;
    int m_id_projection_matrix;
    int m_id_normal_matrix;
    int m_id_light_pos[m_max_lights];
    int m_id_light_amb[m_max_lights];
    int m_id_light_dif[m_max_lights];
    int m_id_light_spec[m_max_lights];
    int m_id_ka;
    int m_id_kd;
    int m_id_ks;
    int m_id_alpha;
    int m_id_diffuse_texture;
    int m_id_specular_texture;

    virtual const QString &vertexShader() const;
    virtual const QString &fragmentShader() const;
    virtual char const *const *attributeNames() const;
    virtual void updatePerTansformNode(GLTransformNode *);
    virtual void updateRenderState(RenderState *);
};

#endif // GLSHADER_H
