#ifndef GLSHADER_H
#define GLSHADER_H

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class GLRenderNode;
class GLTransformNode;

class GLShader
{
public:
    enum ShaderType {
        PHONG = 0,
        PHONG_DIFFUSE_TEXTURE,
        NUM_SHADERS
    };

    struct RenderState {
        QMatrix4x4 projection_matrix;
        float opacity;
        QVector3D light_pos;
        QVector3D light_amb;
        QVector3D light_dif;
        QVector3D light_spec;

        bool projection_matrix_dirty;
        bool opacity_dirty;
        bool light_pos_dirty;
        bool light_amb_dirty;
        bool light_dif_dirty;
        bool light_spec_dirty;

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

        void setLightPos(const QVector3D &value) {
            if (light_pos != value) {
                light_pos = value;
                light_pos_dirty = true;
            }
        }

        void setLightAmb(const QVector3D &value) {
            if (light_amb != value) {
                light_amb = value;
                light_amb_dirty = true;
            }
        }

        void setLightDif(const QVector3D &value) {
            if (light_dif != value) {
                light_dif = value;
                light_dif_dirty = true;
            }
        }

        void setLightSpec(const QVector3D &value) {
            if (light_spec != value) {
                light_spec = value;
                light_spec_dirty = true;
            }
        }

        void setDirty() {
            projection_matrix_dirty = true;
            opacity_dirty = true;
            light_pos_dirty = true;
            light_amb_dirty = true;
            light_dif_dirty = true;
            light_spec_dirty = true;
        }

        void resetDirty() {
            projection_matrix_dirty = false;
            opacity_dirty = false;
            light_pos_dirty = false;
            light_amb_dirty = false;
            light_dif_dirty = false;
            light_spec_dirty = false;
        }
    };

    GLShader();
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
    virtual const char *fragmentShader() const = 0;
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
    GLPhongShader();

protected:
    virtual void resolveUniforms();
    virtual const AttributeSet &attributes() { return defaultAttributes_NormalPoint3D(); }
    virtual ShaderType type() { return PHONG; }
    virtual void updatePerRenderNode(GLRenderNode *n, GLRenderNode *o);

private:
    int m_id_opacity;
    int m_id_modelview_matrix;
    int m_id_projection_matrix;
    int m_id_normal_matrix;
    int m_id_light_pos;
    int m_id_light_amb;
    int m_id_light_dif;
    int m_id_light_spec;
    int m_id_ka;
    int m_id_kd;
    int m_id_ks;
    int m_id_alpha;

    virtual const char *vertexShader() const;
    virtual const char *fragmentShader() const;
    virtual char const *const *attributeNames() const;
    virtual void updatePerTansformNode(GLTransformNode *);
    virtual void updateRenderState(RenderState *);
};

class GLPhongDiffuseTextureShader : public GLPhongShader
{
public:
    GLPhongDiffuseTextureShader();

protected:
    virtual void resolveUniforms();
    virtual const AttributeSet &attributes() { return defaultAttributes_TexturedNormalPoint3D(); }
    virtual ShaderType type() { return PHONG_DIFFUSE_TEXTURE; }

    virtual void bind();
    virtual void release();

private:
    int m_id_diffuse_texture;

    virtual const char *vertexShader() const;
    virtual const char *fragmentShader() const;
    virtual char const *const *attributeNames() const;
    virtual void updatePerRenderNode(GLRenderNode *n, GLRenderNode *o);
};

#endif // GLSHADER_H
