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

    // Arm then Takeoff
    CommandPacket armCmd{0, CommandType::ARM, 0, 0};
    fc.handleCommand(armCmd);

    CommandPacket takeoffCmd{1, CommandType::TAKEOFF, 0, 5.0f}; // Target 5m
    fc.handleCommand(takeoffCmd);

    EXPECT_EQ(fc.getState(), FlightState::TAKEOFF);

    // Simulate 1 second of flight (dt = 1.0)
    // Our logic says climb is 0.5m/s, so altitude should be 0.5m
    fc.update(1.0f);
    EXPECT_NEAR(fc.getAltitude(), 0.5f, 0.001f);
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
