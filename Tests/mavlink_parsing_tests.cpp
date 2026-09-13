#include <gtest/gtest.h>
#include "common/mavlink.h"
#include "standard/mavlink_msg_global_position_int.h"

TEST(MavlinkParsingTests, DecodeAttitudeMessage)
{
    mavlink_message_t tx_msg;
    mavlink_msg_attitude_pack(1, 1, &tx_msg, 12345, 0.1f, 0.2f, 0.3f, 0.0f, 0.0f, 0.0f);
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    uint16_t len = mavlink_msg_to_send_buffer(buffer, &tx_msg);

    mavlink_message_t rx_msg;
    mavlink_status_t status;
    bool got_message = false;

    for(uint16_t i = 0; i < len; ++i)
    {
        if(mavlink_parse_char(MAVLINK_COMM_0, buffer[i], &rx_msg, &status) == MAVLINK_FRAMING_OK)
        {
            got_message = true;
        }
    }

    ASSERT_TRUE(got_message) << "Parser never produced a complete message";
    ASSERT_EQ(rx_msg.msgid, MAVLINK_MSG_ID_ATTITUDE);

    mavlink_attitude_t attitude;
    mavlink_msg_attitude_decode(&rx_msg, &attitude);

    EXPECT_FLOAT_EQ(attitude.roll, 0.1f);
    EXPECT_FLOAT_EQ(attitude.pitch, 0.2f);
    EXPECT_FLOAT_EQ(attitude.yaw, 0.3f);
}

TEST(MavlinkParsingTests, DecodeGlobalPositionIntMessage)
{
    mavlink_message_t msg;
    mavlink_msg_global_position_int_pack(1, 1, &msg, 12345, -353632621, 343784191, 100, 23, 1, 1, 1, 2);
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    uint16_t len = mavlink_msg_to_send_buffer(buffer, &msg);

    mavlink_message_t rx_msg;
    mavlink_status_t status;
    bool got_message = false;

    for(uint16_t i = 0; i < len; ++i)
    {
        if(mavlink_parse_char(MAVLINK_COMM_0, buffer[i], &rx_msg, &status) == MAVLINK_FRAMING_OK)
        {
            got_message = true;
        }
    }

    ASSERT_TRUE(got_message) << "Parser never produced a complete message";
    ASSERT_EQ(rx_msg.msgid, MAVLINK_MSG_ID_GLOBAL_POSITION_INT);

    mavlink_global_position_int_t globalPosition;
    mavlink_msg_global_position_int_decode(&rx_msg, &globalPosition);
    EXPECT_EQ(globalPosition.alt, 100);
    EXPECT_EQ(globalPosition.lat, -353632621);
    EXPECT_EQ(globalPosition.lon, 343784191);
}

TEST(MavlinkParsingTests, DecodeVfrHudMessage)
{
    mavlink_message_t msg;
    mavlink_msg_vfr_hud_pack(1, 1, &msg, 12.5f, 12.0f, 90, 50, 100.0f, 0.5f);
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    uint16_t len = mavlink_msg_to_send_buffer(buffer, &msg);

    mavlink_message_t rx_msg;
    mavlink_status_t status;
    bool got_message = false;

    for (uint16_t i = 0; i < len; ++i)
    {
        if (mavlink_parse_char(MAVLINK_COMM_0, buffer[i], &rx_msg, &status) == MAVLINK_FRAMING_OK)
        {
            got_message = true;
        }
    }

    ASSERT_TRUE(got_message) << "Parser never produced a complete message";
    ASSERT_EQ(rx_msg.msgid, MAVLINK_MSG_ID_VFR_HUD);

    mavlink_vfr_hud_t vfrHud;
    mavlink_msg_vfr_hud_decode(&rx_msg, &vfrHud);

    EXPECT_FLOAT_EQ(vfrHud.airspeed, 12.5f);
    EXPECT_FLOAT_EQ(vfrHud.groundspeed, 12.0f);
    EXPECT_FLOAT_EQ(vfrHud.alt, 100.0f);
}

TEST(MavlinkParsingTests, DecodeSysStatusMessage)
{
    mavlink_message_t msg;
    mavlink_msg_sys_status_pack(1, 1, &msg,
        0, 0, 0,
        0, 12600, 0, 100,
        0, 0, 0, 0, 0, 0,
        0, 0, 0);
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    uint16_t len = mavlink_msg_to_send_buffer(buffer, &msg);

    mavlink_message_t rx_msg;
    mavlink_status_t status;
    bool got_message = false;

    for (uint16_t i = 0; i < len; ++i)
    {
        if (mavlink_parse_char(MAVLINK_COMM_0, buffer[i], &rx_msg, &status) == MAVLINK_FRAMING_OK)
        {
            got_message = true;
        }
    }

    ASSERT_TRUE(got_message) << "Parser never produced a complete message";
    ASSERT_EQ(rx_msg.msgid, MAVLINK_MSG_ID_SYS_STATUS);

    mavlink_sys_status_t sysStatus;
    mavlink_msg_sys_status_decode(&rx_msg, &sysStatus);

    EXPECT_EQ(sysStatus.voltage_battery, 12600);
    EXPECT_EQ(sysStatus.battery_remaining, 100);
}