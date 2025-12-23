#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <audiobackend.h>
#include <QQuickStyle>

int main(int argc, char *argv[])
{
    AudioBackend *audioBackend = new AudioBackend(nullptr);
    QGuiApplication app(argc, argv);

    QQuickStyle::setStyle("Material");

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
