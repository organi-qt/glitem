#include <QQuickWindow>
#include <QThread>
#include "glitem.h"


GLItem::GLItem(QQuickItem *parent)
    : QQuickItem(parent), m_render(0), m_root(0),
      m_status(Null), m_asynchronous(true),
      m_environment(0), m_envparam(0)
{
    connect(this, &GLItem::opacityChanged, this, &GLItem::updateWindow);
}

GLItem::~GLItem()
{
    if (m_envparam) {
        delete m_envparam;
        m_envparam = 0;
    }
}

void GLItem::sync()
{
    if (m_status != Ready)
        return;

    if (!m_render) {
        m_render = new GLRender(m_root, &m_loader, m_envparam);
        if (m_envparam) {
            delete m_envparam;
            m_envparam = 0;
        }

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
        m_gllights[i]->updateState(m_render->state());

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

void GLItem::setModel(const QUrl &value)
{
    if (m_model != value) {
        m_model = value;
        emit modelChanged();
    }
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

            image = image.convertToFormat(QImage::Format_RGB888);
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

    if (m_loader.load(m_model) && (m_root = m_loader.convert())) {
        foreach (GLAnimateNode *node, m_glnodes) {
            if (!bindAnimateNode(m_root, node))
                qWarning() << "no node find in model named: " << node->name();
        }

        if (m_environment) {
            bool hasEnv = false;
            m_envparam = new EnvParam;
            m_envparam->width = m_envparam->height = 0;
            m_envparam->alpha = m_environment->alpha();

            if (m_envparam->alpha > 0 && m_envparam->alpha < 1.0) {
                hasEnv |= loadEnvironmentImage(m_environment->top(), m_envparam->top);
                hasEnv |= loadEnvironmentImage(m_environment->bottom(), m_envparam->bottom);
                hasEnv |= loadEnvironmentImage(m_environment->left(), m_envparam->left);
                hasEnv |= loadEnvironmentImage(m_environment->right(), m_envparam->right);
                hasEnv |= loadEnvironmentImage(m_environment->front(), m_envparam->front);
                hasEnv |= loadEnvironmentImage(m_environment->back(), m_envparam->back);
            }

            if (!hasEnv) {
                delete m_envparam;
                m_envparam = 0;
            }
        }

        m_status = Ready;
        if (window()) {
            connect(window(), &QQuickWindow::beforeSynchronizing, this, &GLItem::sync, Qt::DirectConnection);
            connect(window(), &QQuickWindow::sceneGraphInvalidated, this, &GLItem::cleanup, Qt::DirectConnection);
        }
    }
    else
        m_status = Error;
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

    if (m_model.isValid()) {
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
    if (node->name() == anim->name()) {
        node->setAnimateNode(anim);
        return true;
    }

    for (int i = 0; i < node->transformChildCount(); i++) {
        if (bindAnimateNode(node->transformChildAtIndex(i), anim))
            return true;
    }

    return false;
}

void GLItem::calcModelviewMatrix(GLTransformNode *node, const QMatrix4x4 &modelview)
{
    node->modelviewMatrix() = modelview * node->transformMatrix();
    if (node->animateNode())
        node->animateNode()->applyTo(&node->modelviewMatrix());

    for (int i = 0; i < node->transformChildCount(); i++)
        calcModelviewMatrix(node->transformChildAtIndex(i), node->modelviewMatrix());
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

