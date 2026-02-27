#ifndef CHANNEL_H
#define CHANNEL_H

/**
 * Channel.h
 * ─────────────────────────────────────────────────────────────────────────────
 * Abstract base class  : Channel
 * Concrete classes     : ServerChannel, ClientChannel
 *
 * A Channel owns a pointer to a Socket (composition + polymorphism).
 * The concrete Channel subclass determines how the socket is started (server
 * waits for a connection; client initiates one), while all actual data
 * transfer goes through the Socket interface.
 * ─────────────────────────────────────────────────────────────────────────────
 */

#include "Socket.h"

// ── Abstract base ─────────────────────────────────────────────────────────────
class Channel
{
public:
    Socket *channelSocket = nullptr;   ///< Owned externally; set before start().

    virtual ~Channel() = default;

    virtual void start()                          = 0;
    virtual void stop()                           = 0;
    virtual void send(const std::string &message) = 0;
    virtual void receive()                        = 0;
};

// ── Server channel ────────────────────────────────────────────────────────────
class ServerChannel : public Channel
{
public:
    /** Block until a client connects (TCP) or socket is bound (UDP). */
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

// ── Client channel ────────────────────────────────────────────────────────────
class ClientChannel : public Channel
{
public:
    /** Connect to the server. Returns 0 on success, –1 on failure. */
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

#endif // CHANNEL_H
