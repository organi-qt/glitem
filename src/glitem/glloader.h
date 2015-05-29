#ifndef GLLOADER_H
#define GLLOADER_H

#include "glnode.h"
#include "gllight.h"
#include "glshader.h"
#include "glmaterial.h"
#include <QString>
#include <QMap>
#include <QList>
#include <QVector>
#include <QDir>
#include <QOpenGLTexture>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>


class GLLoader
{
public:
    GLLoader();
    bool load(const QUrl &file);
    GLTransformNode *convert();

    QVector<float> &vertex() { return m_vertex; }
    QVector<ushort> &index() { return m_index; }
    const QList<Light> &lights() { return m_lights; }

    struct Texture {
        QImage image;
        QOpenGLTexture::WrapMode mode;
    };

    QMap<QString, Texture> &textures() { return m_textures; }
    QVector<GLPhongMaterial> &materials() { return m_materials; }
    int num_vertex() { return m_num_vertex; }

private:
    QDir m_model_dir;
    Assimp::Importer m_importer;
    const aiScene *m_scene;
    QVector<float> m_vertex;
    QVector<ushort> m_index;
    QMap<QString, Texture> m_textures;
    QList<Light> m_lights;
    QVector<GLPhongMaterial> m_materials;
    QVector<GLRenderNode::Mesh> m_meshes;
    int m_num_vertex;

    void assign(QVector3D &qv, const aiVector3D &av);
    void assign(QVector3D &qc, const aiColor3D &ac);

    GLTransformNode *convert(aiNode *node);
    void convertLights(GLTransformNode *root);

    void loadPrimitive();
    void loadMaterial();
    bool loadTexture(aiMaterial *material, aiTextureType type, QString &path);

    // debug prints
    static void printMaterial(aiMaterial *);
    static void printCamera(aiCamera *);
    static void printLight(aiLight *);
    void printNode(aiNode *, QString);
    static QString printVector(aiVector3D &);
    static QString printVector(aiColor3D &);
    static QString printVector(aiColor4D &);
};

#endif // GLLOADER_H
