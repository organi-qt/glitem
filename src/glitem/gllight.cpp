#include "gllight.h"
#include "glshader.h"
#include "renderstate.h"

GLLight::GLLight(QObject *parent)
    : QObject(parent), m_light(0)
{
}

void GLLight::setName(const QString &value)
{
    if (m_name != value) {
        m_name = value;
        emit nameChanged();
    }
}

void GLLight::setDiffuse(const QVector3D &value)
{
    if (m_dif != value) {
        m_dif = value;
        emit diffuseChanged();
        emit lightChanged();
    }
}

void GLLight::setLight(Light *value)
{
    value->pos = m_pos;
    value->dif = m_dif;
    value->spec = m_spec;
}

void GLLight::setSpecular(const QVector3D &value)
{
    if (m_spec != value) {
        m_spec = value;
        emit specularChanged();
        emit lightChanged();
    }
}

void GLLight::sync()
{
    if (!m_light)
        return;

    m_light->pos = m_pos;

    if (m_light->dif != m_dif) {
        m_light->dif = m_dif;
        m_light->dif_dirty = true;
    }

    if (m_light->spec != m_spec) {
        m_light->spec = m_spec;
        m_light->spec_dirty = true;
    }
}

GLPointLight::GLPointLight(QObject *parent)
    : GLLight(parent)
{

}

void GLPointLight::setPosition(const QVector3D &value)
{
    if (m_pos != value) {
        m_pos = value;
        emit positionChanged();
        emit lightChanged();
    }
}

void GLPointLight::setLight(Light *value)
{
    GLLight::setLight(value);
    value->type = Light::POINT;
}

GLDirectionalLight::GLDirectionalLight(QObject *parent)
    : GLLight(parent)
{

}

void GLDirectionalLight::setDirection(const QVector3D &value)
{
    if (m_pos != value) {
        m_pos = value;
        emit directionChanged();
        emit lightChanged();
    }
}

void GLDirectionalLight::setLight(Light *value)
{
    GLLight::setLight(value);
    value->type = Light::DIRECTIONAL;
}

