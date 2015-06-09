#ifndef GLJSONLOADMODEL_H
#define GLJSONLOADMODEL_H

#include "glmodel.h"
#include <QUrl>

class GLJSONLoadModel : public GLModel
{
    Q_OBJECT
    Q_PROPERTY(QUrl file READ file WRITE setFile NOTIFY fileChanged)
public:
    GLJSONLoadModel(QObject *parent = 0);

    QUrl file() { return m_file; }
    void setFile(const QUrl &value);

    virtual bool load();

signals:
    void fileChanged();

private:
    QUrl m_file;

    void appendVector(QList<float> &vector, QVector2D &value);
    void appendVector(QList<float> &vector, QVector3D &value);
};

#endif // GLJSONLOADMODEL_H
