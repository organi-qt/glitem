#include "glmodel.h"


GLModel::GLModel(QObject *parent)
    : QObject(parent), m_material(0), m_root(0)
{

}

void GLModel::setMaterial(GLMaterial *value)
{
    if (m_material != value) {
        m_material = value;
        emit materialChanged();
    }
}

void GLModel::release()
{
    m_vertex.clear();
    m_index.clear();

    m_textured_vertex.clear();
    m_textured_vertex_uv.clear();
    m_textured_index.clear();

    m_materials.clear();
    m_lights.clear();
}

