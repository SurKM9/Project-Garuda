/**
 * @file MavlinkParser.hpp
 * @brief Turns a raw MAVLink byte stream into complete messages.
 */

#ifndef MAVLINK_PARSER_HPP
#define MAVLINK_PARSER_HPP

#include <cstdint>
#include <span>
#include <vector>
#include "ardupilotmega/mavlink.h"

/**
 * @class MavlinkParser
 * @brief Reassembles MAVLink frames from bytes received in arbitrary chunks.
 *
 * A single buffer may contain several frames, part of a frame, or noise between
 * frames. The parser keeps its own framing state instead of using the global
 * per-channel state behind mavlink_parse_char(), so every link can own an
 * independent parser and a frame split across two feed() calls is still decoded.
 *
 * It also keeps its own link-quality counters. The library's counters are too
 * narrow (they wrap after 255 errors) and do not count noise bytes at all.
 */
class MavlinkParser
{
public:

    /**
     * @brief Parses the next chunk of received bytes.
     * @param bytes  Bytes as received, e.g. one UDP datagram. They do not need
     *               to start or end on a frame boundary.
     * @return All messages completed by this chunk, in the order they arrived.
     *         Frames that failed the CRC check are not returned.
     */
    std::vector<mavlink_message_t> feed(std::span<const uint8_t> bytes);

    /**
     * @brief Returns the number of noise bytes discarded between frames.
     * @return Bytes skipped while waiting for a frame start marker.
     */
    uint64_t droppedBytes() const noexcept { return m_droppedBytes; }

    /**
     * @brief Returns the number of complete frames rejected by the CRC check.
     * @return Count of corrupted frames.
     */
    uint64_t badCrcCount() const noexcept { return m_badCrcCount; }

private:

    mavlink_message_t m_message{};  ///< Frame being assembled, kept between feed() calls
    mavlink_status_t m_status{};    ///< Framing state machine, kept between feed() calls
    uint64_t m_droppedBytes{};      ///< See droppedBytes()
    uint64_t m_badCrcCount{};       ///< See badCrcCount()
};

#endif // MAVLINK_PARSER_HPP
