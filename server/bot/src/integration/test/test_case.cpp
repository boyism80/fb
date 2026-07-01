#include <fb/bot/integration/test_case.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>
#include <fb/logger.h>
#include <fb/game/protocol.h>
#include <fb/config.h>
#include <format>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

bot_integration_test::bot_integration_test(game_bot_controller& controller, uint32_t bot_count) :
    controller(controller),
    bot_count(bot_count)
{
    // Register common hooks for sequence and position responses
    this->controller.hook(this, this, &bot_integration_test::on_hook_sequence);
    this->controller.hook(this, this, &bot_integration_test::on_hook_position);
    this->controller.hook(this, this, &bot_integration_test::on_hook_update_external);
    this->controller.hook(this, this, &bot_integration_test::on_hook_update_external_brief);
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
    this->try_notify_ready();
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
        if (bot == nullptr)
            return false;

        if (bot->oid() == 0)
            return false;
    }

    return true;
}

void bot_integration_test::mark_bot_logged_in(game_bot& bot)
{
    if (bot.inited() == false)
        bot.inited(true);
}

void bot_integration_test::try_notify_ready()
{
    if (this->is_ready() == false)
        return;

    if (this->get_state() != test_state::idle)
        return;

    this->set_state(test_state::ready);
    this->notify_ready();
}

std::vector<std::shared_ptr<fb::bot::game_bot>> bot_integration_test::get_test_bots() const
{
    return this->_test_bots;
}

async::task<void> bot_integration_test::on_activated(game_bot_controller& controller)
{
    auto ip = controller.container.ipv4(fb::config<std::string_view>("ip"));
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

async::task<bool> bot_integration_test::check_should_skip()
{
    co_return false;
}

async::task<bool> bot_integration_test::execute()
{
    if (this->get_state() == test_state::running || this->get_state() == test_state::completed)
        co_return false;

    this->set_state(test_state::running);

    if (co_await this->check_should_skip())
    {
        fb::logger::debug("{}: test skipped by should_skip", this->name());
        co_await this->on_finished();
        this->set_state(test_state::completed);
        co_return true;
    }

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

async::task<bool>
bot_integration_test::parallel_scenarios(const std::vector<std::pair<uint32_t, scenario_t>>& scenarios)
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

async::task<void> bot_integration_test::on_hook_sequence(fb::bot::game_bot& bot, const game_resp::id& resp)
{
    if (bot.oid() == resp.oid)
        this->mark_bot_logged_in(bot);

    this->try_notify_ready();
    co_return;
}

async::task<void> bot_integration_test::on_hook_position(fb::bot::game_bot& bot, const game_resp::position& resp)
{
    this->mark_bot_logged_in(bot);
    this->try_notify_ready();
    co_return;
}

async::task<void> bot_integration_test::on_hook_update_external(fb::bot::game_bot&                      bot,
                                                                const game_resp::update_external<true>& resp)
{
    if (bot.oid() == 0)
        bot.set_oid(resp.oid);

    if (this->controller.has_transfer_context(bot.name()))
    {
        auto reconnected_index = std::optional<uint32_t>{};
        auto it                = std::find_if(this->_test_bots.begin(), this->_test_bots.end(), [&bot](auto& b) {
            return b.get() == &bot;
        });
        for (int i = 0; i < this->_test_bots.size(); i++)
        {
            auto& b = this->_test_bots[i];
            if (&bot == b.get())
                continue;

            if (b->name() == bot.name())
            {
                reconnected_index = i;
                break;
            }
        }

        fb::logger::debug("bot transfer hook: test={} bot={} bot_id={} oid={} inited={} reconnected_index={}",
                          this->name(),
                          bot.name(),
                          bot.id,
                          bot.oid(),
                          bot.inited(),
                          reconnected_index.has_value() ? std::to_string(reconnected_index.value()) : "none");

        if (reconnected_index.has_value())
        {
            this->_test_bots[reconnected_index.value()] = std::move(*it);
            this->_test_bots.erase(it);
        }

        const auto invoked =
            this->controller.invoke_transfer_context(bot.name(), bot.shared_from_this_as<game_bot>());
        if (invoked == false)
        {
            fb::logger::warn("bot transfer hook: test={} bot={} bot_id={} invoke_transfer_context failed",
                             this->name(),
                             bot.name(),
                             bot.id);
        }
    }

    this->mark_bot_logged_in(bot);
    this->try_notify_ready();
    co_return;
}

async::task<void> bot_integration_test::on_hook_update_external_brief(fb::bot::game_bot&                       bot,
                                                                      const game_resp::update_external<false>& resp)
{
    if (bot.oid() == 0)
        bot.set_oid(resp.oid);

    if (bot.oid() == resp.oid)
        this->mark_bot_logged_in(bot);

    this->try_notify_ready();
    co_return;
}

async::task<void> bot_integration_test::sleep(std::chrono::milliseconds duration)
{
    co_await this->controller.container.threads.current()->sleep(duration);
}
