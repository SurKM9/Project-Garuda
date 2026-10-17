#include "MavlinkParser.hpp"

std::vector<mavlink_message_t> MavlinkParser::feed(std::span<const uint8_t> bytes)
{
    std::vector<mavlink_message_t> result;

    // Outputs of the library call. rx only holds a valid message when framing is OK.
    mavlink_message_t rx;
    mavlink_status_t outStatus;

    // True while the parser is between frames and hunting for a start marker
    auto isWaitingForFrame = [this]{
        return m_status.parse_state == MAVLINK_PARSE_STATE_IDLE ||
               m_status.parse_state == MAVLINK_PARSE_STATE_UNINIT;
    };

    for(uint8_t byte : bytes)
    {
        // A byte that leaves a waiting parser still waiting was not a start marker, so it is noise.
        // Checking before AND after keeps the last byte of a frame (mid-frame -> idle) from being counted.
        const bool wasWaiting = isWaitingForFrame();
        auto framing = mavlink_frame_char_buffer(&m_message, &m_status, byte, &rx, &outStatus);
        if(wasWaiting && isWaitingForFrame())
        {
            m_droppedBytes++;
        }

        switch(framing)
        {
            case MAVLINK_FRAMING_OK:
            {
                // Copy now: rx is overwritten when the next frame in this chunk completes
                result.push_back(rx);
                break;
            }
            case MAVLINK_FRAMING_BAD_CRC:
            {
                m_badCrcCount++;
                break;
            }
            default:
                // INCOMPLETE: still collecting bytes. BAD_SIGNATURE: signing is not used on this link.
                break;
        }
    }

    return result;
}
