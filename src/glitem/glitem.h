#ifndef GLITEM_H
#define GLITEM_H

#include <QQuickItem>
#include "glloader.h"
#include "glrender.h"
#include "gltransform.h"
#include "glanimatenode.h"

class GLItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString model READ model WRITE setModel)
    Q_PROPERTY(qreal glopacity READ glopacity WRITE setGLOpacity NOTIFY glopacityChanged)
    Q_PROPERTY(QQmlListProperty<GLTransform> gltransform READ gltransform DESIGNABLE false FINAL)
    Q_PROPERTY(QQmlListProperty<GLAnimateNode> glnode READ glnode DESIGNABLE false FINAL)
    Q_PROPERTY(QVector3D light_pos READ lightPos WRITE setLightPos NOTIFY lightPosChanged)
    Q_PROPERTY(QVector3D light_amb READ lightAmb WRITE setLightAmb NOTIFY lightAmbChanged)
    Q_PROPERTY(QVector3D light_dif READ lightDif WRITE setLightDif NOTIFY lightDifChanged)
    Q_PROPERTY(QVector3D light_spec READ lightSpec WRITE setLightSpec NOTIFY lightSpecChanged)
    Q_CLASSINFO("DefaultProperty", "glnode")
public:
    GLItem();

    QQmlListProperty<GLTransform> gltransform();
    QQmlListProperty<GLAnimateNode> glnode();

    QString model() { return m_model; }
    void setModel(const QString &value);

    qreal glopacity() { return m_glopacity; }
    void setGLOpacity(qreal value);

    QVector3D lightPos() { return m_light_pos; }
    void setLightPos(const QVector3D &value);

    QVector3D lightAmb() { return m_light_amb; }
    void setLightAmb(const QVector3D &value);

    QVector3D lightDif() { return m_light_dif; }
    void setLightDif(const QVector3D &value);

    QVector3D lightSpec() { return m_light_spec; }
    void setLightSpec(const QVector3D &value);

signals:
    void glopacityChanged();
    void lightPosChanged();
    void lightAmbChanged();
    void lightDifChanged();
    void lightSpecChanged();

public slots:
    void sync();
    void cleanup();
    void updateWindow();

private slots:
    void handleWindowChanged(QQuickWindow *win);

private:
    GLLoader m_loader;
    GLRender *m_render;
    QString m_model;
    GLTransformNode *m_root;
    qreal m_glopacity;
    QVector3D m_light_pos;
    QVector3D m_light_amb;
    QVector3D m_light_dif;
    QVector3D m_light_spec;

    static int gltransform_count(QQmlListProperty<GLTransform> *list);
    static void gltransform_append(QQmlListProperty<GLTransform> *list, GLTransform *);
    static GLTransform *gltransform_at(QQmlListProperty<GLTransform> *list, int);
    static void gltransform_clear(QQmlListProperty<GLTransform> *list);
    QList<GLTransform *> m_gltransforms;

    static int glnode_count(QQmlListProperty<GLAnimateNode> *list);
    static void glnode_append(QQmlListProperty<GLAnimateNode> *list, GLAnimateNode *);
    static GLAnimateNode *glnode_at(QQmlListProperty<GLAnimateNode> *list, int);
    static void glnode_clear(QQmlListProperty<GLAnimateNode> *list);
    QList<GLAnimateNode *> m_glnodes;

    bool bindAnimateNode(GLTransformNode *, GLAnimateNode *);
    void calcModelviewMatrix(GLTransformNode *, const QMatrix4x4 &);
};

#endif // SQUIRCLE_H
