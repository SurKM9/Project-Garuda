// includes
#include "MavlinkParser.hpp"

std::vector<mavlink_message_t> MavlinkParser::feed(std::span<const uint8_t> bytes)
{
    std::vector<mavlink_message_t> result;
    mavlink_message_t rx;
    mavlink_status_t outStatus;

    for(uint8_t byte : bytes)
    {
        auto framing = mavlink_frame_char_buffer(&m_message, &m_status, byte, &rx, &outStatus);
        if(framing == MAVLINK_FRAMING_OK)
        {
            result.push_back(rx);
        }
    }

    return result;
}
