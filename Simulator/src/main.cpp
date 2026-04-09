#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>

// Networking headers
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "TelemetryData.hpp"

int main() {
    // 1. Create a UDP Socket
    // AF_INET = IPv4, SOCK_DGRAM = UDP, 0 = Default Protocol
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error: Could not create socket." << std::endl;
        return -1;
    }

    // 2. Configure the Destination Address
    struct sockaddr_in dest_addr;
    std::memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(14550); // Standard UAV Telemetry Port
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Send to localhost

    // 3. Initialize our Telemetry Object
    TelemetryPacket packet;
    packet.packet_id = 0;
    packet.latitude = 48.1351f;  // Starting point: Munich
    packet.longitude = 11.5820f;
    packet.altitude = 500.0f;
    packet.velocity = 10.5f;
    packet.battery_pct = 100;
    packet.flight_mode = 1;      // Let's say 1 = "Armed/Manual"

    std::cout << "UAV Simulator online. Sending to 127.0.0.1:14550..." << std::endl;

    // 1. Create an additional socket for listening to commands
    int cmdSock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in cmdAddr;
    cmdAddr.sin_family = AF_INET;
    cmdAddr.sin_addr.s_addr = INADDR_ANY;
    cmdAddr.sin_port = htons(14551);
    bind(cmdSock, (const struct sockaddr *)&cmdAddr, sizeof(cmdAddr));

    // 2. Set the command socket to NON-BLOCKING mode
    // This is critical: we don't want the drone to stop flying while waiting for a command!
    fcntl(cmdSock, F_SETFL, O_NONBLOCK);

    bool isLanding = false;

    while (true) {
        // A. Check for incoming commands (Non-blocking)
        CommandPacket cmd;
        if (recvfrom(cmdSock, &cmd, sizeof(cmd), 0, nullptr, nullptr) > 0) {
            if (cmd.command_id == static_cast<uint32_t>(UAVCommand::Land)) {
                isLanding = true;
                std::cout << "[SIM] Command Verified: Initiating Landing Sequence." << std::endl;
            }
        }

        // B. Flight Logic
        if (isLanding) {
            if (packet.altitude > 0.1f) {
                packet.altitude -= 0.5f; // Descend
                packet.velocity = -2.0f;
            } else {
                packet.altitude = 0.0f;
                packet.velocity = 0.0f;
                packet.flight_mode = 0; // Disarmed
            }
        } else {
            packet.altitude += 0.1f; // Normal climb
        }

        // C. Send Telemetry (as before)
        sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    close(sockfd);
    return 0;
}
