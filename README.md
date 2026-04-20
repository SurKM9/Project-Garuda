# 🛸 Project Garuda: UAV Telemetry & Control Suite

[![Project Garuda CI](https://github.com/SurKM9/Project-Garuda/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/SurKM9/Project-Garuda/actions/workflows/ci.yml)

A high-performance, real-time Distributed System designed for Unmanned Aerial Vehicle (UAV) ground control. This suite features a simulated drone flight engine running on Custom Embedded Linux (Yocto Project) and a modern graphical dashboard, communicating via a low-latency UDP protocol.

![Project Garuda Preview](./assets/preview.gif)
![Project Garuda Structure](./assets/architecture.png)
---

## 🚀 Project Overview

Project Garuda demonstrates the integration of low-level Linux systems programming with high-level data visualization. It solves the core challenges of modern robotics: handling asynchronous network data without blocking the user interface or sacrificing performance.

### The System at a Glance:
* **UAV Simulator:** A headless C++ service that simulates flight physics and transmits data at 10Hz using raw UDP sockets.
* **GCS Dashboard:** A multi-threaded Qt 6 application that visualizes telemetry trends and provides a Command & Control (C2) interface.
* **The Bridge:** A thread-safe C++ provider using Mutexes and Atomic flags to move data from network sockets to the QML engine safely.

---

## 🛠 Tech Stack

| Component | Technology |
| :--- | :--- |
| **Language** | C++20 |
| **Framework** | Qt 6.6+ (Quick, Charts, Controls, Widgets) |
| **Networking** | BSD Raw Sockets (UDP / Non-blocking) |
| **Concurrency** | `std::thread`, `std::mutex`, `std::atomic` |
| **Build System** | CMake |
| **Environment** | Linux (Tested on Kubuntu 24.04 / Clang 18) |

---

## 🏗 Architecture

The project is structured into three distinct modules to ensure a clean separation of concerns:

1.  **`UAV_Common`**: The "Contract." Contains memory-packed binary structures (`TelemetryPacket` and `CommandPacket`) to ensure bit-perfect compatibility between the Simulator and the Dashboard. Also provides `GarudaConfig`, a shared header-only config loader that resolves network settings from `/etc/garuda/garuda.conf`, a local `garuda.conf`, QEMU TAP auto-detection, or localhost defaults — in that order.
2.  **`Simulator`**: The "Drone." Manages the flight state machine and an asynchronous command listener. Implements graceful shutdown via Linux signal handling.
3.  **`Dashboard`**: The "Ground Control." Uses a dedicated background worker thread for networking to maintain a responsive 60FPS UI.
4.  **`meta-garuda`**: The Yocto Layer. Contains the BitBake recipes and configurations to build the Flight Controller into a production-ready Linux image.

---

## 🛰 SITL Networking Matrix

The system supports two primary modes of operation. Port 5000 is used for Command Uplink and Port 5001 is used for Telemetry Downlink.

| Environment | Mode | Host (GCS) IP | Drone IP |
| :--- | :--- | :--- | :--- |
| **Local Desktop** | Localhost | `127.0.0.1` | `127.0.0.1` |
| **Yocto / QEMU** | TAP Bridge | `192.168.7.1` | `192.168.7.2` |
| **Yocto / QEMU** | SLIRP (NAT) | `10.0.2.2` | `10.0.2.15` |

### Install All Dependencies
One-shot command to install the entire development environment on Ubuntu/Kubuntu:

```zsh
sudo apt update && sudo apt install -y \
          build-essential \
          cmake \
          qt6-base-dev \
          qt6-declarative-dev \
          qt6-charts-dev \
          libqt6charts6-dev \
          libxkbcommon-dev \
          libgl1-mesa-dev \
          libvulkan-dev \
          libboost-all-dev \
          doxygen \
          graphviz
```

## 🏗 Build Instructions

### Native Desktop Build (Kubuntu)
```zsh
mkdir build && cd build
cmake .. -DBUILD_DASHBOARD=ON -DBUILD_SIMULATOR=ON
make -j$(nproc)
```

### Yocto Embedded Build (Poky)
- To build the Project Garuda Linux distribution:
- Initialize Environment: source oe-init-build-env
- Add Layer: bitbake-layers add-layer /path/to/UAV_System/meta-garuda
- Build Image: bitbake core-image-minimal

### ⚡ Running the System

1. The Drone (Inside QEMU)
Launch the virtual machine. For TAP networking:

```zsh
runqemu qemux86-64
```

Once booted, run the pre-configured launch script:

```zsh
launch-drone  # Automatically connects to the host gateway at 192.168.7.1
```

2. The Dashboard (Native Host)
Run the dashboard on your host machine. It automatically detects whether QEMU TAP networking is active and connects to `192.168.7.2`. If QEMU is not running, it falls back to localhost.

```zsh
./Dashboard/DashboardApp
```

A CLI argument overrides auto-detection for one-off use:
```zsh
./Dashboard/DashboardApp 192.168.7.2
```

## 📈 Key Features

* **Real-time Data Visualization:** Dynamic, auto-scaling line charts using Qt Charts for altitude history.
* **Bi-Directional Communication:** Full-duplex UDP link for telemetry downlink (Port 5001) and command uplink (Port 5000).
* **Thread Safety:** Robust data protection using `std::lock_guard` to prevent race conditions.
* **Resource Management:** Follows RAII principles and controlled object destruction order.

---

📖 API Documentation
This project uses Doxygen for API documentation.

To generate the docs, run make docs in the build folder.

Open build/html/index.html in any browser to view class hierarchies and call graphs.

📜 License
This project is licensed under the MIT License.
