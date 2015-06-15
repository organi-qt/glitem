#include "glmodel.h"
#include "glnode.h"
#include <QUrl>
#include <QDebug>


GLModel::GLModel(QObject *parent)
    : QObject(parent), m_material(0), m_root(0), m_node(0)
{

}

void GLModel::setName(const QString &value)
{
    if (m_name != value) {
        m_name = value;
        emit nameChanged();
    }
}

void GLModel::setMaterial(GLMaterial *value)
{
    if (m_material != value) {
        m_material = value;
        emit materialChanged();
    }
}

void GLModel::setNode(int value)
{
    if (m_node != value) {
        m_node = value;
        emit nodeChanged();
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

bool GLModel::load()
{
    if (!m_node) {
        if (m_root)
            m_tnodes.append(m_root);
        return true;
    }

    for (int i = 0; i < m_node; i++) {
        GLTransformNode *root = new GLTransformNode(m_name + QString::number(i));
        if (m_root)
            root->addChild(m_root);
        root->addChild(rnodes());
        m_tnodes.append(root);
    }
    return true;
}

bool GLModel::urlToPath(const QUrl &url, QString &path)
{
    if (url.scheme() == "file")
        path = url.toLocalFile();
    else if (url.scheme() == "qrc")
        path = ':' + url.path();
    else {
        qWarning() << "invalide path: " << url;
        return false;
    }
    return true;
}
