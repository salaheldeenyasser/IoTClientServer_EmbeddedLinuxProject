# recipes-core/packagegroups/packagegroup-iot-client.bb
# ─────────────────────────────────────────────────────────────────────────────
# Package group that pulls in the IoT client and all its runtime dependencies.
# Include this in IMAGE_INSTALL instead of listing packages individually.
# ─────────────────────────────────────────────────────────────────────────────

SUMMARY     = "IoT Client package group for Raspberry Pi 5"
LICENSE     = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit packagegroup

PACKAGES = "${PN}"

RDEPENDS:${PN} = " \
    iot-client              \
    openssh-sftp-server     \
    iproute2                \
    iputils-ping            \
    procps                  \
    htop                    \
    nano                    \
    gpiod                   \
    gpiod-tools             \
    python3                 \
    python3-pip             \
"
