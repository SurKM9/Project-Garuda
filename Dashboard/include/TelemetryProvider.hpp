#ifndef TELEMETRYPROVIDER_HPP
#define TELEMETRYPROVIDER_HPP

/**
 * @file TelemetryProvider.hpp
 * @brief Handles UDP networking and bridges telemetry data to the QML UI.
 */

#include <QObject>
#include <QThread>
#include <atomic>
#include <mutex>
#include "TelemetryData.hpp"

/**
 * @class TelemetryProvider
 * @brief A thread-safe provider that manages a UDP receiver and emits Qt signals.
 * * This class runs a background worker thread to listen for TelemetryPacket
 * structures and uses a mutex to prevent race conditions during UI updates.
 */
class TelemetryProvider : public QObject
{
    Q_OBJECT
    // These properties make C++ variables visible to QML
    /**
     * @property TelemetryProvider::altitude
     * @brief The altitude of the drone above sea level, updated at 10Hz.
     */
    Q_PROPERTY(float altitude READ altitude NOTIFY altitudeChanged)
    /**
     * @property TelemetryProvider::velocity
     * @brief The velocity of the drone above sea level, updated at 10Hz.
     */
    Q_PROPERTY(float velocity READ velocity NOTIFY velocityChanged)
    /**
     * @property TelemetryProvider::battery
     * @brief The altitude of the drone above sea level, updated at 10Hz.
     */
    Q_PROPERTY(int battery READ battery NOTIFY batteryChanged)

    /**
     * @property Telemetry::flightState
     * @brief current state of the flight
     */
    Q_PROPERTY(int flightState READ flightState NOTIFY flightStateChanged)

public:
    explicit TelemetryProvider(QObject *parent = nullptr);
    ~TelemetryProvider();

    // Getters for QML

    /**
     * @brief Returns the current altitude of the UAV in meters.
     * @return Current altitude as a float.
     */
    float altitude() const { return m_data.altitude; }

    /**
     * @brief velocity
     * @return
     */
    float velocity() const { return m_data.velocity; }

    /**
     * @brief battery
     * @return
     */
    int battery() const { return m_data.battery_pct; }

    /**
     * @brief Sends a command to the UAV to begin landing procedures.
     * @see CommandPacket
     */
    Q_INVOKABLE void sendCommand(int type, float param = 0.0f);

    /**
     * @brief Initiates the background networking thread.
     */
    void start(); // Starts the background networking

    /**
     * @brief stop
     */
    void stop();

    /**
     * @brief flightState
     * @return current state
     */
    int flightState() const;

signals:
    // These signals tell QML to redraw the screen

    /**
     * @brief Signal emitted whenever new telemetry data is processed.
     * This triggers the QML UI to refresh its displays and charts.
     */
    void altitudeChanged();
    void velocityChanged();
    void batteryChanged();
    void flightStateChanged();

private:

    /**
     * @brief runReceiver
     */
    void runReceiver(); // The function that runs in the thread

    /**
     * @brief m_data
     */
    TelemetryPacket m_data;
    std::mutex m_mutex;
    std::atomic<bool> m_running{false};
    std::thread m_workerThread;
    int m_flightState;
};

#endif // TELEMETRYPROVIDER_HPP
