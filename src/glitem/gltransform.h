#ifndef GLTRANSFORM_H
#define GLTRANSFORM_H

#include <QObject>
#include <QMatrix4x4>


class GLTransform : public QObject
{
    Q_OBJECT
public:
    GLTransform(QObject *parent = 0) : QObject(parent) {}

    virtual void applyTo(QMatrix4x4 *matrix) const = 0;

signals:
    void transformChanged();
};

class GLScaleBase : public GLTransform
{
    Q_OBJECT
    Q_PROPERTY(QVector3D origin READ origin WRITE setOrigin NOTIFY originChanged)
    Q_PROPERTY(QVector3D scale READ scale WRITE setScale NOTIFY scaleChanged)
public:
    GLScaleBase(QObject *parent = 0);

    QVector3D origin() const { return m_origin; }
    void setOrigin(const QVector3D &value);

    QVector3D scale() const { return m_scale; }
    void setScale(const QVector3D &value);

    void applyTo(QMatrix4x4 *matrix) const;

signals:
    void originChanged();
    void scaleChanged();

private:
    QVector3D m_origin;
    QVector3D m_scale;
};

class GLScale : public GLScaleBase
{
    Q_OBJECT
    Q_PROPERTY(QVariant scale READ scale WRITE setScale NOTIFY variantScaleChanged)
public:
    GLScale(QObject *parent = 0);

    QVariant scale() const { return GLScaleBase::scale(); }
    void setScale(const QVariant &value);

signals:
    void variantScaleChanged();
};

class GLTranslation : public GLTransform
{
    Q_OBJECT
    Q_PROPERTY(QVector3D translate READ translate WRITE setTranslate NOTIFY translateChanged)
    Q_PROPERTY(float progress READ progress WRITE setProgress NOTIFY progressChanged)
public:
    GLTranslation(QObject *parent = 0);

    QVector3D translate() const { return m_translate; }
    void setTranslate(const QVector3D &value);

    float progress() const { return m_progress; }
    void setProgress(float value);

    void applyTo(QMatrix4x4 *matrix) const;

signals:
    void translateChanged();
    void progressChanged();

private:
    QVector3D m_translate;
    float m_progress;
};

class GLRotation : public GLTransform
{
    Q_OBJECT
    Q_PROPERTY(QVector3D origin READ origin WRITE setOrigin NOTIFY originChanged)
    Q_PROPERTY(float angle READ angle WRITE setAngle NOTIFY angleChanged)
    Q_PROPERTY(QVector3D axis READ axis WRITE setAxis NOTIFY axisChanged)
public:
    GLRotation(QObject *parent = 0);

    QVector3D origin() const { return m_origin; }
    void setOrigin(const QVector3D &value);

    float angle() const { return m_angle; }
    void setAngle(float value);

    QVector3D axis() const { return m_axis; }
    void setAxis(const QVector3D &value);

    void applyTo(QMatrix4x4 *matrix) const;

signals:
    void originChanged();
    void angleChanged();
    void axisChanged();

private:
    QVector3D m_origin;
    QVector3D m_axis;
    float m_angle;
};

#endif // GLTRANSFORM_H
