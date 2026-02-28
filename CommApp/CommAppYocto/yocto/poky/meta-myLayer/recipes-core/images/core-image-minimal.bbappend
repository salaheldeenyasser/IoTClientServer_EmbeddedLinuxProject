
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

