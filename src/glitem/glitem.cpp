#include <QQuickWindow>
#include <QThread>
#include "glitem.h"
#include "glmodel.h"
#include "glnode.h"
#include "glrender.h"
#include "glenvironment.h"
#include "glmaterial.h"
#include "gllight.h"
#include "material.h"


GLItem::GLItem(QQuickItem *parent)
    : QQuickItem(parent), m_render(0), m_root(0),
      m_status(Null), m_asynchronous(true),
      m_environment(0), m_envparam(0), m_has_texture_uv(false)
{
    connect(this, &GLItem::opacityChanged, this, &GLItem::updateWindow);
}

GLItem::~GLItem()
{
    if (m_envparam) {
        delete m_envparam;
        m_envparam = 0;
    }

    if (m_root)
        delete m_root;

    qDeleteAll(m_materials);
    qDeleteAll(m_lights);
}

void GLItem::sync()
{
    if (m_status != Ready)
        return;

    if (!m_render) {
        RenderParam param = {
            .root = m_root,
            .vertex = &m_vertex,
            .index = &m_index,
            .materials = &m_materials,
            .lights = &m_lights,
            .env = m_envparam,
            .has_texture_uv = m_has_texture_uv,
            .num_vertex = m_num_vertex
        };
        m_render = new GLRender(&param);

        if (m_lights.isEmpty())
           m_render->state()->setLightAmb(QVector3D(1, 1, 1));

        if (m_envparam) {
            delete m_envparam;
            m_envparam = 0;
        }

        m_vertex.clear();
        m_index.clear();

        connect(window(), &QQuickWindow::beforeRendering, m_render, &GLRender::render, Qt::DirectConnection);
    }

    QRect viewport(x(), y(), width(), height());
    m_render->setViewport(viewport);

    if (isVisible() && opacity() != 0 &&
        width() > 0 && height() > 0 &&
        QRect(0, 0, window()->width(), window()->height()).intersects(viewport)) {
        m_render->state()->visible = true;
        window()->setClearBeforeRendering(false);
    }
    else {
        m_render->state()->visible = false;
        window()->setClearBeforeRendering(true);
        return;
    }

    QMatrix4x4 modelview;
    calcModelviewMatrix(m_root, modelview);

    m_render->state()->setOpacity(opacity());

    for (int i = 0; i < m_gllights.size(); i++)
        m_gllights[i]->sync();

    m_render->updateLightFinalPos();
}

void GLItem::cleanup()
{
    if (m_render) {
        delete m_render;
        m_render = 0;
    }
}

void GLItem::updateWindow()
{
    if (window())
        window()->update();
}

void GLItem::setAsynchronous(bool value)
{
    if (m_asynchronous != value) {
        m_asynchronous = value;
        emit asynchronousChanged();
    }
}

void GLItem::setEnvironment(GLEnvironment *value)
{
    if (m_environment != value) {
        m_environment = value;
        emit environmentChanged();
    }
}

bool GLItem::loadEnvironmentImage(const QUrl &url, QImage &image)
{
    if (!url.isEmpty()) {
        QString path;
        if (url.scheme() == "file")
            path = url.toLocalFile();
        else if (url.scheme() == "qrc")
            path = ':' + url.path();
        else {
            qWarning() << "invalide environment texture path: " << url;
            return false;
        }

        if (image.load(path)) {
            Q_ASSERT(image.width() == image.height());
            Q_ASSERT(!m_envparam->width || m_envparam->width == image.width());
            Q_ASSERT(!m_envparam->height || m_envparam->height == image.height());

            image = image.convertToFormat(QImage::Format_RGBA8888);
            m_envparam->width = image.width();
            m_envparam->height = image.height();
            return true;
        }
    }
    return false;
}

void GLItem::load()
{
    m_status = Loading;
    emit statusChanged();

    GLTransformNode *view = NULL, *model = NULL;
    for (int i = 0; i < m_glmodels.size(); i++) {
        GLModel *md = m_glmodels[i];

        if (md->load()) {
            if (!m_root) {
                view = new GLTransformNode("view");
                model = new GLTransformNode("model");
                view->addChild(model);
                m_root = view;
            }

            model->addChild(md->tnodes());
            model->addChild(md->rnodes());

            foreach (Light *light, md->lights()) {
                model->addChild(light->node);
            }
            m_lights.append(md->lights());

            m_materials.append(md->materials());
        }
    }

    if (!m_root) {
        m_status = Error;
        emit statusChanged();
        return;
    }

    foreach (GLLight *gllight, m_gllights) {
        bool found = false;
        foreach (Light *light, m_lights) {
            if (gllight->name() == light->name) {
                // use view node when controled by GLLight
                model->removeChild(light->node);
                if (gllight->view())
                    light->node = view;
                else
                    light->node = model;

                gllight->setLight(light);
                found = true;
                break;
            }
        }

        if (!found) {
            Light *light = new Light;
            light->name = gllight->name();
            if (gllight->view())
                light->node = view;
            else
                light->node = model;

            gllight->setLight(light);
            m_lights.append(light);
        }
    }

    for (int i = 0; i < m_glmaterials.size(); i++)
        m_materials.append(m_glmaterials[i]->material());

    QList<float> vertex;
    QList<ushort> index;
    // build textured vertex array
    for (int i = 0; i < m_glmodels.size(); i++) {
        GLModel *md = m_glmodels[i];

        int ibase = vertex.size() / 6;
        for (int j = 0; j < md->texturedIndex().size(); j++)
            md->texturedIndex()[j] += ibase;

        int ioff = index.size() * sizeof(ushort);
        for (int j = 0; j < md->meshes().size(); j++)
            if (md->meshes()[j].type == Mesh::TEXTURED)
                md->meshes()[j].index_offset += ioff;

        vertex.append(md->texturedVertex());
        index.append(md->texturedIndex());
    }

    // build normal vertex array
    for (int i = 0; i < m_glmodels.size(); i++) {
        GLModel *md = m_glmodels[i];

        int ibase = vertex.size() / 6;
        for (int j = 0; j < md->index().size(); j++)
            md->index()[j] += ibase;

        int ioff = index.size() * sizeof(ushort);
        for (int j = 0; j < md->meshes().size(); j++)
            if (md->meshes()[j].type == Mesh::NORMAL)
                md->meshes()[j].index_offset += ioff;

        vertex.append(md->vertex());
        index.append(md->index());
    }

    m_num_vertex = vertex.size() / 6;
    Q_ASSERT(m_num_vertex < USHRT_MAX);

    // build texture uv array
    for (int i = 0; i < m_glmodels.size(); i++) {
        GLModel *md = m_glmodels[i];
        if (!md->texturedVertexUV().isEmpty())
            m_has_texture_uv = true;
        vertex.append(md->texturedVertexUV());
        // free data stored in model
        md->release();
    }

    m_vertex = vertex.toVector();
    m_index = index.toVector();

    // bind animated node to scene graph
    foreach (GLAnimateNode *node, m_glnodes) {
        if (!bindAnimateNode(m_root, node))
            qWarning() << "no node find in model named: " << node->name();
    }

    // load environment texture
    if (m_environment) {
        bool hasEnv = false;
        m_envparam = new EnvParam;
        m_envparam->width = m_envparam->height = 0;

        hasEnv |= loadEnvironmentImage(m_environment->top(), m_envparam->top);
        hasEnv |= loadEnvironmentImage(m_environment->bottom(), m_envparam->bottom);
        hasEnv |= loadEnvironmentImage(m_environment->left(), m_envparam->left);
        hasEnv |= loadEnvironmentImage(m_environment->right(), m_envparam->right);
        hasEnv |= loadEnvironmentImage(m_environment->front(), m_envparam->front);
        hasEnv |= loadEnvironmentImage(m_environment->back(), m_envparam->back);

        if (!hasEnv) {
            delete m_envparam;
            m_envparam = 0;
        }
    }

    if (window()) {
        connect(window(), &QQuickWindow::beforeSynchronizing, this, &GLItem::sync, Qt::DirectConnection);
        connect(window(), &QQuickWindow::sceneGraphInvalidated, this, &GLItem::cleanup, Qt::DirectConnection);
    }

    m_status = Ready;
    emit statusChanged();
}

class AsyncLoadThread : public QThread
{
public:
    AsyncLoadThread(QObject *parent = 0)
        : QThread(parent)
    {}

protected:
    void run() {
        GLItem *item = qobject_cast<GLItem *>(parent());
        item->load();
    }
};

void GLItem::componentComplete()
{
    QQuickItem::componentComplete();

    if (!m_glmodels.isEmpty()) {
        if (m_asynchronous) {
            QThread *t = new AsyncLoadThread(this);
            connect(t, &QThread::finished, t, &QObject::deleteLater);
            t->start();
        }
        else
            load();
    }
}

QQmlListProperty<GLAnimateNode> GLItem::glnode()
{
    return QQmlListProperty<GLAnimateNode>(this, 0, glnode_append, glnode_count, glnode_at, glnode_clear);
}

int GLItem::glnode_count(QQmlListProperty<GLAnimateNode> *list)
{
    GLItem *object = qobject_cast<GLItem *>(list->object);
    if (object) {
        return object->m_glnodes.count();
    } else {
        qWarning()<<"Warning: could not find GLItem to query for node count.";
        return 0;
    }
}

void GLItem::glnode_append(QQmlListProperty<GLAnimateNode> *list, GLAnimateNode *item)
{
    GLItem *object = qobject_cast<GLItem *>(list->object);
    QList<GLAnimateNode *> *pnodes;
    if (object) {
        pnodes = &object->m_glnodes;
        if (!pnodes->contains(item)) {
            pnodes->append(item);
            QObject::connect(item, &GLAnimateNode::transformChanged,
                             object, &GLItem::updateWindow);
        }
    }
    else
        qWarning()<<"Warning: could not find GLItem to add node to.";
}

GLAnimateNode *GLItem::glnode_at(QQmlListProperty<GLAnimateNode> *list, int idx)
{
    GLItem *object = qobject_cast<GLItem *>(list->object);
    if (object) {
        return object->m_glnodes.at(idx);
    } else {
        qWarning()<<"Warning: could not find GLItem to query for node";
        return 0;
    }
    return 0;
}

void GLItem::glnode_clear(QQmlListProperty<GLAnimateNode> *list)
{

    GLItem *object = qobject_cast<GLItem *>(list->object);
    if (object) {
        object->m_glnodes.clear();
        object->updateWindow();
    }
    else
        qWarning()<<"Warning: could not find GLItem to clear of node";
}

bool GLItem::bindAnimateNode(GLTransformNode *node, GLAnimateNode *anim)
{
    bool ret = false;
    if (node->name() == anim->name()) {
        node->setAnimateNode(anim);
        ret = true;
    }

    foreach (GLTransformNode *tnode, node->transformChildren()) {
        if (bindAnimateNode(tnode, anim))
            ret = true;
    }

    return ret;
}

void GLItem::calcModelviewMatrix(GLTransformNode *node, const QMatrix4x4 &modelview)
{
    node->modelviewMatrix() = modelview * node->transformMatrix();
    if (node->animateNode())
        node->animateNode()->applyTo(&node->modelviewMatrix());

    foreach (GLTransformNode *tnode, node->transformChildren()) {
        calcModelviewMatrix(tnode, node->modelviewMatrix());
    }
}

QQmlListProperty<GLLight> GLItem::gllight()
{
    return QQmlListProperty<GLLight>(this, 0, gllight_append, gllight_count, gllight_at, gllight_clear);
}

int GLItem::gllight_count(QQmlListProperty<GLLight> *list)
{
    GLItem *object = qobject_cast<GLItem *>(list->object);
    if (object) {
        return object->m_gllights.count();
    } else {
        qWarning()<<"Warning: could not find GLItem to query for light count.";
        return 0;
    }
}

void GLItem::gllight_append(QQmlListProperty<GLLight> *list, GLLight *item)
{
    GLItem *object = qobject_cast<GLItem *>(list->object);
    QList<GLLight *> *plights;
    if (object) {
        plights = &object->m_gllights;

        if (!plights->contains(item)) {
            plights->append(item);
            QObject::connect(item, &GLLight::lightChanged,
                             object, &GLItem::updateWindow);
        }
    }
    else
        qWarning()<<"Warning: could not find GLItem to add light to.";
}

GLLight *GLItem::gllight_at(QQmlListProperty<GLLight> *list, int idx)
{
    GLItem *object = qobject_cast<GLItem *>(list->object);
    if (object) {
        return object->m_gllights.at(idx);
    } else {
        qWarning()<<"Warning: could not find GLItem to query for lights";
        return 0;
    }
    return 0;
}

void GLItem::gllight_clear(QQmlListProperty<GLLight> *list)
{
    GLItem *object = qobject_cast<GLItem *>(list->object);
    if (object) {
        object->m_gllights.clear();
        object->updateWindow();
    }
    else
        qWarning()<<"Warning: could not find GLItem to clear of lights";
}

QQmlListProperty<GLModel> GLItem::glmodel()
{
    return QQmlListProperty<GLModel>(this, 0, glmodel_append, glmodel_count, glmodel_at, glmodel_clear);
}

int GLItem::glmodel_count(QQmlListProperty<GLModel> *list)
{
    GLItem *object = qobject_cast<GLItem *>(list->object);
    if (object) {
        return object->m_glmodels.count();
    } else {
        qWarning()<<"Warning: could not find GLItem to query for model count.";
        return 0;
    }
}

void GLItem::glmodel_append(QQmlListProperty<GLModel> *list, GLModel *item)
{
    GLItem *object = qobject_cast<GLItem *>(list->object);
    QList<GLModel *> *pmodels;
    if (object) {
        pmodels = &object->m_glmodels;

        if (!pmodels->contains(item))
            pmodels->append(item);
    }
    else
        qWarning()<<"Warning: could not find GLItem to add model to.";
}

GLModel *GLItem::glmodel_at(QQmlListProperty<GLModel> *list, int idx)
{
    GLItem *object = qobject_cast<GLItem *>(list->object);
    if (object) {
        return object->m_glmodels.at(idx);
    } else {
        qWarning()<<"Warning: could not find GLItem to query for models";
        return 0;
    }
    return 0;
}

void GLItem::glmodel_clear(QQmlListProperty<GLModel> *list)
{
    GLItem *object = qobject_cast<GLItem *>(list->object);
    if (object)
        object->m_glmodels.clear();
    else
        qWarning()<<"Warning: could not find GLItem to clear of models";
}

QQmlListProperty<GLMaterial> GLItem::glmaterial()
{
    return QQmlListProperty<GLMaterial>(this, 0, glmaterial_append, glmaterial_count, glmaterial_at, glmaterial_clear);
}

int GLItem::glmaterial_count(QQmlListProperty<GLMaterial> *list)
{
    GLItem *object = qobject_cast<GLItem *>(list->object);
    if (object) {
        return object->m_glmaterials.count();
    } else {
        qWarning()<<"Warning: could not find GLItem to query for material count.";
        return 0;
    }
}

void GLItem::glmaterial_append(QQmlListProperty<GLMaterial> *list, GLMaterial *item)
{
    GLItem *object = qobject_cast<GLItem *>(list->object);
    QList<GLMaterial *> *pmaterials;
    if (object) {
        pmaterials = &object->m_glmaterials;

        if (!pmaterials->contains(item))
            pmaterials->append(item);
    }
    else
        qWarning()<<"Warning: could not find GLItem to add material to.";
}

GLMaterial *GLItem::glmaterial_at(QQmlListProperty<GLMaterial> *list, int idx)
{
    GLItem *object = qobject_cast<GLItem *>(list->object);
    if (object) {
        return object->m_glmaterials.at(idx);
    } else {
        qWarning()<<"Warning: could not find GLItem to query for materials";
        return 0;
    }
    return 0;
}

void GLItem::glmaterial_clear(QQmlListProperty<GLMaterial> *list)
{
    GLItem *object = qobject_cast<GLItem *>(list->object);
    if (object)
        object->m_glmaterials.clear();
    else
        qWarning()<<"Warning: could not find GLItem to clear of materials";
}

