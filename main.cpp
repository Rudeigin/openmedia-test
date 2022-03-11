#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "filemanager.h"

int main(int argc, char *argv[]) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setOrganizationName("AllaCorporation");

    QGuiApplication app(argc, argv);

    FileManager * manager = new FileManager(&app);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("FileManager", manager);
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
