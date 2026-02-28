#ifndef CHANNEL_H
#define CHANNEL_H




#include "Socket.h"


class Channel
{
public:
    Socket *channelSocket = nullptr;   

    virtual ~Channel() = default;

    virtual void start()                          = 0;
    virtual void stop()                           = 0;
    virtual void send(const std::string &message) = 0;
    virtual void receive()                        = 0;
};


class ServerChannel : public Channel
{
public:
    
    void start() override
    {
        if (channelSocket) channelSocket->waitForConnect();
    }

    void stop() override
    {
        if (channelSocket) channelSocket->shutdown();
    }

    void send(const std::string &message) override
    {
        if (channelSocket) channelSocket->send(message);
    }

    void receive() override
    {
        if (channelSocket) channelSocket->receive();
    }
};


class ClientChannel : public Channel
{
public:
    
    void start() override
    {
        if (channelSocket) channelSocket->connect();
    }

    void stop() override
    {
        if (channelSocket) channelSocket->shutdown();
    }

    void send(const std::string &message) override
    {
        if (channelSocket) channelSocket->send(message);
    }

    void receive() override
    {
        if (channelSocket) channelSocket->receive();
    }
};

#endif 
