#ifndef GLMATERIAL_H
#define GLMATERIAL_H

#include <QObject>
#include <QColor>
#include <QUrl>

class Material;
class BasicMaterial;
class PhongMaterial;

class GLMaterial : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool transparent READ transparent WRITE setTransparent NOTIFY transparentChanged)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
public:
    GLMaterial(QObject *parent = 0);

    bool transparent() { return m_transparent; }
    void setTransparent(bool value);

    qreal opacity() { return m_opacity; }
    void setOpacity(qreal value);

    virtual Material *material();

signals:
    void transparentChanged();
    void opacityChanged();

protected:
    Material *m_material;

    bool urlToPath(const QUrl &url, QString &path);

private:
    bool m_transparent;
    qreal m_opacity;
};

class GLBasicMaterial : public GLMaterial
{
    Q_OBJECT
    Q_PROPERTY(QUrl map READ map WRITE setMap NOTIFY mapChanged)
public:
    GLBasicMaterial(QObject *parent = 0);

    QUrl map() { return m_map; }
    void setMap(const QUrl &value);

    virtual Material *material();

signals:
    void mapChanged();

private:
    QUrl m_map;
};

class GLPhongMaterial : public GLMaterial
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QColor specular READ specular WRITE setSpecular NOTIFY specularChanged)
    Q_PROPERTY(qreal shininess READ shininess WRITE setShininess NOTIFY shininessChanged)
    Q_PROPERTY(QUrl map READ map WRITE setMap NOTIFY mapChanged)
    Q_PROPERTY(QUrl specularMap READ specularMap WRITE setSpecularMap NOTIFY specularMapChanged)
    Q_PROPERTY(bool envMap READ envMap WRITE setEnvMap NOTIFY envMapChanged)
    Q_PROPERTY(qreal reflectivity READ reflectivity WRITE setReflectivity NOTIFY reflectivityChanged)
public:
    GLPhongMaterial(QObject *parent = 0);

    QColor color() { return m_color; }
    void setColor(const QColor &value);

    QColor specular() { return m_specular; }
    void setSpecular(const QColor &value);

    qreal shininess() { return m_shininess; }
    void setShininess(qreal value);

    QUrl map() { return m_map; }
    void setMap(const QUrl &value);

    QUrl specularMap() { return m_specular_map; }
    void setSpecularMap(const QUrl &value);

    bool envMap() { return m_env_map; }
    void setEnvMap(bool value);

    qreal reflectivity() { return m_reflectivity; }
    void setReflectivity(qreal value);

    virtual Material *material();

signals:
    void colorChanged();
    void specularChanged();
    void shininessChanged();
    void mapChanged();
    void specularMapChanged();
    void envMapChanged();
    void reflectivityChanged();

private:
    QColor m_color;
    QColor m_specular;
    qreal m_shininess;
    QUrl m_map;
    QUrl m_specular_map;
    bool m_env_map;
    qreal m_reflectivity;
};

#endif // GLMATERIAL_H
