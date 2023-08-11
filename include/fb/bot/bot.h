#ifndef __BOT_H__
#define __BOT_H__

#include <map>
#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <random>
#include <sstream>
#include <fb/core/stream.h>
#include <fb/core/socket.h>
#include <fb/core/thread.h>
#include <fb/protocol/gateway.h>
#include <fb/protocol/login.h>
#include <fb/protocol/game.h>

using namespace std::chrono_literals;

namespace fb { namespace bot {

class bot_container;

class base_bot : public fb::awaitable_socket<void*>
{
protected:
    bot_container&                              _owner;
    fb::cryptor                                 _cryptor;
    std::map<uint8_t, std::function<void()>>    _handler_dict;

public:
    const uint32_t                              id;

protected:
    base_bot(bot_container& owner, uint32_t id);

public:
    virtual ~base_bot();

private:
    void                                        on_receive(fb::base::socket<>& socket);
    void                                        on_closed(fb::base::socket<>& socket);

protected:
    virtual void                                on_connected();
    virtual void                                on_disconnected();
    virtual bool                                on_encrypt(fb::ostream& out);
    virtual bool                                on_wrap(fb::ostream& out);
    virtual bool                                is_decrypt(int cmd) const;

public:
    void connect(const boost::asio::ip::tcp::endpoint& endpoint);
    virtual void on_timer(std::chrono::steady_clock::duration now) {}

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
    gateway_bot(bot_container& owner, uint32_t id);
    ~gateway_bot();

private:
    void                                        handle_welcome(const fb::protocol::gateway::response::welcome& response);
    void                                        handle_crt(const fb::protocol::gateway::response::crt& response);
    void                                        handle_hosts(const fb::protocol::gateway::response::hosts& response);
    void                                        handle_transfer(const fb::protocol::response::transfer& response);

protected:
    bool                                        is_decrypt(int cmd) const;
};


class login_bot : public base_bot
{
private:
    std::string                                 _id, _pw;
    bool                                        _try_login = false;

public:
    login_bot(bot_container& owner, uint32_t id);
    login_bot(bot_container& owner, uint32_t id, const fb::buffer& params);
    ~login_bot();

private:
    task                                        co_login(std::string id, std::string pw);

protected:
    void                                        on_connected();
    bool                                        is_decrypt(int cmd) const;

public:
    void                                        handle_agreement(const fb::protocol::login::response::agreement& response);
    void                                        handle_message(const fb::protocol::login::response::message& response);
    void                                        handle_transfer(const fb::protocol::response::transfer& response);
};


class game_bot : public base_bot
{
private:
    typedef struct _pattern_params_tag
    {
        std::function<void()>                   fn;
        std::chrono::steady_clock::duration     min, max;
    } pattern_params;


private:
    uint32_t                                    _sequence = 0;
    point16_t                                   _position;
    fb::buffer                                  _transfer_buffer;
    std::vector<pattern_params>                 _pattern_params;
    std::chrono::steady_clock::duration         _next_action_time;
    bool                                        _inited = false;

public:
    game_bot(bot_container& owner, uint32_t id);
    game_bot(bot_container& owner, uint32_t id, const fb::buffer& params);
    ~game_bot();

private:
    void                                        pattern(std::function<void()> fn, const std::chrono::steady_clock::duration& min, const std::chrono::steady_clock::duration& max);

protected:
    void                                        on_connected();

public:
    void                                        on_timer(std::chrono::steady_clock::duration now);

public:
    void                                        handle_init(const fb::protocol::game::response::init& response);
    void                                        handle_time(const fb::protocol::game::response::time& response);
    void                                        handle_state(const fb::protocol::game::response::session::state& response);
    void                                        handle_option(const fb::protocol::game::response::session::option& response);
    void                                        handle_message(const fb::protocol::game::response::message& response);
    void                                        handle_sequence(const fb::protocol::game::response::session::id& response);
    void                                        handle_spell_update(const fb::protocol::game::response::spell::update& response);
    void                                        handle_chat(const fb::protocol::game::response::chat& response);
    void                                        handle_action(const fb::protocol::game::response::life::action& response);
    void                                        handle_direction(const fb::protocol::game::response::object::direction& response);
    void                                        handle_position(const fb::protocol::game::response::session::position& response);
    void                                        handle_move(const fb::protocol::game::response::object::move& response);
    void                                        handle_map(const fb::protocol::game::response::map::config& response);
    void                                        handle_transfer(const fb::protocol::response::transfer& response);

public:
    void                                        pattern_chat();
    void                                        pattern_attack();
    void                                        pattern_direction();
    void                                        pattern_move();
    void                                        pattern_pickup();
    void                                        pattern_emotion();
};

class bot_container
{
private:
    uint32_t                                    _sequence = 0;
    boost::asio::io_context&                    _context;
    std::map<uint16_t, base_bot*>               _bots;
    fb::threads                                 _threads;
    std::mutex                                  _bots_lock;
    bool                                        _exit = false;

public:
    bot_container(boost::asio::io_context& context);
    ~bot_container();

public:
    boost::asio::io_context&                    context() const;

    template <typename T>
    T* create()
    {   MUTEX_GUARD(this->_bots_lock)
        
        auto id = this->_sequence++;
        auto bot = new T(*this, id);
        this->_bots.insert({ bot->id, bot });
        return bot;
    }

    template <typename T>
    T* create(const fb::buffer& params)
    {   MUTEX_GUARD(this->_bots_lock)
        
        auto id = this->_sequence++;
        auto bot = new T(*this, id, params);
        this->_bots.insert({ bot->id, bot });
        return bot;
    }
    void remove(base_bot& bot);
    void handle_timer(std::chrono::steady_clock::duration now, std::thread::id id);
    void dispatch(uint32_t id, std::function<void(uint32_t)>&& fn);
};

} }

#endif