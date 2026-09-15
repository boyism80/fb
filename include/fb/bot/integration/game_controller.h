#ifndef __BOT_GAME_INTEGRATION_CONTROLLER_H__
#define __BOT_GAME_INTEGRATION_CONTROLLER_H__

#include <fb/bot/game_controller.h>
#include <fb/game/protocol.h>
#include <fb/bot/game_bot.h>
#include <fb/bot/integration/test_case.h>
#include <fb/synchronized.h>
#include <fb/model/loader.h>
#include <memory>
#include <queue>
#include <deque>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <typeindex>
#include <functional>
#include <shared_mutex>
#include <mutex>
#include <optional>
#include <chrono>

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

    std::deque<bot_integration_test*> _parallel_queue;
    std::deque<bot_integration_test*> _serial_queue;

    std::unordered_set<uint32_t>                        _free_seats;
    std::unordered_map<bot_integration_test*, uint32_t> _active_seats;
    uint32_t                                            _max_parallel_tests{4};
    bool                                                _serial_phase_started{false};
    bool                                                _finished{false};

    std::unordered_map<uint32_t, bot_integration_test*> _bot_owners;
    std::mutex                                          _schedule_mutex;

    struct test_result
    {
        std::string name;
        bool        success;
        std::string message;
    };
    std::vector<test_result>              _test_results;
    std::mutex                            _results_mutex;
    std::chrono::steady_clock::time_point _suite_start{};

    using hook_function = std::function<async::task<void>(game_bot&, const fb::protocol::header&)>;
    std::unordered_map<bot_integration_test*, std::unordered_map<uint8_t, std::vector<hook_function>>> _test_hooks;
    std::shared_mutex                                                                                  _hook_mutex;

public:
    game_bot_controller(bot_container& container);

    void initialize() override;

    void                  own(uint32_t bot_id, bot_integration_test* test);
    void                  reown(uint32_t old_bot_id, uint32_t new_bot_id);
    bot_integration_test* owner_of(uint32_t bot_id);
    void                  clear_ownership_for_test(bot_integration_test* test);

    void     notify_test_ready(bot_integration_test* test);
    void     enqueue_test(std::unique_ptr<bot_integration_test> test, bool serial, bool extra_slot);
    bool     has_more_tests() const;
    void     print_final_test_results();
    uint32_t max_parallel_tests() const;

private:
    async::task<void> on_timer();
    async::task<void> on_time(game_bot& bot, const game_resp::time& response);
    async::task<void> on_state(game_bot& bot, const game_resp::update_internal_v550& response);
    async::task<void> on_map(game_bot& bot, const game_resp::map_config_v550& response);
    async::task<void> on_message(game_bot& bot, const game_resp::message& response);
    async::task<void> on_sequence(game_bot& bot, const game_resp::id& response);
    async::task<void> on_position(game_bot& bot, const game_resp::position& response);
    async::task<void> on_move(game_bot& bot, const game_resp::move& response);
    async::task<void> on_transfer(game_bot& bot, const fb::protocol::response::transfer& response);

    void              try_schedule_parallel();
    void              start_serial_phase();
    void              on_test_complete(bot_integration_test* test, bool success);
    async::task<void> run_one(bot_integration_test* test);
    void              detach_run_one(bot_integration_test* test);
    void              finish_suite_if_done();

protected:
    async::task<void> on_integration_hook_execution(uint8_t                     opcode,
                                                    game_bot&                   bot,
                                                    const fb::protocol::header& header) override;

public:
    virtual async::task<void> on_bot_connected(game_bot& bot) override;
    virtual async::task<void> on_bot_disconnected(game_bot& bot) override;

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
        this->_test_hooks[test][ResponseType::opcode].push_back(hook_func);
    }

    void hook_opcode(bot_integration_test* test, uint8_t opcode, hook_function fn);
    void unhook_opcode(bot_integration_test* test, uint8_t opcode);
};

} // namespace fb::bot::integration

#endif
