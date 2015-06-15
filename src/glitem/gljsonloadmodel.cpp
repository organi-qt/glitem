#include "gljsonloadmodel.h"
#include "glmaterial.h"
#include "glnode.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHash>
#include <QVector2D>
#include <QVector3D>
#include <QDebug>


GLJSONLoadModel::GLJSONLoadModel(QObject *parent)
    : GLModel(parent)
{

}

void GLJSONLoadModel::setFile(const QUrl &value)
{
    if (m_file != value) {
        m_file = value;
        emit fileChanged();
    }
}

bool GLJSONLoadModel::load()
{
    if (m_file.isEmpty())
        return false;

    QString path;
    if (!urlToPath(m_file, path))
        return false;

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

    QJsonObject meta = json.value("metadata").toObject();
    QJsonArray va = json.value("vertices").toArray();
    QJsonArray na = json.value("normals").toArray();
    QJsonArray ta = json.value("uvs").toArray();
    QJsonArray fa = json.value("faces").toArray();

    // now only support one uv channel
    Q_ASSERT(ta.count() == 1);
    ta = ta[0].toArray();

    int nvertices = meta.value("vertices").toInt(1024);
    int nfaces = meta.value("triangles").toInt(1024);
    m_vertex.reserve(nvertices * 3);
    m_index.reserve(nfaces * 3);
    m_textured_vertex.reserve(nvertices * 3);
    m_textured_vertex_uv.reserve(nvertices * 3);
    m_textured_index.reserve(nfaces * 3);

    typedef QHash<u_int64_t, ushort> VertexMap;

    int i = 0;
    int vi = 0, tvi = 0;
    VertexMap vertex_lookup, textured_vertex_lookup;
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

        u_int64_t key[nov];

        QVector3D v[nov];
        for (int j = 0; j < nov; j++) {
            int k = fa[i++].toInt();
            key[j] = k;

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
                key[j] <<= 16;
                key[j] |= k;

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
                for (int j = 0; j < nov; j++) {
                    key[j] <<= 16;
                    key[j] |= k;

                    n[j] = nn;
                }
            }
        }

        if (hasFaceVertexNormal) {
            for (int j = 0; j < nov; j++) {
                int k = fa[i++].toInt();
                key[j] <<= 16;
                key[j] |= k;

                n[j].setX(na[k * 3 + 0].toDouble());
                n[j].setY(na[k * 3 + 1].toDouble());
                n[j].setZ(na[k * 3 + 2].toDouble());
            }
        }

        /*
        if (!hasFaceNormal && !hasFaceVertexNormal) {
            QVector3D nn = QVector3D::normal(v[0], v[1], v[2]);
            for (int j = 0; j < nov; j++)
                n[j] = nn;
        }
        */
        Q_ASSERT(hasFaceNormal || hasFaceVertexNormal);

        if (hasFaceColor)
            i++;

        if (hasFaceVertexColor)
            i += nov;

        int *pvi;
        ushort tia[nov];
        VertexMap *plookup;
        QList<float> *pvv;
        QList<ushort> *piv;
        if (hasFaceVertexUv) {
            plookup = &textured_vertex_lookup;
            pvv = &m_textured_vertex;
            piv = &m_textured_index;
            pvi = &tvi;
        }
        else {
            plookup = &vertex_lookup;
            pvv = &m_vertex;
            piv = &m_index;
            pvi = &vi;
        }

        for (int j = 0; j < nov; j++) {
            VertexMap::iterator it = plookup->find(key[j]);
            if (it == plookup->end()) {
                tia[j] = *pvi;
                plookup->insert(key[j], (*pvi)++);
                appendVector(pvv, v[j]);
                appendVector(pvv, n[j]);
                if (hasFaceVertexUv)
                    appendVector(&m_textured_vertex_uv, t[j]);
            }
            else
                tia[j] = it.value();
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

    Q_ASSERT(m_material);

    int nmeshes = 0;
    if (!m_index.isEmpty()) {
        m_meshes.resize(nmeshes + 1);

        m_meshes[nmeshes].type = Mesh::NORMAL;
        m_meshes[nmeshes].index_offset = 0;
        m_meshes[nmeshes].index_count = m_index.size();

        m_rnodes.append(new GLRenderNode(&m_meshes[nmeshes], m_material->material()));
        nmeshes++;
    }

    if (!m_textured_index.isEmpty()) {
        m_meshes.resize(nmeshes + 1);

        m_meshes[nmeshes].type = Mesh::TEXTURED;
        m_meshes[nmeshes].index_offset = 0;
        m_meshes[nmeshes].index_count = m_textured_index.size();

        m_rnodes.append(new GLRenderNode(&m_meshes[nmeshes], m_material->material()));
    }

    if (name().isEmpty())
        setName(m_file.fileName());

    return GLModel::load();
}

void GLJSONLoadModel::appendVector(QList<float> *vector, QVector2D &value)
{
    vector->append(value.x());
    vector->append(value.y());
}

void GLJSONLoadModel::appendVector(QList<float> *vector, QVector3D &value)
{
    vector->append(value.x());
    vector->append(value.y());
    vector->append(value.z());
}

