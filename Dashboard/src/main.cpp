#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include "TelemetryProvider.hpp"
#include "GarudaConfig.hpp"

using namespace Qt::StringLiterals;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    GarudaConfig cfg = loadConfig();

    // CLI arg overrides config file (useful for one-off testing)
    if (argc > 1) {
        cfg.drone_ip = argv[1];
        qDebug() << "[Config] CLI override: Drone IP =" << argv[1];
    }

    qDebug() << "[Config] Drone IP:" << cfg.drone_ip.c_str()
             << "| Command port:" << cfg.command_port
             << "| Telemetry port:" << cfg.telemetry_port;

    TelemetryProvider provider;
    provider.setDroneIp(QString::fromStdString(cfg.drone_ip));
    provider.setCommandPort(cfg.command_port);
    provider.setTelemetryPort(cfg.telemetry_port);

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
