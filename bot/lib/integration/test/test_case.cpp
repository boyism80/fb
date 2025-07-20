#include <fb/bot/integration/test_case.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>
#include <fb/logger.h>
#include <fb/game/protocol.h>
#include <fb/config.h>
#include <format>

using namespace std::chrono_literals;

namespace fb::bot::integration {

bot_integration_test::bot_integration_test(game_bot_controller& controller, uint32_t bot_count) :
    controller(controller),
    bot_count(bot_count)
{
    // Register common hooks for sequence and position responses
    this->controller.hook_external(this, this, &bot_integration_test::on_hook_sequence);
    this->controller.hook_external(this, this, &bot_integration_test::on_hook_position);
    this->controller.hook_external(this, this, &bot_integration_test::on_hook_update_external);
}

bot_integration_test::test_state bot_integration_test::get_state() const
{
    return this->_state;
}

void bot_integration_test::set_state(test_state state)
{
    this->_state = state;
}

bool bot_integration_test::is_complete() const
{
    return this->_state == test_state::completed;
}

bool bot_integration_test::is_running() const
{
    return this->_state == test_state::running;
}

void bot_integration_test::on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot)
{
    this->_test_bots.push_back(bot);
    fb::logger::debug("{}: Bot {} added to collection", this->name(), bot->fd());
}

void bot_integration_test::on_bot_disconnected(std::shared_ptr<fb::bot::game_bot> bot)
{
    fb::logger::debug("{}: Bot {} disconnected", this->name(), bot->fd());
}

void bot_integration_test::notify_ready()
{
    this->controller.notify_test_ready();
}

async::task<void> bot_integration_test::on_finished()
{
    for (auto& bot : this->get_test_bots())
    {
        if (bot)
            bot->close();
    }

    fb::logger::debug("{} test finished - all bots disconnected", this->name());
    co_return;
}

bool bot_integration_test::is_ready() const
{
    auto bots = this->get_test_bots();
    if (bots.size() < this->bot_count)
        return false;

    for (auto& bot : bots)
    {
        if (bot->inited() == false)
            return false;
    }

    return true;
}

std::vector<std::shared_ptr<fb::bot::game_bot>> bot_integration_test::get_test_bots() const
{
    return this->_test_bots;
}

async::task<void> bot_integration_test::on_active(game_bot_controller& controller)
{
    auto ip = controller.container.ipv4(fb::config<std::string>("ip"));
    auto endpoint =
        boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), fb::config<uint16_t>("port"));

    fb::logger::debug("{} initializing and spawning {} bots", this->name(), this->bot_count);

    for (auto i = 0u; i < this->bot_count; i++)
    {
        auto gateway_bot = controller.container.gateway->create();
        gateway_bot->connect(endpoint);
    }

    fb::logger::debug("{} initialization completed - {} bots spawned", this->name(), this->bot_count);

    auto scenario_generator = this->on_generate_scenario();
    while (scenario_generator.next())
    {
        auto scenario = scenario_generator.value();
        this->_scenario_queue.push(scenario);
    }

    co_return;
}

async::task<void> bot_integration_test::on_initialize(game_bot_controller& controller)
{
    auto bots = this->get_test_bots();
    for (int i = 0; i < bots.size(); i++)
    {
        auto& bot = bots[i];
        if (bot->position().x == 6 && bot->position().y == 6)
            continue;

        co_await bot->map_move("낙랑의방", 6, 6, DEFAULT_TIMEOUT);
    }
}

async::task<bool> bot_integration_test::execute()
{
    if (this->get_state() == test_state::running || this->get_state() == test_state::completed)
        co_return false;

    this->set_state(test_state::running);

    auto failed         = false;
    auto scenario_index = 0;
    fb::logger::debug("{}: Starting test execution", this->name());
    co_await this->on_initialize(this->controller);
    while (this->_scenario_queue.empty() == false)
    {
        auto scenario = this->_scenario_queue.front();
        this->_scenario_queue.pop();
        try
        {
            co_await this->on_scenario_started(scenario_index);
            if (co_await scenario() == false)
                failed = true;
        }
        catch (const std::exception& e)
        {
            fb::logger::fatal("{}: Scenario {} failed: {}", this->name(), scenario_index, e.what());
            failed = true;
        }

        co_await this->on_scenario_finished(scenario_index);
        scenario_index++;
    }

    co_await this->on_finished();
    if (failed)
        this->set_state(test_state::failed);
    else
        this->set_state(test_state::completed);

    co_return !failed;
}

async::task<void> bot_integration_test::on_scenario_started(uint32_t scenario_index)
{
    co_return;
}

async::task<void> bot_integration_test::on_scenario_finished(uint32_t scenario_index)
{
    co_return;
}

async::task<void> bot_integration_test::on_parallel_scenario_started(uint32_t id)
{
    co_return;
}

async::task<void> bot_integration_test::on_parallel_scenario_finished(uint32_t id)
{
    co_return;
}

async::task<void> bot_integration_test::execute_parallel_scenario(std::shared_ptr<parallel_scenarios_context> context,
                                                                  uint32_t                                    index)
{
    auto& queue = context->queues[index];
    while (queue.empty() == false)
    {
        auto scenario = queue.front();
        queue.pop();

        try
        {
            co_await this->on_parallel_scenario_started(index);
            if (co_await scenario() == false)
                context->success = false;
        }
        catch (const std::exception& e)
        {
            context->success = false;
        }
        catch (...)
        {
            context->success = false;
        }

        co_await this->on_parallel_scenario_finished(index);
        context->processed++;
        if (context->processed == context->count)
            context->promise->set_value(context->success);
    }
}

async::task<bool> bot_integration_test::parallel_scenarios(std::vector<std::pair<uint32_t, scenario_t>> scenarios)
{
    auto context       = std::make_shared<parallel_scenarios_context>();
    context->promise   = std::make_shared<async::task_completion_source<bool>>();
    context->processed = 0;
    context->success   = true;
    context->queues    = std::unordered_map<uint32_t, std::queue<scenario_t>>();

    for (auto& [index, scenario] : scenarios)
    {
        if (context->queues.contains(index) == false)
            context->queues.insert({index, std::queue<scenario_t>()});

        context->queues[index].push(scenario);
        context->count++;
    }

    for (auto& [index, queue] : context->queues)
    {
        async::awaitable_then(this->execute_parallel_scenario(context, index), [this](auto result) {
            try
            {
                result();
            }
            catch (const std::exception& e)
            {
                fb::logger::fatal("{}: Parallel scenario failed: {}", this->name(), e.what());
            }
            catch (...)
            {
                fb::logger::fatal("{}: Parallel scenario failed", this->name());
            }
        });
    }

    return context->promise->task();
}

async::task<void> bot_integration_test::on_hook_sequence(fb::bot::game_bot&                      bot,
                                                         const fb::protocol::game::response::id& response)
{
    if (this->is_ready() == false)
        co_return;

    if (this->get_state() == test_state::running)
        co_return;

    fb::logger::debug("{}: All bots ready, notifying controller", this->name());
    this->set_state(test_state::ready);
    this->notify_ready();
    co_return;
}

async::task<void> bot_integration_test::on_hook_position(fb::bot::game_bot&                            bot,
                                                         const fb::protocol::game::response::position& response)
{
    if (this->is_ready() == false)
        co_return;

    if (this->get_state() == test_state::running)
        co_return;

    fb::logger::debug("{}: All bots ready, notifying controller", this->name());
    this->set_state(test_state::ready);
    this->notify_ready();
    co_return;
}

async::task<void>
bot_integration_test::on_hook_update_external(fb::bot::game_bot&                                         bot,
                                              const fb::protocol::game::response::update_external<true>& response)
{
    if (bot.inited() == false)
    {
        bot.inited(true);

        auto id = bot.id;
        this->controller.invoke_transfer_context(bot.name(), bot.shared_from_this_as<game_bot>());
    }

    if (this->is_ready() == false)
        co_return;

    if (this->get_state() >= test_state::running)
        co_return;

    fb::logger::debug("{}: All bots ready, notifying controller", this->name());
    this->set_state(test_state::ready);
    this->notify_ready();
    co_return;
}

async::task<void> bot_integration_test::sleep(std::chrono::milliseconds duration)
{
    co_await this->controller.container.threads.current()->sleep(duration);
}

async::task<void> bot_integration_test::arrange_bots_in_line_formation()
{
    auto bots = this->get_test_bots();
    for (int i = static_cast<int>(bots.size()) - 1; i >= 1; --i)
    {
        auto& bot = bots[i];

        auto current_position = bot->position();
        auto target_position  = current_position;
        co_await bot->move(DIRECTION::RIGHT, i, DEFAULT_INTERVAL);

        co_await bot->direction(DIRECTION::BOTTOM, DEFAULT_INTERVAL);
        fb::logger::debug("Bot {} positioned at ({}, {}) facing BOTTOM",
                          bot->name(),
                          target_position.x,
                          target_position.y);
    }
}

async::task<void>
bot_integration_test::arrange_bots_in_grid_formation(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y)
{
    auto bots = this->get_test_bots();

    // Calculate grid dimensions
    auto grid_width  = end_x - start_x + 1;
    auto grid_height = end_y - start_y + 1;

    // Position bots in a 2D grid from (start_x, start_y) to (end_x, end_y)
    for (int i = 0; i < static_cast<int>(bots.size()); ++i)
    {
        auto& bot = bots[i];

        // Calculate grid position
        int grid_x = start_x + (i % grid_width);
        int grid_y = start_y + (i / grid_width);

        // Move bot to calculated position
        if (bot == bots.back())
            co_await bot->map_move("낙랑의방", grid_x, grid_y, DEFAULT_TIMEOUT);
        else
            std::ignore = bot->map_move("낙랑의방", grid_x, grid_y, DEFAULT_TIMEOUT);

        fb::logger::debug("Bot {} positioned at ({}, {}) facing BOTTOM",
                          bot->fd(),
                          bot->position().x,
                          bot->position().y);
    }
}

async::task<void> bot_integration_test::form_group()
{
    auto  bots   = this->get_test_bots();
    auto& caster = bots.front();

    // Caster invites all other bots to the group
    for (size_t i = 1; i < bots.size(); ++i)
    {
        auto& target_bot = bots[i];
        fb::logger::debug("Inviting bot {} to group", target_bot->name());

        // Send group invitation
        auto group_request = fb::protocol::game::request::group{};
        group_request.name = target_bot->name();
        auto&& resp        = co_await caster->request<fb::protocol::game::response::message>(
            group_request,
            [](auto& resp) -> bool {
                if (resp.type != MESSAGE_TYPE::STATE)
                    return false;

                return resp.text.find("님 그룹 참여") != std::string::npos;
            },
            DEFAULT_TIMEOUT);
        caster->chat(resp.text);
    }

    fb::logger::debug("Group formation completed with {} members", bots.size());
}

async::task<void> bot_integration_test::cleanup_group()
{
    auto bots = this->get_test_bots();
    fb::logger::debug("Cleaning up group formation");

    // Cleanup group option for all bots
    for (auto& bot : bots)
    {
        for (int i = 0; i < 2; i++)
        {
            std::ignore = co_await bot->request<fb::protocol::game::response::message>(
                fb::protocol::game::request::update_option(OPTION::GROUP, false),
                [](auto& resp) -> bool {
                    return resp.type == MESSAGE_TYPE::STATE;
                },
                DEFAULT_TIMEOUT);
        }
    }

    fb::logger::debug("Group cleanup completed");
}

} // namespace fb::bot::integration