#ifndef GLSHADER_H
#define GLSHADER_H

#include "gllight.h"
#include <QList>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class GLRenderNode;
class GLTransformNode;

struct RenderState {
    QMatrix4x4 projection_matrix;
    float opacity;

    struct RSLight {
        Light light;
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
        if (i >= lights.size())
            return;

        if (lights[i].light.pos != value) {
            lights[i].light.pos = value;
            lights[i].pos_dirty = true;
        }
    }

    void setLightAmb(int i, const QVector3D &value) {
        if (i >= lights.size())
            return;

        if (lights[i].light.amb != value) {
            lights[i].light.amb = value;
            lights[i].amb_dirty = true;
        }
    }

    void setLightDif(int i, const QVector3D &value) {
        if (i >= lights.size())
            return;

        if (lights[i].light.dif != value) {
            lights[i].light.dif = value;
            lights[i].dif_dirty = true;
        }
    }

    void setLightSpec(int i, const QVector3D &value) {
        if (i >= lights.size())
            return;

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
        NUM_SHADERS
    };

    GLShader();
    virtual ~GLShader() {}
    QOpenGLShaderProgram *program() { return &m_program; }

    void initialize();
    void loadBuffer(GLTransformNode *root, int vertex_buffer_size, int index_buffer_size);
    void render(GLTransformNode *root, RenderState *state);

protected:
    struct Attribute {
        int position;
        int type;
        int tuple_size;

        Attribute(int npos, int ntype, int ntuple)
            : position(npos), type(ntype), tuple_size(ntuple) {}
    };

    struct AttributeSet {
        int count;
        int stride;
        const Attribute *attributes;
    };

    GLRenderNode *m_last_node;

    virtual const AttributeSet &attributes() = 0;
    virtual ShaderType type() = 0;
    virtual void resolveUniforms() = 0;
    static const AttributeSet &defaultAttributes_NormalPoint3D();
    static const AttributeSet &defaultAttributes_TexturedNormalPoint3D();

    virtual void bind();
    virtual void release();

private:
    virtual const char *vertexShader() const = 0;
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
    QOpenGLBuffer m_vertex_buffer;
    QOpenGLBuffer m_index_buffer;
};

class GLPhongShader : public GLShader
{
public:
    GLPhongShader(const QList<Light> &lights);
    virtual ~GLPhongShader();

protected:
    const QList<Light> m_lights;
    const int m_num_lights;

    virtual void resolveUniforms();
    virtual const AttributeSet &attributes() { return defaultAttributes_NormalPoint3D(); }
    virtual ShaderType type() { return PHONG; }
    virtual void updatePerRenderNode(GLRenderNode *n, GLRenderNode *o);

private:
    static const int m_max_lights = 5;
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

    virtual const char *vertexShader() const;
    virtual const QString &fragmentShader() const;
    virtual char const *const *attributeNames() const;
    virtual void updatePerTansformNode(GLTransformNode *);
    virtual void updateRenderState(RenderState *);
};

class GLPhongDiffuseTextureShader : public GLPhongShader
{
public:
    GLPhongDiffuseTextureShader(const QList<Light> &lights);

protected:
    virtual void resolveUniforms();
    virtual const AttributeSet &attributes() { return defaultAttributes_TexturedNormalPoint3D(); }
    virtual ShaderType type() { return PHONG_DIFFUSE_TEXTURE; }

    virtual void bind();
    virtual void release();

private:
    int m_id_diffuse_texture;

    virtual const char *vertexShader() const;
    virtual const QString &fragmentShader() const;
    virtual char const *const *attributeNames() const;
    virtual void updatePerRenderNode(GLRenderNode *n, GLRenderNode *o);
};

#endif // GLSHADER_H
