#ifndef __BOT_INTEGRATION_TEST_CASE_H__
#define __BOT_INTEGRATION_TEST_CASE_H__

#include <fb/bot/game_bot.h>
#include <fb/synchronized.h>
#include <async/task.h>
#include <memory>
#include <vector>
#include <atomic>
#include <string>
#include <boost/asio.hpp>
#include <chrono>
#include <functional>
#include <optional>
#include <fb/generator.h>

namespace fb::bot::integration {

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

class game_bot_controller;

struct spawned_monster_info
{
    uint32_t                   oid;
    fb::model::point<uint16_t> position;
    uint32_t                   look;
};

class bot_integration_test
{
public:
    using test_bots_t = std::vector<std::shared_ptr<game_bot>>;
    using scenario_t  = std::function<async::task<bool>()>;

    enum class test_state : uint8_t
    {
        idle,
        ready,
        running,
        completed,
        failed
    };

    struct parallel_scenarios_context
    {
        std::unordered_map<uint32_t, std::queue<scenario_t>> queues;
        std::shared_ptr<async::task_completion_source<bool>> promise;
        std::atomic<int>                                     processed;
        std::atomic<bool>                                    success;
        int                                                  count;
    };

private:
    test_bots_t            _test_bots;
    test_state             _state{test_state::idle};
    std::queue<scenario_t> _scenario_queue;

public:
    game_bot_controller& controller;
    const uint32_t       bot_count;

protected:
#ifdef _DEBUG
    static constexpr auto DEFAULT_TIMEOUT  = 1h;
    static constexpr auto DEFAULT_INTERVAL = 100ms;
#else
    static constexpr auto DEFAULT_TIMEOUT  = 10s;
    static constexpr auto DEFAULT_INTERVAL = 100ms;
#endif

protected:
    bot_integration_test(game_bot_controller& controller, uint32_t bot_count = 1);

public:
    virtual ~bot_integration_test() = default;

private:
    async::task<void> execute_parallel_scenario(std::shared_ptr<parallel_scenarios_context> context, uint32_t index);

protected:
    virtual generator<scenario_t> on_generate_scenario() = 0;
    virtual async::task<void>     on_scenario_started(uint32_t scenario_index);
    virtual async::task<void>     on_scenario_finished(uint32_t scenario_index);
    virtual async::task<void>     on_parallel_scenario_started(uint32_t id);
    virtual async::task<void>     on_parallel_scenario_finished(uint32_t id);
    async::task<bool>             parallel_scenarios(const std::vector<std::pair<uint32_t, scenario_t>>& scenarios);
    virtual async::task<void>     on_hook_sequence(game_bot& bot, const game_resp::id& resp);
    virtual async::task<void>     on_hook_position(game_bot& bot, const game_resp::position& resp);
    virtual async::task<void>     on_hook_update_external(game_bot& bot, const game_resp::update_external<true>& resp);

    [[nodiscard]] async::task<void> sleep(std::chrono::milliseconds duration);
    [[nodiscard]] async::task<void> arrange_bots_in_line_formation();
    [[nodiscard]] async::task<void>
    arrange_bots_in_grid_formation(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y);
    [[nodiscard]] async::task<void> form_group();
    [[nodiscard]] async::task<void> cleanup_group();

public:
    test_state                             get_state() const;
    void                                   set_state(test_state state);
    bool                                   is_complete() const;
    bool                                   is_running() const;
    virtual void                           on_bot_connected(std::shared_ptr<game_bot> bot);
    virtual void                           on_bot_disconnected(std::shared_ptr<game_bot> bot);
    void                                   notify_ready();
    virtual bool                           is_ready() const;
    std::vector<std::shared_ptr<game_bot>> get_test_bots() const;
    virtual async::task<void>              on_activated(game_bot_controller& controller);
    virtual async::task<void>              on_initialize(game_bot_controller& controller);
    virtual async::task<void>              on_finished();
    async::task<bool>                      execute();
    virtual std::string                    name() const = 0;
};

} // namespace fb::bot::integration

#endif // __BOT_INTEGRATION_TEST_CASE_H__