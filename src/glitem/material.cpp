#include "material.h"
#include "glshader.h"
#include <QHash>


Material::ShaderMap Material::m_shaders;

PhongMaterial::PhongMaterial()
    : m_env_map(false), m_diffuse_texture_image(0), m_specular_texture_image(0),
      m_diffuse_texture(0), m_specular_texture(0)
{

}

PhongMaterial::~PhongMaterial()
{
    if (m_diffuse_texture_image)
        delete m_diffuse_texture_image;
    if (m_specular_texture_image)
        delete m_specular_texture_image;
    if (m_diffuse_texture)
        delete m_diffuse_texture;
    if (m_specular_texture)
        delete m_specular_texture;
}

bool PhongMaterial::loadDiffuseTexture(const QString &path, QOpenGLTexture::WrapMode mode)
{
    QImage image;
    if (image.load(path)) {
        m_diffuse_texture_image = new QImage(image.mirrored());
        m_diffuse_texture_mode = mode;
        return true;
    }
    return false;
}

bool PhongMaterial::loadSpecularTexture(const QString &path, QOpenGLTexture::WrapMode mode)
{
    QImage image;
    if (image.load(path)) {
        m_specular_texture_image = new QImage(image.mirrored());
        m_specular_texture_mode = mode;
        return true;
    }
    return false;
}

bool PhongMaterial::init(const QList<Light *> *lights, bool has_env_map)
{
    if (m_diffuse_texture_image) {
        m_diffuse_texture = new QOpenGLTexture(*m_diffuse_texture_image);
        m_diffuse_texture->setMinificationFilter(QOpenGLTexture::Linear);
        m_diffuse_texture->setMagnificationFilter(QOpenGLTexture::Linear);
        m_diffuse_texture->setWrapMode(m_diffuse_texture_mode);

        delete m_diffuse_texture_image;
        m_diffuse_texture_image = 0;
    }

    if (m_specular_texture_image) {
        m_specular_texture = new QOpenGLTexture(*m_specular_texture_image);
        m_specular_texture->setMinificationFilter(QOpenGLTexture::Linear);
        m_specular_texture->setMagnificationFilter(QOpenGLTexture::Linear);
        m_specular_texture->setWrapMode(m_specular_texture_mode);

        delete m_specular_texture_image;
        m_specular_texture_image = 0;
    }

    uint key = 0;
    if (m_diffuse_texture)
        key |= 0xff;
    if (m_specular_texture)
        key |= 0xff00;
    if (m_env_map && has_env_map)
        key |= 0xff0000;

    ShaderMap::iterator it = m_shaders.find(key);
    if (it == m_shaders.end()) {
        m_shader = new GLPhongShader(lights,
                                     m_diffuse_texture != NULL,
                                     m_specular_texture != NULL,
                                     m_env_map && has_env_map);
        m_shaders.insert(key, m_shader);
        return true;
    }
    else {
        m_shader = it.value();
        return false;
    }
}
