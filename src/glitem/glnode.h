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
                const QVector3D &nka, const QVector3D &nkd, const QVector3D &nks, float nalpha);
    virtual GLShader::ShaderType type() { return GLShader::PHONG; }
    virtual int stride() { return sizeof(NormalPoint3D); }

    QVector3D &ka() { return m_ka; }
    QVector3D &kd() { return m_kd; }
    QVector3D &ks() { return m_ks; }
    float &alpha() { return m_alpha; }

private:
    QVector3D m_ka;
    QVector3D m_kd;
    QVector3D m_ks;
    float m_alpha;
};

class GLPhongDiffuseTextureNode : public GLPhongNode
{
public:
    GLPhongDiffuseTextureNode(int voff, int ioff, int numVertex, int numIndex,
        const QVector3D &nka, const QVector3D &nkd, const QVector3D &nks, float nalpha,
        const QString &image_path);
    virtual GLShader::ShaderType type() { return GLShader::PHONG_DIFFUSE_TEXTURE; }
    virtual int stride() { return sizeof(TexturedNormalPoint3D); }

    const QString &imagePath() { return m_image_path; }
    QOpenGLTexture *texture() { return m_texture; }
    void setTexture(QOpenGLTexture *tex) { m_texture = tex; }

private:
    QString m_image_path;
    QOpenGLTexture *m_texture;
};

#endif // GLNODE_H
