#ifndef MATERIAL_H
#define MATERIAL_H

#include <QVector3D>
#include <QOpenGLTexture>

class QImage;
class GLShader;
class Light;

class Material
{
public:
    Material() : m_shader(0) {}
    virtual ~Material() {}

    GLShader *shader() { return m_shader; }

    const QString &name() const { return m_name; }
    void setName(const QString &value) { m_name = value; }

    virtual bool init(const QList<Light *> *lights, bool has_env_map) = 0;

protected:
    typedef QHash<uint, GLShader *> ShaderMap;
    static ShaderMap m_shaders;

    GLShader *m_shader;

private:
    QString m_name;
};

class PhongMaterial : public Material {
public:
    PhongMaterial();
    virtual ~PhongMaterial();

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

    void setEnvMap(float reflectivity) {
        m_env_alpha = reflectivity;
        m_env_map = true;
    }

    float env_alpha() { return m_env_alpha; }

    virtual bool init(const QList<Light *> *lights, bool has_env_map);

private:
    QVector3D m_ka;
    QVector3D m_kd;
    QVector3D m_ks;
    float m_alpha;

    bool m_env_map;
    float m_env_alpha;

    QImage *m_diffuse_texture_image;
    QImage *m_specular_texture_image;
    QOpenGLTexture::WrapMode m_diffuse_texture_mode;
    QOpenGLTexture::WrapMode m_specular_texture_mode;

    QOpenGLTexture *m_diffuse_texture;
    QOpenGLTexture *m_specular_texture;
};

#endif // MATERIAL_H

