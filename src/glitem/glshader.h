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
    virtual ~GLShader() {}
    QOpenGLShaderProgram *program() { return &m_program; }

    void initialize();
    void render(GLTransformNode *root, RenderState *state);

    bool *attributeActivities() { return m_attribute_activities; }

protected:
    GLRenderNode *m_last_node;
    bool m_attribute_activities[3];

    virtual void resolveUniforms() = 0;
    virtual void bind();
    virtual void release();

private:
    virtual QString vertexShader() = 0;
    virtual QString fragmentShader() = 0;
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
    GLPhongShader(const QList<Light *> *lights, bool has_diffuse_texture,
                  bool has_specular_texture, bool has_env_map);
    virtual ~GLPhongShader();

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

    int m_id_opacity;
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
