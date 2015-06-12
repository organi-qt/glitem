#ifndef GLLIGHT_H
#define GLLIGHT_H

#include <QObject>
#include <QVector3D>
#include <QString>

class RenderState;

class GLLight : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QVector3D pos READ pos WRITE setPos NOTIFY posChanged)
    Q_PROPERTY(QVector3D dif READ dif WRITE setDif NOTIFY difChanged)
    Q_PROPERTY(QVector3D spec READ spec WRITE setSpec NOTIFY specChanged)
public:
    GLLight(QObject *parent = 0);
    void updateState(RenderState *);

    QString name() { return m_name; }
    void setName(const QString &value);

    QVector3D pos() { return m_pos; }
    void setPos(const QVector3D &value);

    QVector3D dif() { return m_dif; }
    void setDif(const QVector3D &value);

    QVector3D spec() { return m_spec; }
    void setSpec(const QVector3D &value);

signals:
    void lightChanged();
    void nameChanged();
    void posChanged();
    void difChanged();
    void specChanged();

private:
    QString m_name;
    QVector3D m_pos;
    QVector3D m_dif;
    QVector3D m_spec;
    bool m_pos_dirty;
    bool m_dif_dirty;
    bool m_spec_dirty;
    int m_state_index;
};

#endif // GLLIGHT_H
