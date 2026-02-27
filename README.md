# Embedded Linux IoT Communication System

A comprehensive embedded Linux IoT solution featuring a Qt6-based desktop GUI server and a lightweight C++ client application deployed on Raspberry Pi 5 via Yocto.

## Project Overview

This project is part of the **Embedded Linux Diploma Program** and demonstrates a complete IoT communication system with:
- Real-time data monitoring and visualization
- TCP/UDP networking protocols
- GPIO device control via temperature-based thresholds
- Custom Yocto layer for embedded Linux distribution
- Object-oriented C++ architecture with Socket/Channel hierarchy

## Directory Structure

```
EmbeddedLinuxFinalProject/
├── CommApp/
│   ├── CommAppQT/              # Qt6 Desktop Server Application
│   │   ├── main.cpp            # Application entry point
│   │   ├── mainwindow.h/cpp    # Main window implementation
│   │   ├── Channel.h           # Communication channel abstraction
│   │   ├── Socket.h            # Socket wrapper class
│   │   ├── Gauge.qml           # Circular gauge visualization
│   │   ├── CircularGauge.qml   # Gauge component
│   │   ├── mainwindow.ui       # UI layout definition
│   │   ├── Photos.qrc          # Resource file for icons/images
│   │   ├── CMakeLists.txt      # CMake build configuration
│   │   └── build/              # Build artifacts
│   │
│   └── CommAppYocto/            # Yocto Embedded Linux Configuration
│       └── yocto/poky/
│           ├── conf/            # Bitbake configuration
│           │   ├── bblayers.conf
│           │   └── local.conf
│           └── meta-myLayer/    # Custom Yocto layer
│               ├── recipes-connectivity/  # Network configuration
│               ├── recipes-core/          # Images and packagegroups
│               └── recipes-iot/           # IoT client application
│
└── Resources/                   # Images, icons, and assets
```

## Components

### 1. Qt6 Server Application (CommAppQT)

A feature-rich desktop application that serves as the central hub for IoT monitoring and control.

**Features:**
- **Real-Time Monitor Tab** - Displays sensor data on an interactive circular gauge
- **Historical Analysis Tab** - Shows scrolling line graph of the last 60 samples
- **Configuration Tab** - Adjust temperature threshold via slider control
- **Quick Access Tab** - Shortcut buttons to social media and external resources

**Networking:**
- TCP Server on port 8080 - Handles one client connection at a time
- UDP Socket on port 8081 - Ready for UDP protocol extension
- 1-second QTimer loop - Drives the communication protocol

**Technologies:**
- Qt 6.8.3
- C++17
- SQLite (for historical data)
- Qt Quick (QML)
- Qt Charts

### 2. IoT Client Application (CommAppYocto)

A lightweight command-line C++ client that runs on Raspberry Pi 5 and communicates with the server.

**Features:**
- Connects to the Qt6 server via TCP on port 8080
- Receives temperature threshold data from the server
- Controls GPIO LED output based on temperature conditions
- Object-oriented architecture with Socket/Channel abstraction
- Systemd service integration for automatic startup

**Specifications:**
- **Target Platform:** Raspberry Pi 5 (aarch64)
- **OS:** Custom Yocto Scarthgap (5.0) distribution
- **Build System:** CMake
- **Language:** C++17

### 3. Yocto Layer (meta-myLayer)

Custom Yocto meta-layer that packages the IoT client for embedded Linux deployment.

**Components:**
- **Network Configuration** - Ethernet setup via systemd-networkd
- **IoT Client Recipe** - BitBake recipe for compiling and deploying the client
- **Core Image** - Minimal Yocto image with IoT client pre-installed
- **Package Groups** - Curated collection of essential packages

## Installation & Setup

### Prerequisites

**For Qt6 Server:**
- Qt Creator 6.8+ or manual Qt installation
- GCC/Clang compiler supporting C++17
- CMake 3.20+
- Linux development libraries

**For Yocto Client:**
- Yocto Scarthgap (5.0) build system
- BitBake
- aarch64 cross-compiler toolchain
- Raspberry Pi 5 hardware

### Building the Qt6 Server

```bash
cd CommApp/CommAppQT
mkdir build && cd build
cmake ..
make
./untitled  # or run from Qt Creator
```

### Building the Yocto Distribution

```bash
cd CommApp/CommAppYocto/yocto/poky
source oe-init-build-env
bitbake iot-client-image
```

This generates a ready-to-flash image for Raspberry Pi 5.

## Communication Protocol

The system uses a request-response pattern:
1. **Client** connects to server via TCP port 8080
2. **Server** sends temperature threshold and monitoring parameters
3. **Client** reads sensor data and reports status
4. **Client** controls GPIO LED based on received threshold
5. **Server** displays real-time data in the GUI

## Architecture

### Object-Oriented Design

The codebase implements a clean abstraction hierarchy:
- **Socket Class** - Low-level networking wrapper (TCP/UDP)
- **Channel Class** - High-level communication abstraction
- **MainWindow** - Server GUI and business logic

### Data Flow

```
Raspberry Pi (IoT Client)
         ↓
    [Socket/Channel OOP]
         ↓
    TCP Server (Qt6 App)
         ↓
      [QTimer Loop]
         ↓
    [GUI Visualization]
    - Real-time Gauge
    - Historical Charts
    - Configuration Panel
```

## Key Features

✅ **Real-Time Monitoring** - Live sensor data visualization  
✅ **Data Logging** - Historical analysis with scrolling graphs  
✅ **Remote Control** - Temperature threshold configuration  
✅ **GPIO Integration** - Hardware LED control on embedded device  
✅ **Networking** - TCP/UDP communication protocols  
✅ **Embedded Linux** - Custom Yocto distribution for Raspberry Pi 5  
✅ **Systemd Service** - Background IoT client as Linux service  
✅ **Responsive UI** - Modern Qt Quick interface with QML  

## Files Overview

| File | Purpose |
|------|---------|
| `main.cpp` | Entry point for Qt6 application |
| `mainwindow.h/cpp` | Core application logic and GUI |
| `Channel.h` | Communication channel interface |
| `Socket.h` | TCP/UDP socket implementation |
| `Gauge.qml` | Circular gauge visualization component |
| `CircularGauge.qml` | Gauge styling and rendering |
| `iot-client_1.0.bb` | Yocto BitBake recipe for client |
| `CMakeLists.txt` | Build configuration for client application |

## License

MIT License - See individual files for license headers.

## Author

Edges For Training - Embedded Linux Diploma Program

## References

- [Qt Documentation](https://doc.qt.io/)
- [Yocto Project](https://www.yoctoproject.org/)
- [Raspberry Pi 5 Specifications](https://www.raspberrypi.com/products/raspberry-pi-5/)
- [systemd-networkd Documentation](https://www.freedesktop.org/wiki/Software/systemd/networkd/)

## Getting Help

For issues or questions:
1. Check the individual component READMEs in each subdirectory
2. Review the source code comments and documentation
3. Consult the official Qt and Yocto documentation
4. Contact Edges For Training through their website

---

*Last Updated: February 2026*