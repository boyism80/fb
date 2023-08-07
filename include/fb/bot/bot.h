#ifndef __BOT_H__
#define __BOT_H__

#include <map>
#include <boost/asio.hpp>
#include <random>
#include <sstream>
#include <fb/core/stream.h>
#include <fb/core/socket.h>
#include <fb/protocol/gateway.h>
#include <fb/protocol/login.h>
#include <fb/protocol/game.h>

using namespace std::chrono_literals;

namespace fb { namespace bot {

class bot_container;

class base_bot : public fb::awaitable_socket<void*>
{
protected:
    bot_container& _owner;
    fb::cryptor _cryptor;
    std::map<uint8_t, std::function<void()>> _handler_dict;

protected:
    base_bot(bot_container& owner);

public:
    virtual ~base_bot();

private:
    void on_receive(fb::base::socket<>& socket);
    void on_closed(fb::base::socket<>& socket);

protected:
    virtual void on_connected();
    virtual void on_disconnected();
    virtual bool on_encrypt(fb::ostream& out);
    virtual bool on_wrap(fb::ostream& out);
    virtual bool is_decrypt(int cmd) const;

public:
    void connect(const boost::asio::ip::tcp::endpoint& endpoint);

    template <typename T>
    void bind(int cmd, const std::function<void(T&)> fn)
    {
        this->_handler_dict[cmd] = [this, fn]
        {
            auto& in_stream = this->in_stream();
            T     header;
            header.deserialize(in_stream);
            this->invoke_awaiter(header.__id, header);
            fn(header);
        };
    }

    template <typename T>
    void bind(const std::function<void(T&)> fn)
    {
        auto id = T().__id;
        bind(id, fn);
    }
};


class gateway_bot : public base_bot
{
public:
    gateway_bot(bot_container& owner);
    ~gateway_bot();

private:
    void handle_welcome(const fb::protocol::gateway::response::welcome& response);
    void handle_crt(const fb::protocol::gateway::response::crt& response);
    void handle_hosts(const fb::protocol::gateway::response::hosts& response);
    void handle_transfer(const fb::protocol::response::transfer& response);

protected:
    bool is_decrypt(int cmd) const;
};


class login_bot : public base_bot
{
private:
    std::string _id, _pw;
    bool _try_login = false;

public:
    login_bot(bot_container& owner);
    login_bot(bot_container& owner, const fb::buffer& params);
    ~login_bot();

private:
    static std::string new_name();

protected:
    void on_connected();
    bool is_decrypt(int cmd) const;

public:
    void handle_agreement(const fb::protocol::login::response::agreement& response);
    void handle_message(const fb::protocol::login::response::message& response);
    void handle_transfer(const fb::protocol::response::transfer& response);
};


class game_bot : public base_bot
{
private:
    fb::buffer _transfer_buffer;

public:
    game_bot(bot_container& owner);
    game_bot(bot_container& owner, const fb::buffer& params);
    ~game_bot();

protected:
    void on_connected();

public:
    void handle_init(const fb::protocol::game::response::init& response);
    void handle_time(const fb::protocol::game::response::time& response);
    void handle_state(const fb::protocol::game::response::session::state& response);
    void handle_option(const fb::protocol::game::response::session::option& response);
    void handle_message(const fb::protocol::game::response::message& response);
    void handle_spell_update(const fb::protocol::game::response::spell::update& response);
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

    template <typename T>
    T* create(const fb::buffer& params)
    {
        auto bot = new T(*this, params);
        
        this->_bots.insert({bot->fd(), bot});
        return bot;
    }

    void close(base_bot& bot);
};

} }

#endif