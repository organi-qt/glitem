#include "glnode.h"


GLTransformNode::GLTransformNode(const QString &name, const QMatrix4x4 &transform)
    : m_transform(transform), m_modelview_matrix(), m_animate_node(0), m_name(name)
{

}

GLTransformNode::~GLTransformNode()
{
    for (int i = 0; i < m_render_children.size(); i++)
        delete m_render_children[i];

    for (int i = 0; i < m_transform_children.size(); i++)
        delete m_transform_children[i];
}
