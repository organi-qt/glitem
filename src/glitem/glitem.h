#ifndef GLITEM_H
#define GLITEM_H

#include <QQuickItem>


class GLTransformNode;
class GLRender;
class GLEnvironment;
class GLModel;
class GLMaterial;
class GLAnimateNode;
class GLLight;
class EnvParam;
class Light;
class Material;

class GLItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<GLAnimateNode> glnode READ glnode DESIGNABLE false FINAL)
    Q_PROPERTY(QQmlListProperty<GLLight> gllight READ gllight DESIGNABLE false FINAL)
    Q_PROPERTY(QQmlListProperty<GLModel> glmodel READ glmodel DESIGNABLE false FINAL)
    Q_PROPERTY(QQmlListProperty<GLMaterial> glmaterial READ glmaterial DESIGNABLE false FINAL)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool asynchronous READ asynchronous WRITE setAsynchronous NOTIFY asynchronousChanged)
    Q_PROPERTY(GLEnvironment *environment READ environment WRITE setEnvironment NOTIFY environmentChanged)
    Q_CLASSINFO("DefaultProperty", "glnode")
public:
    GLItem(QQuickItem *parent = 0);
    ~GLItem();

    QQmlListProperty<GLAnimateNode> glnode();
    QQmlListProperty<GLLight> gllight();
    QQmlListProperty<GLModel> glmodel();
    QQmlListProperty<GLMaterial> glmaterial();

    enum Status { Null, Ready, Loading, Error };
    Status status() const { return m_status; }

    bool asynchronous() const { return m_asynchronous; }
    void setAsynchronous(bool value);

    GLEnvironment *environment() const { return m_environment; }
    void setEnvironment(GLEnvironment *value);

    void componentComplete();
    void load();

signals:
    void statusChanged();
    void asynchronousChanged();
    void environmentChanged();

public slots:
    void sync();
    void cleanup();
    void updateWindow();

private:
    GLRender *m_render;
    GLTransformNode *m_root;
    Status m_status;
    bool m_asynchronous;
    GLEnvironment *m_environment;
    EnvParam *m_envparam;

    QVector<float> m_vertex;
    QVector<ushort> m_index;
    QList<Light *> m_lights;
    QList<Material *> m_materials;
    bool m_has_texture_uv;
    int m_num_vertex;

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

    static int glmodel_count(QQmlListProperty<GLModel> *list);
    static void glmodel_append(QQmlListProperty<GLModel> *list, GLModel *);
    static GLModel *glmodel_at(QQmlListProperty<GLModel> *list, int);
    static void glmodel_clear(QQmlListProperty<GLModel> *list);
    QList<GLModel *> m_glmodels;

    static int glmaterial_count(QQmlListProperty<GLMaterial> *list);
    static void glmaterial_append(QQmlListProperty<GLMaterial> *list, GLMaterial *);
    static GLMaterial *glmaterial_at(QQmlListProperty<GLMaterial> *list, int);
    static void glmaterial_clear(QQmlListProperty<GLMaterial> *list);
    QList<GLMaterial *> m_glmaterials;

    bool bindAnimateNode(GLTransformNode *, GLAnimateNode *);
    void calcModelviewMatrix(GLTransformNode *, const QMatrix4x4 &);
};

#endif // GLITEM_H
