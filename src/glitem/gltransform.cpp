#include "gltransform.h"


GLScaleBase::GLScaleBase(QObject *parent)
    : GLTransform(parent), m_origin(0, 0, 0), m_scale(1, 1, 1)
{

}

void GLScaleBase::setOrigin(const QVector3D &value)
{
    if (m_origin != value) {
        m_origin = value;
        emit originChanged();
        emit transformChanged();
    }
}

void GLScaleBase::setScale(const QVector3D &value)
{
    if (m_scale != value) {
        m_scale = value;
        emit scaleChanged();
        emit transformChanged();
    }
}

void GLScaleBase::applyTo(QMatrix4x4 *matrix) const
{
    matrix->translate(m_origin);
    matrix->scale(m_scale);
    matrix->translate(-m_origin);
}

GLScale::GLScale(QObject *parent)
    : GLScaleBase(parent)
{

}

void GLScale::setScale(const QVariant &value)
{
    QVector3D newScale;
    if (value.type() == QVariant::Vector3D) {
        newScale = value.value<QVector3D>();
    }
    else {
        bool ok = false;
        float val = value.toFloat(&ok);
        if (!ok) {
            qWarning("GLScale: scale value is not a vector3D or single floating-point value");
            return;
        }
        newScale = QVector3D(val, val, val);
    }
    if (newScale != GLScaleBase::scale()) {
        GLScaleBase::setScale(newScale);
        emit variantScaleChanged();
    }
}

GLTranslation::GLTranslation(QObject *parent)
    : GLTransform(parent), m_translate(0, 0, 0), m_progress(1)
{

}

void GLTranslation::setTranslate(const QVector3D &value)
{
    if (m_translate != value) {
        m_translate = value;
        emit translateChanged();
        emit transformChanged();
    }
}

void GLTranslation::setProgress(float value)
{
    if (m_progress != value) {
        m_progress = value;
        emit progressChanged();
        emit transformChanged();
    }
}

void GLTranslation::applyTo(QMatrix4x4 *matrix) const
{
    matrix->translate(m_translate * m_progress);
}

GLRotation::GLRotation(QObject *parent)
    : GLTransform(parent), m_origin(0, 0, 0), m_axis(0, 0, 1), m_angle(0)
{

}

void GLRotation::setOrigin(const QVector3D &value)
{
    if (m_origin != value) {
        m_origin = value;
        emit originChanged();
        emit transformChanged();
    }
}

void GLRotation::setAxis(const QVector3D &value)
{
    if (m_axis != value) {
        m_axis = value;
        emit axisChanged();
        emit transformChanged();
    }
}

void GLRotation::setAngle(float value)
{
    if (m_angle != value) {
        m_angle = value;
        emit angleChanged();
        emit transformChanged();
    }
}

void GLRotation::applyTo(QMatrix4x4 *matrix) const
{
    matrix->translate(m_origin);
    matrix->rotate(m_angle, m_axis);
    matrix->translate(-m_origin);
}


