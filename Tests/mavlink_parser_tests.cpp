#include <gtest/gtest.h>
#include "MavlinkParser.hpp"

TEST(MavlinkParserTests, SingleFrameYieldsOneMessage)
{
    mavlink_message_t tx;
    mavlink_msg_attitude_pack(1, 1, &tx, 12345, 0.1f, 0.2f, 0.3f, 0.0f, 0.0f, 0.0f);
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN] = {};
    const uint16_t len = mavlink_msg_to_send_buffer(buffer, &tx);

    MavlinkParser parser;
    const auto messages = parser.feed(std::span<const uint8_t>(buffer, len));

    ASSERT_EQ(messages.size(), 1u);
    EXPECT_EQ(messages[0].msgid, MAVLINK_MSG_ID_ATTITUDE);

    mavlink_attitude_t attitude;
    mavlink_msg_attitude_decode(&messages[0], &attitude);
    EXPECT_FLOAT_EQ(attitude.roll, 0.1f);
}

TEST(MavlinkParserTests, TwoFramesInOneBuffer)
{
    mavlink_message_t tx1;
    mavlink_message_t tx2;
    mavlink_msg_attitude_pack(1, 1, &tx1, 12345, 0.1f, 0.2f, 0.3f, 0.0f, 0.0f, 0.0f);
    mavlink_msg_attitude_pack(1, 1, &tx2, 12345, 0.6f, 0.2f, 0.3f, 0.0f, 0.0f, 0.0f);

    uint8_t buffer[2 * MAVLINK_MAX_PACKET_LEN] = {};
    const uint16_t lenA = mavlink_msg_to_send_buffer(buffer, &tx1);
    const uint16_t lenB = mavlink_msg_to_send_buffer(buffer+lenA, &tx2);

    MavlinkParser parser;
    const auto messages = parser.feed(std::span<const uint8_t>(buffer, (lenA+lenB)));

    ASSERT_EQ(messages.size(), 2u);

    mavlink_attitude_t attitudeA;
    mavlink_msg_attitude_decode(&messages[0], &attitudeA);
    EXPECT_FLOAT_EQ(attitudeA.roll, 0.1f);

    mavlink_attitude_t attitudeB;
    mavlink_msg_attitude_decode(&messages[1], &attitudeB);
    EXPECT_FLOAT_EQ(attitudeB.roll, 0.6f);
}

TEST(MavlinkParserTests, FrameSplitAcrossTwoFeeds)
{
    mavlink_message_t tx;
    mavlink_msg_attitude_pack(1, 1, &tx, 12345, 0.1f, 0.2f, 0.3f, 0.0f, 0.0f, 0.0f);

    const size_t split = 15;
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN] = {};
    const uint16_t len = mavlink_msg_to_send_buffer(buffer, &tx);

    ASSERT_LT(split, len);

    MavlinkParser parser;
    const auto messagesA = parser.feed(std::span<const uint8_t>(buffer, split));
    EXPECT_EQ(messagesA.size(), 0u);

    const auto messagesB = parser.feed(std::span<const uint8_t>(buffer + split, len - split));
    ASSERT_EQ(messagesB.size(), 1u);

    EXPECT_EQ(messagesB[0].msgid, MAVLINK_MSG_ID_ATTITUDE);

    mavlink_attitude_t attitude;
    mavlink_msg_attitude_decode(&messagesB[0], &attitude);
    EXPECT_FLOAT_EQ(attitude.roll, 0.1f);
}
