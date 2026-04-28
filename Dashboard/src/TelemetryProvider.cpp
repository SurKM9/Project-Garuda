#include "TelemetryProvider.hpp"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <QDebug>

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
    // 1. Setup the raw socket (Same as you had)
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // timer for periodic waking of socket
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000; // 500ms timeout
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in servaddr;
    std::memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(m_telemetryPort);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        return;
    }

    // Temporary buffer to hold the incoming raw bytes
    TelemetryPacket buffer;

    while (m_running) {
        // 2. Receive the data
        // recvfrom returns the number of bytes received
        ssize_t n = recvfrom(sockfd, &buffer, sizeof(buffer), 0, nullptr, nullptr);

        // 3. Size check
        // If recvfrom timed out, n will be -1.
        // The loop will simply restart and check m_running
        if (n == sizeof(TelemetryPacket)) {
            std::lock_guard<std::mutex> lock(m_mutex);

            // 4. Check for changes to avoid unnecessary UI refreshes
            bool altChanged = (m_data.altitude != buffer.altitude);
            bool velChanged = (m_data.velocity != buffer.velocity);
            bool batChanged = (m_data.battery_pct != buffer.battery_pct);
            bool stateChanged = (m_data.state != buffer.state);
            bool latChanged = (m_data.latitude != buffer.latitude);
            bool lonChanged = (m_data.longitude != buffer.longitude);

            // Update the internal data model
            m_data = buffer;

            // 5. Emit signals (This wakes up the QML UI)
            if (altChanged) emit altitudeChanged();
            if (velChanged) emit velocityChanged();
            if (batChanged) emit batteryChanged();
            if (stateChanged) emit flightStateChanged();
            if (latChanged) emit latitudeChanged();
            if (lonChanged) emit longitudeChanged();

        } else if (n > 0) {
            std::cout << "[Dashboard] Warning: Received packet of unexpected size: " << n << std::endl;
        }
    }
    close(sockfd);
}

void TelemetryProvider::sendCommand(int type, float param) {

    int sendSock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in simAddr;
    simAddr.sin_family = AF_INET;
    simAddr.sin_port = htons(m_commandPort);
    inet_pton(AF_INET, m_droneIp.c_str(), &simAddr.sin_addr);

    CommandPacket cmd;
    cmd.command_seq = 0;
    cmd.type = static_cast<CommandType>(type);
    cmd.reserved = 0;
    cmd.param1 = param;

    sendto(sendSock, &cmd, sizeof(cmd), 0, (struct sockaddr*)&simAddr, sizeof(simAddr));
    close(sendSock);

    qDebug() << "[GCS] Sent Command Type:" << type << "Param:" << param;
}

int TelemetryProvider::flightState() const
{
    return static_cast<int>(m_data.state);
}
