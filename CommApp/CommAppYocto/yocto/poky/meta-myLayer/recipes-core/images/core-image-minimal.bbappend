# meta-iot-client/recipes-core/images/core-image-minimal.bbappend
# ─────────────────────────────────────────────────────────────────────────────
# Appends the IoT client package into core-image-minimal.
# This means you do NOT need to switch to iot-client-image.bb —
# just keep using:   bitbake core-image-minimal
# and our package will be included automatically.
# ─────────────────────────────────────────────────────────────────────────────

# Add the IoT client and its runtime dependencies
IMAGE_INSTALL:append = " \
    iot-client          \
    openssh-sftp-server \
    iproute2            \
"

IMAGE_FEATURES:append = " ssh-server-openssh"

DISTRO_FEATURES:append = " systemd"
VIRTUAL-RUNTIME_init_manager               = "systemd"
VIRTUAL-RUNTIME_initscripts               = "systemd-compat-units"
DISTRO_FEATURES_BACKFILL_CONSIDERED:append = " sysvinit"

