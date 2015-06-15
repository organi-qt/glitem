#ifndef GLMODEL_H
#define GLMODEL_H

#include <QObject>
#include <QList>
#include <QVector>
#include "mesh.h"

class GLMaterial;
class GLTransformNode;
class GLRenderNode;
class GLRenderNode;
class Light;
class Material;

class GLModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(GLMaterial *material READ material WRITE setMaterial NOTIFY materialChanged)
    Q_PROPERTY(int node READ node WRITE setNode NOTIFY nodeChanged)
public:
    GLModel(QObject *parent = 0);

    QString name() { return m_name; }
    void setName(const QString &value);

    GLMaterial *material() { return m_material; }
    void setMaterial(GLMaterial *value);

    int node() { return m_node; }
    void setNode(int value);

    QList<float> &vertex() { return m_vertex; }
    QList<ushort> &index() { return m_index; }
    QList<float> &texturedVertex() { return m_textured_vertex; }
    QList<float> &texturedVertexUV() { return m_textured_vertex_uv; }
    QList<ushort> &texturedIndex() { return m_textured_index; }

    QVector<Mesh> &meshes() { return m_meshes; }
    QList<Material *> &materials() { return m_materials; }
    QList<Light *> &lights() { return m_lights; }
    QList<GLTransformNode *> &tnodes() { return m_tnodes; }
    QList<GLRenderNode *> &rnodes() { return m_rnodes; }

    virtual bool load();
    virtual void release();

signals:
    void nameChanged();
    void materialChanged();
    void nodeChanged();

protected:
    GLMaterial *m_material;

    QList<float> m_vertex;
    QList<ushort> m_index;
    QList<float> m_textured_vertex;
    QList<float> m_textured_vertex_uv;
    QList<ushort> m_textured_index;

    QVector<Mesh> m_meshes;
    QList<Material *> m_materials;
    QList<Light *> m_lights;
    GLTransformNode *m_root;

    QList<GLTransformNode *> m_tnodes;
    QList<GLRenderNode *> m_rnodes;

    bool urlToPath(const QUrl &url, QString &path);

private:
    QString m_name;
    int m_node;
};

#endif // GLMODEL_H
