# 🛸 Project Garuda: UAV Telemetry & Control Suite

[![Project Garuda CI](https://github.com/SurKM9/Project-Garuda/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/SurKM9/Project-Garuda/actions/workflows/ci.yml)

A high-performance, real-time UAV ground control suite. Features a simulated drone flight engine running on Custom Embedded Linux (Yocto Project) and a modern Qt 6 dashboard, communicating over a low-latency UDP protocol.

![Project Garuda Preview](./assets/preview.gif)
![Project Garuda Structure](./assets/architecture.png)
---

## 🚀 Project Overview

Project Garuda demonstrates the integration of low-level Linux systems programming with high-level data visualization. It solves the core challenges of modern robotics: handling asynchronous network data without blocking the user interface or sacrificing performance.

### The System at a Glance:
* **UAV Simulator:** A headless C++ service that simulates flight physics (altitude, velocity, attitude) and transmits rich telemetry at 10Hz over raw UDP sockets.
* **GCS Dashboard:** A multi-threaded Qt 6 application with real-time charts, a GPS map view, and an attitude indicator for live roll/pitch visualisation.
* **The Bridge:** A lock-free `SpscQueue<TelemetryPacket, 4>` (single-producer single-consumer ring buffer) moves packets from the network thread to the Qt main thread with zero mutex contention.

---

## 🛠 Tech Stack

| Component | Technology |
| :--- | :--- |
| **Language** | C++20 |
| **Framework** | Qt 6.6+ (Quick, Charts, Controls, Widgets) |
| **Networking** | BSD Raw Sockets (UDP / Non-blocking) |
| **Concurrency** | `std::thread`, lock-free SPSC ring buffer, `std::atomic` |
| **Build System** | CMake |
| **Environment** | Linux (Tested on Kubuntu 24.04 / Clang 18) |

---

## 🏗 Architecture

The project is structured into three distinct modules to ensure a clean separation of concerns:

1.  **`UAV_Common`**: The "Contract." Contains `TelemetryPacket` (altitude, velocity, roll, pitch, yaw, battery voltage, GPS, flight mode — 39 bytes, enforced by `static_assert`) and `CommandPacket`. Also provides `GarudaConfig`, a header-only config loader that resolves network settings from `/etc/garuda/garuda.conf`, a local `garuda.conf`, QEMU TAP auto-detection, or localhost defaults — in that order. Houses the lock-free `SpscQueue<T, N>` template.
2.  **`Simulator`**: The "Drone." Manages a 6-state flight FSM (IDLE → ARMED → TAKEOFF → FLYING → LANDING → EMERGENCY), a physics engine, and simulates attitude (pitch proportional to climb rate, yaw incrementing during flight). Validated by 22 GoogleTest unit tests.
3.  **`Dashboard`**: The "Ground Control." Background `std::thread` pushes packets into an SPSC queue; the Qt main thread drains it via `QMetaObject::invokeMethod`. UI includes real-time altitude chart, GPS map with flight path trail, and an attitude indicator (artificial horizon).
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
One-shot command to install the core development environment on Ubuntu/Kubuntu:

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

> **Note:** `QtLocation` and `QtPositioning` (required for the map view in the Dashboard)
> are **not available in Ubuntu's apt repositories**. Install them via the
> [Qt Online Installer](https://www.qt.io/download-qt-installer) — select
> **Qt 6.x → Qt Location** and **Qt Positioning** under the Desktop component.

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

* **Rich Telemetry:** `TelemetryPacket` carries altitude, velocity, GPS, roll, pitch, yaw, battery percentage, battery voltage, flight mode, and flight state — layout enforced at compile time with `static_assert`.
* **Lock-Free Concurrency:** A custom `SpscQueue<TelemetryPacket, 4>` replaces `std::mutex` in the telemetry pipeline — the network thread pushes, the Qt main thread pops, with no locks.
* **Attitude Indicator:** Artificial horizon rendered on a QML `Canvas` — horizon rotates with roll, shifts vertically with pitch.
* **GPS Map View:** Live drone position on an OpenStreetMap tile layer with a real-time flight path polyline.
* **Real-time Charts:** Auto-scrolling, auto-scaling altitude history chart via Qt Charts.
* **Bi-Directional Control:** Full-duplex UDP link — telemetry downlink on port 5001, command uplink on port 5000.
* **Flight Safety Logic:** Low-battery forced landing, critical-battery emergency stop, mid-air disarm rejection — all covered by 22 unit tests.

---

📖 API Documentation
This project uses Doxygen for API documentation.

To generate the docs, run make docs in the build folder.

Open build/html/index.html in any browser to view class hierarchies and call graphs.

📜 License
This project is licensed under the MIT License.
