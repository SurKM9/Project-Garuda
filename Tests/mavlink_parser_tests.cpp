#include <gtest/gtest.h>
#include <iterator>
#include <algorithm>
#include "MavlinkParser.hpp"

// One complete frame in one chunk decodes to exactly one message
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

// Several frames can arrive in one UDP datagram; all must come out, in arrival order
TEST(MavlinkParserTests, TwoFramesInOneBuffer)
{
    mavlink_message_t tx1;
    mavlink_message_t tx2;
    mavlink_msg_attitude_pack(1, 1, &tx1, 12345, 0.1f, 0.2f, 0.3f, 0.0f, 0.0f, 0.0f);
    mavlink_msg_attitude_pack(1, 1, &tx2, 12345, 0.6f, 0.2f, 0.3f, 0.0f, 0.0f, 0.0f);

    // Different roll values tell the two messages apart when checking the order
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

// On stream links (serial, TCP) a frame can be cut across reads; the parser must
// keep the first half and complete the message on the next feed()
TEST(MavlinkParserTests, FrameSplitAcrossTwoFeeds)
{
    mavlink_message_t tx;
    mavlink_msg_attitude_pack(1, 1, &tx, 12345, 0.1f, 0.2f, 0.3f, 0.0f, 0.0f, 0.0f);

    const size_t split = 15;
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN] = {};
    const uint16_t len = mavlink_msg_to_send_buffer(buffer, &tx);

    // The split point must land inside the frame, otherwise len - split wraps around
    ASSERT_LT(split, len);

    // Same parser for both halves: the partial frame lives in its member state
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

// Noise between frames is skipped and counted, and both frames still decode
TEST(MavlinkParserTests, GarbageBetweenFramesIsCountedAsDropped)
{
    mavlink_message_t tx1;
    mavlink_message_t tx2;
    mavlink_msg_attitude_pack(1, 1, &tx1, 12345, 0.1f, 0.2f, 0.3f, 0.0f, 0.0f, 0.0f);
    mavlink_msg_attitude_pack(1, 1, &tx2, 12345, 0.6f, 0.2f, 0.3f, 0.0f, 0.0f, 0.0f);

    // Must not contain 0xFD or 0xFE: those are frame start markers (MAVLink v2/v1)
    constexpr uint8_t junk[] = { 0x00, 0x11, 0x22, 0x33, 0x44 };
    constexpr size_t junkLen = std::size(junk);

    // Layout: [ frame A ][ junk ][ frame B ]
    uint8_t buffer[2 * MAVLINK_MAX_PACKET_LEN + junkLen] = {};
    const uint16_t lenA = mavlink_msg_to_send_buffer(buffer, &tx1);

    std::copy(std::begin(junk), std::end(junk), buffer+lenA);

    const uint16_t lenB = mavlink_msg_to_send_buffer(buffer + lenA + junkLen, &tx2);

    MavlinkParser parser;
    const auto messages = parser.feed(std::span<const uint8_t>(buffer, lenA + junkLen + lenB));

    ASSERT_EQ(messages.size(), 2u);

    mavlink_attitude_t attitudeA;
    mavlink_msg_attitude_decode(&messages[0], &attitudeA);
    EXPECT_FLOAT_EQ(attitudeA.roll, 0.1f);

    mavlink_attitude_t attitudeB;
    mavlink_msg_attitude_decode(&messages[1], &attitudeB);
    EXPECT_FLOAT_EQ(attitudeB.roll, 0.6f);

    EXPECT_EQ(parser.droppedBytes(), junkLen);
    // Noise must not be mistaken for a corrupted frame
    EXPECT_EQ(parser.badCrcCount(), 0u);
}

// A frame damaged in transit is rejected and counted as a bad CRC
TEST(MavlinkParserTests, CorruptedFrameIsCountedAsBadCrc)
{
    mavlink_message_t tx;
    mavlink_msg_attitude_pack(1, 1, &tx, 12345, 0.1f, 0.2f, 0.3f, 0.0f, 0.0f, 0.0f);
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN] = {};
    const uint16_t len = mavlink_msg_to_send_buffer(buffer, &tx);

    // Flip every bit of the last CRC byte. XOR always changes the value, and the
    // header stays intact, so the frame is read in full and fails at the CRC check.
    buffer[len - 1] ^= 0xFF;

    MavlinkParser parser;
    const auto messages = parser.feed(std::span<const uint8_t>(buffer, len));

    EXPECT_EQ(messages.size(), 0u);

    // The rejected frame's bytes must not also be counted as noise
    EXPECT_EQ(parser.droppedBytes(), 0u);
    EXPECT_EQ(parser.badCrcCount(), 1u);
}
