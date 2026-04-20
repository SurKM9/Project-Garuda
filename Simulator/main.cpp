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
#include "GarudaConfig.hpp"

int main(int argc, char* argv[]) {
    std::cout << "--- Project Garuda: UAV Simulator Starting ---" << std::endl;

    GarudaConfig cfg = loadConfig();

    // CLI arg overrides config file (useful for one-off testing)
    if (argc > 1) {
        cfg.gcs_ip = argv[1];
        std::cout << "[Config] CLI override: GCS IP = " << cfg.gcs_ip << "\n";
    }

    std::cout << "[Config] GCS IP: " << cfg.gcs_ip
              << " | Command port: " << cfg.command_port
              << " | Telemetry port: " << cfg.telemetry_port << "\n";

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
    servaddr.sin_port = htons(cfg.command_port);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    // Setup destination address for Telemetry
    struct sockaddr_in cliaddr{};
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(cfg.telemetry_port);
    inet_pton(AF_INET, cfg.gcs_ip.c_str(), &cliaddr.sin_addr);

    // 3. Timing Setup
    auto last_time = std::chrono::steady_clock::now();
    auto last_telemetry_time = std::chrono::steady_clock::now();
    uint32_t packet_counter = 0;

    std::cout << "Simulator loop running. Listening on port " << cfg.command_port << "..." << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    // 4. Main Simulation Loop
    while (true) {
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
            tx_packet.velocity = controller.getVelocity();
            tx_packet.battery_pct = controller.getBattery();
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
