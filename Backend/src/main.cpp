#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <cstring>

// Networking
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "TelemetryData.hpp"

// Global (or shared) state
TelemetryPacket latest_data;
std::mutex data_mutex;
std::atomic<bool> running{true};

// This function will run in a SEPARATE thread
void receiver_thread_func(int sockfd) {
    TelemetryPacket buffer;
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);

    while (running) {
        ssize_t n = recvfrom(sockfd, &buffer, sizeof(buffer),
                             MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);

        if (n > 0) {
            // Protect the shared data while we copy it
            std::lock_guard<std::mutex> lock(data_mutex);
            latest_data = buffer;
        }
    }
    std::cout << "Receiver thread shutting down..." << std::endl;
}

int main() {
    // [Socket Setup - Same as before]
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in servaddr;
    std::memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(14550);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        return -1;
    }

    // 1. Start the background thread
    // We pass the socket file descriptor to the thread
    std::thread worker(receiver_thread_func, sockfd);

    std::cout << "Backend started with Multi-threading!" << std::endl;

    // 2. The Main Loop (Consumer)
    // This thread is now FREE to do other things while the worker waits for data
    while (true) {
        {
            // Lock briefly just to copy the data
            std::lock_guard<std::mutex> lock(data_mutex);
            std::cout << "[MAIN] Current Alt: " << latest_data.altitude << "m" << std::endl;
        }

        // Simulate doing other work (like updating a UI or checking a database)
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Safety break for our demo
        if(latest_data.packet_id > 1000) break;
    }

    running = false; // Tell the thread to stop
    shutdown(sockfd, SHUT_RD); // Wake up the blocking recvfrom
    worker.join(); // Cleanly wait for the thread to finish
    close(sockfd);

    return 0;
}
