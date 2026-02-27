# Embedded Linux IoT Communication System

A comprehensive embedded Linux IoT solution featuring a Qt6-based desktop GUI server and a lightweight C++ terminal client deployed on Raspberry Pi 5 via Yocto. The project demonstrates modern C++ OOP principles, POSIX socket programming, and embedded Linux system integration.

## Project Overview

This is an **Embedded Linux Diploma Program** capstone project showcasing:
- **Custom OOP Networking** - POSIX sockets abstraction (Socket.h / Channel.h) without Qt networking libraries
- **Real-time GUI Monitoring** - Qt6 desktop application with live data visualization
- **Embedded Deployment** - Custom Yocto Scarthgap (5.0) image for Raspberry Pi 5
- **GPIO Hardware Control** - LED control via Linux sysfs interface based on temperature thresholds
- **Systemd Integration** - IoT client runs as automatic background service
- **Configuration Management** - Runtime configuration files for server IP and GPIO settings

## Architecture

### High-Level Overview

```
Raspberry Pi 5 (IoT Client)        Desktop / Laptop (Qt6 Server)
┌─────────────────────────┐        ┌──────────────────────────┐
│  Yocto Linux (aarch64)  │        │   Ubuntu/Fedora (x86-64) │
│                         │        │                          │
│  client_main.cpp        │◄─TCP──►│  mainwindow.cpp          │
│  ├─ Socket.h (POSIX)    │ :8080  │  ├─ Socket.h (POSIX)     │
│  ├─ Channel.h (OOP)     │        │  ├─ Channel.h (OOP)      │
│  └─ GPIO LED control    │        │  ├─ Gauge.qml            │
│                         │        │  ├─ Qt Charts            │
│  [systemd service]      │        │  └─ Configuration UI     │
└─────────────────────────┘        └──────────────────────────┘
```

### Networking Stack

**Custom Implementation** - No Qt networking module used:
- **Socket.h** - Abstract base class with TCPSocket and UDPSocket implementations
- **Channel.h** - Communication abstraction (ServerChannel / ClientChannel)
- **POSIX API** - Direct use of `socket()`, `bind()`, `listen()`, `accept()`, `connect()`, `send()`, `recv()`
- **Qt integration** - `QSocketNotifier` bridges POSIX file descriptors into Qt event loop

## Directory Structure

```
EmbeddedLinuxFinalProject/
├── CommApp/
│   ├── CommAppQT/                          # Qt6 Desktop Server
│   │   ├── main.cpp                        # Entry point
│   │   ├── mainwindow.{h,cpp,ui}          # Main GUI window + 4 tabs
│   │   ├── Socket.h                        # TCP/UDP socket classes (POSIX)
│   │   ├── Channel.h                       # Communication abstraction layer
│   │   ├── Gauge.qml                       # Circular temperature gauge (Qt Quick)
│   │   ├── CircularGauge.qml               # Gauge component styling
│   │   ├── Photos.qrc                      # Resource file (icons, images, QML)
│   │   ├── CMakeLists.txt                  # Qt6 build configuration
│   │   ├── Makefile                        # Qt Creator generated
│   │   ├── Imagesandicons/                 # Assets directory
│   │   └── build/                          # Build artifacts
│   │       └── Desktop_Qt_6_8_3-Debug/
│   │
│   └── CommAppYocto/                       # Yocto Embedded Linux Layer
│       └── yocto/poky/
│           ├── conf/
│           │   ├── bblayers.conf           # Included Yocto layers
│           │   ├── local.conf              # Build configuration
│           │   └── templateconf.cfg
│           │
│           └── meta-myLayer/               # Custom Yocto meta-layer
│               │
│               ├── recipes-connectivity/
│               │   └── network-config/
│               │       ├── network-config_1.0.bb
│               │       └── files/
│               │           └── 10-eth0.network   # Systemd-networkd config
│               │
│               ├── recipes-core/
│               │   ├── images/
│               │   │   ├── core-image-minimal.bbappend
│               │   │   └── iot-client-image.bb   # Custom Yocto image
│               │   │
│               │   └── packagegroups/
│               │       └── packagegroup-iot-client.bb  # Package group
│               │
│               ├── recipes-iot/
│               │   └── iot-client/
│               │       ├── iot-client_1.0.bb    # Main BitBake recipe
│               │       └── files/
│               │           ├── client_main.cpp
│               │           ├── Socket.h
│               │           ├── Channel.h
│               │           ├── CMakeLists.txt
│               │           ├── iot-client.conf   # Runtime config
│               │           └── iot-client.service # Systemd unit
│               │
│               ├── recipes-example/
│               │   └── example/
│               │       └── example_0.1.bb
│               │
│               ├── COPYING.MIT
│               ├── README
│               └── conf/
│                   └── layer.conf
│
└── Resources/
    └── Images and icons/
```

## Components

### 1. **Qt6 Server (CommAppQT)** - Desktop GUI Application

A feature-rich graphical interface that runs on a desktop/laptop and acts as the central IoT hub.

#### Four-Tab Interface

1. **Real-Time Monitor Tab**
   - Interactive circular gauge showing current temperature
   - Color-coded: green (below threshold) ↔ red (at/above threshold)
   - Live updates every 1 second
   - Custom Qt Quick QML implementation (Gauge.qml)

2. **Historical Analysis Tab**
   - Scrolling line graph displaying last 60 temperature samples
   - Two series: actual temperature + threshold line
   - Qt Charts library with value axes
   - Real-time graph updates without blocking UI

3. **Configuration Tab**
   - Horizontal slider to adjust temperature threshold (0–100°C)
   - LCD number display of current threshold
   - TCP/UDP radio buttons (mutual exclusion)
   - **Connect/Disconnect** button for client management

4. **Quick Access Tab**
   - Shortcut buttons for social media and external resources
   - Uses `QDesktopServices::openUrl()` for browser integration

#### Technical Details

- **Build System:** CMake 3.16+ with Qt6 integration
- **Qt Modules Used:** 
  - `Qt6::Widgets` - Main UI framework
  - `Qt6::Charts` - Historical data visualization
  - `Qt6::Quick` - QML engine
  - `Qt6::QuickWidgets` - Embed QML inside Qt Widgets
  - `Qt6::Qml` - QML context properties
  - ⚠️ **NOT used:** `Qt6::Network` (custom Socket.h/Channel.h instead)

- **C++ Standard:** C++17
- **Listening Port:** TCP 8080 (one client at a time)
- **Protocol Timer:** 1-second `QTimer` for periodic data exchange
- **GUI Threading:** `QSocketNotifier` ensures non-blocking network I/O

### 2. **IoT Client (CommAppYocto)** - Embedded Terminal Application

A lightweight command-line C++ application that runs on Raspberry Pi 5 as a systemd service.

#### Features

- **Networking:** Connects to Qt6 server via TCP on port 8080
- **Configuration:** Reads server IP from `/etc/iot-client/iot-client.conf` (with fallback)
- **Temperature Sensing:** 
  - Manual input override (user types numeric values)
  - Automatic reading from `/sys/class/thermal/thermal_zone0/temp` (SoC temperature)
- **GPIO LED Control:**
  - Uses Linux sysfs interface: `/sys/class/gpio/gpio{N}/`
  - Default GPIO: 17 (configurable via CMake `-DLED_GPIO=`)
  - No external GPIO libraries needed
- **Systemd Service:**
  - Auto-start on boot
  - Automatic restart on failure (5-second retry)
  - Logging to systemd journal

#### Build & Deployment

- **Build System:** CMake with cross-compilation support
- **Target:** aarch64 (ARM Cortex-A76) Raspberry Pi 5
- **Yocto Integration:** BitBake recipe with CMake inheritance
- **Compilation Flags:** `-march=armv8.2-a+crypto -mtune=cortex-a76`
- **Dependencies:** Only POSIX (no external libraries)
- **Output Binary:** `/usr/bin/iot_client`

#### Configuration File

Location: `/etc/iot-client/iot-client.conf`

```properties
SERVER_IP=192.168.1.100
SERVER_PORT=8080
# LED_GPIO is set at compile-time
```

#### Systemd Service File

File: `/etc/systemd/system/iot-client.service`
- Type: `simple`
- User: `root` (for GPIO access)
- Restart: `on-failure` (with 5-second backoff)
- Logging: Direct to systemd journal

### 3. **Yocto Meta-Layer (meta-myLayer)** - Embedded Linux Customization

Custom BitBake configuration for Raspberry Pi 5 image generation.

#### Image Specification

**Target:** `iot-client-image.bb`

Base: `core-image-minimal` + custom packages

Included Packages:
- `iot-client` - The binary application
- `openssh-sftp-server` - Remote file transfer
- Network utilities: `iproute2`, `iputils-ping`
- System tools: `procps`, `htop`, `nano`
- GPIO tools: `gpiod`, `gpiod-tools`
- Python stack: `python3`, `python3-pip`
- NTP services: `ntp`, `ntpdate`, `tzdata`
- Firmware: `linux-firmware-rpidistro`, `raspberrypi-firmware`, `rpi-gpio`

**Features:**
- SSH server (OpenSSH)
- Package management support
- Debug tweaks for troubleshooting
- Kernel modules and firmware for Raspberry Pi 5

#### Package Group

**Target:** `packagegroup-iot-client.bb`

Centralizes all client dependencies for easy inclusion in images.

#### Network Configuration

**File:** `recipes-connectivity/network-config/10-eth0.network`

Systemd-networkd configuration for Ethernet (static or DHCP).

## Object-Oriented Design (Socket/Channel Hierarchy)

### Class Diagram

```
Socket (abstract base)
├── TCPSocket : public Socket
│   ├── waitForConnect() → server-side accept
│   ├── connect()       → client-side connect
│   └── fd()            → return listening or connected socket fd
│
└── UDPSocket : public Socket
    ├── waitForConnect() → server-side bind
    ├── connect()        → client-side bind
    └── fd()             → return udp socket fd

Channel (abstract base)
├── ServerChannel : public Channel
│   ├── channelSocket : Socket*  (owns Socket)
│   ├── startListening() → calls socket→waitForConnect()
│   └── fd()             → forwarded from Socket
│
└── ClientChannel : public Channel
    ├── channelSocket : Socket*  (owns Socket)
    ├── start()         → calls socket→connect()
    └── fd()            → forwarded from Socket
```

### Key Design Patterns

- **Abstraction** - Pure virtual interfaces (`Socket`, `Channel`)
- **Inheritance** - Concrete implementations (`TCPSocket`, `UDPSocket`, `ServerChannel`, `ClientChannel`)
- **Composition** - `Channel` owns `Socket*` for flexibility
- **Polymorphism** - Runtime type selection via pointers
- **POSIX File Descriptors** - Exposed via `fd()` for Qt integration

## Communication Protocol

### Connection Establishment (TCP)

1. **Server startup:**
   - `TCPSocket::waitForConnect()` → `socket()`, `bind()`, `listen()`
   - `QSocketNotifier` watches listening fd for incoming connections
   - GUI shows "Waiting for client..."

2. **Client connection:**
   - `TCPSocket::connect()` → `socket()`, `connect()` to server IP:8080
   - Client enters interactive temperature input loop

3. **Server accepts client:**
   - `onListenFdActivated()` callback fires when client connects
   - `accept()` creates connected socket fd
   - `QSocketNotifier` switches to watch connected fd for data
   - GUI updates threshold value from config and starts 1-second timer

### Data Exchange (Per-Second Loop)

Server (MainWindow::onServerTick):
1. Read incoming temperature from client socket
2. Update m_temperature property → QML gauge updates
3. Check if threshold changed (slider) → send new value to client
4. Record sample to historical series (capacity: 60 samples)
5. Repeat in 1 second

Client (client_main.cpp loop):
1. Receive current threshold from server
2. Read temperature (manual or SoC sensor)
3. Control GPIO LED based on threshold comparison
4. Send temperature back to server
5. Repeat in ~1 second

### Disconnection

- Client closes socket → `onClientFdReadable()` detects EOF
- Disarm notifier, show GUI status labels
- Ready to accept next client connection

## Building & Deployment

### Building the Qt6 Server (Desktop)

```bash
cd CommApp/CommAppQT
mkdir build && cd build
cmake ..                    # Qt6 CMake integration
cmake --build .             # or: make
./IoTServer                 # Run the executable
```

**Tested on:**
- Ubuntu 22.04 LTS with Qt6
- Fedora 39+ with Qt6
- macOS with Qt6 (Qt Creator)

### Building the Yocto Image (Embedded)

```bash
cd CommApp/CommAppYocto/yocto/poky
source oe-init-build-env    # Initialize Yocto build environment
bitbake iot-client-image    # Build the full image
```

**Output:** `tmp/deploy/images/raspberrypi5/iot-client-image-raspberrypi5.rpi-sdimg`

### Flashing to Raspberry Pi 5

```bash
# Linux/macOS
sudo dd if=iot-client-image-raspberrypi5.rpi-sdimg \
         of=/dev/sdX bs=4M status=progress conv=fsync
        
# Where /dev/sdX is your SD card (check with lsblk first)
```

### Post-Boot Configuration

On Raspberry Pi 5:
1. Edit `/etc/iot-client/iot-client.conf` to set correct server IP
2. Restart service: `systemctl restart iot-client`
3. Monitor logs: `journalctl -u iot-client -f`

## Key Features & Design Decisions

✅ **No Qt Networking Module** - Custom POSIX sockets for learning/control  
✅ **Non-blocking GUI** - QSocketNotifier integrates I/O into Qt event loop  
✅ **Header-only OOP** - Socket.h and Channel.h as template-like abstractions  
✅ **Real-time Visualization** - 1-second updates with smooth gauge + charts  
✅ **GPIO via sysfs** - No kernel drivers needed; pure POSIX file I/O  
✅ **Configuration Files** - Runtime changes without recompilation  
✅ **Systemd Integration** - Industrial-grade service management  
✅ **Cross-platform Build** - Qt Creator projects work on Linux/macOS/Windows  
✅ **Yocto Best Practices** - Proper recipe, image, and layer organization  
✅ **Educational Value** - Extensive inline documentation and clear OOP hierarchy  

## File Reference

| File | Location | Purpose |
|------|----------|---------|
| `main.cpp` | CommAppQT/ | Qt application entry point |
| `mainwindow.{h,cpp,ui}` | CommAppQT/ | Main window, 4 tabs, protocol loop |
| `Socket.h` | CommAppQT/ | TCP/UDP POSIX socket classes |
| `Channel.h` | CommAppQT/ | Communication abstraction layer |
| `Gauge.qml` | CommAppQT/ | Custom circular gauge (Qt Quick) |
| `CircularGauge.qml` | CommAppQT/ | Gauge styling component |
| `Photos.qrc` | CommAppQT/ | Resource file (images, QML, icons) |
| `CMakeLists.txt` | CommAppQT/ | Qt6 build system |
| `client_main.cpp` | CommAppYocto/.../files/ | IoT client main loop |
| `iot-client_1.0.bb` | CommAppYocto/.../recipes-iot/ | BitBake recipe |
| `iot-client-image.bb` | CommAppYocto/.../recipes-core/images/ | Yocto image definition |
| `packagegroup-iot-client.bb` | CommAppYocto/.../packagegroups/ | Package dependencies |
| `iot-client.conf` | CommAppYocto/.../files/ | Runtime config template |
| `iot-client.service` | CommAppYocto/.../files/ | Systemd unit file |

## Technology Stack

### Server (Qt6)
- **Language:** C++17
- **Framework:** Qt 6.8.3
- **GUI:** Qt Widgets + Qt Quick (QML)
- **Visualization:** Qt Charts
- **Networking:** POSIX sockets (custom abstraction)
- **Platform:** Linux, macOS, Windows

### Client (Embedded)
- **Language:** C++17
- **Platform:** Linux (Yocto Scarthgap 5.0)
- **Target Board:** Raspberry Pi 5 (aarch64)
- **Build System:** CMake + Yocto BitBake
- **Hardware Interface:** Linux sysfs (GPIO)
- **Sensors:** Thermal zone (SoC temperature)
- **Service Manager:** systemd

### Build & Deployment
- **Qt Tools:** Qt Creator 6.8+, CMake 3.16+
- **Embedded:** Yocto Scarthgap (5.0), BitBake, poky
- **Toolchain:** aarch64 GCC with C++17 support
- **Cross-compilation:** Yocto SDK

## Learning Outcomes

This capstone project demonstrates proficiency in:

1. **Modern C++** - OOP (abstraction, inheritance, polymorphism), POSIX APIs
2. **GUI Development** - Qt6 widgets, Qt Quick/QML, event-driven architecture
3. **Embedded Systems** - GPIO control, sysfs, systemd services
4. **Networking** - Socket programming, TCP/UDP, non-blocking I/O
5. **Linux Distributions** - Yocto/BitBake, custom recipes, cross-compilation
6. **Desktop Applications** - Multi-threaded UI, real-time data visualization
7. **Software Engineering** - Clean architecture, separation of concerns, documentation

## References

- [Qt 6 Documentation](https://doc.qt.io/qt-6/)
- [Qt Quick (QML) Guide](https://doc.qt.io/qt-6/qtquick-index.html)
- [POSIX Socket API](https://www.man7.org/linux/man-pages/man2/socket.2.html)
- [Yocto Project Manual](https://docs.yoctoproject.org/)
- [Raspberry Pi 5 Specifications](https://www.raspberrypi.com/products/raspberry-pi-5/)
- [systemd Documentation](https://www.freedesktop.org/wiki/Software/systemd/)
- [Linux GPIO via sysfs](https://www.kernel.org/doc/html/latest/userspace-api/gpio/sysfs.html)

## License

**MIT License**

All source files include MIT license headers. See [COPYING.MIT](CommApp/CommAppYocto/yocto/poky/meta-myLayer/COPYING.MIT) in the Yocto layer.

## Author & Attribution

**Edges For Training** - Embedded Linux Diploma Program

---

*Last Updated: February 28, 2026*