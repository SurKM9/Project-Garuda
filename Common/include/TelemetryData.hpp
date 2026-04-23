#ifndef TELEMETRY_DATA_HPP
#define TELEMETRY_DATA_HPP

#include <cstdint>

enum class FlightState : uint8_t {
    IDLE = 0, ARMED, TAKEOFF, FLYING, LANDING, EMERGENCY
};

enum class CommandType : uint8_t {
    NONE = 0,
    ARM = 1,
    DISARM = 2,
    TAKEOFF = 3,
    LAND = 4,
    EMERGENCY_STOP = 99
};

#pragma pack(push, 1)

/**
 * @struct TelemetryPacket
 * @brief Fixed-size packed packet sent from the Simulator to the GCS at 10Hz over UDP.
 */
struct TelemetryPacket {
    uint32_t    packet_id;    ///< Monotonically increasing sequence number
    float       latitude;     ///< Latitude in decimal degrees
    float       longitude;    ///< Longitude in decimal degrees
    float       altitude;     ///< Altitude above ground in metres
    float       velocity;     ///< Vertical velocity in m/s (positive = climbing)
    uint8_t     battery_pct;  ///< Remaining battery charge as a percentage (0–100)
    uint8_t     flight_mode;  ///< Reserved for future flight-mode flags
    FlightState state;        ///< Current state machine state of the flight controller
};

/**
 * @struct CommandPacket
 * @brief Fixed-size packed packet sent from the GCS to the Simulator to issue flight commands.
 */
struct CommandPacket {
    uint32_t    command_seq;  ///< Monotonically increasing sequence number for command tracking
    CommandType type;         ///< The command to execute (ARM, DISARM, TAKEOFF, etc.)
    uint8_t     reserved;     ///< Padding byte to maintain 4-byte alignment
    float       param1;       ///< Optional command parameter (e.g. target altitude for TAKEOFF)
};

#pragma pack(pop)

#endif
