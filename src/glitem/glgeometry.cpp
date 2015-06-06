#include "glgeometry.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHash>
#include <QVector2D>
#include <QVector3D>
#include <QDebug>


GLGeometry::GLGeometry(QObject *parent)
    : QObject(parent), m_material(0)
{

}

void GLGeometry::setMaterial(GLMaterial *value)
{
    if (m_material != value) {
        m_material = value;
        emit materialChanged();
    }
}

GLLoadGeometry::GLLoadGeometry(QObject *parent)
    : GLGeometry(parent)
{

}

void GLLoadGeometry::setFile(const QUrl &value)
{
    if (m_file != value) {
        m_file = value;
        emit fileChanged();
    }
}

inline uint qHash(const QVector3D &key)
{
    return key.x() + key.y() + key.z();
}

inline uint qHash(const QVector2D &key)
{
    return key.x() + key.y();
}

struct NormalVertex {
    QVector3D v, n;
};

inline bool operator==(const NormalVertex &v0, const NormalVertex &v1)
{
    return (v0.v == v1.v) && (v0.n == v1.n);
}

inline uint qHash(const NormalVertex &key)
{
    return qHash(key.v) ^ qHash(key.n);
}

struct TexturedVertex {
    QVector3D v, n;
    QVector2D t;
};

inline bool operator==(const TexturedVertex &v0, const TexturedVertex &v1)
{
    return (v0.v == v1.v) && (v0.n == v1.n) && (v0.t == v1.t);
}

inline uint qHash(const TexturedVertex &key)
{
    return qHash(key.v) ^ qHash(key.n) ^ qHash(key.t);
}

bool GLLoadGeometry::load()
{
    if (m_file.isEmpty())
        return false;

    QString path;
    if (m_file.scheme() == "file")
        path = m_file.toLocalFile();
    else if (m_file.scheme() == "qrc")
        path = ':' + m_file.path();
    else {
        qWarning() << "invalide model path: " << m_file;
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "can't open model file: " << path;
        return false;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "json file parse error: " << path << "|" << error.errorString();
        return false;
    }

    if (!doc.isObject()) {
        qWarning() << "json file format error: " << path << "|not object";
        return false;
    }

    QJsonObject json = doc.object();
    float scale = 1.0 / json.value("scale").toDouble(1.0);

    //QJsonObject meta = json.value("metadata").toObject();
    QJsonArray va = json.value("vertices").toArray();
    QJsonArray na = json.value("normals").toArray();
    QJsonArray ta = json.value("uvs").toArray();
    QJsonArray fa = json.value("faces").toArray();

    // now only support one uv channel
    Q_ASSERT(ta.count() == 1);
    ta = ta[0].toArray();

    typedef QHash<NormalVertex, ushort> NormalVertexMap;
    typedef QHash<TexturedVertex, ushort> TexturedVertexMap;

    int i = 0;
    int vi = 0, tvi = 0;
    NormalVertexMap vertex_lookup;
    TexturedVertexMap textured_vertex_lookup;
    while (i < fa.count()) {
        int type = fa[i++].toInt();
        bool isQuad              = type & (1 << 0);
        bool hasMaterial         = type & (1 << 1);
        bool hasFaceVertexUv     = type & (1 << 3);
        bool hasFaceNormal       = type & (1 << 4);
        bool hasFaceVertexNormal = type & (1 << 5);
        bool hasFaceColor        = type & (1 << 6);
        bool hasFaceVertexColor  = type & (1 << 7);

        int nov;
        if (isQuad)
            nov = 4;
        else
            nov = 3;

        QVector3D v[nov];
        for (int j = 0; j < nov; j++) {
            int k = fa[i++].toInt();
            v[j].setX(va[k * 3 + 0].toDouble() * scale);
            v[j].setY(va[k * 3 + 1].toDouble() * scale);
            v[j].setZ(va[k * 3 + 2].toDouble() * scale);
        }

        // now we drop the material
        if (hasMaterial) i++;

        QVector2D t[nov];
        if (hasFaceVertexUv) {
            for (int j = 0; j < nov; j++) {
                int k = fa[i++].toInt();
                t[j].setX(ta[k * 2 + 0].toDouble());
                t[j].setY(ta[k * 2 + 1].toDouble());
            }
        }

        QVector3D n[nov];
        if (hasFaceNormal) {
            // vertex normal overwrite face normal
            if (hasFaceVertexNormal)
                i++;
            else {
                int k = fa[i++].toInt();
                QVector3D nn(
                    na[k * 3 + 0].toDouble(),
                    na[k * 3 + 1].toDouble(),
                    na[k * 3 + 2].toDouble()
                );
                for (int j = 0; j < nov; j++)
                    n[j] = nn;
            }
        }

        if (hasFaceVertexNormal) {
            for (int j = 0; j < nov; j++) {
                int k = fa[i++].toInt();
                n[j].setX(na[k * 3 + 0].toDouble());
                n[j].setY(na[k * 3 + 1].toDouble());
                n[j].setZ(na[k * 3 + 2].toDouble());
            }
        }

        if (!hasFaceNormal && !hasFaceVertexNormal) {
            QVector3D nn = QVector3D::normal(v[0], v[1], v[2]);
            for (int j = 0; j < nov; j++)
                n[j] = nn;
        }

        if (hasFaceColor)
            i++;

        if (hasFaceVertexColor)
            i += nov;

        ushort tia[nov];
        QVector<ushort> *piv;
        if (hasFaceVertexUv) {
            for (int j = 0; j < nov; j++) {
                TexturedVertex tv;
                tv.v = v[j]; tv.n = n[j]; tv.t = t[j];
                TexturedVertexMap::iterator it = textured_vertex_lookup.find(tv);
                if (it == textured_vertex_lookup.end()) {
                    tia[j] = tvi;
                    textured_vertex_lookup[tv] = tvi++;
                    appendVector(m_textured_vertex, v[j]);
                    appendVector(m_textured_vertex, n[j]);
                    appendVector(m_textured_vertex_uv, t[j]);
                }
                else
                    tia[j] = it.value();
            }

            piv = &m_textured_index;
        }
        else {
            for (int j = 0; j < nov; j++) {
                NormalVertex nv;
                nv.v = v[j]; nv.n = n[j];
                NormalVertexMap::iterator it = vertex_lookup.find(nv);
                if (it == vertex_lookup.end()) {
                    tia[j] = vi;
                    vertex_lookup[nv] = vi++;
                    appendVector(m_vertex, v[j]);
                    appendVector(m_vertex, n[j]);
                }
                else
                    tia[j] = it.value();
            }

            piv = &m_index;
        }

        if (isQuad) {
            piv->append(tia[0]);
            piv->append(tia[1]);
            piv->append(tia[3]);

            piv->append(tia[1]);
            piv->append(tia[2]);
            piv->append(tia[3]);
        }
        else {
            piv->append(tia[0]);
            piv->append(tia[1]);
            piv->append(tia[2]);
        }
    }
    return true;
}

void GLLoadGeometry::appendVector(QVector<float> &vector, QVector2D &value)
{
    vector.append(value.x());
    vector.append(value.y());
}

void GLLoadGeometry::appendVector(QVector<float> &vector, QVector3D &value)
{
    vector.append(value.x());
    vector.append(value.y());
    vector.append(value.z());
}
