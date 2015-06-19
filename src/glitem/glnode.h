#ifndef GLNODE_H
#define GLNODE_H

#include "glanimatenode.h"
#include <QList>
#include <QMatrix4x4>

class Mesh;
class Material;

class GLNode
{
public:
    GLNode() : m_ref_count(0), m_visible(true) {}

    int incRef() { return m_ref_count++; }
    int decRef() { return --m_ref_count; }

    bool visible() { return m_visible; }
    void setVisible(bool value) { m_visible = value; }

private:
    int m_ref_count;
    bool m_visible;
};

class GLRenderNode : public GLNode
{
public:
    GLRenderNode(Mesh *nmesh, Material *nmaterial)
        : GLNode(), m_mesh(nmesh), m_material(nmaterial)
    {}

    Mesh *mesh() { return m_mesh; }
    Material *material() { return m_material; }

private:
    Mesh *m_mesh;
    Material *m_material;
};

class GLTransformNode : public GLNode
{
public:
    GLTransformNode(const QString &name, const QMatrix4x4 &transform = QMatrix4x4());
    ~GLTransformNode();

    void addChild(GLRenderNode *node) {
        m_render_children.append(node);
        node->incRef();
    }

    void addChild(GLTransformNode *node) {
        m_transform_children.append(node);
        node->incRef();
    }

    void addChild(QList<GLRenderNode *> &nodes) {
        m_render_children.append(nodes);
        foreach (GLRenderNode *node, nodes) {
            node->incRef();
        }
    }

    void addChild(QList<GLTransformNode *> &nodes) {
        m_transform_children.append(nodes);
        foreach (GLTransformNode *node, nodes) {
            node->incRef();
        }
    }

    void removeChild(GLRenderNode *node) {
        m_render_children.removeAll(node);
        if (!node->decRef())
            delete node;
    }

    void removeChild(GLTransformNode *node) {
        m_transform_children.removeAll(node);
        if (!node->decRef())
            delete node;
    }

    QMatrix4x4 &transformMatrix() { return m_transform; }
    QMatrix4x4 &modelviewMatrix() { return m_modelview_matrix; }

    QList<GLRenderNode *> &renderChildren() { return m_render_children; }
    QList<GLTransformNode *> &transformChildren() { return m_transform_children; }

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
