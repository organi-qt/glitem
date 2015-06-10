#ifndef GLNODE_H
#define GLNODE_H

#include "glanimatenode.h"
#include <QList>
#include <QMatrix4x4>

class Mesh;
class Material;

class GLRenderNode {
public:
    GLRenderNode(Mesh *nmesh, Material *nmaterial)
        : m_mesh(nmesh), m_material(nmaterial)
    {}

    Mesh *mesh() { return m_mesh; }
    Material *material() { return m_material; }

private:
    Mesh *m_mesh;
    Material *m_material;
};

class GLTransformNode
{
public:
    GLTransformNode(const QString &name, const QMatrix4x4 &transform = QMatrix4x4());
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

#endif // GLNODE_H
