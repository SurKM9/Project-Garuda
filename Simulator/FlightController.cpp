#include <algorithm>
#include <iostream>
#include <cmath>
#include "FlightController.hpp"


FlightController::FlightController() :
    m_currentState(FlightState::IDLE),
    m_altitude(0.0f),
    m_targetAltitude(0.0f),
    m_battery(100),
    m_velocity(0.0f),
    m_batteryDrainAccum(0.0f),
    m_latitude(48.1351),
    m_longitude(11.5820),
    m_roll(0.0f),
    m_pitch(0.0f),
    m_yaw(0.0f)
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
        break;

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
        if (m_currentState == FlightState::FLYING || m_currentState == FlightState::TAKEOFF) {
            m_currentState = FlightState::EMERGENCY;
            std::cout << "[FlightController] EMERGENCY STOP TRIGGERED!\n";
        }
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
            if (m_altitude >= m_targetAltitude) m_currentState = FlightState::FLYING;
            break;

        case FlightState::FLYING: {
            if (m_battery < 5) {
                m_currentState = FlightState::EMERGENCY;
                std::cout << "[FlightController] Critical battery! Emergency stop initiated.\n";
                break;
            }
            if (m_battery < 15) {
                m_currentState = FlightState::LANDING;
                std::cout << "[FlightController] Low battery! Forced landing initiated.\n";
                break;
            }

            const double h_speed = 10.0;
            double heading_rad = CRUISE_HEADING * M_PI / 180.0;
            m_latitude += (h_speed * std::cos(heading_rad) * dt) / 111000.0;
            m_longitude += (h_speed * std::sin(heading_rad) * dt) / (111000.0 * std::cos(m_latitude * M_PI / 180.0));

            float alt_error = m_targetAltitude - m_altitude;
            thrust = GRAVITY + (alt_error * 2.0f) - (m_velocity * 1.5f);
            break;
        }

        case FlightState::LANDING:
            thrust = 7.0f; // Less than gravity to descend
            if (m_altitude <= 0.0f && std::abs(m_velocity) < 0.1f) {
                m_currentState = FlightState::ARMED;
                std::cout << "[FlightController] Touchdown. Ready to disarm.\n";
            }
            break;

        case FlightState::IDLE:
            break;
        case FlightState::EMERGENCY:
            thrust = 0.0f;
            if(m_altitude <= 0.0f)
            {
                m_currentState = FlightState::IDLE;
                m_velocity = 0.0f;
            }
            break;
        default:
            thrust = 0.0f;
            break;
    }

    // Only drain when motors are actively spinning; IDLE/ARMED/EMERGENCY draw negligible current
    if(m_currentState == FlightState::TAKEOFF ||
        m_currentState == FlightState::FLYING ||
        m_currentState == FlightState::LANDING)
    {
        // Base draw (avionics) + thrust-proportional draw (motor current scales with load)
        m_batteryDrainAccum += (0.05f + 0.05f * thrust) * dt;

        // Commit whole percent points only — m_battery is uint8_t and can't hold fractions
        if(m_batteryDrainAccum >= 1.0f)
        {
            // Truncate to the integer part; remainder stays in accum so no drain is lost
            uint8_t drop = static_cast<uint8_t>(m_batteryDrainAccum);
            // Guard against underflow: uint8_t wraps around if subtracted below 0
            m_battery = (m_battery > drop) ? m_battery - drop : 0;
            m_batteryDrainAccum -= drop;
        }
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

    m_pitch=std::clamp(m_velocity*5.0f,-30.0f,30.0f);
    if(m_currentState==FlightState::FLYING){
        m_yaw = CRUISE_HEADING;
    }
}

FlightState FlightController::state() const
{
    return m_currentState;
}

float FlightController::altitude() const
{
    return m_altitude;
}

float FlightController::velocity() const
{
    return m_velocity;
}

uint8_t FlightController::battery() const
{
    return m_battery;
}

void FlightController::setBattery(uint8_t pct)
{
    m_battery = pct;
}

double FlightController::latitude() const
{
    return m_latitude;
}

double FlightController::longitude() const
{
    return m_longitude;
}

float FlightController::roll() const
{
    return m_roll;
}

float FlightController::pitch() const
{
    return m_pitch;
}

float FlightController::yaw() const
{
    return m_yaw;
}

float FlightController::batteryVoltage() const
{
    return 3.0f + (m_battery / 100.0f) * 1.2f;
}

bool FlightController::canArm() const
{
    return (m_currentState == FlightState::IDLE && m_battery > 15.0f);
}

bool FlightController::canTakeoff() const
{
    return (m_currentState == FlightState::ARMED);
}
