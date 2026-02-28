#ifndef CHANNEL_H
#define CHANNEL_H

#include "Socket.h"

class Channel
{
public:
    Socket *channelSocket = nullptr;

    virtual ~Channel() = default;

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void send(const std::string &message) = 0;
    virtual void receive() = 0;

    int fd() const { return channelSocket ? channelSocket->fd() : -1; }
};

class ServerChannel : public Channel
{
public:
    void start() override
    {
        if (channelSocket)
            channelSocket->waitForConnect();
    }

    int startListening()
    {
        if (!channelSocket)
            return -1;
        return channelSocket->waitForConnect();
    }

    void stop() override
    {
        if (channelSocket)
            channelSocket->shutdown();
    }
    void send(const std::string &msg) override
    {
        if (channelSocket)
            channelSocket->send(msg);
    }
    void receive() override
    {
        if (channelSocket)
            channelSocket->receive();
    }
};

class ClientChannel : public Channel
{
public:
    void start() override
    {
        if (channelSocket)
            channelSocket->connect();
    }
    void stop() override
    {
        if (channelSocket)
            channelSocket->shutdown();
    }
    void send(const std::string &msg) override
    {
        if (channelSocket)
            channelSocket->send(msg);
    }
    void receive() override
    {
        if (channelSocket)
            channelSocket->receive();
    }
};

#endif
