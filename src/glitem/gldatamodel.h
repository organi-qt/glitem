#ifndef GLDATAMODEL_H
#define GLDATAMODEL_H

#include "glmodel.h"

class GLDataModel : public GLModel
{
    Q_OBJECT
    Q_PROPERTY(QList<qreal> vertices READ vertices WRITE setVertices)
    Q_PROPERTY(QList<qreal> normals READ normals WRITE setNormals)
    Q_PROPERTY(QList<qreal> uvs READ uvs WRITE setUvs)
    Q_PROPERTY(QList<int> indices READ indices WRITE setIndices)
public:
    GLDataModel(QObject *parent = 0);

    QList<qreal> vertices() { return m_vertices; }
    void setVertices(const QList<qreal> &value) { m_vertices = value; }

    QList<qreal> normals() { return m_normals; }
    void setNormals(const QList<qreal> &value) { m_normals = value; }

    QList<qreal> uvs() { return m_uvs; }
    void setUvs(const QList<qreal> &value) { m_uvs = value; }

    QList<int> indices() { return m_indices; }
    void setIndices(const QList<int> &value) { m_indices = value; }

    virtual bool load();

private:
    QList<qreal> m_vertices;
    QList<qreal> m_normals;
    QList<qreal> m_uvs;
    QList<int> m_indices;
};

#endif // GLDATAMODEL_H
