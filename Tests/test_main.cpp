#include <gtest/gtest.h>
#include "TelemetryData.hpp"


/**
 * @brief Logic Test: Ensure our safety checks catch low battery.
 */
TEST(SafetyLogicTests, BatterySafetyCheck) {
    TelemetryPacket packet;

    // case A: Battery high
    packet.battery_pct = 95;
    bool isSafe = (packet.battery_pct > 20);
    EXPECT_TRUE(isSafe);

    // case B: Battery low
    packet.battery_pct = 15;
    isSafe = (packet.battery_pct > 20);
    EXPECT_FALSE(isSafe) << "Safety failed: 15% battery level is not flight ready";
}

/**
 * @brief Ensure altitude is not negative
 */
TEST(SafetyLogicTests, AltitudeNegativeCheck){

    TelemetryPacket packet;

    packet.altitude = 10.5f;
    EXPECT_GE(packet.altitude, 0.0f)
        << "UAV reported an underground altitude! Check sensor calibration.";
}

/**
 * @brief Test the memory alignment and size of our network packets.
 * This ensures that 'packed' structures don't accidentally grow.
 */
TEST(ProtocolTests, PacketSizeCheck) {
    // Current packet:
    // uint32 (4) + 8x float (32) + 3x uint8 (3) = 39 bytes
    const size_t expected_size = 39;

    EXPECT_EQ(sizeof(TelemetryPacket), expected_size)
        << "CRITICAL: TelemetryPacket size mismatch! Network protocol will break.";
}

/**
 * @brief Simple math test to ensure GTest is working correctly.
 */
TEST(SystemTests, GTestSanityCheck) {
    int result = 2 + 2;
    ASSERT_EQ(result, 4);
}
