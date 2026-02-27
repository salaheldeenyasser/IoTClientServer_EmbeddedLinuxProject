# recipes-core/images/iot-client-image.bb
# ─────────────────────────────────────────────────────────────────────────────
# Custom Yocto image for Raspberry Pi 5 running the IoT client.
# Based on core-image-minimal + networking + SSH + IoT client service.
#
# Build:   bitbake iot-client-image
# Flash:   sudo dd if=iot-client-image-raspberrypi5.rpi-sdimg of=/dev/sdX bs=4M status=progress
# ─────────────────────────────────────────────────────────────────────────────

require recipes-core/images/core-image-minimal.bb

SUMMARY     = "IoT Client image for Raspberry Pi 5 — Edges For Training"
LICENSE     = "MIT"

# ── Image features ────────────────────────────────────────────────────────────
IMAGE_FEATURES += " \
    ssh-server-openssh  \
    package-management  \
    debug-tweaks        \
"

# ── Extra packages ────────────────────────────────────────────────────────────
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

# ── Root filesystem size ──────────────────────────────────────────────────────
#IMAGE_ROOTFS_SIZE       ?= "524288"    # 512 MB minimum rootfs
#IMAGE_ROOTFS_EXTRA_SPACE = "524288"    # 512 MB extra headroom

# ── Filesystem types ──────────────────────────────────────────────────────────
#IMAGE_FSTYPES = "ext4 rpi-sdimg"

# ── Hostname ──────────────────────────────────────────────────────────────────
#hostname_pn-base-files = "iot-rpi5"

# ── Post-process: set root password (for debug-tweaks images only) ────────────

