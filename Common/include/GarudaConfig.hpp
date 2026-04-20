#ifndef GARUDA_CONFIG_HPP
#define GARUDA_CONFIG_HPP

#include <string>
#include <fstream>
#include <cstdint>
#include <iostream>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct GarudaConfig {
    std::string gcs_ip         = "127.0.0.1";
    std::string drone_ip       = "127.0.0.1";
    uint16_t    telemetry_port = 5001;
    uint16_t    command_port   = 5000;
};

// Checks whether a QEMU TAP interface is active by looking for 192.168.7.1
// assigned to any local network interface.
inline bool detectQemuTap()
{
    struct ifaddrs* ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == -1) return false;

    bool found = false;
    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) continue;

        char buf[INET_ADDRSTRLEN];
        auto* sin = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr);
        inet_ntop(AF_INET, &sin->sin_addr, buf, sizeof(buf));

        if (std::string(buf) == "192.168.7.1") {
            found = true;
            break;
        }
    }

    freeifaddrs(ifaddr);
    return found;
}

// Config resolution order:
//   1. /etc/garuda/garuda.conf  (embedded target, installed by Yocto)
//   2. ./garuda.conf            (local override for development)
//   3. QEMU TAP auto-detect     (192.168.7.1 present on host → use 192.168.7.x)
//   4. Defaults                 (localhost)
inline GarudaConfig loadConfig()
{
    GarudaConfig cfg;

    const char* search_paths[] = {
        "/etc/garuda/garuda.conf",
        "./garuda.conf"
    };

    std::ifstream file;
    const char* loaded_from = nullptr;

    for (const char* path : search_paths) {
        file.open(path);
        if (file.is_open()) {
            loaded_from = path;
            break;
        }
    }

    if (!file.is_open()) {
        if (detectQemuTap()) {
            cfg.gcs_ip   = "192.168.7.1";
            cfg.drone_ip = "192.168.7.2";
            std::cout << "[Config] QEMU TAP detected. Using 192.168.7.x network.\n";
        } else {
            std::cout << "[Config] No QEMU TAP detected. Using defaults (localhost).\n";
        }
        return cfg;
    }

    std::cout << "[Config] Loaded from: " << loaded_from << "\n";

    auto trim = [](std::string& s) {
        const char* ws = " \t\r\n";
        size_t start = s.find_first_not_of(ws);
        size_t end   = s.find_last_not_of(ws);
        s = (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    };

    std::string line;
    while (std::getline(file, line)) {
        trim(line);
        if (line.empty() || line[0] == '#') continue;

        auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key   = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        trim(key);
        trim(value);

        if      (key == "GCS_IP")        cfg.gcs_ip         = value;
        else if (key == "DRONE_IP")      cfg.drone_ip       = value;
        else if (key == "TELEMETRY_PORT") cfg.telemetry_port = static_cast<uint16_t>(std::stoi(value));
        else if (key == "COMMAND_PORT")  cfg.command_port   = static_cast<uint16_t>(std::stoi(value));
    }

    return cfg;
}

#endif // GARUDA_CONFIG_HPP
