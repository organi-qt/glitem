#ifndef MATERIAL_H
#define MATERIAL_H

#include "glshader.h"
#include <QVector3D>

class QOpenGLTexture;

class Material
{
public:
    virtual GLShader::ShaderType type() const = 0;
    const QString &name() const { return m_name; }
    void setName(const QString &nname) { m_name = nname; }

private:
    QString m_name;
};

class PhongMaterial : public Material {
public:
    virtual GLShader::ShaderType type() const { return m_type; }
    void setType(GLShader::ShaderType ntype) { m_type = ntype; }

    void setMaterial(const QVector3D &nka, const QVector3D &nkd, const QVector3D &nks,
                     float nalpha) {
        m_ka = nka; m_kd = nkd; m_ks = nks; m_alpha = nalpha;
    }

    void setDiffuseTexturePath(const QString &path) { m_diffuse_texture_path = path; }
    void setSpecularTexturePath(const QString &path) { m_specular_texture_path = path; }

    const QString &diffuseTexturePath() { return m_diffuse_texture_path; }
    const QString &specularTexturePath() { return m_specular_texture_path; }

    void setDiffuseTexture(QOpenGLTexture *texture) { m_diffuse_texture = texture; }
    void setSpecularTexture(QOpenGLTexture *texture) { m_specular_texture = texture; }

    QOpenGLTexture *diffuseTexture() { return m_diffuse_texture; }
    QOpenGLTexture *specularTexture() { return m_specular_texture; }

    QVector3D &ka() { return m_ka; }
    QVector3D &kd() { return m_kd; }
    QVector3D &ks() { return m_ks; }
    float &alpha() { return m_alpha; }

private:
    GLShader::ShaderType m_type;

    QVector3D m_ka;
    QVector3D m_kd;
    QVector3D m_ks;
    float m_alpha;

    QString m_diffuse_texture_path;
    QString m_specular_texture_path;
    QOpenGLTexture *m_diffuse_texture;
    QOpenGLTexture *m_specular_texture;
};

#endif // MATERIAL_H

