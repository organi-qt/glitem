#ifndef GLMODEL_H
#define GLMODEL_H

#include <QObject>
#include <QList>
#include <QVector>
#include "mesh.h"

class GLMaterial;
class GLTransformNode;
class GLRenderNode;
class Light;
class Material;

class GLModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(GLMaterial *material READ material WRITE setMaterial NOTIFY materialChanged)
public:
    GLModel(QObject *parent = 0);

    GLMaterial *material() { return m_material; }
    void setMaterial(GLMaterial *value);

    QList<float> &vertex() { return m_vertex; }
    QList<ushort> &index() { return m_index; }
    QList<float> &texturedVertex() { return m_textured_vertex; }
    QList<float> &texturedVertexUV() { return m_textured_vertex_uv; }
    QList<ushort> &texturedIndex() { return m_textured_index; }

    QVector<Mesh> &meshes() { return m_meshes; }
    QList<Material *> &materials() { return m_materials; }
    QList<Light *> &lights() { return m_lights; }
    GLTransformNode *root() { return m_root; }

    virtual bool load() { return true; }
    virtual void release();

signals:
    void materialChanged();

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
};

#endif // GLMODEL_H
