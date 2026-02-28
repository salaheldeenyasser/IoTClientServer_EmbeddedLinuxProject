#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <iostream>

class Socket
{
public:
    virtual ~Socket() = default;

    virtual int waitForConnect() = 0;

    virtual int connect() = 0;

    virtual void send(const std::string &message) = 0;

    virtual void receive() = 0;

    virtual void shutdown() = 0;

    virtual int fd() const = 0;
};

class TCPSocket : public Socket
{
private:
    int m_listenFd = -1;
    int m_sockfd = -1;
    struct sockaddr_in m_serverAddr{};
    struct sockaddr_in m_clientAddr{};
    socklen_t m_addrLen = sizeof(m_clientAddr);

public:
    TCPSocket()
    {
        std::memset(&m_serverAddr, 0, sizeof(m_serverAddr));
        std::memset(&m_clientAddr, 0, sizeof(m_clientAddr));
    }

    ~TCPSocket() override { shutdown(); }

    int waitForConnect() override
    {
        m_listenFd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (m_listenFd < 0)
        {
            std::cerr << "[TCPSocket] socket() failed\n";
            return -1;
        }

        int opt = 1;
        setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        m_serverAddr.sin_family = AF_INET;
        m_serverAddr.sin_addr.s_addr = INADDR_ANY;
        m_serverAddr.sin_port = htons(8080);

        if (::bind(m_listenFd,
                   reinterpret_cast<sockaddr *>(&m_serverAddr),
                   sizeof(m_serverAddr)) < 0)
        {
            std::cerr << "[TCPSocket] bind() failed\n";
            ::close(m_listenFd);
            m_listenFd = -1;
            return -1;
        }

        if (::listen(m_listenFd, 5) < 0)
        {
            std::cerr << "[TCPSocket] listen() failed\n";
            ::close(m_listenFd);
            m_listenFd = -1;
            return -1;
        }

        return m_listenFd;
    }

    int acceptConnection()
    {
        m_sockfd = ::accept(m_listenFd,
                            reinterpret_cast<sockaddr *>(&m_clientAddr),
                            &m_addrLen);
        return m_sockfd;
    }

    int fd() const override { return m_sockfd; }

    int listenFd() const { return m_listenFd; }

    int connect() override
    {
        m_sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (m_sockfd < 0)
        {
            std::cerr << "[TCPSocket] socket() failed\n";
            return -1;
        }

        m_serverAddr.sin_family = AF_INET;
        m_serverAddr.sin_port = htons(8080);
        inet_pton(AF_INET, "127.0.0.1", &m_serverAddr.sin_addr);

        if (::connect(m_sockfd,
                      reinterpret_cast<sockaddr *>(&m_serverAddr),
                      sizeof(m_serverAddr)) < 0)
        {
            std::cerr << "[TCPSocket] connect() failed\n";
            ::close(m_sockfd);
            m_sockfd = -1;
            return -1;
        }
        return 0;
    }

    void send(const std::string &message) override
    {
        if (m_sockfd < 0)
            return;
        ::send(m_sockfd, message.c_str(), message.size(), 0);
    }

    void receive() override
    {
        if (m_sockfd < 0)
            return;
        char buf[1024];
        int n = ::recv(m_sockfd, buf, sizeof(buf) - 1, 0);
        if (n > 0)
        {
            buf[n] = '\0';
            std::cout << "[TCP] Received: " << buf << "\n";
        }
    }

    void shutdown() override
    {
        if (m_sockfd >= 0)
        {
            ::close(m_sockfd);
            m_sockfd = -1;
        }
        if (m_listenFd >= 0)
        {
            ::close(m_listenFd);
            m_listenFd = -1;
        }
    }
};

class UDPSocket : public Socket
{
private:
    int m_sockfd = -1;
    struct sockaddr_in m_remoteAddr{};
    socklen_t m_addrLen = sizeof(m_remoteAddr);

public:
    UDPSocket() { std::memset(&m_remoteAddr, 0, sizeof(m_remoteAddr)); }
    ~UDPSocket() override { shutdown(); }

    int waitForConnect() override
    {
        m_sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (m_sockfd < 0)
        {
            std::cerr << "[UDPSocket] socket() failed\n";
            return -1;
        }

        m_remoteAddr.sin_family = AF_INET;
        m_remoteAddr.sin_addr.s_addr = INADDR_ANY;
        m_remoteAddr.sin_port = htons(8081);

        if (::bind(m_sockfd,
                   reinterpret_cast<sockaddr *>(&m_remoteAddr),
                   sizeof(m_remoteAddr)) < 0)
        {
            std::cerr << "[UDPSocket] bind() failed\n";
            ::close(m_sockfd);
            m_sockfd = -1;
            return -1;
        }
        std::cout << "[UDPSocket] Bound on port 8081\n";
        return m_sockfd;
    }

    int connect() override
    {
        m_sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (m_sockfd < 0)
        {
            std::cerr << "[UDPSocket] socket() failed\n";
            return -1;
        }
        m_remoteAddr.sin_family = AF_INET;
        m_remoteAddr.sin_port = htons(8081);
        inet_pton(AF_INET, "127.0.0.1", &m_remoteAddr.sin_addr);
        return 0;
    }

    void send(const std::string &message) override
    {
        if (m_sockfd < 0)
            return;
        ::sendto(m_sockfd, message.c_str(), message.size(), 0,
                 reinterpret_cast<sockaddr *>(&m_remoteAddr), sizeof(m_remoteAddr));
    }

    void receive() override
    {
        if (m_sockfd < 0)
            return;
        char buf[1024];
        int n = ::recvfrom(m_sockfd, buf, sizeof(buf) - 1, 0,
                           reinterpret_cast<sockaddr *>(&m_remoteAddr), &m_addrLen);
        if (n > 0)
        {
            buf[n] = '\0';
            std::cout << "[UDP] Received: " << buf << "\n";
        }
    }

    std::string receiveFrom()
    {
        char buf[1024];
        int n = ::recvfrom(m_sockfd, buf, sizeof(buf) - 1, 0,
                           reinterpret_cast<sockaddr *>(&m_remoteAddr), &m_addrLen);
        if (n > 0)
        {
            buf[n] = '\0';
            return std::string(buf);
        }
        return {};
    }

    void sendReply(const std::string &message)
    {
        ::sendto(m_sockfd, message.c_str(), message.size(), 0,
                 reinterpret_cast<sockaddr *>(&m_remoteAddr), sizeof(m_remoteAddr));
    }

    void shutdown() override
    {
        if (m_sockfd >= 0)
        {
            ::close(m_sockfd);
            m_sockfd = -1;
        }
    }

    int fd() const override { return m_sockfd; }
};

#endif
