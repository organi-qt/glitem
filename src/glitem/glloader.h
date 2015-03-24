#ifndef GLLOADER_H
#define GLLOADER_H

#include "glnode.h"
#include "glshader.h"
#include <QString>
#include <QMap>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

class GLLoader
{
public:
    GLLoader();
    bool load(const QString &path, const QString &file);
    GLTransformNode *convert();
    int *vertexBufferSize() { return m_vertex_buffer_size; }
    int *indexBufferSize() { return m_index_buffer_size; }

    struct Texture {
        QImage image;
        QOpenGLTexture::WrapMode mode;
    };

    const QMap<QString, Texture> &textures() { return m_textures; }

private:
    QString m_path;
    Assimp::Importer m_importer;
    const aiScene *m_scene;
    int m_vertex_buffer_size[GLShader::NUM_SHADERS];
    int m_index_buffer_size[GLShader::NUM_SHADERS];
    uint m_num_vertex[GLShader::NUM_SHADERS];
    QMap<QString, Texture> m_textures;

    GLTransformNode *convert(aiNode *node);
    GLRenderNode *convert(aiMesh *mesh);
    void loadVertex(GLRenderNode *node, aiVector3D *vertices, aiVector3D *normals);
    void loadVertex(GLRenderNode *node, aiVector3D *vertices, aiVector3D *normals,
                    aiVector3D *texCoords);
    void loadIndex(GLRenderNode *node, aiFace *faces);

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
