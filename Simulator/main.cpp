#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <csignal>

#include "TelemetryData.hpp"
#include "FlightController.hpp"

// Networking Constants
const int LISTEN_PORT = 5000;      // Port to receive commands from Dashboard
const int SEND_PORT = 5001;        // Port to send telemetry to Dashboard
const char* GCS_IP = "127.0.0.1";  // Localhost for now

int main() {
    std::cout << "--- Project Garuda: UAV Simulator Starting ---" << std::endl;

    // 1. Initialize our Flight Controller (The Brain)
    FlightController controller;

    // 2. Setup UDP Networking
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Set socket to non-blocking so the simulation loop never stops
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    // Bind for incoming commands
    struct sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(LISTEN_PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    // Setup destination address for Telemetry
    struct sockaddr_in cliaddr{};
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(SEND_PORT);
    inet_pton(AF_INET, GCS_IP, &cliaddr.sin_addr);

    // 3. Timing Setup
    auto last_time = std::chrono::steady_clock::now();
    auto last_telemetry_time = std::chrono::steady_clock::now();
    uint32_t packet_counter = 0;

    std::cout << "Simulator loop running. Listening on port " << LISTEN_PORT << "..." << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    // 4. Main Simulation Loop
    while (keepRunning) {
        // A. Calculate Delta Time (dt)
        auto current_time = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = current_time - last_time;
        float dt = elapsed.count();
        last_time = current_time;

        // B. Receive Commands (Non-blocking)
        CommandPacket cmd_in;
        struct sockaddr_in sender_addr;
        socklen_t len = sizeof(sender_addr);

        ssize_t n = recvfrom(sockfd, &cmd_in, sizeof(CommandPacket), 0,
                             (struct sockaddr *)&sender_addr, &len);

        if (n == sizeof(CommandPacket)) {
            std::cout << "[Network] Received command sequence: " << cmd_in.command_seq << std::endl;
            controller.handleCommand(cmd_in);
        }

        // C. Update Flight Logic (Physics/FSM)
        controller.update(dt);

        // D. Send Telemetry (10Hz - every 100ms)
        auto time_since_telemetry = std::chrono::duration_cast<std::chrono::milliseconds>(
                                        current_time - last_telemetry_time).count();

        if (time_since_telemetry >= 100) {
            TelemetryPacket tx_packet;
            tx_packet.packet_id = packet_counter++;
            tx_packet.latitude = 48.1351f;   // Fixed Munich coordinate for now
            tx_packet.longitude = 11.5820f;
            tx_packet.altitude = controller.getAltitude();
            tx_packet.velocity = 0.0f;
            tx_packet.battery_pct = 100;
            tx_packet.flight_mode = 1;       // Manual
            tx_packet.state = controller.getState();

            sendto(sockfd, &tx_packet, sizeof(TelemetryPacket), 0,
                   (const struct sockaddr *)&cliaddr, sizeof(cliaddr));

            last_telemetry_time = current_time;
        }

        // E. Sleep to prevent 100% CPU usage (~100 FPS sim rate)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "[System] Closing sockets and exiting gracefully." << std::endl;
    close(sockfd);
    return 0;
}
