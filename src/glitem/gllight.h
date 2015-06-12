#ifndef GLLIGHT_H
#define GLLIGHT_H

#include <QObject>
#include <QVector3D>
#include <QString>

class Light;

class GLLight : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QVector3D diffuse READ diffuse WRITE setDiffuse NOTIFY diffuseChanged)
    Q_PROPERTY(QVector3D specular READ specular WRITE setSpecular NOTIFY specularChanged)
public:
    GLLight(QObject *parent = 0);
    void sync();

    QString name() { return m_name; }
    void setName(const QString &value);

    QVector3D diffuse() { return m_dif; }
    void setDiffuse(const QVector3D &value);

    QVector3D specular() { return m_spec; }
    void setSpecular(const QVector3D &value);

    virtual void setLight(Light *value);

signals:
    void lightChanged();
    void nameChanged();
    void diffuseChanged();
    void specularChanged();

protected:
    QVector3D m_pos;

private:
    QString m_name;
    QVector3D m_dif;
    QVector3D m_spec;
    Light *m_light;
};

class GLPointLight : public GLLight
{
    Q_OBJECT
    Q_PROPERTY(QVector3D position READ position WRITE setPosition NOTIFY positionChanged)
public:
    GLPointLight(QObject *parent = 0);

    QVector3D position() { return m_pos; }
    void setPosition(const QVector3D &value);

    virtual void setLight(Light *value);

signals:
    void positionChanged();
};

class GLDirectionalLight : public GLLight
{
    Q_OBJECT
    Q_PROPERTY(QVector3D direction READ direction WRITE setDirection NOTIFY directionChanged)
public:
    GLDirectionalLight(QObject *parent = 0);

    QVector3D direction() { return m_pos; }
    void setDirection(const QVector3D &value);

    virtual void setLight(Light *value);

signals:
    void directionChanged();
};

#endif // GLLIGHT_H
