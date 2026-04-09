#ifndef TELEMETRY_DATA_HPP
#define TELEMETRY_DATA_HPP

#include <cstdint>

enum class UAVCommand : uint32_t {
    None = 0,
    Land = 1,
    Takeoff = 2,
    EmergencyStop = 3
};

/**
 * @brief TelemetryPacket
 * We use #pragma pack(1) to ensure the structure is exactly 22 bytes.
 * This prevents the compiler from adding "padding" bytes for alignment,
 * which is critical when sending raw bytes over a network socket.
 */
#pragma pack(push, 1)
struct TelemetryPacket {
    uint32_t packet_id;    // 4 bytes
    float latitude;        // 4 bytes
    float longitude;       // 4 bytes
    float altitude;        // 4 bytes
    float velocity;        // 4 bytes
    uint8_t battery_pct;   // 1 byte
    uint8_t flight_mode;   // 1 byte
};

struct CommandPacket {
    uint32_t command_id; // 1 = Land, 2 = Takeoff, 3 = Emergency Stop
    float param1;        // Extra data (e.g., target altitude)
};

#pragma pack(pop)

#endif // TELEMETRY_DATA_HPP
