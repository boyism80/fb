#ifndef __BOT_H__
#define __BOT_H__

#include <random>
#include <fb/socket.h>
#include <fb/thread_container.h>
#include <fb/protocol/gateway.h>
#include <fb/protocol/login.h>
#include <fb/protocol/game.h>

using namespace std::chrono_literals;

namespace fb { namespace bot {

class bot_container;

class base_bot : public fb::socket<void*>
{
private:
    using handle_func    = std::function<async::task<void>(fb::protocol::base::header&)>;
    using deserilze_func = std::function<async::task<fb::protocol::base::header*>(fb::stream_reader<big_endian>&)>;

protected:
    bot_container&                              _owner;
    fb::cryptor                                 _cryptor;
    std::unordered_map<uint8_t, handle_func>    _handler;
    std::unordered_map<uint8_t, deserilze_func> _deserializer;

public:
    const uint32_t id;

protected:
    base_bot(bot_container& owner, uint32_t id);

public:
    virtual ~base_bot();

private:
    async::task<void> on_receive(fb::socket<>& socket, fb::stream& stream);
    async::task<void> on_closed(fb::socket<>& socket);

protected:
    virtual void on_connected();
    virtual void on_disconnected();
    virtual bool on_encrypt(fb::stream& out);
    virtual bool on_wrap(fb::stream& out);
    virtual bool decrypt_policy(int cmd) const;

public:
    void                      connect(const boost::asio::ip::tcp::endpoint& endpoint);
    virtual async::task<void> on_timer(const fb::model::datetime& now)
    {
        co_return;
    }

    template <typename T>
    void bind(int cmd, const std::function<async::task<void>(T&)>& fn)
    {
        // this->_handler.insert({cmd, [this, fn](const std::function<void()>& callback) -> async::task<void> {
        //                            co_await this->reader<async::task<void>>(
        //                                [this, fn, &callback](auto& reader) -> async::task<void> {
        //                                    T header;
        //                                    header.deserialize(reader);
        //                                    callback();
        //                                    this->invoke_promise(header.__id, header);

        //                                   co_await fn(header);
        //                               });
        //                       }});
    }

    template <typename Class, typename Response>
    void bind(async::task<void> (Class::*fn)(const Response&))
    {
        auto header = Response::header;
        this->_deserializer.insert({header, [](auto& reader) -> async::task<fb::protocol::base::header*> {
                                        auto protocol = new Response();
                                        co_await protocol->deserialize(reader);
                                        co_return protocol;
                                    }});

        auto c_fn = std::bind(fn, static_cast<Class*>(this), std::placeholders::_1);
        this->_handler.insert({header, [c_fn](auto& header) -> async::task<void> {
                                   auto protocol = static_cast<Response&>(header);
                                   co_await c_fn(protocol);
                               }});
    }

    template <typename ResponseType>
    async::task<ResponseType> request(const fb::protocol::base::header& protocol)
    {
        auto promise = std::make_shared<async::task_completion_source<ResponseType>>();
        return promise->task();
    }
};

class gateway_bot : public base_bot
{
public:
    gateway_bot(bot_container& owner, uint32_t id);
    ~gateway_bot();

private:
    async::task<void> handle_welcome(const fb::protocol::gateway::response::welcome& response);
    async::task<void> handle_crt(const fb::protocol::gateway::response::crt& response);
    async::task<void> handle_hosts(const fb::protocol::gateway::response::hosts& response);
    async::task<void> handle_transfer(const fb::protocol::response::transfer& response);

protected:
    bool decrypt_policy(int cmd) const;
};

class login_bot : public base_bot
{
private:
    std::string _id, _pw;
    bool        _try_login = false;

public:
    login_bot(bot_container& owner, uint32_t id);
    login_bot(bot_container& owner, uint32_t id, const fb::stream& params);
    ~login_bot();

private:
    async::task<void> co_login(std::string id, std::string pw);

protected:
    void on_connected();
    bool decrypt_policy(int cmd) const;

public:
    async::task<void> handle_agreement(const fb::protocol::login::response::agreement& response);
    async::task<void> handle_message(const fb::protocol::login::response::message& response);
    async::task<void> handle_transfer(const fb::protocol::response::transfer& response);
};

class game_bot : public base_bot
{
private:
    typedef struct _pattern_params_tag
    {
        std::function<async::task<void>()>  fn;
        std::chrono::steady_clock::duration min, max;
    } pattern_params;

private:
    uint32_t                    _sequence = 0;
    point16_t                   _position;
    fb::stream                  _transfer_buffer;
    std::vector<pattern_params> _pattern_params;
    datetime                    _next_action_time;
    bool                        _inited = false;

public:
    game_bot(bot_container& owner, uint32_t id);
    game_bot(bot_container& owner, uint32_t id, const fb::stream& params);
    ~game_bot();

private:
    template <typename Class>
    void pattern(async::task<void> (Class::*fn)(),
                 const std::chrono::steady_clock::duration& min,
                 const std::chrono::steady_clock::duration& max)
    {
        this->_pattern_params.push_back(pattern_params{std::bind(fn, static_cast<Class*>(this)), min, max});
    }

protected:
    void on_connected();

public:
    async::task<void> on_timer(const fb::model::datetime& now) override final;

public:
    async::task<void> handle_init(const fb::protocol::game::response::init& response);
    async::task<void> handle_time(const fb::protocol::game::response::time& response);
    async::task<void> handle_state(const fb::protocol::game::response::character::state& response);
    async::task<void> handle_option(const fb::protocol::game::response::character::option& response);
    async::task<void> handle_message(const fb::protocol::game::response::message& response);
    async::task<void> handle_sequence(const fb::protocol::game::response::character::id& response);
    async::task<void> handle_spell_update(const fb::protocol::game::response::spell::update& response);
    async::task<void> handle_chat(const fb::protocol::game::response::chat& response);
    async::task<void> handle_action(const fb::protocol::game::response::life::action& response);
    async::task<void> handle_direction(const fb::protocol::game::response::object::direction& response);
    async::task<void> handle_position(const fb::protocol::game::response::character::position& response);
    async::task<void> handle_move(const fb::protocol::game::response::object::move& response);
    async::task<void> handle_map(const fb::protocol::game::response::map::config& response);
    async::task<void> handle_transfer(const fb::protocol::response::transfer& response);

public:
    async::task<void> pattern_chat();
    async::task<void> pattern_attack();
    async::task<void> pattern_direction();
    async::task<void> pattern_move();
    async::task<void> pattern_pickup();
    async::task<void> pattern_emotion();
    async::task<void> pattern_board_sections();
};

class bot_container : public fb::context
{
private:
    uint32_t                      _sequence = 0;
    boost::asio::io_context&      _context;
    std::map<uint16_t, base_bot*> _bots;
    bool                          _exit = false;

public:
    bot_container(boost::asio::io_context& context);
    ~bot_container();

public:
    boost::asio::io_context& context() const;

    template <typename T>
    T* create()
    {
        auto id  = this->_sequence++;
        auto bot = new T(*this, id);
        this->_bots.insert({bot->id, bot});
        return bot;
    }

    template <typename T>
    T* create(const fb::stream& params)
    {
        auto id  = this->_sequence++;
        auto bot = new T(*this, id, params);
        this->_bots.insert({bot->id, bot});
        return bot;
    }
    void              remove(base_bot& bot);
    async::task<void> handle_timer(const fb::model::datetime& now, std::thread::id id);
    async::task<void> dispatch(uint32_t id, std::function<async::task<void>(fb::thread&)>&& fn);
};

}} // namespace fb::bot

#endif