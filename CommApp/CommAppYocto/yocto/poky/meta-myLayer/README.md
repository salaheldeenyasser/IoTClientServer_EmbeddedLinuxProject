# meta-myLayer — IoT Client Yocto Layer

**Author:** SalahEldeen Yasser  
**Target:** Raspberry Pi 5 (aarch64, Cortex-A76)  
**Yocto release:** Scarthgap (5.0)

---

## Layer structure

```
meta-myLayer/
├── conf/
│   └── layer.conf
├── recipes-core/
│   └── images/
│       └── core-image-minimal.bbappend
└── recipes-myApp/
    └── iot-client/
        ├── recipe-myApp.bb
        └── files/
            ├── main.cpp
            ├── Socket.h
            ├── Channel.h
            └── CMakeLists.txt
```

---

## Prerequisites

Install the host dependencies required by Yocto on Ubuntu 22.04 / 24.04:

```bash
sudo apt update
sudo apt install -y \
    gawk wget git diffstat unzip texinfo gcc build-essential \
    chrpath socat cpio python3 python3-pip python3-pexpect \
    xz-utils debianutils iputils-ping python3-git python3-jinja2 \
    python3-subunit zstd liblz4-tool file locales libacl1
sudo locale-gen en_US.UTF-8
```

Minimum free disk space: **100 GB**.

---

## Setup

### 1. Clone Poky and meta-raspberrypi

```bash
mkdir ~/yocto && cd ~/yocto

git clone -b scarthgap https://git.yoctoproject.org/poky
git clone -b scarthgap https://git.yoctoproject.org/meta-raspberrypi
```

### 2. Copy this layer into the workspace

```bash
cp -r /path/to/meta-myLayer ~/yocto/meta-myLayer
```

### 3. Initialise the build environment

```bash
cd ~/yocto
source poky/oe-init-build-env rpi-build
```

This creates `~/yocto/rpi-build/` and drops you inside it.

### 4. Add layers to bblayers.conf

Edit `~/yocto/rpi-build/conf/bblayers.conf` so `BBLAYERS` contains:

```
BBLAYERS ?= " \
  /home/<user>/yocto/poky/meta \
  /home/<user>/yocto/poky/meta-poky \
  /home/<user>/yocto/poky/meta-yocto-bsp \
  /home/<user>/yocto/meta-raspberrypi \
  /home/<user>/yocto/meta-myLayer \
"
```

Replace `<user>` with your actual username.

### 5. Configure local.conf

Edit `~/yocto/rpi-build/conf/local.conf` and set / add these lines:

```
MACHINE = "raspberrypi5"

DISTRO_FEATURES:append = " systemd"
VIRTUAL-RUNTIME_init_manager = "systemd"
VIRTUAL-RUNTIME_initscripts = "systemd-compat-units"
DISTRO_FEATURES_BACKFILL_CONSIDERED:append = " sysvinit"

IMAGE_FEATURES:append = " ssh-server-openssh"

ENABLE_UART = "1"
```

---

## Build

```bash
cd ~/yocto/rpi-build
bitbake core-image-minimal
```

The finished image will be at:

```
tmp/deploy/images/raspberrypi5/core-image-minimal-raspberrypi5.rpi-sdimg
```

---

## Flash

```bash
sudo dd \
  if=tmp/deploy/images/raspberrypi5/core-image-minimal-raspberrypi5.rpi-sdimg \
  of=/dev/sdX \
  bs=4M status=progress
sync
```

Replace `/dev/sdX` with your SD card device.

---

## Hardware wiring

| Signal | Pi 5 header pin | BCM GPIO |
|--------|----------------|----------|
| LED +  | Pin 11         | GPIO 17  |
| GND    | Pin 9          | GND      |

Connect a 330 Ω resistor in series with the LED anode to GPIO 17, cathode to GND.

---

## Running the application

The binary is installed at `/usr/bin/iot-client`.

### Default (TCP, server at 192.168.1.100, GPIO 17)

```bash
iot-client
```

### Select protocol and server IP

```bash
iot-client --proto tcp --ip 192.168.1.100 --gpio 17
iot-client --proto udp --ip 192.168.1.100 --gpio 17
```

### All options

| Option | Default | Description |
|--------|---------|-------------|
| `--proto tcp\|udp` | `tcp` | Communication protocol |
| `--ip <address>` | `192.168.1.100` | Server IP address |
| `--gpio <pin>` | `17` | BCM GPIO pin for the LED |
| `--help` | | Print usage and exit |

### What the display shows

```
IoT Client
==========
Temperature : 42.3 C
Threshold   : 50.0 C
LED Status  : OFF
```

The display refreshes on every `get temp` cycle (every 1 second driven by the server).

---

## Protocol

| Direction | Message |
|-----------|---------|
| Server → Client | `set threshold\n` followed by `<value>\n` |
| Server → Client | `get temp\n` |
| Client → Server | `<temperature_float>\n` |

Temperature is read from `/sys/class/thermal/thermal_zone0/temp` (SoC internal sensor). The LED turns ON when temperature ≥ threshold.
