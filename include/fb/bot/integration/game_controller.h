#ifndef __BOT_GAME_INTEGRATION_CONTROLLER_H__
#define __BOT_GAME_INTEGRATION_CONTROLLER_H__

#include <fb/bot/game_controller.h>
#include <fb/game/protocol.h>
#include <fb/bot/game_bot.h>
#include <fb/bot/integration/test_case.h>
#include <fb/locker.h>
#include <fb/model/loader.h>
#include <memory>
#include <queue>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <shared_mutex>

namespace fb::bot::integration {

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

class game_bot_controller : public fb::bot::game_bot_controller
{
private:
    using super    = fb::bot::game_bot_controller;
    using bot_type = game_bot;

private:
    std::vector<std::unique_ptr<bot_integration_test>> _test_instances;
    std::queue<bot_integration_test*>                  _test_queue;
    bot_integration_test*                              _current_test{nullptr};

    struct test_result
    {
        std::string name;
        bool        success;
        std::string message;
    };
    std::vector<test_result> _test_results;

    using hook_function = std::function<async::task<void>(game_bot&, const fb::protocol::header&)>;
    std::unordered_map<bot_integration_test*, std::unordered_map<uint8_t, std::vector<hook_function>>> _test_hooks;
    std::shared_mutex                                                                                  _hook_mutex;

public:
    game_bot_controller(bot_container& container);

    void initialize() override;

private:
    async::task<void> on_timer();
    async::task<void> on_time(game_bot& bot, const game_resp::time& response);
    async::task<void> on_state(game_bot& bot, const game_resp::update_internal& response);
    async::task<void> on_map(game_bot& bot, const game_resp::map_config& response);
    async::task<void> on_message(game_bot& bot, const game_resp::message& response);
    async::task<void> on_sequence(game_bot& bot, const game_resp::id& response);
    async::task<void> on_position(game_bot& bot, const game_resp::position& response);
    async::task<void> on_move(game_bot& bot, const game_resp::move& response);
    async::task<void> on_transfer(game_bot& bot, const fb::protocol::response::transfer& response);

protected:
    async::task<void> on_integration_hook_execution(uint8_t                     cmd,
                                                    game_bot&                   bot,
                                                    const fb::protocol::header& header) override;

public:
    virtual async::task<void> on_bot_connected(game_bot& bot) override;
    virtual async::task<void> on_bot_disconnected(game_bot& bot) override;

    void              notify_test_ready();
    async::task<void> start_current_test();
    void              enqueue_test(std::unique_ptr<bot_integration_test> test);
    void              start_next_test();
    bool              has_more_tests() const;
    void              print_final_test_results();
    async::task<void> active_test();

    template <typename ResponseType> void
    hook_for_test(bot_integration_test*                                                   test,
                  const std::function<async::task<void>(game_bot&, const ResponseType&)>& fn)
    {
        auto hook_func = [fn](game_bot& bot, const fb::protocol::header& header) -> async::task<void> {
            auto& protocol = static_cast<const ResponseType&>(header);
            co_await fn(bot, protocol);
        };

        auto unique_lock = std::unique_lock<std::shared_mutex>(this->_hook_mutex);
        this->_test_hooks[test][ResponseType::header].push_back(hook_func);
    }

    template <typename Class, typename ResponseType> void
    hook(bot_integration_test* test, Class* instance, async::task<void> (Class::*fn)(game_bot&, const ResponseType&))
    {
        auto hook_func = [instance, fn](game_bot& bot, const fb::protocol::header& header) -> async::task<void> {
            auto& protocol = static_cast<const ResponseType&>(header);
            co_await (instance->*fn)(bot, protocol);
        };

        auto unique_lock = std::unique_lock<std::shared_mutex>(this->_hook_mutex);
        this->_test_hooks[test][ResponseType::header].push_back(hook_func);
    }
};

} // namespace fb::bot::integration

#endif