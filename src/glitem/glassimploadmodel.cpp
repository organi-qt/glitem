#include "glassimploadmodel.h"
#include "glnode.h"
#include "material.h"
#include "light.h"
#include "ailoaderiosystem.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <QOpenGLTexture>
#include <QDebug>

GLAssimpLoadModel::GLAssimpLoadModel(QObject *parent)
    : GLModel(parent), m_ignore_light(false), m_scene(0)
{

}

GLAssimpLoadModel::~GLAssimpLoadModel()
{

}

void GLAssimpLoadModel::setFile(const QUrl &value)
{
    if (m_file != value) {
        m_file = value;
        emit fileChanged();
    }
}

void GLAssimpLoadModel::setIgnoreLight(bool value)
{
    if (m_ignore_light != value) {
        m_ignore_light = value;
        emit ignoreLightChanged();
    }
}

bool GLAssimpLoadModel::load()
{
    QString path;
    if (!urlToPath(m_file, path))
        return false;

    Assimp::Importer importer;
    importer.SetIOHandler(new AiLoaderIOSystem());

    m_scene = importer.ReadFile(path.toStdString(),
                       aiProcess_CalcTangentSpace       |
                       aiProcess_Triangulate            |
                       aiProcess_JoinIdenticalVertices  |
                       aiProcess_SortByPType);
    if (!m_scene) {
        qWarning() << "load file " << m_file << " fail: " << importer.GetErrorString();
        return false;
    }

    if (m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        qWarning() << "file " << m_file << " with incomplete scene";
        m_scene = NULL;
        return false;
    }

/*
    if (m_scene->HasCameras())
        for (uint i = 0; i < m_scene->mNumCameras; i++)
            printCamera(m_scene->mCameras[i]);
//*/
/*
    if (m_scene->HasLights())
        for (uint i = 0; i < m_scene->mNumLights; i++)
            printLight(m_scene->mLights[i]);
//*/
/*
    if (m_scene->HasMaterials())
        for (uint i = 0; i < m_scene->mNumMaterials; i++)
            printMaterial(m_scene->mMaterials[i]);
//*/
    //printNode(m_scene->mRootNode, "  ");

    m_model_dir.setPath(path);
    m_model_dir.cdUp();

    loadPrimitive();

    if (!m_material)
        loadMaterial();

    m_root = loadNode(m_scene->mRootNode);

    if (!m_ignore_light)
        loadLight();

    m_scene = NULL;

    if (name().isEmpty())
        setName(m_file.fileName());

    return GLModel::load();
}

void GLAssimpLoadModel::loadPrimitive()
{
    uint np = 0, ntp = 0, nf = 0, ntf = 0;
    for (uint i = 0; i < m_scene->mNumMeshes; i++) {
        aiMesh *mesh = m_scene->mMeshes[i];
        if (mesh->HasNormals()) {
            if (mesh->HasTextureCoords(0)) {
                ntp += mesh->mNumVertices;
                ntf += mesh->mNumFaces;
            }
            else {
                np += mesh->mNumVertices;
                nf += mesh->mNumFaces;
            }
        }
    }

    m_vertex.reserve(np * 6);
    m_index.reserve(nf * 3);
    m_textured_vertex.reserve(ntp * 6);
    m_textured_vertex_uv.reserve(ntp * 2);
    m_textured_index.reserve(ntf * 3);

    m_meshes.resize(m_scene->mNumMeshes);

    for (uint i = 0; i < m_scene->mNumMeshes; i++) {
        aiMesh *mesh = m_scene->mMeshes[i];
        Q_ASSERT(mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE);
        Q_ASSERT(mesh->HasNormals());

        QList<float> *va;
        QList<ushort> *ia;
        if (mesh->HasTextureCoords(0)) {
            m_meshes[i].type = Mesh::TEXTURED;
            va = &m_textured_vertex;
            ia = &m_textured_index;

            for (uint j = 0; j < mesh->mNumVertices; j++) {
                m_textured_vertex_uv.append(mesh->mTextureCoords[0][j].x);
                m_textured_vertex_uv.append(mesh->mTextureCoords[0][j].y);
            }
        }
        else {
            m_meshes[i].type = Mesh::NORMAL;
            va = &m_vertex;
            ia = &m_index;
        }

        int ibase = va->size() / 6;
        m_meshes[i].index_offset = ia->size() * sizeof(ushort);
        m_meshes[i].index_count = mesh->mNumFaces * 3;

        for (uint j = 0; j < mesh->mNumVertices; j++) {
            va->append(mesh->mVertices[j].x);
            va->append(mesh->mVertices[j].y);
            va->append(mesh->mVertices[j].z);

            va->append(mesh->mNormals[j].x);
            va->append(mesh->mNormals[j].y);
            va->append(mesh->mNormals[j].z);
        }

        aiFace *faces = mesh->mFaces;
        for (uint j = 0; j < mesh->mNumFaces; j++) {
            Q_ASSERT(faces[j].mNumIndices == 3);
            ia->append(faces[j].mIndices[0] + ibase);
            ia->append(faces[j].mIndices[1] + ibase);
            ia->append(faces[j].mIndices[2] + ibase);
        }
    }
}

void GLAssimpLoadModel::loadMaterial()
{
    if (m_scene->HasMaterials())
        m_materials.reserve(m_scene->mNumMaterials);

    for (uint i = 0; i < m_scene->mNumMaterials; i++) {
        aiMaterial *material = m_scene->mMaterials[i];

        int shadingModel;
        material->Get(AI_MATKEY_SHADING_MODEL, shadingModel);
        if (shadingModel != aiShadingMode_Phong) {
            qDebug() << "material not Phong shading detacted";
            //return 0;
        }

        aiColor3D amb(1.0f, 1.0f, 1.0f);
        aiColor3D dif(1.0f, 1.0f, 1.0f);
        aiColor3D spec(1.0f, 1.0f, 1.0f);
        float shine = 30.0f;

        if (material->Get(AI_MATKEY_COLOR_AMBIENT, amb) != aiReturn_SUCCESS)
            qWarning("material get amb fail");
        if (material->Get(AI_MATKEY_COLOR_DIFFUSE, dif) != aiReturn_SUCCESS)
            qWarning("material get dif fail");
        if (material->Get(AI_MATKEY_COLOR_SPECULAR, spec) != aiReturn_SUCCESS)
            qWarning("material get spec fail");
        if (material->Get(AI_MATKEY_SHININESS, shine) != aiReturn_SUCCESS)
            qWarning("material get shine fail");

        PhongMaterial *pm = new PhongMaterial;
        loadTexture(material, aiTextureType_DIFFUSE, pm);
        loadTexture(material, aiTextureType_SPECULAR, pm);

        aiString name;
        if (material->Get(AI_MATKEY_NAME, name) == aiReturn_SUCCESS)
            pm->setName(name.C_Str());
        //qDebug() << "mat name" << name.C_Str();

        pm->setMaterial(
                QVector3D(amb.r, amb.g, amb.b),
                QVector3D(dif.r, dif.g, dif.b),
                QVector3D(spec.r, spec.g, spec.b),
                shine);

        m_materials.append(pm);
    }
}

GLTransformNode *GLAssimpLoadModel::loadNode(aiNode *node)
{
    // meaningless node for render
    if (node->mNumChildren == 0 && node->mNumMeshes == 0)
        return NULL;

    GLTransformNode *root =
            new GLTransformNode(node->mName.C_Str(), QMatrix4x4(node->mTransformation[0]));

    for (uint i = 0; i < node->mNumMeshes; i++) {
        GLRenderNode *rnode = new GLRenderNode(
                    &m_meshes[node->mMeshes[i]],
                    m_material ? 0 : m_materials[m_scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]
                );
        root->addChild(rnode);
    }

    for (uint i = 0; i < node->mNumChildren; i++) {
        GLTransformNode *tnode = loadNode(node->mChildren[i]);
        if (tnode)
            root->addChild(tnode);
    }

    return root;
}

void GLAssimpLoadModel::assign(QVector3D &qc, const aiColor3D &ac)
{
    qc.setX(ac.r);
    qc.setY(ac.g);
    qc.setZ(ac.b);
}

void GLAssimpLoadModel::assign(QVector3D &qv, const aiVector3D &av)
{
    qv.setX(av.x);
    qv.setY(av.y);
    qv.setZ(av.z);
}

void GLAssimpLoadModel::loadLight()
{
    if (m_scene->HasLights())
        m_lights.reserve(m_scene->mNumLights);

    for (uint i = 0; i < m_scene->mNumLights; i++) {
        aiLight *srcLight = m_scene->mLights[i];
        aiMatrix4x4 trans;
        aiNode *node = m_scene->mRootNode->FindNode(srcLight->mName);
        if (node) {
            aiNode *pn = node;
            do {
                trans = pn->mTransformation * trans;
                pn = pn->mParent;
            } while (pn);
        }
        else
            qWarning() << "glloader: no node found for light: "
                       << srcLight->mName.C_Str();

        Light *light = new Light;
        switch (srcLight->mType) {
        case aiLightSource_DIRECTIONAL:
            light->type = Light::DIRECTIONAL;
            assign(light->pos, srcLight->mDirection);
            break;
        case aiLightSource_POINT:
            light->type = Light::POINT;
            assign(light->pos, srcLight->mPosition);
            break;
        case aiLightSource_SPOT:
            light->type = Light::SPOT;
            break;
        default:
            continue;
        }

        assign(light->dif, srcLight->mColorDiffuse);
        assign(light->spec, srcLight->mColorSpecular);
        light->name = srcLight->mName.C_Str();
        light->node = new GLTransformNode(light->name, QMatrix4x4(trans[0]));
        m_lights.append(light);
    }
}

bool GLAssimpLoadModel::loadTexture(aiMaterial *material, aiTextureType type,
                                    PhongMaterial *pmaterial)
{
    aiString tpath;
    aiTextureMapMode mode;
    if (material->GetTextureCount(type) > 0 &&
        material->GetTexture(type, 0, &tpath, NULL, NULL, NULL, NULL, &mode)
            == aiReturn_SUCCESS) {
        QOpenGLTexture::WrapMode wmode;
        switch (mode) {
        case aiTextureMapMode_Clamp:
            wmode = QOpenGLTexture::ClampToEdge;
            break;
        case aiTextureMapMode_Wrap:
            wmode = QOpenGLTexture::Repeat;
            break;
        case aiTextureMapMode_Mirror:
            wmode = QOpenGLTexture::MirroredRepeat;
            break;
        case aiTextureMapMode_Decal:
            wmode = QOpenGLTexture::ClampToBorder;
            break;
        default:
            wmode = QOpenGLTexture::Repeat;
            break;
        }

        QString path = m_model_dir.filePath(tpath.C_Str());
        switch (type) {
        case aiTextureType_DIFFUSE:
            return pmaterial->loadDiffuseTexture(path, wmode);
        case aiTextureType_SPECULAR:
            return pmaterial->loadSpecularTexture(path, wmode);
        default:
            return false;
        }
    }
    return false;
}

void GLAssimpLoadModel::printCamera(aiCamera *camera)
{
    qDebug() << "camera" << " name:" << camera->mName.C_Str()
             << " pos: " << printVector(camera->mPosition)
             << " lookat: " << printVector(camera->mLookAt)
             << " up: " << printVector(camera->mUp)
             << " near: " << camera->mClipPlaneNear
             << " far: " << camera->mClipPlaneFar;
}

void GLAssimpLoadModel::printLight(aiLight *light)
{
    QString type;
    switch (light->mType) {
    case aiLightSource_DIRECTIONAL:
        type = "DIRECTIONAL";
        break;
    case aiLightSource_POINT:
        type = "POINT";
        break;
    case aiLightSource_SPOT:
        type = "SPOT";
        break;
    default:
        type = "UNDEFINED";
        break;
    }
    qDebug() << "  light" << " name: " << light->mName.C_Str()
             << " type: " << type
             << " pos: " << printVector(light->mPosition)
             << " dir: " << printVector(light->mDirection)
             << " amb: " << printVector(light->mColorAmbient)
             << " dif: " << printVector(light->mColorDiffuse)
             << " spec: " << printVector(light->mColorSpecular);
}

QString GLAssimpLoadModel::printVector(aiVector3D &v)
{
    return QString("x=%1 y=%2 z=%3").arg(v.x).arg(v.y).arg(v.z);
}

QString GLAssimpLoadModel::printVector(aiColor3D &c)
{
    return QString("r=%1 g=%2 b=%3").arg(c.r).arg(c.g).arg(c.b);
}

QString GLAssimpLoadModel::printVector(aiColor4D &c)
{
    return QString("r=%1 g=%2 b=%3 a=%4").arg(c.r).arg(c.g).arg(c.b).arg(c.a);
}

void GLAssimpLoadModel::printMaterial(aiMaterial *material)
{
    int shadingModel;
    material->Get(AI_MATKEY_SHADING_MODEL, shadingModel);
    switch (shadingModel) {
    case aiShadingMode_Phong:
        qDebug() << "Shader type:Phone";
        break;
    case aiShadingMode_Gouraud:
        qDebug() << "Shader type:Gouraud";
        break;
    default:
        qDebug() << "Shader type:other " << shadingModel;
        break;
    }

    aiString path;
    aiReturn ret;
    int nat = material->GetTextureCount(aiTextureType_AMBIENT);
    qDebug() << "AMBIENT texture count:" << nat;
    if (nat) {
        ret = material->GetTexture(aiTextureType_AMBIENT, 0, &path);
        if (ret == aiReturn_SUCCESS)
            qDebug() << "  path:" << path.C_Str();
    }
    int ndf = material->GetTextureCount(aiTextureType_DIFFUSE);
    qDebug() << "DIFFUSE texture count:" << ndf;
    if (ndf) {
        ret = material->GetTexture(aiTextureType_DIFFUSE, 0, &path);
        if (ret == aiReturn_SUCCESS)
            qDebug() << "  path:" << path.C_Str();
    }
    int nsp = material->GetTextureCount(aiTextureType_SPECULAR);
    qDebug() << "SPECULAR texture count:" << nsp;
    if (nsp) {
        ret = material->GetTexture(aiTextureType_SPECULAR, 0, &path);
        if (ret == aiReturn_SUCCESS)
            qDebug() << "  path:" << path.C_Str();
    }

    aiColor3D dif(0.f,0.f,0.f);
    aiColor3D amb(0.f,0.f,0.f);
    aiColor3D spec(0.f,0.f,0.f);
    float shine = 0.0;
    ret = material->Get(AI_MATKEY_COLOR_AMBIENT, amb);
    if (ret == aiReturn_SUCCESS)
        qDebug() << "amb: " << printVector(amb);
    ret = material->Get(AI_MATKEY_COLOR_DIFFUSE, dif);
    if (ret == aiReturn_SUCCESS)
        qDebug() << "dif: " << printVector(dif);
    ret = material->Get(AI_MATKEY_COLOR_SPECULAR, spec);
    if (ret == aiReturn_SUCCESS)
        qDebug() << "spec: " << printVector(spec);
    ret = material->Get(AI_MATKEY_SHININESS, shine);
    if (ret == aiReturn_SUCCESS)
        qDebug() << "shine: " << shine;
}

void GLAssimpLoadModel::printNode(aiNode *node, QString tab)
{
    qDebug() << tab << "node: " << node->mName.C_Str();

    for (uint i = 0; i < node->mNumMeshes; i++)
        qDebug() << tab << "  mesh: " << m_scene->mMeshes[node->mMeshes[i]]->mName.C_Str();

    for (uint i = 0; i < node->mNumChildren; i++) {
        qDebug() << tab << "  subnode: " << node->mChildren[i]->mName.C_Str();
        printNode(node->mChildren[i], tab + "    ");
    }
}

