#include "glmodel.h"
#include "glnode.h"
#include "glmaterial.h"
#include <QUrl>
#include <QDebug>


GLModel::GLModel(QObject *parent)
    : QObject(parent), m_material(0), m_root(0), m_node(0),
      m_visible(true), m_visible_dirty(false), m_material_dirty(false)
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
        m_material_dirty = true;
        emit materialChanged();
        emit modelChanged();
    }
}

void GLModel::setNode(int value)
{
    if (m_node != value) {
        m_node = value;
        emit nodeChanged();
    }
}

void GLModel::setVisible(bool value)
{
    if (m_visible != value) {
        m_visible = value;
        m_visible_dirty = true;
        emit visibleChanged();
        emit modelChanged();
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
        if (m_root) {
            m_tnodes.append(m_root);
            m_root->setVisible(m_visible);
        }

        foreach (GLRenderNode *rnode, m_rnodes) {
            rnode->setVisible(m_visible);
        }

        return true;
    }

    for (int i = 0; i < m_node; i++) {
        GLTransformNode *root = new GLTransformNode(m_name + QString::number(i));
        if (m_root)
            root->addChild(m_root);
        root->addChild(m_rnodes);
        root->setVisible(m_visible);
        m_tnodes.append(root);
    }
    m_rnodes.clear();

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

void GLModel::sync()
{
    if (m_visible_dirty) {
        foreach (GLTransformNode *tnode, m_tnodes) {
            tnode->setVisible(m_visible);
        }

        foreach (GLRenderNode *rnode, m_rnodes) {
            rnode->setVisible(m_visible);
        }

        m_visible_dirty = false;
    }

    if (m_material_dirty) {
        foreach (GLTransformNode *tnode, m_tnodes) {
            updateMaterial(tnode);
        }

        foreach (GLRenderNode *rnode, m_rnodes) {
            rnode->setMaterial(m_material->material());
        }

        m_material_dirty = false;
    }
}

void GLModel::updateMaterial(GLTransformNode *n)
{
    foreach (GLRenderNode *rnode, n->renderChildren()) {
        rnode->setMaterial(m_material->material());
    }

    foreach (GLTransformNode *tnode, n->transformChildren()) {
        updateMaterial(tnode);
    }
}
