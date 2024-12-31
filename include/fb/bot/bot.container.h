#ifndef __BOT_CONTAINER_H__
#define __BOT_CONTAINER_H__

#include <fb/bot/bot.h>
#include <fb/thread_container.h>

namespace fb::bot {

class bot_thread_params
{
public:
    std::unordered_map<uint32_t, std::shared_ptr<base_bot>> bots;
};

class bot_container : public fb::context
{
private:
    uint32_t                 _remained_count;
    uint32_t                 _sequence          = 0;
    uint32_t                 _gateway_bot_count = 0;
    uint32_t                 _login_bot_count   = 0;
    uint32_t                 _game_bot_count    = 0;
    boost::asio::io_context& _context;
    bool                     _exit = false;
    std::mutex               _mutex;

public:
    bot_container(boost::asio::io_context& context);
    ~bot_container();

private:
    async::task<void> handle_bot_spawn();

public:
    boost::asio::io_context& context() const;

    async::task<void> on_receive(fb::socket<>& socket, fb::stream& stream)
    {
        co_await static_cast<base_bot&>(socket).on_receive(stream);
    }

    async::task<void> on_closed(fb::socket<>& socket)
    {
        auto& bot   = static_cast<base_bot&>(socket);
        std::ignore = bot.thread()->dispatch([&bot](auto& thread) -> async::task<void> {
            co_await bot.on_closed();
            auto params = thread.template data<bot_thread_params>();
            params->bots.erase(bot.id);
        });
        co_return;
    }

    template <typename T>
    std::shared_ptr<T> create()
    {
        this->_mutex.lock();
        auto id = this->_sequence++;
        this->_mutex.unlock();
        auto bot    = std::make_shared<T>(*this, id);
        std::ignore = bot->thread()->dispatch([id, bot](auto& thread) -> async::task<void> {
            auto params = thread.template data<bot_thread_params>();
            params->bots.insert({id, bot});
            co_return;
        });
        return bot;
    }

    template <typename T>
    std::shared_ptr<T> create(const fb::stream& params)
    {
        this->_mutex.lock();
        auto id = this->_sequence++;
        this->_mutex.unlock();
        auto bot    = std::make_shared<T>(*this, id, params);
        std::ignore = bot->thread()->dispatch([id, bot](auto& thread) -> async::task<void> {
            auto params = thread.template data<bot_thread_params>();
            params->bots.insert({id, bot});
            co_return;
        });
        return bot;
    }
    async::task<void> handle_timer(const fb::model::datetime& now, std::thread::id id);
    async::task<void> dispatch(uint32_t id, std::function<async::task<void>(fb::thread&)>&& fn);
    static void       display_spawned_bots();
};

} // namespace fb::bot

#endif