#ifndef FLIGHT_CONTROLLER_HPP
#define FLIGHT_CONTROLLER_HPP

#include "TelemetryData.hpp"

/**
 * @class FlightController
 * @brief Simulates UAV flight logic including state machine transitions and physics.
 *
 * Processes incoming CommandPackets to drive state transitions (IDLE → ARMED → TAKEOFF →
 * FLYING → LANDING / EMERGENCY) and runs a physics engine each tick to update altitude
 * and velocity based on thrust and gravity.
 */
class FlightController{

public:

    /**
     * @brief Constructs a FlightController in the IDLE state with full battery.
     */
    FlightController();

    /**
     * @brief handle incoming commands
     */
    void handleCommand(const CommandPacket& cmd);

    /**
     * @brief updates the physics/logic
     * @param dt
     */
    void update(float dt);

    /**
     * @brief Returns the current FSM state.
     * @return Current FlightState enum value.
     */
    FlightState state() const;

    /**
     * @brief Returns altitude above ground in metres.
     * @return Altitude as a float.
     */
    float altitude() const;

    /**
     * @brief Returns vertical velocity in m/s (positive = climbing).
     * @return Velocity as a float.
     */
    float velocity() const;

    /**
     * @brief Returns remaining battery charge as a percentage (0–100).
     * @return Battery percentage as a uint8_t.
     */
    uint8_t battery() const;

    /**
     * @brief Returns current latitude in decimal degrees.
     * @return Latitude as a double.
     */
    double latitude() const;

    /**
     * @brief Returns current longitude in decimal degrees.
     * @return Longitude as a double.
     */
    double longitude() const;

    /**
     * @brief Returns roll angle in degrees (positive = right wing down).
     * @return Roll as a float.
     */
    float roll() const;

    /**
     * @brief Returns pitch angle in degrees (positive = nose up).
     * @return Pitch as a float.
     */
    float pitch() const;

    /**
     * @brief Returns yaw/heading in degrees (0–360, clockwise from north).
     * @return Yaw as a float.
     */
    float yaw() const;

    /**
     * @brief Returns battery voltage in volts (3.0V empty → 4.2V full).
     * @return Voltage as a float.
     */
    float batteryVoltage() const;

    /**
     * @brief Overrides the battery level. Intended for use in unit tests only.
     * @param pct Battery percentage to set (0–100).
     */
    void setBattery(uint8_t pct);

private:

    FlightState m_currentState;
    float m_altitude;
    float m_targetAltitude;
    uint8_t m_battery;
    float m_velocity;
    float m_batteryDrainAccum;
    double m_latitude;
    double m_longitude;
    float m_roll;
    float m_pitch;
    float m_yaw;
    static constexpr float CRUISE_HEADING = 90.0f;

    bool canArm() const;
    bool canTakeoff() const;
};


#endif //FLIGHT_CONTROLLER_HPP
