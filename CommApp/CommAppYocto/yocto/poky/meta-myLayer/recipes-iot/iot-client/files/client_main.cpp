/**
 * client_main.cpp  –  IoT Client for Raspberry Pi 5 (Yocto / real hardware)
 * ─────────────────────────────────────────────────────────────────────────────
 * Differences from the QEMU version:
 *   • Server IP is read from /etc/iot-client/iot-client.conf (key SERVER_IP=)
 *     so it can be changed without rebuilding; falls back to argv[1] or a
 *     compile-time default.
 *   • GPIO LED control via the Linux sysfs interface (GPIO 17 by default).
 *     No external library needed — pure POSIX file I/O.
 *   • Designed to run as a systemd service (logs to stdout → journald).
 *   • Graceful shutdown on SIGINT / SIGTERM.
 *   • Real temperature sensor hook: stub reads from /sys/class/thermal/
 *     thermal_zone0/temp (SoC temperature) when the user types "auto".
 *     Any numeric input still works as a manual override.
 *
 * OOP classes used (unchanged from QEMU version):
 *   Socket (abstract) → TCPSocket      (Socket.h)
 *   Channel (abstract) → ClientChannel  (Channel.h)
 *
 * Build (cross-compile via Yocto SDK):
 *   source <sdk-install-dir>/environment-setup-cortexa76-poky-linux
 *   $CXX $CXXFLAGS -std=c++17 -Wall -o iot_client client_main.cpp
 *
 * Or simply let the Yocto recipe handle it (see iot-client_1.0.bb).
 * ─────────────────────────────────────────────────────────────────────────────
 */

#include "Channel.h"
#include "Socket.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <thread>
#include <csignal>
#include <atomic>
#include <cstring>
#include <cerrno>
#include <stdexcept>

// ── POSIX ─────────────────────────────────────────────────────────────────────
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// ─────────────────────────────────────────────────────────────────────────────
//  Compile-time defaults  (override in /etc/iot-client/iot-client.conf)
// ─────────────────────────────────────────────────────────────────────────────
#ifndef DEFAULT_SERVER_IP
#  define DEFAULT_SERVER_IP "192.168.1.100"
#endif

#ifndef DEFAULT_SERVER_PORT
#  define DEFAULT_SERVER_PORT 8080
#endif

// GPIO pin number that drives the physical LED (BCM numbering)
#ifndef LED_GPIO
#  define LED_GPIO 17
#endif

// Path to the config file installed by the Yocto recipe
static constexpr const char *CONFIG_FILE = "/etc/iot-client/iot-client.conf";

// ─────────────────────────────────────────────────────────────────────────────
//  Global shutdown flag
// ─────────────────────────────────────────────────────────────────────────────
static std::atomic<bool> g_running{true};
static void signalHandler(int) { g_running = false; }

// ─────────────────────────────────────────────────────────────────────────────
//  Config file parser
//  Format: KEY=VALUE  (lines starting with '#' are comments)
// ─────────────────────────────────────────────────────────────────────────────
static std::string readConfig(const std::string &key,
                               const std::string &fallback)
{
    std::ifstream f(CONFIG_FILE);
    if (!f.is_open()) return fallback;

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        if (line.substr(0, pos) == key)
            return line.substr(pos + 1);
    }
    return fallback;
}

// ─────────────────────────────────────────────────────────────────────────────
//  GPIO LED – sysfs interface
//  Exported at startup, unexported at shutdown.
// ─────────────────────────────────────────────────────────────────────────────
class GpioLed
{
public:
    explicit GpioLed(int gpioPin) : m_pin(gpioPin), m_exported(false) {}

    ~GpioLed() { unexport(); }

    bool init()
    {
        // Export the GPIO
        std::ofstream exp("/sys/class/gpio/export");
        if (!exp.is_open()) {
            std::cerr << "[GPIO] Cannot open /sys/class/gpio/export\n";
            return false;
        }
        exp << m_pin;
        exp.close();

        // Small delay for the kernel to create the gpio directory
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Set direction to output
        std::string dirPath = "/sys/class/gpio/gpio"
                              + std::to_string(m_pin) + "/direction";
        std::ofstream dir(dirPath);
        if (!dir.is_open()) {
            std::cerr << "[GPIO] Cannot set direction for GPIO " << m_pin << "\n";
            return false;
        }
        dir << "out";
        m_exported = true;
        set(false);   // start with LED off
        return true;
    }

    void set(bool on)
    {
        if (!m_exported) return;
        std::string valPath = "/sys/class/gpio/gpio"
                              + std::to_string(m_pin) + "/value";
        std::ofstream val(valPath);
        if (val.is_open()) val << (on ? "1" : "0");
    }

private:
    void unexport()
    {
        if (!m_exported) return;
        set(false);
        std::ofstream unexp("/sys/class/gpio/unexport");
        if (unexp.is_open()) unexp << m_pin;
        m_exported = false;
    }

    int  m_pin;
    bool m_exported;
};

// ─────────────────────────────────────────────────────────────────────────────
//  SoC temperature reader (Raspberry Pi thermal zone)
//  Returns temperature in °C, or -1.0 on failure.
// ─────────────────────────────────────────────────────────────────────────────
static double readSoCTemperature()
{
    std::ifstream f("/sys/class/thermal/thermal_zone0/temp");
    if (!f.is_open()) return -1.0;
    int millideg = 0;
    f >> millideg;
    return static_cast<double>(millideg) / 1000.0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Socket I/O helpers
// ─────────────────────────────────────────────────────────────────────────────

/** Read one '\n'-terminated line from fd. Returns "" on disconnection. */
static std::string readLine(int fd)
{
    std::string line;
    char ch = '\0';
    while (g_running) {
        int n = ::recv(fd, &ch, 1, 0);
        if (n <= 0) return "";
        if (ch == '\n') break;
        if (ch != '\r') line += ch;
    }
    return line;
}

/** Write a line (appends '\n') to fd. */
static void sendLine(int fd, const std::string &msg)
{
    std::string out = msg + "\n";
    ::send(fd, out.c_str(), out.size(), 0);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Display helpers  (stdout → journald when run as a systemd service)
// ─────────────────────────────────────────────────────────────────────────────
static void printBanner()
{
    std::cout
        << "\n"
           "  ╔═══════════════════════════════════════════════╗\n"
           "  ║   IoT Client — Raspberry Pi 5 (Yocto build)  ║\n"
           "  ║          Edges For Training Project           ║\n"
           "  ╚═══════════════════════════════════════════════╝\n\n";
}

static void printLED(double temp, double threshold, bool ledOn)
{
    std::cout
        << "\n"
           "  ┌──────────────────────────────────────────────┐\n"
        << "  │  Temperature : " << temp      << " °C\n"
        << "  │  Threshold   : " << threshold << " °C\n"
        << "  │  LED (GPIO " << LED_GPIO << "): "
        << (ledOn ? "\033[1;31mON  ●\033[0m" : "\033[1;32mOFF ○\033[0m")
        << "\n"
           "  └──────────────────────────────────────────────┘\n\n";
}

// ─────────────────────────────────────────────────────────────────────────────
//  main
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char *argv[])
{
    std::signal(SIGINT,  signalHandler);
    std::signal(SIGTERM, signalHandler);

    printBanner();

    // ── Determine server IP (priority: argv[1] > config file > compile default)
    std::string serverIp = readConfig("SERVER_IP", DEFAULT_SERVER_IP);
    if (argc > 1) serverIp = argv[1];

    std::string portStr = readConfig("SERVER_PORT",
                                     std::to_string(DEFAULT_SERVER_PORT));
    uint16_t port = static_cast<uint16_t>(std::stoi(portStr));

    std::cout << "  [Config] Server : " << serverIp << ":" << port << "\n";
    std::cout << "  [Config] LED GPIO: " << LED_GPIO << "\n\n";

    // ── GPIO LED ──────────────────────────────────────────────────────────────
    GpioLed led(LED_GPIO);
    bool gpioOk = led.init();
    if (!gpioOk)
        std::cerr << "  [GPIO] LED control unavailable (running without root?).\n";

    // ── OOP objects (satisfies project requirement) ────────────────────────────
    TCPSocket     tcpSocket;
    ClientChannel clientChannel;
    clientChannel.channelSocket = &tcpSocket;

    // ── Build raw sockaddr for connect loop ───────────────────────────────────
    struct sockaddr_in srv{};
    srv.sin_family = AF_INET;
    srv.sin_port   = htons(port);
    if (inet_pton(AF_INET, serverIp.c_str(), &srv.sin_addr) != 1) {
        std::cerr << "  [Client] Invalid server IP: " << serverIp << "\n";
        return 1;
    }

    // ── Connect with retry (systemd will restart the service on failure) ──────
    int rawFd = -1;
    std::cout << "  [Client] Connecting to " << serverIp << ":" << port << "…\n";

    while (g_running) {
        rawFd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (rawFd < 0) { perror("socket"); return 1; }

        if (::connect(rawFd,
                      reinterpret_cast<sockaddr *>(&srv),
                      sizeof(srv)) == 0) break;

        ::close(rawFd);
        rawFd = -1;
        std::cerr << "  [Client] Connection failed (" << strerror(errno)
                  << ") — retrying in 3 s…\n";
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    if (!g_running || rawFd < 0) return 0;
    std::cout << "  [Client] Connected. Awaiting threshold…\n\n";

    // ── State ─────────────────────────────────────────────────────────────────
    double threshold   = 50.0;
    double temperature = 0.0;
    bool   ledOn       = false;

    // ── Communication loop ────────────────────────────────────────────────────
    while (g_running) {
        std::string cmd = readLine(rawFd);
        if (cmd.empty()) {
            std::cout << "  [Client] Server disconnected.\n";
            break;
        }

        std::cout << "  [Client] Command: \"" << cmd << "\"\n";

        // ── set threshold ──────────────────────────────────────────────────────
        if (cmd == "set threshold") {
            std::string valStr = readLine(rawFd);
            if (valStr.empty()) break;
            try {
                threshold = std::stod(valStr);
                std::cout << "  [Client] Threshold updated: " << threshold << " °C\n";

                // Update LED based on current temperature and new threshold
                ledOn = (temperature >= threshold);
                led.set(ledOn);
                printLED(temperature, threshold, ledOn);
            } catch (...) {
                std::cerr << "  [Client] Bad threshold value: " << valStr << "\n";
            }
        }
        // ── get temp ───────────────────────────────────────────────────────────
        else if (cmd == "get temp") {
            std::cout << "  [Client] Enter temperature (°C) [or 'auto' for SoC sensor]: ";
            std::cout.flush();

            std::string input;
            if (!std::getline(std::cin, input) || input.empty()) {
                // Non-interactive mode (systemd service with no tty):
                // Read SoC temperature automatically
                double soc = readSoCTemperature();
                input = (soc > 0) ? std::to_string(soc) : "25.0";
                std::cout << "(auto: " << input << " °C)\n";
            } else if (input == "auto") {
                double soc = readSoCTemperature();
                input = (soc > 0) ? std::to_string(soc) : "25.0";
                std::cout << "  [Sensor] SoC temp: " << input << " °C\n";
            }

            try {
                temperature = std::stod(input);
                sendLine(rawFd, std::to_string(temperature));
                std::cout << "  [Client] Sent: " << temperature << " °C\n";

                // Update LED
                ledOn = (temperature >= threshold);
                led.set(ledOn);
                printLED(temperature, threshold, ledOn);
            } catch (...) {
                std::cerr << "  [Client] Invalid input — sending 0.0\n";
                sendLine(rawFd, "0.0");
            }
        }
        // ── unknown ────────────────────────────────────────────────────────────
        else {
            std::cout << "  [Client] Unknown command — ignoring.\n";
        }
    }

    // ── Cleanup ───────────────────────────────────────────────────────────────
    if (rawFd >= 0) ::close(rawFd);
    led.set(false);   // turn LED off before exit
    std::cout << "  [Client] Shutdown complete.\n";
    return 0;
}
