#include "glnode.h"


GLTransformNode::GLTransformNode(const QString &name, const QMatrix4x4 &transform)
    : GLNode(), m_transform(transform), m_modelview_matrix(),
      m_animate_node(0), m_name(name)
{

}

GLTransformNode::~GLTransformNode()
{
    foreach (GLRenderNode *node, m_render_children) {
        removeChild(node);
    }

    foreach (GLTransformNode *node, m_transform_children) {
        removeChild(node);
    }
}
