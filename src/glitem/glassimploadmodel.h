#ifndef GLASSIMPLOADMODEL_H
#define GLASSIMPLOADMODEL_H

#include "glmodel.h"
#include <QUrl>
#include <QDir>
#include <assimp/material.h>

class aiMaterial;
class aiCamera;
class aiLight;
class aiNode;
class aiScene;
class PhongMaterial;

class GLAssimpLoadModel : public GLModel
{
    Q_OBJECT
    Q_PROPERTY(QUrl file READ file WRITE setFile NOTIFY fileChanged)
    Q_PROPERTY(bool ignoreLight READ ignoreLight WRITE setIgnoreLight NOTIFY ignoreLightChanged)
public:
    GLAssimpLoadModel(QObject *parent = 0);
    ~GLAssimpLoadModel();

    QUrl file() { return m_file; }
    void setFile(const QUrl &value);

    bool ignoreLight() { return m_ignore_light; }
    void setIgnoreLight(bool value);

    virtual bool load();

signals:
    void fileChanged();
    void ignoreLightChanged();

private:
    QUrl m_file;
    QDir m_model_dir;
    bool m_ignore_light;

    const aiScene *m_scene;

    void assign(QVector3D &qv, const aiVector3D &av);
    void assign(QVector3D &qc, const aiColor3D &ac);

    void loadPrimitive();
    void loadMaterial();
    bool loadTexture(aiMaterial *material, aiTextureType type, PhongMaterial *pmaterial);
    GLTransformNode *loadNode(aiNode *node);
    void loadLight();

    // debug prints
    static void printMaterial(aiMaterial *);
    static void printCamera(aiCamera *);
    static void printLight(aiLight *);
    void printNode(aiNode *, QString);
    static QString printVector(aiVector3D &);
    static QString printVector(aiColor3D &);
    static QString printVector(aiColor4D &);
};

#endif // GLASSIMPLOADMODEL_H
