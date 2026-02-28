


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


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>




#ifndef DEFAULT_SERVER_IP
#  define DEFAULT_SERVER_IP "192.168.1.100"
#endif

#ifndef DEFAULT_SERVER_PORT
#  define DEFAULT_SERVER_PORT 8080
#endif


#ifndef LED_GPIO
#  define LED_GPIO 17
#endif


static constexpr const char *CONFIG_FILE = "/etc/iot-client/iot-client.conf";




static std::atomic<bool> g_running{true};
static void signalHandler(int) { g_running = false; }





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





class GpioLed
{
public:
    explicit GpioLed(int gpioPin) : m_pin(gpioPin), m_exported(false) {}

    ~GpioLed() { unexport(); }

    bool init()
    {
        
        std::ofstream exp("/sys/class/gpio/export");
        if (!exp.is_open()) {
            std::cerr << "[GPIO] Cannot open /sys/class/gpio/export\n";
            return false;
        }
        exp << m_pin;
        exp.close();

        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        
        std::string dirPath = "/sys/class/gpio/gpio"
                              + std::to_string(m_pin) + "/direction";
        std::ofstream dir(dirPath);
        if (!dir.is_open()) {
            std::cerr << "[GPIO] Cannot set direction for GPIO " << m_pin << "\n";
            return false;
        }
        dir << "out";
        m_exported = true;
        set(false);   
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





static double readSoCTemperature()
{
    std::ifstream f("/sys/class/thermal/thermal_zone0/temp");
    if (!f.is_open()) return -1.0;
    int millideg = 0;
    f >> millideg;
    return static_cast<double>(millideg) / 1000.0;
}






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


static void sendLine(int fd, const std::string &msg)
{
    std::string out = msg + "\n";
    ::send(fd, out.c_str(), out.size(), 0);
}




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




int main(int argc, char *argv[])
{
    std::signal(SIGINT,  signalHandler);
    std::signal(SIGTERM, signalHandler);

    printBanner();

    
    std::string serverIp = readConfig("SERVER_IP", DEFAULT_SERVER_IP);
    if (argc > 1) serverIp = argv[1];

    std::string portStr = readConfig("SERVER_PORT",
                                     std::to_string(DEFAULT_SERVER_PORT));
    uint16_t port = static_cast<uint16_t>(std::stoi(portStr));

    std::cout << "  [Config] Server : " << serverIp << ":" << port << "\n";
    std::cout << "  [Config] LED GPIO: " << LED_GPIO << "\n\n";

    
    GpioLed led(LED_GPIO);
    bool gpioOk = led.init();
    if (!gpioOk)
        std::cerr << "  [GPIO] LED control unavailable (running without root?).\n";

    
    TCPSocket     tcpSocket;
    ClientChannel clientChannel;
    clientChannel.channelSocket = &tcpSocket;

    
    struct sockaddr_in srv{};
    srv.sin_family = AF_INET;
    srv.sin_port   = htons(port);
    if (inet_pton(AF_INET, serverIp.c_str(), &srv.sin_addr) != 1) {
        std::cerr << "  [Client] Invalid server IP: " << serverIp << "\n";
        return 1;
    }

    
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

    
    double threshold   = 50.0;
    double temperature = 0.0;
    bool   ledOn       = false;

    
    while (g_running) {
        std::string cmd = readLine(rawFd);
        if (cmd.empty()) {
            std::cout << "  [Client] Server disconnected.\n";
            break;
        }

        std::cout << "  [Client] Command: \"" << cmd << "\"\n";

        
        if (cmd == "set threshold") {
            std::string valStr = readLine(rawFd);
            if (valStr.empty()) break;
            try {
                threshold = std::stod(valStr);
                std::cout << "  [Client] Threshold updated: " << threshold << " °C\n";

                
                ledOn = (temperature >= threshold);
                led.set(ledOn);
                printLED(temperature, threshold, ledOn);
            } catch (...) {
                std::cerr << "  [Client] Bad threshold value: " << valStr << "\n";
            }
        }
        
        else if (cmd == "get temp") {
            std::cout << "  [Client] Enter temperature (°C) [or 'auto' for SoC sensor]: ";
            std::cout.flush();

            std::string input;
            if (!std::getline(std::cin, input) || input.empty()) {
                
                
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

                
                ledOn = (temperature >= threshold);
                led.set(ledOn);
                printLED(temperature, threshold, ledOn);
            } catch (...) {
                std::cerr << "  [Client] Invalid input — sending 0.0\n";
                sendLine(rawFd, "0.0");
            }
        }
        
        else {
            std::cout << "  [Client] Unknown command — ignoring.\n";
        }
    }

    
    if (rawFd >= 0) ::close(rawFd);
    led.set(false);   
    std::cout << "  [Client] Shutdown complete.\n";
    return 0;
}
