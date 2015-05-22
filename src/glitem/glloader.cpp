#include "glloader.h"
#include "ailoaderiosystem.h"
#include <assimp/postprocess.h>
#include <QDebug>
#include <QFile>

GLLoader::GLLoader()
    : m_scene(0), m_vertex_buffer_size{}, m_index_buffer_size{},
      m_num_vertex{}
{
    m_importer.SetIOHandler(new AiLoaderIOSystem());
}

bool GLLoader::load(const QUrl &file)
{
    QString path;
    if (file.scheme() == "file")
        path = file.toLocalFile();
    else if (file.scheme() == "qrc")
        path = ':' + file.path();
    else {
        qWarning() << "invalide model path: " << file;
        return false;
    }

    m_scene = m_importer.ReadFile(path.toStdString(),
                       aiProcess_CalcTangentSpace       |
                       aiProcess_Triangulate            |
                       aiProcess_JoinIdenticalVertices  |
                       aiProcess_SortByPType);
    if (!m_scene) {
        qWarning() << "load file " << file << " fail: " << m_importer.GetErrorString();
        return false;
    }

    if (m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        qWarning() << "file " << file << " with incomplete scene";
        m_scene = NULL;
        m_importer.FreeScene();
        return false;
    }

    m_model_dir.setPath(path);
    m_model_dir.cdUp();
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

    return true;
}

GLTransformNode *GLLoader::convert()
{
    if (!m_scene)
        return NULL;

    GLTransformNode *view = new GLTransformNode("view", QMatrix4x4());
    GLTransformNode *model = new GLTransformNode("model", QMatrix4x4());
    GLTransformNode *root = convert(m_scene->mRootNode);
    view->addChild(model);
    model->addChild(root);

    convertLights(view);

    m_scene = NULL;
    m_importer.FreeScene();
    return view;
}

GLRenderNode *GLLoader::convert(aiMesh *mesh)
{
    if (!mesh->HasNormals()) {
        qDebug() << "mesh without normal detacted: " << mesh->mName.C_Str();
        return 0;
    }

    aiMaterial *material = m_scene->mMaterials[mesh->mMaterialIndex];

    int shadingModel;
    material->Get(AI_MATKEY_SHADING_MODEL, shadingModel);
    if (shadingModel != aiShadingMode_Phong) {
        qDebug() << "mesh not Phong shading detacted: " << mesh->mName.C_Str();
        //return 0;
    }

    aiColor3D amb(0.0f, 0.0f, 0.0f);
    aiColor3D dif(0.8f, 0.2f, 0.3f);
    aiColor3D spec(0.5f, 0.6f, 0.1f);
    float shine = 40.0f;

    if (material->Get(AI_MATKEY_COLOR_AMBIENT, amb) != aiReturn_SUCCESS)
        qWarning("material get amb fail");
    if (material->Get(AI_MATKEY_COLOR_DIFFUSE, dif) != aiReturn_SUCCESS)
        qWarning("material get dif fail");
    if (material->Get(AI_MATKEY_COLOR_SPECULAR, spec) != aiReturn_SUCCESS)
        qWarning("material get spec fail");
    if (material->Get(AI_MATKEY_SHININESS, shine) != aiReturn_SUCCESS)
        qWarning("material get shine fail");

    GLShader::ShaderType type = GLShader::PHONG;

    QString dpath, spath;
    if (mesh->GetNumUVChannels() > 0) {
        if (loadTexture(material, aiTextureType_DIFFUSE, dpath))
            type = GLShader::PHONG_DIFFUSE_TEXTURE;
        if (loadTexture(material, aiTextureType_SPECULAR, spath)) {
            if (type == GLShader::PHONG)
                type = GLShader::PHONG_SPECULAR_TEXTURE;
            else
                type = GLShader::PHONG_DIFFUSE_SPECULAR_TEXTURE;
        }
    }

    GLPhongNode *node = new GLPhongNode(
                m_vertex_buffer_size[type], m_index_buffer_size[type],
                mesh->mNumVertices, mesh->mNumFaces * 3, type);
    node->setMaterial(
                QVector3D(amb.r, amb.g, amb.b),
                QVector3D(dif.r, dif.g, dif.b),
                QVector3D(spec.r, spec.g, spec.b),
                shine);
    node->allocateData();
    if (type == GLShader::PHONG)
        loadVertex(node, mesh->mVertices, mesh->mNormals);
    else
        loadVertex(node, mesh->mVertices, mesh->mNormals, mesh->mTextureCoords[0]);
    loadIndex(node, mesh->mFaces);

    if (!dpath.isEmpty())
        node->setDiffuseTexturePath(dpath);
    if (!spath.isEmpty())
        node->setSpecularTexturePath(spath);

    m_vertex_buffer_size[type] += node->vertexCount() * node->stride();
    m_index_buffer_size[type] += node->indexCount() * sizeof(ushort);
    m_num_vertex[type] += node->vertexCount();
    if (m_num_vertex[type] >= USHRT_MAX)
        qFatal("model with too many vertex");
    return node;
}

GLTransformNode *GLLoader::convert(aiNode *node)
{
    // meaningless node for render
    if (node->mNumChildren == 0 && node->mNumMeshes == 0)
        return NULL;

    GLTransformNode *root =
            new GLTransformNode(node->mName.C_Str(), QMatrix4x4(node->mTransformation[0]));

    for (uint i = 0; i < node->mNumMeshes; i++) {
        GLRenderNode *rnode = convert(m_scene->mMeshes[node->mMeshes[i]]);
        if (rnode)
            root->addChild(rnode);
    }

    for (uint i = 0; i < node->mNumChildren; i++) {
        GLTransformNode *tnode = convert(node->mChildren[i]);
        if (tnode)
            root->addChild(tnode);
    }

    return root;
}

void GLLoader::assign(QVector3D &qc, const aiColor3D &ac)
{
    qc.setX(ac.r);
    qc.setY(ac.g);
    qc.setZ(ac.b);
}

void GLLoader::assign(QVector3D &qv, const aiVector3D &av)
{
    qv.setX(av.x);
    qv.setY(av.y);
    qv.setZ(av.z);
}

void GLLoader::convertLights(GLTransformNode *root)
{
    m_lights.clear();

    if (m_scene->HasLights()) {
        for (uint i = 0; i < m_scene->mNumLights; i++) {
            Light light;
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

            switch (srcLight->mType) {
            case aiLightSource_DIRECTIONAL:
                light.type = Light::SUN;
                assign(light.pos, srcLight->mDirection);
                break;
            case aiLightSource_POINT:
                light.type = Light::POINT;
                assign(light.pos, srcLight->mPosition);
                break;
            default:
                continue;
            }
            assign(light.amb, srcLight->mColorAmbient);
            assign(light.dif, srcLight->mColorDiffuse);
            assign(light.spec, srcLight->mColorSpecular);
            light.name = srcLight->mName.C_Str();
            light.node = new GLTransformNode(light.name, QMatrix4x4(trans[0]));
            m_lights.append(light);

            root->addChild(light.node);
        }
    }

    // at least one light
    if (m_lights.size() == 0) {
        Light light;
        light.pos = QVector3D(100, 100, 100);
        light.amb = QVector3D(1, 1, 1);
        light.dif = QVector3D(1, 1, 1);
        light.spec = QVector3D(1, 1, 1);
        light.type = Light::POINT;
        light.name = "default_light";
        light.node = new GLTransformNode(light.name, QMatrix4x4());

        m_lights.append(light);
        root->addChild(light.node);
    }
}

void GLLoader::loadVertex(GLRenderNode *node, aiVector3D *vertices, aiVector3D *normals)
{
    GLRenderNode::NormalPoint3D *data =
            static_cast<GLRenderNode::NormalPoint3D *>(node->vertexData());
    for (int i = 0; i < node->vertexCount(); i++)
        data[i].set(vertices + i, normals + i);
}

void GLLoader::loadVertex(GLRenderNode *node, aiVector3D *vertices, aiVector3D *normals, aiVector3D *texCoords)
{
    GLRenderNode::TexturedNormalPoint3D *data =
            static_cast<GLRenderNode::TexturedNormalPoint3D *>(node->vertexData());
    for (int i = 0; i < node->vertexCount(); i++)
        data[i].set(vertices + i, normals + i, texCoords + i);
}

void GLLoader::loadIndex(GLRenderNode *node, aiFace *faces)
{
    ushort *index = static_cast<ushort *>(node->indexData());
    for (int i = 0; i < node->indexCount() / 3; i++) {
        Q_ASSERT(faces[i].mNumIndices == 3);
        index[i*3] = faces[i].mIndices[0] + m_num_vertex[node->type()];
        index[i*3 + 1] = faces[i].mIndices[1] + m_num_vertex[node->type()];
        index[i*3 + 2] = faces[i].mIndices[2] + m_num_vertex[node->type()];
    }
}

bool GLLoader::loadTexture(aiMaterial *material, aiTextureType type, QString &path)
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

        path = tpath.C_Str();
        if (m_textures.contains(path)) {
            if (m_textures[path].mode != wmode)
                qWarning() << "texture with different wrap mode detacted: "
                           << m_model_dir.filePath(path);
            return true;
        }
        else {
            Texture texture;
            if (texture.image.load(m_model_dir.filePath(path))) {
                texture.mode = wmode;
                m_textures[path] = texture;
                return true;
            }
            else {
                qWarning() << "load texture image fail: "
                           << m_model_dir.filePath(path);
                return false;
            }
        }
    }
    return false;
}

void GLLoader::printCamera(aiCamera *camera)
{
    qDebug() << "camera" << " name:" << camera->mName.C_Str()
             << " pos: " << printVector(camera->mPosition)
             << " lookat: " << printVector(camera->mLookAt)
             << " up: " << printVector(camera->mUp)
             << " near: " << camera->mClipPlaneNear
             << " far: " << camera->mClipPlaneFar;
}

void GLLoader::printLight(aiLight *light)
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

QString GLLoader::printVector(aiVector3D &v)
{
    return QString("x=%1 y=%2 z=%3").arg(v.x).arg(v.y).arg(v.z);
}

QString GLLoader::printVector(aiColor3D &c)
{
    return QString("r=%1 g=%2 b=%3").arg(c.r).arg(c.g).arg(c.b);
}

QString GLLoader::printVector(aiColor4D &c)
{
    return QString("r=%1 g=%2 b=%3 a=%4").arg(c.r).arg(c.g).arg(c.b).arg(c.a);
}

void GLLoader::printMaterial(aiMaterial *material)
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

void GLLoader::printNode(aiNode *node, QString tab)
{
    qDebug() << tab << "node: " << node->mName.C_Str();

    for (uint i = 0; i < node->mNumMeshes; i++)
        qDebug() << tab << "  mesh: " << m_scene->mMeshes[node->mMeshes[i]]->mName.C_Str();

    for (uint i = 0; i < node->mNumChildren; i++) {
        qDebug() << tab << "  subnode: " << node->mChildren[i]->mName.C_Str();
        printNode(node->mChildren[i], tab + "    ");
    }
}
