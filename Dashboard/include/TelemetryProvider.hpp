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
 *
 * Runs a background worker thread to listen for TelemetryPacket structures over UDP
 * and uses a mutex to prevent race conditions during UI updates. Exposes telemetry
 * values as QML-bindable properties and provides sendCommand() for GCS control input.
 */
class TelemetryProvider : public QObject
{
    Q_OBJECT

    /**
     * @property TelemetryProvider::altitude
     * @brief Altitude of the drone above ground in metres, updated at 10Hz.
     */
    Q_PROPERTY(float altitude READ altitude NOTIFY altitudeChanged)

    /**
     * @property TelemetryProvider::velocity
     * @brief Vertical velocity of the drone in m/s, updated at 10Hz.
     */
    Q_PROPERTY(float velocity READ velocity NOTIFY velocityChanged)

    /**
     * @property TelemetryProvider::battery
     * @brief Remaining battery charge as a percentage (0–100), updated at 10Hz.
     */
    Q_PROPERTY(int battery READ battery NOTIFY batteryChanged)

    /**
     * @property TelemetryProvider::flightState
     * @brief Current flight state machine value as an integer (maps to FlightState enum).
     */
    Q_PROPERTY(int flightState READ flightState NOTIFY flightStateChanged)

    /**
     * @property TelemetryProvider::latitude
     * @brief Current latitude value as degrees.
     */
    Q_PROPERTY(float latitude READ latitude NOTIFY latitudeChanged)

    /**
     * @property TelemetryProvider::longitude
     * @brief Current longitude value as degrees.
     */
    Q_PROPERTY(float longitude READ longitude NOTIFY longitudeChanged)

public:
    /**
     * @brief Constructs the TelemetryProvider and initialises internal state.
     * @param parent Optional Qt parent object.
     */
    explicit TelemetryProvider(QObject *parent = nullptr);
    ~TelemetryProvider();

    /**
     * @brief Returns the current altitude of the UAV in metres.
     * @return Current altitude as a float.
     */
    float altitude() const { return m_data.altitude; }

    /**
     * @brief Returns the current vertical velocity of the UAV in m/s.
     * @return Current velocity as a float.
     */
    float velocity() const { return m_data.velocity; }

    /**
     * @brief Returns the current battery level as a percentage.
     * @return Battery percentage as an int (0–100).
     */
    int battery() const { return m_data.battery_pct; }

    /**
     * @brief Returns the current latitude in degrees.
     * @return latitude in degrees.
     */
    float latitude() const { return m_data.latitude; }

    /**
     * @brief Returns the current longitude in degrees.
     * @return longitude in degrees.
     */
    float longitude() const { return m_data.longitude; }

    /**
     * @brief Sends a command packet to the Simulator over UDP.
     * @param type CommandType value cast to int (e.g. 1 = ARM, 99 = EMERGENCY_STOP).
     * @param param Optional float parameter (e.g. target altitude for TAKEOFF).
     * @see CommandPacket
     */
    Q_INVOKABLE void sendCommand(int type, float param = 0.0f);

    /**
     * @brief Sets the IP address of the Simulator (drone) for outgoing command packets.
     * @param ip IP address string (e.g. "127.0.0.1").
     */
    void setDroneIp(const QString& ip)  { m_droneIp = ip.toStdString(); }

    /**
     * @brief Sets the UDP port the Simulator listens on for commands.
     * @param port Port number.
     */
    void setCommandPort(uint16_t port)  { m_commandPort = port; }

    /**
     * @brief Sets the UDP port this provider listens on for incoming telemetry.
     * @param port Port number.
     */
    void setTelemetryPort(uint16_t port){ m_telemetryPort = port; }

    /**
     * @brief Starts the background UDP receiver thread.
     */
    void start();

    /**
     * @brief Stops the background UDP receiver thread and joins it.
     */
    void stop();

    /**
     * @brief Returns the current flight state as an integer.
     * @return Integer representation of the FlightState enum value.
     */
    int flightState() const;

signals:
    /**
     * @brief Emitted when the altitude value changes in the latest telemetry packet.
     */
    void altitudeChanged();

    /**
     * @brief Emitted when the velocity value changes in the latest telemetry packet.
     */
    void velocityChanged();

    /**
     * @brief Emitted when the battery level changes in the latest telemetry packet.
     */
    void batteryChanged();

    /**
     * @brief Emitted when the flight state changes in the latest telemetry packet.
     */
    void flightStateChanged();

    /**
     * @brief Emitted when latitude changes in the latest telemetry packet
     */
    void latitudeChanged();

    /**
     * @brief Emitted when longitude changes in the latest telemetry packet
     */
    void longitudeChanged();

private:

    /**
     * @brief Entry point for the background worker thread. Blocks on UDP recvfrom.
     */
    void runReceiver();

    TelemetryPacket      m_data;
    std::mutex           m_mutex;
    std::atomic<bool>    m_running{false};
    std::thread          m_workerThread;
    int                  m_flightState;
    std::string          m_droneIp{"127.0.0.1"};
    uint16_t             m_commandPort{5000};
    uint16_t             m_telemetryPort{5001};
};

#endif // TELEMETRYPROVIDER_HPP
