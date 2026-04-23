#include <gtest/gtest.h>
#include "../Simulator/FlightController.hpp"

// Test 1: Basic state transistions
TEST(FlightLogic, ArmingSequence){

    FlightController fc;
  	
	// Should start in idle state
	EXPECT_EQ(fc.getState(), FlightState::IDLE);

    // Requesting takeoff while IDLE should be ignored
    CommandPacket cmd;
    cmd.type = CommandType::TAKEOFF;
    cmd.param1 = 10.5f;
    fc.handleCommand(cmd);
    EXPECT_EQ(fc.getState(), FlightState::IDLE);

    // Requesting ARM should work
    CommandPacket armCmd;
    armCmd.type = CommandType::ARM;
    fc.handleCommand(armCmd);
    EXPECT_EQ(fc.getState(), FlightState::ARMED);
}

// Test 2: Physics Update (The Climb)
TEST(FlightLogic, TakeoffAndClimb) {
    FlightController fc;

    // 1. Arm and Takeoff
    fc.handleCommand({0, CommandType::ARM, 0, 0});
    fc.handleCommand({1, CommandType::TAKEOFF, 0, 10.0f});

    // 2. Simulate 1 second of flight (dt = 1.0)
    fc.update(1.0f);

    // 3. New Physics Expectations:
    // Velocity should be: 15.0 (Thrust) - 9.81 (Gravity) = 5.19 m/s
    EXPECT_NEAR(fc.getVelocity(), 5.19f, 0.001f);

    // Altitude should be: v * dt = 5.19 * 1.0 = 5.19 m
    EXPECT_NEAR(fc.getAltitude(), 5.19f, 0.001f);
}

// Test 3: Prevent midair disarm
TEST(FlightLogic, PreventMidAirDisarm)
{
    FlightController fc;

    // Get into a flying state
    fc.handleCommand({0, CommandType::ARM, 0, 0});
    fc.handleCommand({1, CommandType::TAKEOFF, 0, 10.0f});
    fc.update(5.0f); // Now we are at ~2.5m altitude

    // Try to disarm while in the air
    CommandPacket disarmCmd{2, CommandType::DISARM, 0, 0};
    fc.handleCommand(disarmCmd);

    // State should NOT be IDLE; it should still be TAKEOFF or FLYING
    EXPECT_NE(fc.getState(), FlightState::IDLE);
}

TEST(FlightLogic, SafetyMidAirDisarm)
{
    FlightController fc;

    // Get the drone into the air
    fc.handleCommand({0, CommandType::ARM, 0, 0.0f});
    fc.handleCommand({1, CommandType::TAKEOFF, 0, 10.0f});

    // Simulate some time so it climbs to ~2.5 meters
    fc.update(5.0f);
    ASSERT_GT(fc.getAltitude(), 1.0f);

    // Attempt to disarm mid-flight
    CommandPacket disarm{0, CommandType::DISARM, 0, 0.0f};
    fc.handleCommand(disarm);

    // Expect to ignore disarm command and stay in flying or takeoff state
    EXPECT_NE(fc.getState(), FlightState::IDLE);
}

// Fixed: use small dt steps for realistic physics; verify state only (recovery tested separately)
TEST(FlightLogic, EmergencyStopTransition)
{
    FlightController fc;

    fc.handleCommand({0, CommandType::ARM, 0, 0.0f});
    fc.handleCommand({1, CommandType::TAKEOFF, 0, 10.0f});
    for (int i = 0; i < 100; ++i) fc.update(0.1f);
    ASSERT_GT(fc.getAltitude(), 2.0f);

    fc.handleCommand({2, CommandType::EMERGENCY_STOP, 0, 0.0f});

    EXPECT_EQ(fc.getState(), FlightState::EMERGENCY);
}

// EMERGENCY triggered while in FLYING state (the real-world scenario)
TEST(FlightLogic, EmergencyStopFromFlying)
{
    FlightController fc;

    fc.handleCommand({0, CommandType::ARM, 0, 0.0f});
    fc.handleCommand({1, CommandType::TAKEOFF, 0, 10.0f});
    for (int i = 0; i < 200; ++i) fc.update(0.1f); // climb until stable in FLYING

    ASSERT_EQ(fc.getState(), FlightState::FLYING);

    fc.handleCommand({2, CommandType::EMERGENCY_STOP, 0, 0.0f});
    EXPECT_EQ(fc.getState(), FlightState::EMERGENCY);
}

// After EMERGENCY (thrust=0), drone falls; once grounded it recovers to IDLE
TEST(FlightLogic, EmergencyRecovery)
{
    FlightController fc;

    fc.handleCommand({0, CommandType::ARM, 0, 0.0f});
    fc.handleCommand({1, CommandType::TAKEOFF, 0, 10.0f});
    for (int i = 0; i < 200; ++i) fc.update(0.1f);

    fc.handleCommand({2, CommandType::EMERGENCY_STOP, 0, 0.0f});
    ASSERT_EQ(fc.getState(), FlightState::EMERGENCY);

    for (int i = 0; i < 500; ++i) fc.update(0.1f);

    EXPECT_EQ(fc.getState(), FlightState::IDLE);
    EXPECT_NEAR(fc.getAltitude(), 0.0f, 0.01f);
    EXPECT_NEAR(fc.getVelocity(), 0.0f, 0.01f);
}

// DISARM should succeed when already on the ground
TEST(FlightLogic, DisarmSucceedsOnGround)
{
    FlightController fc;

    fc.handleCommand({0, CommandType::ARM, 0, 0.0f});
    ASSERT_EQ(fc.getState(), FlightState::ARMED);

    fc.handleCommand({1, CommandType::DISARM, 0, 0.0f});
    EXPECT_EQ(fc.getState(), FlightState::IDLE);
}

// ARM should be rejected when battery is below the 15% threshold
TEST(FlightLogic, LowBatteryPreventsArm)
{
    FlightController fc;
    fc.setBattery(10);

    fc.handleCommand({0, CommandType::ARM, 0, 0.0f});
    EXPECT_EQ(fc.getState(), FlightState::IDLE);
}

// TAKEOFF should be rejected when not in ARMED state
TEST(FlightLogic, TakeoffRejectedWhenNotArmed)
{
    FlightController fc; // starts IDLE

    fc.handleCommand({0, CommandType::TAKEOFF, 0, 10.0f});
    EXPECT_EQ(fc.getState(), FlightState::IDLE);
}

// LAND should be ignored when the drone is not airborne
TEST(FlightLogic, LandRejectedWhenNotInFlight)
{
    FlightController fc;

    fc.handleCommand({0, CommandType::ARM, 0, 0.0f});
    ASSERT_EQ(fc.getState(), FlightState::ARMED);

    fc.handleCommand({1, CommandType::LAND, 0, 0.0f});
    EXPECT_EQ(fc.getState(), FlightState::ARMED);
}
