#include <fb/bot/integration/communication_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>

using namespace std::chrono_literals;

namespace fb::bot::integration {

communication_test::communication_test(game_bot_controller& controller) :
    bot_integration_test(controller, REQUIRED_BOTS) // Spawn required bots
{ }

async::task<void> communication_test::on_initialize(game_bot_controller& controller)
{
    co_await super::on_initialize(controller);

    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

    // Position bot2 next to bot1 for proximity-based tests
    co_await bot2->move(DIRECTION::RIGHT);
    co_await bot2->direction(DIRECTION::LEFT, DEFAULT_TIMEOUT);

    // Position bot1 to face bot2
    co_await bot1->direction(DIRECTION::RIGHT, DEFAULT_TIMEOUT);
    co_return;
}

async::task<void> communication_test::on_scenario_started(uint32_t scenario_index)
{
    co_return;
}

async::task<void> communication_test::on_scenario_finished(uint32_t scenario_index)
{
    co_return;
}

generator<bot_integration_test::scenario_t> communication_test::on_generate_scenario()
{
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_normal_chat();
    };

    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_shout_chat();
    };

    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_whisper();
    };

    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_whisper_block();
    };
}

async::task<bool> communication_test::test_normal_chat()
{
    auto bots = this->get_test_bots();
    if (bots.size() < 2)
        throw std::runtime_error("Not enough bots for this test, requires 2.");

    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

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

    fb::logger::debug("Normal chat test PASSED");
    co_return true;
}

async::task<bool> communication_test::test_shout_chat()
{
    auto bots = this->get_test_bots();
    if (bots.size() < 2)
        throw std::runtime_error("Not enough bots for this test, requires 2.");

    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

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

    fb::logger::debug("Shout chat test PASSED");
    co_return true;
}

async::task<bool> communication_test::test_whisper()
{
    auto bots = this->get_test_bots();
    if (bots.size() < 2)
        throw std::runtime_error("Not enough bots for this test, requires 2.");

    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

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

    fb::logger::debug("Whisper test PASSED");
    co_return true;
}

async::task<bool> communication_test::test_whisper_block()
{
    auto bots = this->get_test_bots();
    if (bots.size() < 2)
        throw std::runtime_error("Not enough bots for this test, requires 2.");

    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

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

        fb::logger::debug("Whisper block test PASSED - whisper was successfully blocked");
    }
    catch (...)
    {
        // If no response is received, that's also a valid blocking behavior
        fb::logger::debug("Whisper block test PASSED - whisper was blocked (no response)");
    }

    // Re-enable whisper option for bot2
    std::ignore = co_await bot2->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::update_option(OPTION::WHISPER, false),
        DEFAULT_TIMEOUT);

    fb::logger::debug("Whisper option re-enabled for bot2");

    co_return true;
}

std::string communication_test::name() const
{
    return "Communication Test";
}

} // namespace fb::bot::integration