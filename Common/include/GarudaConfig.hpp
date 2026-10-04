#ifndef GARUDA_CONFIG_HPP
#define GARUDA_CONFIG_HPP

#include <string>
#include <fstream>
#include <cstdint>
#include <iostream>

/**
 * @struct GarudaConfig
 * @brief Network configuration for the GCS and Simulator.
 * Populated by loadConfig() using a priority-ordered search across config file paths
 * and localhost defaults.
 */
struct GarudaConfig {
    std::string gcs_ip         = "127.0.0.1"; ///< IP address of the Ground Control Station
    std::string drone_ip       = "127.0.0.1"; ///< IP address of the Simulator (drone)
    uint16_t    telemetry_port = 5001;        ///< UDP port the GCS listens on for telemetry
    uint16_t    command_port   = 5000;        ///< UDP port the Simulator listens on for commands
};

// Config resolution order:
//   1. /etc/garuda/garuda.conf  (system-wide config)
//   2. ./garuda.conf            (local override for development)
//   3. Defaults                 (localhost)
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
        std::cout << "[Config] No config file found. Using defaults (localhost).\n";
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
