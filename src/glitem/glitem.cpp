#include <QQuickWindow>
#include <QFileInfo>
#include "glitem.h"

GLItem::GLItem()
    : m_render(0), m_root(0), m_glopacity(1)
{

}

void GLItem::handleWindowChanged(QQuickWindow *win)
{
    if (win) {
        connect(win, SIGNAL(beforeSynchronizing()), this, SLOT(sync()), Qt::DirectConnection);
        connect(win, SIGNAL(sceneGraphInvalidated()), this, SLOT(cleanup()), Qt::DirectConnection);
        win->setClearBeforeRendering(false);
    }
}

void GLItem::sync()
{
    if (!m_render) {
        m_render = new GLRender(m_root, QRect(x(), y(), width(), height()), &m_loader);
        connect(window(), SIGNAL(beforeRendering()), m_render, SLOT(render()), Qt::DirectConnection);
    }

    QMatrix4x4 modelview;
    calcModelviewMatrix(m_root, modelview);

    m_render->state()->setOpacity(m_glopacity);

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

void GLItem::setModel(const QString &value)
{
    if (m_model != value) {
        m_model = value;

        QFileInfo info(value);
        if (info.exists() && m_loader.load(info.path(), info.fileName())) {
            m_root = m_loader.convert();
            connect(this, SIGNAL(windowChanged(QQuickWindow*)),
                    this, SLOT(handleWindowChanged(QQuickWindow*)));
        }
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
            if (!object->bindAnimateNode(object->m_root, item)) {
                qWarning() << "no node find in model named: " << item->name();
                return;
            }

            pnodes->append(item);
            QObject::connect(item, SIGNAL(transformChanged()),
                             object, SLOT(updateWindow()));
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

void GLItem::setGLOpacity(qreal value)
{
    if (m_glopacity != value) {
        m_glopacity = value;
        emit glopacityChanged();
        updateWindow();
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
            QObject::connect(item, SIGNAL(lightChanged()),
                             object, SLOT(updateWindow()));
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

