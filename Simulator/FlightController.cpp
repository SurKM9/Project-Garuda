#include <iostream>
#include "FlightController.hpp"


FlightController::FlightController() :
    m_currentState(FlightState::IDLE),
    m_altitude(0.0f),
    m_targetAltitude(0.0f),
    m_battery(100),
    m_velocity(0.0f)
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
        // Only allow disarming if we are very close to the ground (e.g., < 0.3m)
        if (m_altitude < 0.3f) {
            m_currentState = FlightState::IDLE;
            std::cout << "[FlightController] System DISARMED safely on ground." << std::endl;
        } else {
            std::cout << "[REJECTED] Cannot disarm while in mid-air! Altitude: "
                      << m_altitude << "m" << std::endl;
        }
        break;
    case CommandType::LAND:
        if (m_currentState == FlightState::FLYING || m_currentState == FlightState::TAKEOFF) {
            m_currentState = FlightState::LANDING;
            std::cout << "[FlightController] Landing initiated..." << std::endl;
        }
        break;
    case CommandType::EMERGENCY_STOP:
        m_currentState = FlightState::EMERGENCY;
        std::cout << "[FlightController] EMERGENCY STOP TRIGGERED!\n";
        break;

    default:
        break;
    }
}

void FlightController::update(float dt) {
    const float GRAVITY = 9.81f;
    float thrust = 0.0f;

    switch (m_currentState) {
        case FlightState::TAKEOFF:
            thrust = 15.0f; // More than gravity to climb
            if (m_altitude >= 10.0f) m_currentState = FlightState::FLYING;
            break;

        case FlightState::FLYING: {
            // Simple "Target-Seeking" Logic
            float target_alt = 10.0f;
            float alt_error = target_alt - m_altitude;

            // If we are above the target, we need less thrust than gravity to drop back down
            // If we are below, we need more.
            // We also use velocity as a "damper" to prevent oscillation.
            thrust = GRAVITY + (alt_error * 2.0f) - (m_velocity * 1.5f);
            break;
        }

        case FlightState::LANDING:
            thrust = 7.0f; // Less than gravity to descend
            break;

        case FlightState::IDLE:
        default:
            thrust = 0.0f;
            break;
    }

    // Physics Engine Calculation
    float acceleration = thrust - GRAVITY;
    m_velocity += acceleration * dt;
    m_altitude += m_velocity * dt;

    // Ground Collision Safety
    if (m_altitude < 0.0f) {
        m_altitude = 0.0f;
        m_velocity = 0.0f;
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
