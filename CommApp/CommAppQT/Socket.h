#ifndef SOCKET_H
#define SOCKET_H

/**
 * Socket.h
 * ─────────────────────────────────────────────────────────────────────────────
 * Abstract base class  : Socket
 * Concrete classes     : TCPSocket, UDPSocket
 *
 * OOP concepts:
 *  • Abstraction   – pure-virtual interface
 *  • Encapsulation – raw fd is private; exposed only via fd()
 *  • Inheritance   – TCPSocket / UDPSocket derive from Socket
 *  • Polymorphism  – ServerChannel holds Socket* at runtime
 *
 * Qt integration note:
 *  The server side (MainWindow) uses QSocketNotifier to watch the file
 *  descriptors returned by fd() so incoming data fires Qt signals without
 *  blocking the GUI thread.  No Qt networking headers (QTcpServer etc.)
 *  are used anywhere in the project.
 * ─────────────────────────────────────────────────────────────────────────────
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <iostream>

// ── Abstract base ─────────────────────────────────────────────────────────────
class Socket
{
public:
    virtual ~Socket() = default;

    /** Server side: bind + listen + accept (TCP) or just bind (UDP).
     *  Returns the connected/bound file descriptor, or -1 on error.   */
    virtual int  waitForConnect()                   = 0;

    /** Client side: connect to server.
     *  Returns 0 on success, -1 on failure.                            */
    virtual int  connect()                          = 0;

    /** Send a message over the socket.                                  */
    virtual void send(const std::string &message)   = 0;

    /** Receive data; prints to stdout (used by client terminal).        */
    virtual void receive()                          = 0;

    /** Close the socket.                                                */
    virtual void shutdown()                         = 0;

    /** Expose the raw file descriptor so callers can use QSocketNotifier
     *  or perform line-at-a-time reads without subclassing.             */
    virtual int  fd() const = 0;
};

// ── TCPSocket ─────────────────────────────────────────────────────────────────
class TCPSocket : public Socket
{
private:
    int                m_listenFd  = -1;   // listener (server only)
    int                m_sockfd    = -1;   // connected socket
    struct sockaddr_in m_serverAddr{};
    struct sockaddr_in m_clientAddr{};
    socklen_t          m_addrLen   = sizeof(m_clientAddr);

public:
    TCPSocket()
    {
        std::memset(&m_serverAddr, 0, sizeof(m_serverAddr));
        std::memset(&m_clientAddr, 0, sizeof(m_clientAddr));
    }

    ~TCPSocket() override { shutdown(); }

    // ── Server: bind → listen → accept ───────────────────────────────────────
    int waitForConnect() override
    {
        m_listenFd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (m_listenFd < 0) { std::cerr << "[TCPSocket] socket() failed\n"; return -1; }

        int opt = 1;
        setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        m_serverAddr.sin_family      = AF_INET;
        m_serverAddr.sin_addr.s_addr = INADDR_ANY;
        m_serverAddr.sin_port        = htons(8080);

        if (::bind(m_listenFd,
                   reinterpret_cast<sockaddr*>(&m_serverAddr),
                   sizeof(m_serverAddr)) < 0)
        {
            std::cerr << "[TCPSocket] bind() failed\n";
            ::close(m_listenFd); m_listenFd = -1; return -1;
        }

        if (::listen(m_listenFd, 5) < 0) {
            std::cerr << "[TCPSocket] listen() failed\n";
            ::close(m_listenFd); m_listenFd = -1; return -1;
        }

        // Return the *listen* fd — MainWindow watches it with QSocketNotifier.
        // When readable, MainWindow calls acceptConnection().
        return m_listenFd;
    }

    /** Accept a pending connection.  Call after waitForConnect() signals
     *  the listen fd is readable.  Returns the new connected fd.        */
    int acceptConnection()
    {
        m_sockfd = ::accept(m_listenFd,
                            reinterpret_cast<sockaddr*>(&m_clientAddr),
                            &m_addrLen);
        return m_sockfd;
    }

    /** fd() returns the *connected* socket (used for send/recv/notifier). */
    int fd() const override { return m_sockfd; }

    /** The listening fd (needed to close it separately).                */
    int listenFd() const { return m_listenFd; }

    // ── Client: connect to server ─────────────────────────────────────────────
    int connect() override
    {
        m_sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (m_sockfd < 0) { std::cerr << "[TCPSocket] socket() failed\n"; return -1; }

        m_serverAddr.sin_family = AF_INET;
        m_serverAddr.sin_port   = htons(8080);
        inet_pton(AF_INET, "127.0.0.1", &m_serverAddr.sin_addr);

        if (::connect(m_sockfd,
                      reinterpret_cast<sockaddr*>(&m_serverAddr),
                      sizeof(m_serverAddr)) < 0)
        {
            std::cerr << "[TCPSocket] connect() failed\n";
            ::close(m_sockfd); m_sockfd = -1; return -1;
        }
        return 0;
    }

    void send(const std::string &message) override
    {
        if (m_sockfd < 0) return;
        ::send(m_sockfd, message.c_str(), message.size(), 0);
    }

    void receive() override
    {
        if (m_sockfd < 0) return;
        char buf[1024];
        int n = ::recv(m_sockfd, buf, sizeof(buf) - 1, 0);
        if (n > 0) { buf[n] = '\0'; std::cout << "[TCP] Received: " << buf << "\n"; }
    }

    void shutdown() override
    {
        if (m_sockfd   >= 0) { ::close(m_sockfd);   m_sockfd   = -1; }
        if (m_listenFd >= 0) { ::close(m_listenFd); m_listenFd = -1; }
    }
};

// ── UDPSocket ─────────────────────────────────────────────────────────────────
class UDPSocket : public Socket
{
private:
    int                m_sockfd = -1;
    struct sockaddr_in m_remoteAddr{};
    socklen_t          m_addrLen = sizeof(m_remoteAddr);

public:
    UDPSocket() { std::memset(&m_remoteAddr, 0, sizeof(m_remoteAddr)); }
    ~UDPSocket() override { shutdown(); }

    // ── Server: bind ──────────────────────────────────────────────────────────
    int waitForConnect() override
    {
        m_sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (m_sockfd < 0) { std::cerr << "[UDPSocket] socket() failed\n"; return -1; }

        m_remoteAddr.sin_family      = AF_INET;
        m_remoteAddr.sin_addr.s_addr = INADDR_ANY;
        m_remoteAddr.sin_port        = htons(8081);

        if (::bind(m_sockfd,
                   reinterpret_cast<sockaddr*>(&m_remoteAddr),
                   sizeof(m_remoteAddr)) < 0)
        {
            std::cerr << "[UDPSocket] bind() failed\n";
            ::close(m_sockfd); m_sockfd = -1; return -1;
        }
        std::cout << "[UDPSocket] Bound on port 8081\n";
        return m_sockfd;   // watch this fd for incoming datagrams
    }

    int connect() override
    {
        m_sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (m_sockfd < 0) { std::cerr << "[UDPSocket] socket() failed\n"; return -1; }
        m_remoteAddr.sin_family = AF_INET;
        m_remoteAddr.sin_port   = htons(8081);
        inet_pton(AF_INET, "127.0.0.1", &m_remoteAddr.sin_addr);
        return 0;
    }

    void send(const std::string &message) override
    {
        if (m_sockfd < 0) return;
        ::sendto(m_sockfd, message.c_str(), message.size(), 0,
                 reinterpret_cast<sockaddr*>(&m_remoteAddr), sizeof(m_remoteAddr));
    }

    void receive() override
    {
        if (m_sockfd < 0) return;
        char buf[1024];
        int n = ::recvfrom(m_sockfd, buf, sizeof(buf) - 1, 0,
                           reinterpret_cast<sockaddr*>(&m_remoteAddr), &m_addrLen);
        if (n > 0) { buf[n] = '\0'; std::cout << "[UDP] Received: " << buf << "\n"; }
    }

    /** Receive a datagram and return its content as std::string.
     *  Also captures the sender address so we can reply.               */
    std::string receiveFrom()
    {
        char buf[1024];
        int n = ::recvfrom(m_sockfd, buf, sizeof(buf) - 1, 0,
                           reinterpret_cast<sockaddr*>(&m_remoteAddr), &m_addrLen);
        if (n > 0) { buf[n] = '\0'; return std::string(buf); }
        return {};
    }

    void sendReply(const std::string &message)
    {
        // Send back to whichever address last called receiveFrom()
        ::sendto(m_sockfd, message.c_str(), message.size(), 0,
                 reinterpret_cast<sockaddr*>(&m_remoteAddr), sizeof(m_remoteAddr));
    }

    void shutdown() override
    {
        if (m_sockfd >= 0) { ::close(m_sockfd); m_sockfd = -1; }
    }

    int fd() const override { return m_sockfd; }
};

#endif // SOCKET_H
