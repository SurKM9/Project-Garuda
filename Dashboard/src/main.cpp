#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "TelemetryProvider.hpp"

using namespace Qt::StringLiterals;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    TelemetryProvider provider;
    provider.start();

    {
        // We put the engine in its own scope {}
        QQmlApplicationEngine engine;
        engine.rootContext()->setContextProperty(u"telemetry"_s, &provider);

        const QUrl url(u"qrc:/UAV_Dashboard/qml/Main.qml"_s);
        engine.load(url);

        app.exec();
        // When the window is closed, execution hits this line.
        // The engine is still alive here, but we are about to exit the scope.
    }

    // Explicitly stop the thread BEFORE the provider object leaves the stack
    provider.stop();

    return 0;
}
