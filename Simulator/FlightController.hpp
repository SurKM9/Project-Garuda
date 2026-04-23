#ifndef FLIGHT_CONTROLLER_HPP
#define FLIGHT_CONTROLLER_HPP

#include "TelemetryData.hpp"

class FlightController{

public:

    /**
     * @brief FlightController
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

    uint8_t getBattery() const {return m_battery;}

    /**
     * @brief setBattery
     * @param pct
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
