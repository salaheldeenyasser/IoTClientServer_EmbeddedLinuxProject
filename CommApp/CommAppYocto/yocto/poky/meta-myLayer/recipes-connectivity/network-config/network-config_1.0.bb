# recipes-connectivity/network-config/network-config_1.0.bb
# ─────────────────────────────────────────────────────────────────────────────
# Installs a static IP configuration for eth0 via systemd-networkd.
# Edit files/10-eth0.network before building to set your lab network address.
# ─────────────────────────────────────────────────────────────────────────────

SUMMARY     = "Static network configuration for Raspberry Pi 5 IoT client"
LICENSE     = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://10-eth0.network"
S = "${WORKDIR}"

inherit allarch

do_install() {
    install -d ${D}${sysconfdir}/systemd/network
    install -m 0644 ${WORKDIR}/10-eth0.network \
                    ${D}${sysconfdir}/systemd/network/10-eth0.network
}

FILES:${PN} += "${sysconfdir}/systemd/network/10-eth0.network"

# Enable systemd-networkd in the image
RRECOMMENDS:${PN} = "systemd-networkd"
