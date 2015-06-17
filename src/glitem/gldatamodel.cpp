#include "gldatamodel.h"
#include "glnode.h"
#include "glmaterial.h"
#include <QDebug>


GLDataModel::GLDataModel(QObject *parent)
    : GLModel(parent)
{

}

bool GLDataModel::load()
{
    if (m_vertices.size() < 9 ||
        m_vertices.size() % 3 ||
        m_indices.size() < 3 ||
        m_indices.size() % 3 ||
        m_normals.size() != m_vertices.size() ||
        (!m_uvs.isEmpty() && m_uvs.size() * 3 != m_vertices.size() * 2)) {
        qWarning() << "data model data invalid: " << name();
        return false;
    }

    int nv = m_vertices.size() / 3;
    for (int i = 0; i < m_indices.size(); i += 3) {
        if (m_indices[i] >= nv ||
            m_indices[i + 1] >= nv ||
            m_indices[i + 2] >= nv ||
            m_indices[i] == m_indices[i + 1] ||
            m_indices[i] == m_indices[i + 2] ||
            m_indices[i + 1] == m_indices[i + 2]) {
            qWarning() << "data model index invalid: " << name();
            return false;
        }
    }

    QList<float> *va;
    QList<ushort> *ia;
    Mesh::Type type;
    if (m_uvs.isEmpty()) {
        va = &m_vertex;
        ia = &m_index;
        type = Mesh::NORMAL;
    }
    else {
        va = &m_textured_vertex;
        ia = &m_textured_index;
        type = Mesh::TEXTURED;
        m_textured_vertex_uv.reserve(m_uvs.size());
    }

    va->reserve(m_vertices.size() * 2);
    ia->reserve(m_indices.size());

    for (int i = 0; i < m_vertices.size(); i += 3) {
        va->append(m_vertices[i]);
        va->append(m_vertices[i + 1]);
        va->append(m_vertices[i + 2]);

        va->append(m_normals[i]);
        va->append(m_normals[i + 1]);
        va->append(m_normals[i + 2]);
    }

    for (int i = 0; i < m_indices.size(); i++)
        ia->append(m_indices[i]);

    for (int i = 0; i < m_uvs.size(); i++)
        m_textured_vertex_uv.append(m_uvs[i]);

    Q_ASSERT(m_material);

    m_meshes.resize(1);
    m_meshes[0].type = type;
    m_meshes[0].index_offset = 0;
    m_meshes[0].index_count = ia->size();

    m_rnodes.append(new GLRenderNode(&m_meshes[0], m_material->material()));

    return GLModel::load();
}

