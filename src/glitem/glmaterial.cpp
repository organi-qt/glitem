#include "glmaterial.h"
#include "material.h"
#include <QDebug>


GLMaterial::GLMaterial(QObject *parent)
    : QObject(parent)
{

}

bool GLMaterial::urlToPath(const QUrl &url, QString &path)
{
    if (url.scheme() == "file")
        path = url.toLocalFile();
    else if (url.scheme() == "qrc")
        path = ':' + url.path();
    else
        return false;
    return true;
}

GLBasicMaterial::GLBasicMaterial(QObject *parent)
    : GLMaterial(parent)
{

}

void GLBasicMaterial::setMap(const QUrl &value)
{
    if (m_map != value) {
        m_map = value;
        emit mapChanged();
    }
}

Material *GLBasicMaterial::material()
{
    if (!m_material) {
        m_material = new BasicMaterial();
        QString path;
        if (urlToPath(m_map, path))
            m_material->loadTexture(path, QOpenGLTexture::ClampToEdge);
    }
    return m_material;
}

GLPhongMaterial::GLPhongMaterial(QObject *parent)
    : GLMaterial(parent), m_color(255, 255, 255), m_specular(17, 17, 17),
      m_shininess(30), m_env_map(false), m_reflectivity(1), m_material(0)
{

}

void GLPhongMaterial::setColor(const QColor &value)
{
    if (m_color != value) {
        m_color = value;
        emit colorChanged();
    }
}

void GLPhongMaterial::setSpecular(const QColor &value)
{
    if (m_specular != value) {
        m_specular = value;
        emit specularChanged();
    }
}

void GLPhongMaterial::setShininess(qreal value)
{
    if (m_shininess != value) {
        m_shininess = value;
        emit shininessChanged();
    }
}

void GLPhongMaterial::setMap(const QUrl &value)
{
    if (m_map != value) {
        m_map = value;
        emit mapChanged();
    }
}

void GLPhongMaterial::setSpecularMap(const QUrl &value)
{
    if (m_specular_map != value) {
        m_specular_map = value;
        emit specularMapChanged();
    }
}

void GLPhongMaterial::setEnvMap(bool value)
{
    if (m_env_map != value) {
        m_env_map = value;
        emit envMapChanged();
    }
}

void GLPhongMaterial::setReflectivity(qreal value)
{
    if (m_reflectivity != value) {
        m_reflectivity = value;
        emit reflectivityChanged();
    }
}

Material *GLPhongMaterial::material()
{
    if (!m_material) {
        m_material = new PhongMaterial();
        m_material->setMaterial(
                    QVector3D(),
                    QVector3D(m_color.redF(), m_color.greenF(), m_color.blueF()),
                    QVector3D(m_specular.redF(), m_specular.greenF(), m_specular.blueF()),
                    m_shininess
                    );

        if (m_env_map)
            m_material->setEnvMap(m_reflectivity);

        QString path;
        if (urlToPath(m_map, path))
            m_material->loadDiffuseTexture(path, QOpenGLTexture::ClampToEdge);
        if (urlToPath(m_specular_map, path))
            m_material->loadSpecularTexture(path, QOpenGLTexture::ClampToEdge);
    }
    return m_material;
}
