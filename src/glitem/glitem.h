#ifndef GLITEM_H
#define GLITEM_H

#include <QQuickItem>
#include "glloader.h"
#include "glrender.h"
#include "gltransform.h"
#include "glanimatenode.h"
#include "glenvironment.h"


class GLItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QUrl model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(QQmlListProperty<GLAnimateNode> glnode READ glnode DESIGNABLE false FINAL)
    Q_PROPERTY(QQmlListProperty<GLLight> gllight READ gllight DESIGNABLE false FINAL)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool asynchronous READ asynchronous WRITE setAsynchronous NOTIFY asynchronousChanged)
    Q_PROPERTY(GLEnvironment *environment READ environment WRITE setEnvironment NOTIFY environmentChanged)
    Q_CLASSINFO("DefaultProperty", "glnode")
public:
    GLItem(QQuickItem *parent = 0);
    ~GLItem();

    QQmlListProperty<GLAnimateNode> glnode();
    QQmlListProperty<GLLight> gllight();

    QUrl model() { return m_model; }
    void setModel(const QUrl &value);

    enum Status { Null, Ready, Loading, Error };
    Status status() const { return m_status; }

    bool asynchronous() const { return m_asynchronous; }
    void setAsynchronous(bool value);

    GLEnvironment *environment() const { return m_environment; }
    void setEnvironment(GLEnvironment *value);

    void componentComplete();
    void load();

signals:
    void modelChanged();
    void statusChanged();
    void asynchronousChanged();
    void environmentChanged();

public slots:
    void sync();
    void cleanup();
    void updateWindow();

private:
    GLLoader m_loader;
    GLRender *m_render;
    QUrl m_model;
    GLTransformNode *m_root;
    Status m_status;
    bool m_asynchronous;
    GLEnvironment *m_environment;
    EnvParam *m_envparam;

    bool loadEnvironmentImage(const QUrl &url, QImage &image);

    static int glnode_count(QQmlListProperty<GLAnimateNode> *list);
    static void glnode_append(QQmlListProperty<GLAnimateNode> *list, GLAnimateNode *);
    static GLAnimateNode *glnode_at(QQmlListProperty<GLAnimateNode> *list, int);
    static void glnode_clear(QQmlListProperty<GLAnimateNode> *list);
    QList<GLAnimateNode *> m_glnodes;

    static int gllight_count(QQmlListProperty<GLLight> *list);
    static void gllight_append(QQmlListProperty<GLLight> *list, GLLight *);
    static GLLight *gllight_at(QQmlListProperty<GLLight> *list, int);
    static void gllight_clear(QQmlListProperty<GLLight> *list);
    QList<GLLight *> m_gllights;

    bool bindAnimateNode(GLTransformNode *, GLAnimateNode *);
    void calcModelviewMatrix(GLTransformNode *, const QMatrix4x4 &);
};

#endif // SQUIRCLE_H
