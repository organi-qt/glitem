#ifndef LIGHT
#define LIGHT

#include <QString>
#include <QVector3D>

class GLTransformNode;

struct Light {
    enum { POINT, DIRECTIONAL, SPOT } type;
    QString name;
    QVector3D pos;
    QVector3D dif;
    QVector3D spec;
    GLTransformNode *node;
};

#endif // LIGHT

