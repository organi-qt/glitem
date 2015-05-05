#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSurfaceFormat>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
//*
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);
//*/
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("main.qml")));

    return app.exec();
}
