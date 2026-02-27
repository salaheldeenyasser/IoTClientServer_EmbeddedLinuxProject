#ifndef CHANNEL_H
#define CHANNEL_H

/**
 * Channel.h
 * ─────────────────────────────────────────────────────────────────────────────
 * Abstract base class  : Channel
 * Concrete classes     : ServerChannel, ClientChannel
 *
 * A Channel owns a Socket* (polymorphism / composition).
 * ServerChannel calls waitForConnect(); ClientChannel calls connect().
 * ─────────────────────────────────────────────────────────────────────────────
 */

#include "Socket.h"

class Channel
{
public:
    Socket *channelSocket = nullptr;

    virtual ~Channel() = default;

    virtual void start()                            = 0;
    virtual void stop()                             = 0;
    virtual void send(const std::string &message)   = 0;
    virtual void receive()                          = 0;

    int fd() const { return channelSocket ? channelSocket->fd() : -1; }
};

// ── Server channel ─────────────────────────────────────────────────────────────
class ServerChannel : public Channel
{
public:
    /** Starts listening / binding.  Returns the fd to watch, or -1.    */
    void start() override
    {
        if (channelSocket) channelSocket->waitForConnect();
    }

    /** Returns the fd from waitForConnect() directly — used by
     *  MainWindow to create the QSocketNotifier.                        */
    int startListening()
    {
        if (!channelSocket) return -1;
        return channelSocket->waitForConnect();
    }

    void stop()    override { if (channelSocket) channelSocket->shutdown(); }
    void send(const std::string &msg) override { if (channelSocket) channelSocket->send(msg); }
    void receive() override { if (channelSocket) channelSocket->receive(); }
};

// ── Client channel ─────────────────────────────────────────────────────────────
class ClientChannel : public Channel
{
public:
    void start() override { if (channelSocket) channelSocket->connect(); }
    void stop()  override { if (channelSocket) channelSocket->shutdown(); }
    void send(const std::string &msg) override { if (channelSocket) channelSocket->send(msg); }
    void receive() override { if (channelSocket) channelSocket->receive(); }
};

#endif // CHANNEL_H
