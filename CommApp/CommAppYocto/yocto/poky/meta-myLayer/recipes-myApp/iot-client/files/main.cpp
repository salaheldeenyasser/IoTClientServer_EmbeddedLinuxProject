#include "Channel.h"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <csignal>
#include <atomic>
#include <chrono>
#include <thread>
#include <clocale>   // FIX (Bug E.4): force C locale for decimal-point consistency

static std::atomic<bool> g_running{true};

static void handleSignal(int) { g_running = false; }

class TCPClientSocket : public TCPSocket
{
private:
    std::string m_ip;
    uint16_t    m_port;
    int         m_fd = -1;

public:
    TCPClientSocket(const std::string &ip, uint16_t port)
        : m_ip(ip), m_port(port) {}

    int connect() override
    {
        m_fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (m_fd < 0)
            return -1;

        struct sockaddr_in addr{};
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(m_port);
        if (::inet_pton(AF_INET, m_ip.c_str(), &addr.sin_addr) != 1)
        {
            ::close(m_fd);
            m_fd = -1;
            return -1;
        }
        if (::connect(m_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
        {
            ::close(m_fd);
            m_fd = -1;
            return -1;
        }

        // FIX (Bug E.3): set a receive timeout so readLine() does not block
        // indefinitely if the server stops responding (e.g. crash, network
        // drop). After the timeout, recv() returns -1/EAGAIN and the
        // reconnect loop is triggered.
        struct timeval tv{};
        tv.tv_sec  = 10;
        tv.tv_usec = 0;
        setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        // FIX (Bug 7): Socket::connect() contract is 0 on success / -1 on
        // failure. The original returned m_fd (e.g. 4), which works with the
        // >= 0 check but violates the interface and breaks any == 0 check.
        return 0;
    }

    void send(const std::string &message) override
    {
        if (m_fd < 0)
            return;
        // FIX: MSG_NOSIGNAL prevents SIGPIPE from killing the process when
        // the server has already closed the connection.
        ::send(m_fd, message.c_str(), message.size(), MSG_NOSIGNAL);
    }

    void shutdown() override
    {
        if (m_fd >= 0)
        {
            ::close(m_fd);
            m_fd = -1;
        }
    }

    int fd() const override { return m_fd; }
};

class UDPClientSocket : public UDPSocket
{
private:
    std::string        m_ip;
    uint16_t           m_port;
    int                m_fd = -1;
    struct sockaddr_in m_serverAddr{};

public:
    UDPClientSocket(const std::string &ip, uint16_t port)
        : m_ip(ip), m_port(port)
    {
        std::memset(&m_serverAddr, 0, sizeof(m_serverAddr));
    }

    int connect() override
    {
        m_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (m_fd < 0)
            return -1;

        std::memset(&m_serverAddr, 0, sizeof(m_serverAddr));
        m_serverAddr.sin_family = AF_INET;
        m_serverAddr.sin_port   = htons(m_port);
        if (::inet_pton(AF_INET, m_ip.c_str(), &m_serverAddr.sin_addr) != 1)
        {
            ::close(m_fd);
            m_fd = -1;
            return -1;
        }

        struct timeval tv{};
        tv.tv_sec  = 5;
        tv.tv_usec = 0;
        setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        // FIX (Bug 7): return 0 on success to match Socket::connect() contract.
        return 0;
    }

    void send(const std::string &message) override
    {
        if (m_fd < 0)
            return;
        ::sendto(m_fd, message.c_str(), message.size(), 0,
                 reinterpret_cast<const sockaddr *>(&m_serverAddr),
                 sizeof(m_serverAddr));
    }

    std::string receiveFrom()
    {
        char buf[256] = {};
        struct sockaddr_in from{};
        socklen_t fromLen = sizeof(from);
        int n = ::recvfrom(m_fd, buf, sizeof(buf) - 1, 0,
                           reinterpret_cast<sockaddr *>(&from), &fromLen);
        if (n <= 0)
            return {};
        buf[n] = '\0';
        return std::string(buf);
    }

    void shutdown() override
    {
        if (m_fd >= 0)
        {
            ::close(m_fd);
            m_fd = -1;
        }
    }

    int fd() const override { return m_fd; }
};

static double readTemperature()
{
    std::ifstream f("/sys/class/thermal/thermal_zone0/temp");
    if (!f.is_open())
        return 25.0;
    int millideg = 0;
    f >> millideg;
    return static_cast<double>(millideg) / 1000.0;
}

static void setLed(int gpio, bool on)
{
    std::string g = std::to_string(gpio);
    { std::ofstream f("/sys/class/gpio/export"); if (f.is_open()) f << g; }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    { std::ofstream f("/sys/class/gpio/gpio" + g + "/direction"); if (f.is_open()) f << "out"; }
    { std::ofstream f("/sys/class/gpio/gpio" + g + "/value");     if (f.is_open()) f << (on ? "1" : "0"); }
}

static void printDisplay(double temp, double threshold, bool ledOn)
{
    std::cout << "\033[2J\033[H";
    std::cout << "IoT Client\n";
    std::cout << "==========\n";
    std::cout << "Temperature : " << temp      << " C\n";
    std::cout << "Threshold   : " << threshold << " C\n";
    std::cout << "LED Status  : " << (ledOn ? "ON" : "OFF") << "\n";
    std::cout.flush();
}

// FIX (Bug E.3): readLine() now returns "" on both clean disconnect and on
// SO_RCVTIMEO expiry (EAGAIN/EWOULDBLOCK), letting the caller trigger a
// reconnect instead of blocking forever.
static std::string readLine(ClientChannel &ch)
{
    std::string line;
    char c = '\0';
    while (g_running)
    {
        int n = ::recv(ch.fd(), &c, 1, 0);
        if (n < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // SO_RCVTIMEO expired — server may be unresponsive.
                std::cerr << "Read timeout — server not responding.\n";
            }
            return "";  // trigger reconnect in caller
        }
        if (n == 0)
            return "";  // clean EOF
        if (c == '\n')
            break;
        if (c != '\r')
            line += c;
    }
    return line;
}

static void runTCP(const std::string &ip, int gpio)
{
    TCPClientSocket sock(ip, 8080);
    ClientChannel   channel;
    channel.channelSocket = &sock;

    double temperature = readTemperature();
    double threshold   = 50.0;
    bool   ledOn       = false;

    printDisplay(temperature, threshold, ledOn);
    std::cout << "Connecting TCP to " << ip << ":8080 ...\n";
    std::cout.flush();

    while (g_running)
    {
        // FIX (Bug 7): connect() now returns 0 on success.
        if (channel.channelSocket->connect() == 0)
            break;
        std::cerr << "Retrying in 3s...\n";
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    if (!g_running)
    {
        channel.stop();
        return;
    }

    printDisplay(temperature, threshold, ledOn);
    std::cout << "Connected via TCP.\n";
    std::cout.flush();

    while (g_running)
    {
        std::string cmd = readLine(channel);
        if (cmd.empty())
        {
            std::cout << "Server disconnected. Reconnecting in 3s...\n";
            channel.stop();
            std::this_thread::sleep_for(std::chrono::seconds(3));
            // Re-create socket and reconnect
            TCPClientSocket newSock(ip, 8080);
            sock = newSock;
            channel.channelSocket = &sock;
            while (g_running && channel.channelSocket->connect() != 0)
            {
                std::cerr << "Retrying in 3s...\n";
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }
            std::cout << "Reconnected.\n";
            continue;
        }

        // FIX (Bug 5): "set threshold" and its value are now combined into a
        // single message: "set threshold <value>".  The old two-message
        // approach (separate command + value sends) caused the value to be
        // consumed by the wrong readLine() call when TCP coalesced packets.
        if (cmd.rfind("set threshold ", 0) == 0)
        {
            try   { threshold = std::stod(cmd.substr(14)); }
            catch (...) { std::cerr << "Bad threshold value: " << cmd << "\n"; }
            ledOn = (temperature >= threshold);
            setLed(gpio, ledOn);
            printDisplay(temperature, threshold, ledOn);
        }
        else if (cmd == "get temp")
        {
            temperature = readTemperature();
            std::ostringstream oss;
            oss << temperature;
            channel.send(oss.str() + "\n");
            ledOn = (temperature >= threshold);
            setLed(gpio, ledOn);
            printDisplay(temperature, threshold, ledOn);
        }
        else
        {
            std::cerr << "Unknown command: " << cmd << "\n";
        }
    }

    channel.stop();
    setLed(gpio, false);
}

static void runUDP(const std::string &ip, int gpio)
{
    UDPClientSocket sock(ip, 8081);
    ClientChannel   channel;
    channel.channelSocket = &sock;

    double temperature = readTemperature();
    double threshold   = 50.0;
    bool   ledOn       = false;

    printDisplay(temperature, threshold, ledOn);
    std::cout << "Connecting UDP to " << ip << ":8081 ...\n";
    std::cout.flush();

    if (channel.channelSocket->connect() != 0)  // FIX (Bug 7): check == 0
    {
        std::cerr << "Failed to create UDP socket.\n";
        return;
    }

    printDisplay(temperature, threshold, ledOn);
    std::cout << "Ready. Sending initial temperature...\n";
    std::cout.flush();

    std::ostringstream initOss;
    initOss << temperature;
    channel.send(initOss.str() + "\n");

    while (g_running)
    {
        UDPClientSocket *udp = static_cast<UDPClientSocket *>(channel.channelSocket);
        std::string pkt = udp->receiveFrom();

        if (pkt.empty())
        {
            // Timeout or error — send a keepalive temperature reading so the
            // server stays aware we are still alive (server needs at least
            // one datagram to capture the client's address for sendReply()).
            temperature = readTemperature();
            std::ostringstream oss;
            oss << temperature;
            channel.send(oss.str() + "\n");
            continue;
        }

        while (!pkt.empty() && (pkt.back() == '\n' || pkt.back() == '\r'))
            pkt.pop_back();

        // FIX (Bug 5): parse combined "set threshold <value>" message.
        // Old code called receiveFrom() a second time to get the value, which
        // is unreliable over UDP (packets can be reordered or dropped).
        if (pkt.rfind("set threshold ", 0) == 0)
        {
            try   { threshold = std::stod(pkt.substr(14)); }
            catch (...) { std::cerr << "Bad threshold value: " << pkt << "\n"; }
            ledOn = (temperature >= threshold);
            setLed(gpio, ledOn);
            printDisplay(temperature, threshold, ledOn);
        }
        else if (pkt == "get temp")
        {
            temperature = readTemperature();
            std::ostringstream oss;
            oss << temperature;
            channel.send(oss.str() + "\n");
            ledOn = (temperature >= threshold);
            setLed(gpio, ledOn);
            printDisplay(temperature, threshold, ledOn);
        }
        else
        {
            std::cerr << "Unknown packet: " << pkt << "\n";
        }
    }

    channel.stop();
    setLed(gpio, false);
}

int main(int argc, char *argv[])
{
    // FIX (Bug E.4): force C locale so std::ostringstream always uses '.' as
    // the decimal separator regardless of the rootfs locale setting.
    // Without this, a locale like de_DE causes "36,7" to be sent instead of
    // "36.7", which QString::toDouble() on the server then rejects (ok=false),
    // silently dropping every temperature reading.
    std::setlocale(LC_ALL, "C");

    std::signal(SIGINT,  handleSignal);
    std::signal(SIGTERM, handleSignal);

    std::string proto = "tcp";
    std::string ip    = "192.168.1.100";
    int         gpio  = 17;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--proto" && i + 1 < argc)
            proto = argv[++i];
        else if (arg == "--ip" && i + 1 < argc)
            ip = argv[++i];
        else if (arg == "--gpio" && i + 1 < argc)
            gpio = std::stoi(argv[++i]);
        else if (arg == "--help")
        {
            std::cout << "Usage: iot-client [--proto tcp|udp] [--ip <server_ip>] [--gpio <bcm_pin>]\n";
            std::cout << "Defaults: --proto tcp  --ip 192.168.1.100  --gpio 17\n";
            return 0;
        }
    }

    if (proto != "tcp" && proto != "udp")
    {
        std::cerr << "Invalid protocol. Use tcp or udp.\n";
        return 1;
    }

    if (proto == "tcp")
        runTCP(ip, gpio);
    else
        runUDP(ip, gpio);

    return 0;
}
