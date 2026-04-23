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
     * @brief getState
     * @return
     */
    FlightState getState() const;

    /**
     * @brief getAltitude
     * @return
     */
    float getAltitude() const;

    /**
     * @brief getVelocity
     * @return
     */
    float getVelocity() const;

    /**
     * @brief Returns the current battery level as a percentage (0–100).
     */
    uint8_t getBattery() const {return m_battery;}

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

    bool canArm() const;
    bool canTakeoff() const;
};


#endif //FLIGHT_CONTROLLER_HPP
