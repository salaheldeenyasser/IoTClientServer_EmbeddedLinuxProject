SUMMARY = "IoT Client"
AUTHOR  = "SalahEldeen Yasser"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
    file://main.cpp        \
    file://Socket.h        \
    file://Channel.h       \
    file://CMakeLists.txt  \
    file://iot-client.service \
"

S = "${WORKDIR}"

# FIX (Bug 2): inherit systemd so Yocto generates the proper enable/preset
# hooks. Without this the service file is never activated and the binary
# never starts on boot regardless of IMAGE_INSTALL.
inherit cmake systemd

EXTRA_OECMAKE = ""

# FIX (Bug 8): declare the runtime C++ library dependency explicitly.
# core-image-minimal does not guarantee libstdc++ is present; without this
# RDEPENDS the binary can fail at startup with "error while loading shared
# libraries: libstdc++.so.6".
RDEPENDS:${PN} = "libstdc++"

# FIX (Bug 2): tell the systemd class which unit to install and enable.
SYSTEMD_SERVICE:${PN} = "iot-client.service"
SYSTEMD_AUTO_ENABLE    = "enable"

do_install:append() {
    # Install the systemd unit into the correct system unit directory.
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/iot-client.service \
                    ${D}${systemd_system_unitdir}/iot-client.service
}

FILES:${PN} = " \
    ${bindir}/iot-client \
    ${systemd_system_unitdir}/iot-client.service \
"
