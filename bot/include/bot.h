#ifndef __BOT_H__
#define __BOT_H__

#include <random>
#include <any>
#include <fb/socket.h>
#include <fb/thread_container.h>
#include <fb/protocol/gateway.h>
#include <fb/protocol/login.h>
#include <fb/protocol/game.h>
#include <async/awaitable_get.h>
#include <boost/endian/conversion.hpp>
#include <fb/model/model.h>

using namespace std::chrono_literals;
using namespace fb::model;

namespace fb::bot {

class bot_container;

class base_bot : public fb::socket<void*>
{
private:
    using handle_func         = std::function<async::task<void>(fb::protocol::base::header&)>;
    using deserilze_func      = std::function<async::task<fb::protocol::base::header*>(fb::stream_reader<big_endian>&)>;
    using hook_func           = std::function<bool(const fb::protocol::base::header&)>;
    using hook_func_container = std::unordered_map<uint8_t, std::vector<hook_func>>;

protected:
    bot_container&                              _owner;
    fb::cryptor                                 _cryptor;
    std::unordered_map<uint8_t, handle_func>    _handler;
    std::unordered_map<uint8_t, deserilze_func> _deserializer;
    hook_func_container                         _hooks;

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

    template <typename ResponseType>
    void bind(const std::function<async::task<void>(ResponseType&)>& fn)
    {
        this->_deserializer.insert({ResponseType::header, [](auto& reader) -> async::task<fb::protocol::base::header*> {
                                        auto protocol = new ResponseType();
                                        co_await protocol->deserialize(reader);
                                        co_return protocol;
                                    }});

        this->_handler.insert({ResponseType::header, [this, fn](auto& header) -> async::task<void> {
                                   if (this->_hooks.contains(ResponseType::header))
                                   {
                                       auto& matched_hooks = this->_hooks.at(ResponseType::header);
                                       int   found         = -1;
                                       for (int i = 0; i < matched_hooks.size(); i++)
                                       {
                                           if (matched_hooks.at(i)(header))
                                           {
                                               found = i;
                                               break;
                                           }
                                       }

                                       if (found != -1)
                                           matched_hooks.erase(matched_hooks.begin() + found);
                                   }

                                   auto protocol = static_cast<ResponseType&>(header);
                                   co_await fn(protocol);
                               }});
    }

    template <typename Class, typename ResponseType>
    void bind(async::task<void> (Class::*fn)(const ResponseType&))
    {
        this->bind<ResponseType>(std::bind(fn, static_cast<Class*>(this), std::placeholders::_1));
    }

    template <typename ResponseType>
    async::task<ResponseType> request(const fb::protocol::base::header&                    protocol,
                                      const std::function<bool(const ResponseType& resp)>& condition,
                                      bool                                                 encrypt = true,
                                      bool                                                 wrap    = true)
    {
        if (!this->_handler.contains(ResponseType::header))
        {
            this->bind<ResponseType>([](const ResponseType& resp) -> async::task<void> {
                co_return;
            });
        }

        auto promise = std::make_shared<async::task_completion_source<ResponseType>>();
        if (this->_hooks.contains(ResponseType::header) == false)
            this->_hooks.insert({ResponseType::header, {}});

        this->_hooks[ResponseType::header].push_back([promise, condition](const auto& header) {
            auto& protocol = static_cast<const ResponseType&>(header);
            if (!condition(protocol))
                return false;

            promise->set_value(protocol);
            return true;
        });
        this->send(protocol, encrypt, wrap);
        return promise->task();
    }

    template <typename ResponseType>
    async::task<ResponseType> request(const fb::protocol::base::header& protocol, bool encrypt = true, bool wrap = true)
    {
        co_return co_await this->request<ResponseType>(
            protocol,
            [](auto& resp) -> bool {
                return true;
            },
            encrypt,
            wrap);
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
public:
    login_bot(bot_container& owner, uint32_t id);
    login_bot(bot_container& owner, uint32_t id, const fb::stream& params);
    ~login_bot();

protected:
    void on_connected() override final;
    bool decrypt_policy(int cmd) const;

public:
    async::task<void> handle_agreement(const fb::protocol::login::response::agreement& response);
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
    point<uint16_t>             _position;
    fb::stream                  _transfer_buffer;
    std::vector<pattern_params> _pattern_params;
    datetime                    _next_action_time;

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
    void on_connected() override final;

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

} // namespace fb::bot

#endif