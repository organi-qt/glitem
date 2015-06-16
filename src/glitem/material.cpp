#include "material.h"
#include "glshader.h"
#include <QHash>


Material::ShaderMap Material::m_shaders;

bool Material::init(const QList<Light *> *, bool)
{
    if (m_shader) {
        if (m_transparent)
            m_shader->hasTransparency();
        else
            m_shader->hasOpaque();
        return true;
    }
    return false;
}

BasicMaterial::BasicMaterial()
    : Material(), m_texture_image(0), m_texture(0)
{

}

BasicMaterial::~BasicMaterial()
{
    if (m_texture_image)
        delete m_texture_image;
    if (m_texture)
        delete m_texture;
}

bool BasicMaterial::loadTexture(const QString &path, QOpenGLTexture::WrapMode mode)
{
    QImage image;
    if (image.load(path)) {
        m_texture_image = new QImage(image.mirrored());
        m_texture_mode = mode;
        return true;
    }
    return false;
}

bool BasicMaterial::init(const QList<Light *> *a1, bool a2)
{
    if (m_texture_image) {
        m_texture = new QOpenGLTexture(*m_texture_image);
        m_texture->setMinificationFilter(QOpenGLTexture::Linear);
        m_texture->setMagnificationFilter(QOpenGLTexture::Linear);
        m_texture->setWrapMode(m_texture_mode);

        delete m_texture_image;
        m_texture_image = 0;
    }

    uint key = 0x8000;
    if (m_texture)
        key |= 0x0100;

    bool ret;
    ShaderMap::iterator it = m_shaders.find(key);
    if (it == m_shaders.end()) {
        m_shader = new GLBasicShader(m_texture != NULL);
        m_shaders.insert(key, m_shader);
        ret = true;
    }
    else {
        m_shader = it.value();
        ret = false;
    }

    Material::init(a1, a2);
    return ret;
}

PhongMaterial::PhongMaterial()
    : Material(), m_env_map(false),
      m_diffuse_texture_image(0), m_specular_texture_image(0),
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

    uint key = 0x80;
    if (m_diffuse_texture)
        key |= 0x01;
    if (m_specular_texture)
        key |= 0x02;
    if (m_env_map && has_env_map)
        key |= 0x04;

    bool ret;
    ShaderMap::iterator it = m_shaders.find(key);
    if (it == m_shaders.end()) {
        m_shader = new GLPhongShader(lights,
                                     m_diffuse_texture != NULL,
                                     m_specular_texture != NULL,
                                     m_env_map && has_env_map);
        m_shaders.insert(key, m_shader);
        ret = true;
    }
    else {
        m_shader = it.value();
        ret = false;
    }

    Material::init(lights, has_env_map);
    return ret;
}
