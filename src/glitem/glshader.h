#ifndef GLSHADER_H
#define GLSHADER_H

#include <QList>
#include <QOpenGLShaderProgram>

class Light;
class RenderState;
class GLRenderNode;
class GLTransformNode;

class GLShader
{
public:
    GLShader();
    QOpenGLShaderProgram *program() { return &m_program; }

    void initialize();
    void render(GLTransformNode *root, RenderState *state);

    bool *attributeActivities() { return m_attribute_activities; }

    void hasTransparency() { m_has_transparency = true; }
    void hasOpaque() { m_has_opaque = true; }
    bool setBlend(bool mode);

protected:
    bool m_has_transparency;
    bool m_has_opaque;
    bool m_blend_mode;

    float m_global_opacity;
    GLRenderNode *m_last_node;
    bool m_attribute_activities[3];

    virtual void resolveUniforms();
    virtual void bind();
    virtual void release();

    virtual void updatePerRenderNode(GLRenderNode *, GLRenderNode *);
    virtual void updatePerTansformNode(GLTransformNode *) {}
    virtual void updateRenderState(RenderState *);

private:
    float m_opacity;
    int m_id_opacity;
    QOpenGLShaderProgram m_program;

    virtual QString vertexShader() = 0;
    virtual QString fragmentShader() = 0;
    virtual char const *const *attributeNames() const = 0;
    void updatePerRenderNode(GLRenderNode *n) {
        updatePerRenderNode(n, m_last_node);
        m_last_node = n;
    }
    void loadVertexBuffer(GLTransformNode *);
    void loadIndexBuffer(GLTransformNode *);

    void renderNode(GLTransformNode *);
};

class GLBasicShader : public GLShader
{
public:
    GLBasicShader(bool has_texture);

protected:
    virtual void bind();
    virtual void release();

    virtual void resolveUniforms();
    virtual void updatePerRenderNode(GLRenderNode *n, GLRenderNode *o);

private:
    bool m_has_texture;
    QMatrix4x4 m_projection_matrix;

    int m_id_texture_map;
    int m_id_combined_matrix;

    virtual QString vertexShader();
    virtual QString fragmentShader();
    virtual char const *const *attributeNames() const;
    virtual void updatePerTansformNode(GLTransformNode *);
    virtual void updateRenderState(RenderState *);
};

class GLPhongShader : public GLShader
{
public:
    GLPhongShader(const QList<Light *> *lights, bool has_diffuse_texture,
                  bool has_specular_texture, bool has_env_map);

protected:
    const QList<Light *> *m_lights;
    const int m_num_lights;

    virtual void bind();
    virtual void release();

    virtual void resolveUniforms();
    virtual void updatePerRenderNode(GLRenderNode *n, GLRenderNode *o);

private:
    static const int m_max_lights = 5;
    bool m_has_diffuse_texture;
    bool m_has_specular_texture;
    bool m_has_env_map;

    int m_id_modelview_matrix;
    int m_id_projection_matrix;
    int m_id_normal_matrix;
    int m_id_light_amb;
    int m_id_light_pos[m_max_lights];
    int m_id_light_dif[m_max_lights];
    int m_id_light_spec[m_max_lights];
    int m_id_ka;
    int m_id_kd;
    int m_id_ks;
    int m_id_alpha;
    int m_id_diffuse_texture;
    int m_id_specular_texture;
    int m_id_env_alpha;
    int m_id_env_map;

    virtual QString vertexShader();
    virtual QString fragmentShader();
    virtual char const *const *attributeNames() const;
    virtual void updatePerTansformNode(GLTransformNode *);
    virtual void updateRenderState(RenderState *);
};

#endif // GLSHADER_H
