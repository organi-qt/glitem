#include "glanimatenode.h"

GLAnimateNode::GLAnimateNode(QObject *parent) :
    QObject(parent)
{
}

QQmlListProperty<GLTransform> GLAnimateNode::transform()
{
    return QQmlListProperty<GLTransform>(this, 0, transform_append, transform_count, transform_at, transform_clear);
}

int GLAnimateNode::transform_count(QQmlListProperty<GLTransform> *list)
{
    GLAnimateNode *object = qobject_cast<GLAnimateNode *>(list->object);
    if (object) {
        return object->m_transforms.count();
    } else {
        qWarning()<<"Warning: could not find GLAnimateNode to query for transformation count.";
        return 0;
    }
}

void GLAnimateNode::transform_append(QQmlListProperty<GLTransform> *list, GLTransform *item)
{
    GLAnimateNode *object = qobject_cast<GLAnimateNode *>(list->object);
    QList<GLTransform *> *ptrans;
    if (object) {
        ptrans = &object->m_transforms;
        if (!ptrans->contains(item)) {
            ptrans->append(item);
            QObject::connect(item, &GLTransform::transformChanged,
                             object, &GLAnimateNode::transformChanged);
        }
    }
    else
        qWarning()<<"Warning: could not find GLAnimateNode to add transformation to.";
}

GLTransform *GLAnimateNode::transform_at(QQmlListProperty<GLTransform> *list, int idx)
{
    GLAnimateNode *object = qobject_cast<GLAnimateNode *>(list->object);
    if (object) {
        return object->m_transforms.at(idx);
    } else {
        qWarning()<<"Warning: could not find GLAnimateNode to query for transformations";
        return 0;
    }
    return 0;
}

void GLAnimateNode::transform_clear(QQmlListProperty<GLTransform> *list)
{

    GLAnimateNode *object = qobject_cast<GLAnimateNode *>(list->object);
    if (object) {
        object->m_transforms.clear();
        object->transformChanged();
    }
    else
        qWarning()<<"Warning: could not find GLAnimateNode to clear of transformations";
}

void GLAnimateNode::setName(const QString &value)
{
    if (m_name != value)
        m_name = value;
}

void GLAnimateNode::applyTo(QMatrix4x4 *matrix)
{
    for (int i = 0; i < m_transforms.size(); i++)
        m_transforms[i]->applyTo(matrix);
}
