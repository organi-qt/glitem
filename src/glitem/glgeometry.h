#ifndef GLGEOMETRY_H
#define GLGEOMETRY_H

#include <QObject>
#include <QUrl>
#include <QVector>

class GLMaterial;

class GLGeometry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(GLMaterial *material READ material WRITE setMaterial NOTIFY materialChanged)
public:
    GLGeometry(QObject *parent = 0);

    GLMaterial *material() { return m_material; }
    void setMaterial(GLMaterial *value);

    QVector<float> *vertex() { return &m_vertex; }
    QVector<ushort> *index() { return &m_index; }
    QVector<float> *texturedVertex() { return &m_textured_vertex; }
    QVector<float> *texturedVertexUV() { return &m_textured_vertex_uv; }
    QVector<ushort> *texturedIndex() { return &m_textured_index; }

    virtual bool load() { return true; }

signals:
    void materialChanged();

protected:
    GLMaterial *m_material;

    QVector<float> m_vertex;
    QVector<ushort> m_index;
    QVector<float> m_textured_vertex;
    QVector<float> m_textured_vertex_uv;
    QVector<ushort> m_textured_index;

};

class GLLoadGeometry : public GLGeometry
{
    Q_OBJECT
    Q_PROPERTY(QUrl file READ file WRITE setFile NOTIFY fileChanged)
public:
    GLLoadGeometry(QObject *parent = 0);

    QUrl file() { return m_file; }
    void setFile(const QUrl &value);

    virtual bool load();

signals:
    void fileChanged();

private:
    QUrl m_file;

    void appendVector(QVector<float> &vector, QVector2D &value);
    void appendVector(QVector<float> &vector, QVector3D &value);
};

#endif // GLGEOMETRY_H
