#ifndef SOCKET_H
#define SOCKET_H

/**
 * Socket.h
 * ─────────────────────────────────────────────────────────────────────────────
 * Abstract base class  : Socket
 * Concrete classes     : TCPSocket, UDPSocket
 *
 * OOP concepts applied:
 *   • Abstraction    – Socket defines the interface with pure-virtual functions.
 *   • Encapsulation  – Raw socket descriptors are private; behaviour is exposed
 *                      only through the public API.
 *   • Inheritance    – TCPSocket and UDPSocket inherit from Socket.
 *   • Polymorphism   – Channel holds a Socket*, calling the right implementation
 *                      at runtime.
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

    /** Server side: bind, listen, accept – returns 0 on success. */
    virtual int  waitForConnect()                        = 0;

    /** Client side: connect to server – returns 0 on success. */
    virtual int  connect()                               = 0;

    /** Send a message over the socket. */
    virtual void send(const std::string &message)        = 0;

    /** Receive data and print to stdout. */
    virtual void receive()                               = 0;

    /** Close the socket. */
    virtual void shutdown()                              = 0;
};

// ── TCP implementation ────────────────────────────────────────────────────────
class TCPSocket : public Socket
{
private:
    int                sockfd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t          addr_len;

public:
    TCPSocket() : sockfd(-1), addr_len(sizeof(client_addr))
    {
        std::memset(&server_addr, 0, sizeof(server_addr));
        std::memset(&client_addr, 0, sizeof(client_addr));
    }

    ~TCPSocket() override
    {
        if (sockfd != -1) ::close(sockfd);
    }

    // ── Server: bind → listen → accept ───────────────────────────────────
    int waitForConnect() override
    {
        sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            std::cerr << "[TCPSocket] socket() failed\n";
            return -1;
        }

        // Allow address reuse between runs
        int opt = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        server_addr.sin_family      = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port        = htons(8080);

        if (::bind(sockfd,
                   reinterpret_cast<sockaddr *>(&server_addr),
                   sizeof(server_addr)) < 0) {
            std::cerr << "[TCPSocket] bind() failed\n";
            return -1;
        }

        ::listen(sockfd, 5);
        std::cout << "[TCPSocket] Waiting for client…\n";

        int client_sock = ::accept(sockfd,
                                   reinterpret_cast<sockaddr *>(&client_addr),
                                   &addr_len);
        if (client_sock < 0) {
            std::cerr << "[TCPSocket] accept() failed\n";
            return -1;
        }

        ::close(sockfd);   // close listener; use the accepted socket
        sockfd = client_sock;
        std::cout << "[TCPSocket] Client connected.\n";
        return 0;
    }

    // ── Client: connect to server ─────────────────────────────────────────
    int connect() override
    {
        sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            std::cerr << "[TCPSocket] socket() failed\n";
            return -1;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port   = htons(8080);
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

        if (::connect(sockfd,
                      reinterpret_cast<sockaddr *>(&server_addr),
                      sizeof(server_addr)) < 0) {
            std::cerr << "[TCPSocket] connect() failed\n";
            ::close(sockfd);
            sockfd = -1;
            return -1;
        }

        std::cout << "[TCPSocket] Connected to server.\n";
        return 0;
    }

    void send(const std::string &message) override
    {
        if (sockfd < 0) return;
        ::send(sockfd, message.c_str(), message.size(), 0);
    }

    void receive() override
    {
        if (sockfd < 0) return;
        char buffer[1024];
        int n = ::recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (n > 0) {
            buffer[n] = '\0';
            std::cout << "[TCPSocket] Received: " << buffer << "\n";
        }
    }

    void shutdown() override
    {
        if (sockfd != -1) {
            ::close(sockfd);
            sockfd = -1;
        }
    }

    /** Expose the raw fd so Channel subclasses can do line-at-a-time I/O. */
    int fd() const { return sockfd; }
};

// ── UDP implementation ────────────────────────────────────────────────────────
class UDPSocket : public Socket
{
private:
    int                sockfd;
    struct sockaddr_in remote_addr;   // peer address (set after first recv or connect)
    socklen_t          addr_len;

public:
    UDPSocket() : sockfd(-1), addr_len(sizeof(remote_addr))
    {
        std::memset(&remote_addr, 0, sizeof(remote_addr));
    }

    ~UDPSocket() override
    {
        if (sockfd != -1) ::close(sockfd);
    }

    // ── Server: bind ──────────────────────────────────────────────────────
    int waitForConnect() override
    {
        sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            std::cerr << "[UDPSocket] socket() failed\n";
            return -1;
        }

        remote_addr.sin_family      = AF_INET;
        remote_addr.sin_addr.s_addr = INADDR_ANY;
        remote_addr.sin_port        = htons(8081);

        if (::bind(sockfd,
                   reinterpret_cast<sockaddr *>(&remote_addr),
                   sizeof(remote_addr)) < 0) {
            std::cerr << "[UDPSocket] bind() failed\n";
            return -1;
        }
        std::cout << "[UDPSocket] Bound on port 8081.\n";
        return 0;
    }

    // ── Client: set up remote address ─────────────────────────────────────
    int connect() override
    {
        sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            std::cerr << "[UDPSocket] socket() failed\n";
            return -1;
        }
        remote_addr.sin_family = AF_INET;
        remote_addr.sin_port   = htons(8081);
        inet_pton(AF_INET, "127.0.0.1", &remote_addr.sin_addr);
        return 0;
    }

    void send(const std::string &message) override
    {
        if (sockfd < 0) return;
        ::sendto(sockfd, message.c_str(), message.size(), 0,
                 reinterpret_cast<sockaddr *>(&remote_addr), sizeof(remote_addr));
    }

    void receive() override
    {
        if (sockfd < 0) return;
        char buffer[1024];
        int n = ::recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                           reinterpret_cast<sockaddr *>(&remote_addr), &addr_len);
        if (n > 0) {
            buffer[n] = '\0';
            std::cout << "[UDPSocket] Received: " << buffer << "\n";
        }
    }

    void shutdown() override
    {
        if (sockfd != -1) {
            ::close(sockfd);
            sockfd = -1;
        }
    }
};

#endif // SOCKET_H
