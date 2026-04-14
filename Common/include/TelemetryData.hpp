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
struct TelemetryPacket {
    uint32_t packet_id;
    float latitude;
    float longitude;
    float altitude;
    float velocity;
    uint8_t battery_pct;
    uint8_t flight_mode;
    FlightState state;     // Byte 23
};

struct CommandPacket {
    uint32_t command_seq;  // Formerly command_id (tracking number)
    CommandType type;      // The actual command (Arm, Takeoff, etc.)
    uint8_t reserved;      // Padding for 4-byte alignment
    float param1;          // Extra data (e.g., target altitude)
};
#pragma pack(pop)

#endif
