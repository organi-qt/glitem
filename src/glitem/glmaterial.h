#ifndef GLMATERIAL_H
#define GLMATERIAL_H

#include <QObject>

class GLMaterial : public QObject
{
    Q_OBJECT
public:
    GLMaterial(QObject *parent = 0);
};

class GLPhongMaterial : public GLMaterial
{
    Q_OBJECT
public:
    GLPhongMaterial(QObject *parent = 0);
};

#endif // GLMATERIAL_H
