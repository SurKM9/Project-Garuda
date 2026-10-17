#ifndef MAVLINK_PARSER_HPP
#define MAVLINK_PARSER_HPP

#include <cstdint>
#include <span>
#include <vector>
#include "ardupilotmega/mavlink.h"

class MavlinkParser
{
public:

    std::vector<mavlink_message_t> feed(std::span<const uint8_t> bytes);

private:

    mavlink_message_t m_message{};
    mavlink_status_t m_status{};
};
#endif // MAVLINK_PARSER_HPP
