#ifndef GLNODE_H
#define GLNODE_H

#include "glshader.h"
#include "glanimatenode.h"
#include <QList>
#include <QMatrix4x4>
#include <QOpenGLTexture>
#include <assimp/types.h>

class GLRenderNode {
public:
    struct NormalPoint3D {
        float x, y, z;
        float nx, ny, nz;
        void set(const aiVector3D *v, const aiVector3D *n) {
            x = v->x; y = v->y; z = v->z;
            nx = n->x; ny = n->y; nz = n->z;
        }
    };

    struct TexturedNormalPoint3D {
        float x, y, z;
        float nx, ny, nz;
        float tx, ty;
        void set(const aiVector3D *v, const aiVector3D *n, const aiVector3D *t) {
            x = v->x; y = v->y; z = v->z;
            nx = n->x; ny = n->y; nz = n->z;
            tx = t->x; ty = t->y;
        }
    };

    GLRenderNode(int voff, int ioff, int numVertex, int numIndex);
    virtual ~GLRenderNode();

    virtual GLShader::ShaderType type() = 0;
    int vertexOffset() { return m_vertex_offset; }
    int indexOffset() { return m_index_offset; }

    void *vertexData() { return m_vertex_data; }
    void *indexData() { return m_index_data; }
    int vertexCount() { return m_vertex_count; }
    int indexCount() { return m_index_count; }

    void allocateData();
    void freeVertexData();
    void freeIndexData();

    virtual int stride() { return 0; }

private:
    int m_vertex_offset;
    int m_index_offset;
    uchar *m_vertex_data;
    uchar *m_index_data;
    int m_vertex_count;
    int m_index_count;
};

class GLTransformNode
{
public:
    GLTransformNode(const QString &name, const QMatrix4x4 &transform);
    ~GLTransformNode();

    void addChild(GLRenderNode *node) { m_render_children.append(node); }
    void addChild(GLTransformNode *node) { m_transform_children.append(node); }
    QMatrix4x4 &transformMatrix() { return m_transform; }
    QMatrix4x4 &modelviewMatrix() { return m_modelview_matrix; }
    int renderChildCount() { return m_render_children.size(); }
    int transformChildCount() { return m_transform_children.size(); }
    GLRenderNode *renderChildAtIndex(int i) { return m_render_children[i]; }
    GLTransformNode *transformChildAtIndex(int i) { return m_transform_children[i]; }

    GLAnimateNode *animateNode() { return m_animate_node; }
    void setAnimateNode(GLAnimateNode *node) { m_animate_node = node; }

    const QString &name() { return m_name; }
    void setName(const QString &value) { m_name = value; }

private:
    QList<GLRenderNode *> m_render_children;
    QList<GLTransformNode *> m_transform_children;
    QMatrix4x4 m_transform;
    QMatrix4x4 m_modelview_matrix;
    GLAnimateNode *m_animate_node;
    QString m_name;
};

class GLPhongNode : public GLRenderNode {
public:
    GLPhongNode(int voff, int ioff, int numVertex, int numIndex,
                GLShader::ShaderType type);

    void setMaterial(const QVector3D &nka, const QVector3D &nkd, const QVector3D &nks,
                     float nalpha);
    void setDiffuseTexturePath(const QString &path) { m_diffuse_texture_path = path; }
    void setSpecularTexturePath(const QString &path) { m_specular_texture_path = path; }

    const QString &diffuseTexturePath() { return m_diffuse_texture_path; }
    const QString &specularTexturePath() { return m_specular_texture_path; }

    void setDiffuseTexture(QOpenGLTexture *texture) { m_diffuse_texture = texture; }
    void setSpecularTexture(QOpenGLTexture *texture) { m_specular_texture = texture; }

    QOpenGLTexture *diffuseTexture() { return m_diffuse_texture; }
    QOpenGLTexture *specularTexture() { return m_specular_texture; }

    virtual GLShader::ShaderType type() { return m_type; }
    virtual int stride();

    QVector3D &ka() { return m_ka; }
    QVector3D &kd() { return m_kd; }
    QVector3D &ks() { return m_ks; }
    float &alpha() { return m_alpha; }

private:
    GLShader::ShaderType m_type;

    QVector3D m_ka;
    QVector3D m_kd;
    QVector3D m_ks;
    float m_alpha;

    QString m_diffuse_texture_path;
    QOpenGLTexture *m_diffuse_texture;
    QString m_specular_texture_path;
    QOpenGLTexture *m_specular_texture;
};

#endif // GLNODE_H
