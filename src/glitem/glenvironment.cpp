#include "glenvironment.h"


GLEnvironment::GLEnvironment(QObject *parent)
    : QObject(parent), m_alpha(0.5)
{

}

void GLEnvironment::setTop(const QUrl &value)
{
    if (m_top != value) {
        m_top = value;
        emit topChanged();
    }
}

void GLEnvironment::setBottom(const QUrl &value)
{
    if (m_bottom != value) {
        m_bottom = value;
        emit bottomChanged();
    }
}

void GLEnvironment::setLeft(const QUrl &value)
{
    if (m_left != value) {
        m_left = value;
        emit leftChanged();
    }
}

void GLEnvironment::setRight(const QUrl &value)
{
    if (m_right != value) {
        m_right = value;
        emit rightChanged();
    }
}

void GLEnvironment::setFront(const QUrl &value)
{
    if (m_front != value) {
        m_front = value;
        emit frontChanged();
    }
}

void GLEnvironment::setBack(const QUrl &value)
{
    if (m_back != value) {
        m_back = value;
        emit backChanged();
    }
}

void GLEnvironment::setAlpha(qreal value)
{
    if (m_alpha != value) {
        m_alpha = value;
        emit alphaChanged();
    }
}
