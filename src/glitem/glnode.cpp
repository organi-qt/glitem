#include "glnode.h"

GLRenderNode::GLRenderNode(int voff, int ioff, int numVertex, int numIndex)
    : m_vertex_offset(voff), m_index_offset(ioff),
      m_vertex_data(0), m_index_data(0),
      m_vertex_count(numVertex), m_index_count(numIndex)
{
}

GLRenderNode::~GLRenderNode()
{
    if (m_vertex_data)
        delete m_vertex_data;
    if (m_index_data)
        delete m_index_data;
}

void GLRenderNode::allocateData()
{
    if (!m_vertex_data)
        m_vertex_data = new uchar[m_vertex_count * stride()];
    if (!m_index_data)
        m_index_data = new uchar[m_index_count * sizeof(ushort)];
}

void GLRenderNode::freeVertexData()
{
    if (m_vertex_data) {
        delete m_vertex_data;
        m_vertex_data = 0;
    }
}

void GLRenderNode::freeIndexData()
{
    if (m_index_data) {
        delete m_index_data;
        m_index_data = 0;
    }
}

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

GLPhongNode::GLPhongNode(int voff, int ioff,
                         int numVertex, int numIndex,
                         const QVector3D &nka, const QVector3D &nkd,
                         const QVector3D &nks, float nalpha)
    : GLRenderNode(voff, ioff, numVertex, numIndex),
      m_ka(nka), m_kd(nkd), m_ks(nks), m_alpha(nalpha)
{

}

GLPhongDiffuseTextureNode::GLPhongDiffuseTextureNode(
        int voff, int ioff, int numVertex, int numIndex,
        const QVector3D &nka, const QVector3D &nkd, const QVector3D &nks, float nalpha,
        const QString &image_path)
    : GLPhongNode(voff, ioff, numVertex, numIndex, nka, nkd, nks, nalpha),
      m_image_path(image_path), m_texture(0)
{

}

