# recipes-iot/iot-client/iot-client_1.0.bb
# ─────────────────────────────────────────────────────────────────────────────
# BitBake recipe: IoT Client Application
# Targets : Raspberry Pi 5 (aarch64)
# Yocto   : Scarthgap (5.0)
# ─────────────────────────────────────────────────────────────────────────────

SUMMARY     = "IoT Device Communication Client for Raspberry Pi 5"
DESCRIPTION = "C++ terminal client that connects to the Qt6 IoT server over \
TCP/UDP. Implements the Socket/Channel OOP hierarchy and controls a GPIO LED \
based on the temperature threshold received from the server."
AUTHOR      = "Edges For Training"
HOMEPAGE    = "https://www.edgesfortraining.com"
LICENSE     = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# ── Source files ──────────────────────────────────────────────────────────────
# All source files live in the recipe's files/ subdirectory.
# In a real project, SRC_URI would point to a git repository:
#   SRC_URI = "git://github.com/edgesfortraining/iot-client.git;branch=main;protocol=https"
#   SRCREV  = "${AUTOREV}"
#   S       = "${WORKDIR}/git"
#
# For this local layer we use local files:
SRC_URI = " \
    file://client_main.cpp        \
    file://Socket.h               \
    file://Channel.h              \
    file://CMakeLists.txt         \
    file://iot-client.conf        \
    file://iot-client.service     \
"

# Source directory = the directory BitBake unpacks/copies files into
S = "${WORKDIR}"

# ── Build system ──────────────────────────────────────────────────────────────
inherit cmake systemd

# ── CMake configuration variables ────────────────────────────────────────────
# Change DEFAULT_SERVER_IP here to embed a different IP at build time.
# Users can also override at runtime via /etc/iot-client/iot-client.conf.
EXTRA_OECMAKE = " \
    -DDEFAULT_SERVER_IP='192.168.1.100' \
    -DDEFAULT_SERVER_PORT=8080          \
    -DLED_GPIO=17                       \
"

# ── Compile flags (cross-compile for aarch64 Cortex-A76 — Pi 5) ──────────────
TARGET_CC_ARCH:append = " -march=armv8.2-a+crypto -mtune=cortex-a76"

# ── Dependencies ──────────────────────────────────────────────────────────────
# The client uses only POSIX sockets — no extra runtime libraries needed.
DEPENDS = ""
RDEPENDS:${PN} = "bash"

# ── systemd service ───────────────────────────────────────────────────────────
SYSTEMD_SERVICE:${PN} = "iot-client.service"
SYSTEMD_AUTO_ENABLE   = "enable"

# ── Install ───────────────────────────────────────────────────────────────────
do_install:append() {
    # Binary is installed by cmake to ${D}${bindir} automatically.

    # Install the runtime config file
    install -d ${D}${sysconfdir}/iot-client
    install -m 0644 ${WORKDIR}/iot-client.conf \
                    ${D}${sysconfdir}/iot-client/iot-client.conf

    # Install the systemd unit
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/iot-client.service \
                    ${D}${systemd_system_unitdir}/iot-client.service
}

# ── Files included in the package ────────────────────────────────────────────
FILES:${PN} += " \
    ${bindir}/iot_client                        \
    ${sysconfdir}/iot-client/iot-client.conf    \
    ${systemd_system_unitdir}/iot-client.service \
"

# ── Debug package ─────────────────────────────────────────────────────────────
FILES:${PN}-dbg += "${bindir}/.debug/iot_client"
