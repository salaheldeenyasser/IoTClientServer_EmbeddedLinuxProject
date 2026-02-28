
require recipes-core/images/core-image-minimal.bb

SUMMARY     = "IoT Client image for Raspberry Pi 5 — Edges For Training"
LICENSE     = "MIT"

IMAGE_FEATURES += " \
    ssh-server-openssh  \
    package-management  \
    debug-tweaks        \
"

IMAGE_INSTALL:append = " \
    packagegroup-iot-client     \
    kernel-modules              \
    linux-firmware-rpidistro    \
    raspberrypi-firmware        \
    rpi-gpio                    \
    python3-rpi-gpio            \
    ntp                         \
    ntpdate                     \
    tzdata                      \
    bash                        \
    coreutils                   \
    findutils                   \
    grep                        \
    sed                         \
    systemd-analyze             \
    journalctl                  \
"





