#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <audiobackend.h>
#include <QQuickStyle>
#include <version.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    app.setApplicationVersion(VERSION_STRING);

    AudioBackend *audioBackend = new AudioBackend(nullptr);

    QQuickStyle::setStyle("Fusion");

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("audioBackend", audioBackend);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("QMLWalkieTalkie", "Main");

    return app.exec();
}
