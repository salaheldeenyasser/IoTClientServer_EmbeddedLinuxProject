
SUMMARY     = "IoT Device Communication Client for Raspberry Pi 5"
DESCRIPTION = "C++ terminal client that connects to the Qt6 IoT server over \
TCP/UDP. Implements the Socket/Channel OOP hierarchy and controls a GPIO LED \
based on the temperature threshold received from the server."
AUTHOR      = "Edges For Training"
HOMEPAGE    = "https://www.edgesfortraining.com"
LICENSE     = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
    file://client_main.cpp        \
    file://Socket.h               \
    file://Channel.h              \
    file://CMakeLists.txt         \
    file://iot-client.conf        \
    file://iot-client.service     \
"

S = "${WORKDIR}"

inherit cmake systemd

EXTRA_OECMAKE = " \
    -DDEFAULT_SERVER_IP='192.168.1.100' \
    -DDEFAULT_SERVER_PORT=8080          \
    -DLED_GPIO=17                       \
"

TARGET_CC_ARCH:append = " -march=armv8.2-a+crypto -mtune=cortex-a76"

DEPENDS = ""
RDEPENDS:${PN} = "bash"

SYSTEMD_SERVICE:${PN} = "iot-client.service"
SYSTEMD_AUTO_ENABLE   = "enable"

do_install:append() {

    install -d ${D}${sysconfdir}/iot-client
    install -m 0644 ${WORKDIR}/iot-client.conf \
                    ${D}${sysconfdir}/iot-client/iot-client.conf

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/iot-client.service \
                    ${D}${systemd_system_unitdir}/iot-client.service
}

FILES:${PN} += " \
    ${bindir}/iot_client                        \
    ${sysconfdir}/iot-client/iot-client.conf    \
    ${systemd_system_unitdir}/iot-client.service \
"

FILES:${PN}-dbg += "${bindir}/.debug/iot_client"
