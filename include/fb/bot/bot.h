#ifndef __BOT_H__
#define __BOT_H__

#include <map>
#include <boost/asio.hpp>
#include <fb/core/stream.h>
#include <fb/core/socket.h>
#include <fb/protocol/gateway.h>

namespace fb { namespace bot {

class bot_container;

class base_bot : public fb::base::socket<void*>
{
protected:
    bot_container& _owner;
    std::map<uint8_t, std::function<void()>> _handler_dict;

protected:
    base_bot(bot_container& owner);

public:
    virtual ~base_bot();

private:
    void on_receive(fb::base::socket<>& socket);
    void on_closed(fb::base::socket<>& socket);

public:
    template <typename T>
    void bind(uint8_t cmd, const std::function<void(T&)> fn)
    {
        this->_handler_dict[cmd] = [this, fn]
        {
            auto& in_stream = this->in_stream();
            T     header;
            header.deserialize(in_stream);
            fn(header);
        };
    }
};


class gateway_bot : public base_bot
{
public:
    gateway_bot(bot_container& owner);
    ~gateway_bot();

private:
    void handle_welcome(const fb::protocol::gateway::response::welcome& response);
};


class login_bot : public base_bot
{
public:
    login_bot(bot_container& owner);
    ~login_bot();
};


class game_bot : public base_bot
{
public:
    game_bot(bot_container& owner);
    ~game_bot();
};

class bot_container
{
private:
    boost::asio::io_context& _context;
    std::map<uint16_t, base_bot*> _bots;

public:
    bot_container(boost::asio::io_context& context);
    ~bot_container();

public:
    boost::asio::io_context& context() const;

    template <typename T>
    T* create()
    {
        auto bot = new T(*this);
        
        this->_bots.insert({bot->fd(), bot});
        return bot;
    }

    void close(base_bot& bot);
};

} }

#endif