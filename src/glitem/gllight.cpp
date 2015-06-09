#include "gllight.h"
#include "glshader.h"
#include "renderstate.h"

GLLight::GLLight(QObject *parent)
    : QObject(parent),
      m_pos_dirty(true), m_amb_dirty(true),
      m_dif_dirty(true), m_spec_dirty(true),
      m_state_index(-1)
{
}

void GLLight::setName(const QString &value)
{
    if (m_name != value) {
        m_name = value;
        emit nameChanged();
    }
}

void GLLight::setPos(const QVector3D &value)
{
    if (m_pos != value) {
        m_pos = value;
        m_pos_dirty = true;
        emit posChanged();
        emit lightChanged();
    }
}

void GLLight::setAmb(const QVector3D &value)
{
    if (m_amb != value) {
        m_amb = value;
        m_amb_dirty = true;
        emit ambChanged();
        emit lightChanged();
    }
}

void GLLight::setDif(const QVector3D &value)
{
    if (m_dif != value) {
        m_dif = value;
        m_dif_dirty = true;
        emit difChanged();
        emit lightChanged();
    }
}

void GLLight::setSpec(const QVector3D &value)
{
    if (m_spec != value) {
        m_spec = value;
        m_spec_dirty = true;
        emit specChanged();
        emit lightChanged();
    }
}

void GLLight::updateState(RenderState *state)
{
    if (m_state_index == -1) {
        // first time
        for (int i = 0; i < state->lights.size(); i++) {
            if (state->lights[i].light->name == m_name) {
                m_state_index = i;
                break;
            }
        }
        // not found
        if (m_state_index == -1) {
            qWarning() << "no light found in model with name: " << m_name;
            m_state_index = -2;
        }
    }

    if (m_state_index < -1)
        return;

    if (m_pos_dirty)
        state->setLightPos(m_state_index, m_pos);

    if (m_amb_dirty)
        state->setLightAmb(m_state_index, m_amb);

    if (m_dif_dirty)
        state->setLightDif(m_state_index, m_dif);

    if (m_spec_dirty)
        state->setLightSpec(m_state_index, m_spec);
}
