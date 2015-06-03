#ifndef GLENVIRONMENT_H
#define GLENVIRONMENT_H

#include <QObject>
#include <QImage>
#include <QUrl>

struct EnvParam {
    QImage top;
    QImage bottom;
    QImage left;
    QImage right;
    QImage front;
    QImage back;
    float alpha;
    int width;
    int height;
};

class GLEnvironment : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl top READ top WRITE setTop NOTIFY topChanged)
    Q_PROPERTY(QUrl bottom READ bottom WRITE setBottom NOTIFY bottomChanged)
    Q_PROPERTY(QUrl left READ left WRITE setLeft NOTIFY leftChanged)
    Q_PROPERTY(QUrl right READ right WRITE setRight NOTIFY rightChanged)
    Q_PROPERTY(QUrl front READ front WRITE setFront NOTIFY frontChanged)
    Q_PROPERTY(QUrl back READ back WRITE setBack NOTIFY backChanged)
    Q_PROPERTY(qreal alpha READ alpha WRITE setAlpha NOTIFY alphaChanged)
public:
    GLEnvironment(QObject *parent = 0);

    QUrl top() const { return m_top; }
    QUrl bottom() const { return m_bottom; }
    QUrl left() const { return m_left; }
    QUrl right() const { return m_right; }
    QUrl front() const { return m_front; }
    QUrl back() const { return m_back; }
    qreal alpha() const { return m_alpha; }

    void setTop(const QUrl &value);
    void setBottom(const QUrl &value);
    void setLeft(const QUrl &value);
    void setRight(const QUrl &value);
    void setFront(const QUrl &value);
    void setBack(const QUrl &value);
    void setAlpha(qreal value);

signals:
    void topChanged();
    void bottomChanged();
    void leftChanged();
    void rightChanged();
    void frontChanged();
    void backChanged();
    void alphaChanged();

private:
    QUrl m_top;
    QUrl m_bottom;
    QUrl m_left;
    QUrl m_right;
    QUrl m_front;
    QUrl m_back;
    qreal m_alpha;
};

#endif // GLENVIRONMENT_H
