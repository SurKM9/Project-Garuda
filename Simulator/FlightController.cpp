#include <iostream>
#include "FlightController.hpp"


FlightController::FlightController() :
    m_currentState(FlightState::IDLE),
    m_altitude(0.0f),
    m_targetAltitude(0.0f),
    m_battery(100.0f)
{
}

void FlightController::handleCommand(const CommandPacket &cmd)
{
    switch(cmd.type){

    case CommandType::ARM:
        if(canArm())
        {
            m_currentState = FlightState::ARMED;
            std::cout << "[FlightController] System ARMED\n";
        }

        break;

    case CommandType::TAKEOFF:
        if(canTakeoff()){

            m_currentState = FlightState::TAKEOFF;
            m_targetAltitude = cmd.param1;
            std::cout << "[FlightController] Taking off to " << m_targetAltitude << "m\n";
        }

    case CommandType::NONE:
        break;
    case CommandType::DISARM:
        break;
    case CommandType::LAND:
        break;
    case CommandType::EMERGENCY_STOP:
        m_currentState = FlightState::EMERGENCY;
        std::cout << "[FlightController] EMERGENCY STOP TRIGGERED!\n";
        break;

    default:
        break;
    }
}

void FlightController::update(float dt)
{
    if (m_currentState == FlightState::TAKEOFF) {
        if (m_altitude < m_targetAltitude) {
            m_altitude += 0.5f * dt; // Simple linear climb
        } else {
            m_currentState = FlightState::FLYING;
        }
    }
}

bool FlightController::canArm() const
{
    return (m_currentState == FlightState::IDLE && m_battery > 15.0f);
}

bool FlightController::canTakeoff() const
{
    return (m_currentState == FlightState::ARMED);
}
