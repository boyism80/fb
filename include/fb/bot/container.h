#ifndef __BOT_CONTAINER_H__
#define __BOT_CONTAINER_H__

#include <atomic>
#include <fb/bot/bot.h>
#include <fb/thread_container.h>

namespace fb::bot {

class gateway_bot_controller;
class login_bot_controller;
class game_bot_controller;

class bot_thread_params
{
public:
    std::unordered_map<uint32_t, std::shared_ptr<base_bot>> bots;
};

class bot_container : public fb::async_executor
{
    template <typename BotType>
    friend class bot_controller;

private:
    std::atomic<uint32_t>    _sequence          = 0;
    uint32_t                 _gateway_bot_count = 0;
    uint32_t                 _login_bot_count   = 0;
    uint32_t                 _game_bot_count    = 0;
    boost::asio::io_context& _context;
    bool                     _exit = false;

public:
    fb::model::model                        model;
    std::shared_ptr<gateway_bot_controller> gateway;
    std::shared_ptr<login_bot_controller>   login;
    std::shared_ptr<game_bot_controller>    game;

public:
    bot_container(boost::asio::io_context& context, uint32_t thread_count);
    void set_gateway_bot_controller(std::shared_ptr<gateway_bot_controller> bot_controller);
    void set_login_bot_controller(std::shared_ptr<login_bot_controller> bot_controller);
    void set_game_bot_controller(std::shared_ptr<game_bot_controller> bot_controller);
    void initialize();
    ~bot_container();

private:
    template <typename T> std::shared_ptr<T> create()
    {
        auto id     = this->_sequence++;
        auto bot    = std::make_shared<T>(*this, id);
        std::ignore = bot->thread()->dispatch([id, bot](auto& thread) -> async::task<void> {
            auto params = thread.template data<bot_thread_params>();
            params->bots.insert({id, bot});
            co_return;
        });
        return bot;
    }

    template <typename T, typename Controller> std::shared_ptr<T> create(Controller& bot_controller)
    {
        auto id     = this->_sequence++;
        auto bot    = std::make_shared<T>(bot_controller, id);
        std::ignore = bot->thread()->dispatch([id, bot](auto& thread) -> async::task<void> {
            auto params = thread.template data<bot_thread_params>();
            params->bots.insert({id, bot});
            co_return;
        });
        return bot;
    }

    template <typename T, typename Controller> std::shared_ptr<T> create(Controller& bot_controller, const fb::stream& params)
    {
        auto id     = this->_sequence++;
        auto bot    = std::make_shared<T>(bot_controller, id, params);
        std::ignore = bot->thread()->dispatch([id, bot](auto& thread) -> async::task<void> {
            auto params = thread.template data<bot_thread_params>();
            params->bots.insert({id, bot});
            co_return;
        });
        return bot;
    }

public:
    boost::asio::io_context& context() const;
    async::task<void>        dispatch(uint32_t id, std::function<async::task<void>(fb::thread&)>&& fn);
};

} // namespace fb::bot

#endif