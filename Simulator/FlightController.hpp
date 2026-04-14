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
    FlightState getState() const {return m_currentState;}

    /**
     * @brief getAltitude
     * @return
     */
    float getAltitude() const {return m_altitude;}

private:

    FlightState m_currentState;
    float m_altitude;
    float m_targetAltitude;
    float m_battery;

    bool canArm() const;
    bool canTakeoff() const;
};


#endif //FLIGHT_CONTROLLER_HPP
