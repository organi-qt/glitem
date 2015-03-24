#ifndef GLANIMATENODE_H
#define GLANIMATENODE_H

#include <QObject>
#include <QQmlListProperty>
#include "gltransform.h"

class GLAnimateNode : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QQmlListProperty<GLTransform> transform READ transform DESIGNABLE false FINAL)
public:
    GLAnimateNode(QObject *parent = 0);
    void applyTo(QMatrix4x4 *matrix);

    QQmlListProperty<GLTransform> transform();
    QString name() { return m_name; }
    void setName(const QString &value);

signals:
    void transformChanged();

private:
    QString m_name;

    static int transform_count(QQmlListProperty<GLTransform> *list);
    static void transform_append(QQmlListProperty<GLTransform> *list, GLTransform *);
    static GLTransform *transform_at(QQmlListProperty<GLTransform> *list, int);
    static void transform_clear(QQmlListProperty<GLTransform> *list);
    QList<GLTransform *> m_transforms;
};

#endif // GLANIMATENODE_H
