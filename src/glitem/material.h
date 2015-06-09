#ifndef MATERIAL_H
#define MATERIAL_H

#include "glshader.h"
#include <QVector3D>
#include <QOpenGLTexture>

class QImage;

class Material
{
public:
    Material() {}
    virtual ~Material() {}

    virtual GLShader::ShaderType type() const = 0;

    const QString &name() const { return m_name; }
    void setName(const QString &value) { m_name = value; }

    virtual void init() {}

private:
    QString m_name;
};

class PhongMaterial : public Material {
public:
    PhongMaterial();
    virtual ~PhongMaterial();

    virtual GLShader::ShaderType type() const { return m_type; }
    void setType(GLShader::ShaderType ntype) { m_type = ntype; }

    void setMaterial(const QVector3D &nka, const QVector3D &nkd, const QVector3D &nks,
                     float nalpha) {
        m_ka = nka; m_kd = nkd; m_ks = nks; m_alpha = nalpha;
    }

    bool loadDiffuseTexture(const QString &path, QOpenGLTexture::WrapMode mode);
    bool loadSpecularTexture(const QString &path, QOpenGLTexture::WrapMode mode);

    QOpenGLTexture *diffuseTexture() { return m_diffuse_texture; }
    QOpenGLTexture *specularTexture() { return m_specular_texture; }

    QVector3D &ka() { return m_ka; }
    QVector3D &kd() { return m_kd; }
    QVector3D &ks() { return m_ks; }
    float &alpha() { return m_alpha; }

    virtual void init();

private:
    GLShader::ShaderType m_type;

    QVector3D m_ka;
    QVector3D m_kd;
    QVector3D m_ks;
    float m_alpha;

    QImage *m_diffuse_texture_image;
    QImage *m_specular_texture_image;
    QOpenGLTexture::WrapMode m_diffuse_texture_mode;
    QOpenGLTexture::WrapMode m_specular_texture_mode;

    QOpenGLTexture *m_diffuse_texture;
    QOpenGLTexture *m_specular_texture;
};

#endif // MATERIAL_H

