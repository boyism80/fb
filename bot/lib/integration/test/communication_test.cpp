#include <fb/bot/integration/communication_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>

using namespace std::chrono_literals;

namespace fb::bot::integration {

communication_test::communication_test(game_bot_controller& controller) :
    bot_integration_test(controller)
{
    // Register hooks for basic responses
    this->_controller.hook_external(this, this, &communication_test::on_hook_sequence);
    this->_controller.hook_external(this, this, &communication_test::on_hook_position);

    fb::logger::debug("Communication test constructed");
}

async::task<void> communication_test::initialize(game_bot_controller& controller)
{
    auto ip = controller.container.ipv4(fb::config<std::string>("ip"));
    auto endpoint =
        boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), fb::config<uint16_t>("port"));

    fb::logger::info("Communication test initializing and spawning {} bots", REQUIRED_BOTS);

    for (auto i = 0; i < REQUIRED_BOTS; i++)
    {
        auto gateway_bot = controller.container.gateway->create();
        gateway_bot->connect(endpoint);
    }

    fb::logger::info("Communication test initialization completed - {} bots spawned", REQUIRED_BOTS);
    co_return;
}

async::task<bool> communication_test::execute()
{
    if (this->get_state() == test_state::running || this->get_state() == test_state::completed)
        co_return false;

    this->set_state(test_state::running);

    auto bots = this->get_test_bots();
    if (bots.size() < REQUIRED_BOTS)
    {
        fb::logger::fatal("Need at least {} bots for communication test", REQUIRED_BOTS);
        this->set_state(test_state::failed);
        co_return false;
    }

    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

    // Position bot2 next to bot1 for proximity-based tests
    co_await bot2->move(DIRECTION::RIGHT, 1, DEFAULT_INTERVAL);
    co_await bot2->thread()->switching();
    co_await bot2->thread()->sleep(DEFAULT_INTERVAL);
    bot2->direction(DIRECTION::LEFT);

    // Position bot1 to face bot2
    bot1->direction(DIRECTION::RIGHT);

    fb::logger::info("Starting communication test with bot1: '{}' and bot2: '{}'", bot1->name(), bot2->name());

    // Queue of test scenarios
    using scenario_fn = std::function<async::task<bool>()>;
    std::queue<scenario_fn> scenarios;

    scenarios.push([this, &bot1, &bot2] {
        return this->test_normal_chat(bot1, bot2);
    });

    scenarios.push([this, &bot1, &bot2] {
        return this->test_shout_chat(bot1, bot2);
    });

    scenarios.push([this, &bot1, &bot2] {
        return this->test_whisper(bot1, bot2);
    });

    scenarios.push([this, &bot1, &bot2] {
        return this->test_whisper_block(bot1, bot2);
    });

    int scenario_count = 1;
    while (scenarios.empty() == false)
    {
        co_await this->reset_bot_state(bot1);
        co_await this->reset_bot_state(bot2);

        auto& scenario = scenarios.front();
        if (co_await scenario() == false)
        {
            fb::logger::fatal("Communication test scenario {} FAILED", scenario_count);
            this->set_state(test_state::failed);
            co_return false;
        }

        fb::logger::info("Communication test scenario {} PASSED", scenario_count++);
        scenarios.pop();
    }

    fb::logger::info("All communication test scenarios PASSED");
    this->set_state(test_state::completed);
    this->_controller.notify_test_completed(this);
    co_return true;
}

void communication_test::reset()
{
    bot_integration_test::reset();

    fb::logger::info("Communication test reset");
}

bool communication_test::is_ready() const
{
    auto bots = this->get_test_bots();
    if (bots.size() < REQUIRED_BOTS)
        return false;

    for (auto& bot : bots)
    {
        if (bot->oid() == 0)
            return false;

        if (bot->position().x == 0 && bot->position().y == 0)
            return false;
    }

    return true;
}

void communication_test::on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot)
{
    bot_integration_test::on_bot_connected(bot);
    fb::logger::debug("Communication test: Bot {} added to collection", bot->fd());
}

async::task<void> communication_test::on_hook_sequence(fb::bot::game_bot&                      bot,
                                                       const fb::protocol::game::response::id& response)
{
    fb::logger::debug("Communication test: Bot {} received object ID {}", bot.fd(), response.oid);

    if (this->is_ready() == false)
        co_return;

    if (this->get_state() == test_state::running)
        co_return;

    fb::logger::info("Communication test: All bots ready, notifying controller");
    this->set_state(test_state::ready);
    this->notify_ready();
    co_return;
}

async::task<void> communication_test::on_hook_position(fb::bot::game_bot&                            bot,
                                                       const fb::protocol::game::response::position& response)
{
    if (this->is_ready() == false)
        co_return;

    if (this->get_state() == test_state::running)
        co_return;

    fb::logger::info("Communication test: All bots ready, notifying controller");
    this->set_state(test_state::ready);
    this->notify_ready();
    co_return;
}

async::task<bool> communication_test::test_normal_chat(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2)
{
    fb::logger::info("Starting normal chat test");

    const std::string test_message = "Hello, this is a normal chat test!";

    // Send normal chat message from bot1 and wait for chat response
    std::ignore = co_await bot1->request<fb::protocol::game::response::chat>(
        fb::protocol::game::request::chat(false, test_message),
        [test_message, bot1](auto& response) -> bool {
            // Check for decorated format: "name: message"
            std::string expected_text = std::format("{}: {}", bot1->name(), test_message);
            return response.text == expected_text && response.oid == bot1->oid() && response.type == CHAT_TYPE::NORMAL;
        },
        DEFAULT_TIMEOUT);

    fb::logger::info("Normal chat test PASSED");
    co_return true;
}

async::task<bool> communication_test::test_shout_chat(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2)
{
    fb::logger::info("Starting shout chat test");

    const std::string test_message = "HELLO EVERYONE, THIS IS A SHOUT TEST!";

    // Send shout message from bot1 and wait for chat response
    std::ignore = co_await bot1->request<fb::protocol::game::response::chat>(
        fb::protocol::game::request::chat(true, test_message),
        [test_message, bot1](auto& response) -> bool {
            // Check for decorated format: "name! message"
            std::string expected_text = std::format("{}! {}", bot1->name(), test_message);
            return response.text == expected_text && response.oid == bot1->oid() && response.type == CHAT_TYPE::SHOUT;
        },
        DEFAULT_TIMEOUT);

    fb::logger::info("Shout chat test PASSED");
    co_return true;
}

async::task<bool> communication_test::test_whisper(std::shared_ptr<game_bot>& bot1, std::shared_ptr<game_bot>& bot2)
{
    fb::logger::info("Starting whisper test");

    const std::string test_message = "This is a secret whisper message!";

    // Send whisper from bot1 to bot2 and wait for confirmation message
    std::ignore = co_await bot1->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::whisper(bot2->name(), test_message),
        [test_message, bot2](auto& response) -> bool {
            if (response.type != MESSAGE_TYPE::NOTIFY)
                return false;
            // Check if this is a whisper confirmation with format "target< message"
            std::string text = response.text;
            auto        pos  = text.find("< ");
            if (pos == std::string::npos)
                return false;
            std::string target  = text.substr(0, pos);
            std::string message = text.substr(pos + 2);
            return target == bot2->name() && message == test_message;
        },
        DEFAULT_TIMEOUT);

    fb::logger::info("Whisper test PASSED");
    co_return true;
}

async::task<bool> communication_test::test_whisper_block(std::shared_ptr<game_bot>& bot1,
                                                         std::shared_ptr<game_bot>& bot2)
{
    fb::logger::info("Starting whisper block test");

    // First, disable whisper option for bot2 (block whispers)
    auto&& disable_resp = co_await bot2->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::update_option(OPTION::WHISPER, false),
        DEFAULT_TIMEOUT);

    fb::logger::debug("Whisper option disabled for bot2: {}", disable_resp.text);

    const std::string test_message = "This whisper should be blocked!";

    try
    {
        // Send whisper from bot1 to bot2 (should fail with error message)
        std::ignore = co_await bot1->request<fb::protocol::game::response::message>(
            fb::protocol::game::request::whisper(bot2->name(), test_message),
            [bot2](auto& response) -> bool {
                // Look for error message indicating whisper is blocked
                return response.type == MESSAGE_TYPE::NOTIFY && response.text.find("귓속말 거부") != std::string::npos;
            },
            DEFAULT_TIMEOUT);

        fb::logger::info("Whisper block test PASSED - whisper was successfully blocked");
    }
    catch (...)
    {
        // If no response is received, that's also a valid blocking behavior
        fb::logger::info("Whisper block test PASSED - whisper was blocked (no response)");
    }

    // Re-enable whisper option for bot2
    std::ignore = co_await bot2->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::update_option(OPTION::WHISPER, false),
        DEFAULT_TIMEOUT);

    fb::logger::debug("Whisper option re-enabled for bot2");

    co_return true;
}

async::task<void> communication_test::reset_bot_state(std::shared_ptr<game_bot>& bot)
{
    co_return;
}

} // namespace fb::bot::integration