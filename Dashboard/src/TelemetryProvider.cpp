#include "TelemetryProvider.hpp"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

TelemetryProvider::TelemetryProvider(QObject *parent) : QObject(parent) {}

TelemetryProvider::~TelemetryProvider() { stop(); }

void TelemetryProvider::start() {
    m_running = true;
    m_workerThread = std::thread(&TelemetryProvider::runReceiver, this);
}

void TelemetryProvider::stop() {
    m_running = false;
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void TelemetryProvider::runReceiver() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(14550);

    bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));

    TelemetryPacket buffer;
    while (m_running) {
        ssize_t n = recvfrom(sockfd, &buffer, sizeof(buffer), 0, nullptr, nullptr);
        if (n > 0) {
            std::lock_guard<std::mutex> lock(m_mutex);

            // Check if data actually changed before waking up the UI
            bool altChanged = (m_data.altitude != buffer.altitude);

            m_data = buffer;

            // Emit signals to wake up QML (must stay on the UI thread)
            if (altChanged) emit altitudeChanged();
            emit velocityChanged();
            emit batteryChanged();
        }
    }
    close(sockfd);
}

void TelemetryProvider::sendLandCommand() {
    // 1. Create a temporary socket for sending
    int sendSock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in simAddr;
    simAddr.sin_family = AF_INET;
    simAddr.sin_port = htons(14551); // Simulator will listen on 14551
    simAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 2. Prepare the packet
    CommandPacket cmd;
    cmd.command_id = static_cast<uint32_t>(UAVCommand::Land);
    cmd.param1 = 0.0f; // Target altitude is 0 for landing

    // 3. Blast it!
    sendto(sendSock, &cmd, sizeof(cmd), 0, (struct sockaddr*)&simAddr, sizeof(simAddr));

    std::cout << "[GCS] Sent LAND command to Simulator" << std::endl;
    close(sendSock);
}
