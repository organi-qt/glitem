#include <QQuickWindow>
#include <QFileInfo>
#include "glitem.h"

GLItem::GLItem()
    : m_render(0), m_root(0), m_glopacity(1),
      m_light_pos(100, 100, 100), m_light_amb(1, 1, 1),
      m_light_dif(1, 1, 1), m_light_spec(1, 1, 1)
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
    for (int i = 0; i < m_gltransforms.size(); i++)
        m_gltransforms[i]->applyTo(&modelview);
    calcModelviewMatrix(m_root, modelview);

    m_render->setOpacity(m_glopacity);
    m_render->setLightPos(m_light_pos);
    m_render->setLightAmb(m_light_amb);
    m_render->setLightDif(m_light_dif);
    m_render->setLightSpec(m_light_spec);
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

QQmlListProperty<GLTransform> GLItem::gltransform()
{
    return QQmlListProperty<GLTransform>(this, 0, gltransform_append, gltransform_count, gltransform_at, gltransform_clear);
}

int GLItem::gltransform_count(QQmlListProperty<GLTransform> *list)
{
    GLItem *object = qobject_cast<GLItem *>(list->object);
    if (object) {
        return object->m_gltransforms.count();
    } else {
        qWarning()<<"Warning: could not find GLItem to query for transformation count.";
        return 0;
    }
}

void GLItem::gltransform_append(QQmlListProperty<GLTransform> *list, GLTransform *item)
{
    GLItem *object = qobject_cast<GLItem *>(list->object);
    QList<GLTransform *> *ptrans;
    if (object) {
        ptrans = &object->m_gltransforms;

        //We now need to connect the underlying transform so that any change will update the graphical item.
        if (!ptrans->contains(item)) {
            ptrans->append(item);
            QObject::connect(item, SIGNAL(transformChanged()),
                             object, SLOT(updateWindow()));
        }
    }
    else
        qWarning()<<"Warning: could not find GLItem to add transformation to.";
}

GLTransform *GLItem::gltransform_at(QQmlListProperty<GLTransform> *list, int idx)
{
    GLItem *object = qobject_cast<GLItem *>(list->object);
    if (object) {
        return object->m_gltransforms.at(idx);
    } else {
        qWarning()<<"Warning: could not find GLItem to query for transformations";
        return 0;
    }
    return 0;
}

void GLItem::gltransform_clear(QQmlListProperty<GLTransform> *list)
{

    GLItem *object = qobject_cast<GLItem *>(list->object);
    if (object) {
        object->m_gltransforms.clear();
        object->updateWindow();
    }
    else
        qWarning()<<"Warning: could not find GLItem to clear of transformations";
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
    QList<GLAnimateNode *> *ptrans;
    if (object) {
        ptrans = &object->m_glnodes;
        if (!ptrans->contains(item)) {
            if (!object->bindAnimateNode(object->m_root, item)) {
                qWarning() << "no node find in model named: " << item->name();
                return;
            }

            ptrans->append(item);
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

void GLItem::setLightPos(const QVector3D &value)
{
    if (m_light_pos != value) {
        m_light_pos = value;
        emit lightPosChanged();
        updateWindow();
    }
}

void GLItem::setLightAmb(const QVector3D &value)
{
    if (m_light_amb != value) {
        m_light_amb = value;
        emit lightAmbChanged();
        updateWindow();
    }
}

void GLItem::setLightDif(const QVector3D &value)
{
    if (m_light_dif != value) {
        m_light_dif = value;
        emit lightDifChanged();
        updateWindow();
    }
}

void GLItem::setLightSpec(const QVector3D &value)
{
    if (m_light_spec != value) {
        m_light_spec = value;
        emit lightSpecChanged();
        updateWindow();
    }
}


