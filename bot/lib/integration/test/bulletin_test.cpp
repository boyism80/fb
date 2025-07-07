#include <fb/bot/integration/bulletin_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>

using namespace std::chrono_literals;

namespace fb::bot::integration {

bulletin_test::bulletin_test(game_bot_controller& controller) :
    bot_integration_test(controller)
{
    // Register hook for object ID (sequence) responses
    this->_controller.hook_external(this, this, &bulletin_test::on_hook_sequence);
    this->_controller.hook_external(this, this, &bulletin_test::on_hook_position);

    fb::logger::debug("Bulletin test constructed");
}

async::task<void> bulletin_test::initialize(game_bot_controller& controller)
{
    constexpr auto REQUIRED_BOTS = 1;

    auto ip = controller.container.ipv4(fb::config<std::string>("ip"));
    auto endpoint =
        boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), fb::config<uint16_t>("port"));

    fb::logger::info("Bulletin test initializing and spawning {} bot", REQUIRED_BOTS);

    for (auto i = 0; i < REQUIRED_BOTS; i++)
    {
        auto gateway_bot = controller.container.gateway->create();
        gateway_bot->connect(endpoint);
    }

    fb::logger::info("Bulletin test initialization completed - {} bot spawned", REQUIRED_BOTS);
    co_return;
}

async::task<bool> bulletin_test::execute()
{
    if (this->get_state() == test_state::running || this->get_state() == test_state::completed)
        co_return false;

    this->set_state(test_state::running);

    auto  bots = this->get_test_bots();
    auto& bot  = bots.front();
    fb::logger::info("Starting bulletin test with {} bot", bots.size());

    if (bots.empty())
    {
        fb::logger::fatal("No bots available for bulletin test");
        this->set_state(test_state::failed);
        co_return false;
    }

    // Test bulletin write functionality
    fb::logger::info("Testing bulletin write functionality");

    // First write attempt - should fail due to low level
    fb::logger::info("Testing bulletin write with low level (should fail)");
    if (co_await this->write(bot,
                             "테스트 게시글 제목",
                             "테스트 게시글 내용입니다. 이것은 봇 테스트를 위한 게시글입니다."))
    {
        fb::logger::fatal("Bulletin write test should have failed with low level");
        bot->chat("=== BULLETIN WRITE TEST FAILED - Should have failed with low level ===");
        this->set_state(test_state::failed);
        co_return false;
    }

    fb::logger::info("Bulletin write failed as expected with low level");
    bot->chat("=== BULLETIN WRITE FAILED AS EXPECTED (LOW LEVEL) ===");

    // Change level to 10
    fb::logger::info("Changing level to 10");
    co_await bot->change_level(10, 10s);

    fb::logger::info("Level changed to 10, testing bulletin write again");
    bot->chat("=== LEVEL CHANGED TO 10, TESTING BULLETIN WRITE AGAIN ===");

    // Second write attempt - should succeed with level 10
    if (!co_await this->write(bot,
                              "테스트 게시글 제목",
                              "테스트 게시글 내용입니다. 이것은 봇 테스트를 위한 게시글입니다."))
    {
        fb::logger::fatal("Bulletin write test failed with level 10");
        bot->chat("=== BULLETIN WRITE TEST FAILED WITH LEVEL 10 ===");
        this->set_state(test_state::failed);
        co_return false;
    }

    fb::logger::info("Bulletin write test successful with level 10");
    bot->chat("=== BULLETIN WRITE TEST COMPLETED SUCCESSFULLY WITH LEVEL 10 ===");

    fb::logger::info("Bulletin test completed successfully");

    this->set_state(test_state::completed);

    // Notify controller that this test is completed
    this->_controller.notify_test_completed(this);

    co_return true;
}

async::task<bool> bulletin_test::write(std::shared_ptr<fb::bot::game_bot> bot,
                                       const std::string&                 title,
                                       const std::string&                 contents)
{
    constexpr auto timeout = 10s;

    if (bot == nullptr)
        co_return false;

    // Send the request and wait for response
    auto&& resp = co_await bot->request<fb::protocol::game::response::bulletin_message>(
        fb::protocol::game::request::bulletin(BULLETIN_ACTION::WRITE, 1, 0, 0, title, contents),
        timeout);

    // Check if the response indicates success
    auto expected_message = _TEXT(MESSAGE_BULLETIN_WRITE);
    if (resp.text == expected_message)
        co_return true;
    else
        co_return false;
}

void bulletin_test::reset()
{
    // Call base class reset
    bot_integration_test::reset();

    fb::logger::info("Bulletin test reset");
}

bool bulletin_test::is_ready() const
{
    auto bots = this->get_test_bots();
    if (bots.empty())
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

void bulletin_test::on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot)
{
    // Call base class implementation
    bot_integration_test::on_bot_connected(bot);
    fb::logger::debug("Bulletin test: Bot {} added to collection", bot->fd());
}

async::task<void> bulletin_test::on_hook_sequence(fb::bot::game_bot&                      bot,
                                                  const fb::protocol::game::response::id& response)
{
    fb::logger::debug("Bulletin test: Bot {} received object ID {}", bot.fd(), response.oid);

    if (this->is_ready() == false)
        co_return;

    if (this->get_state() == test_state::running)
        co_return;

    fb::logger::info("Bulletin test: All bots ready, notifying controller");
    this->set_state(test_state::ready);
    this->notify_ready();
    co_return;
}

async::task<void> bulletin_test::on_hook_position(fb::bot::game_bot&                            bot,
                                                  const fb::protocol::game::response::position& response)
{
    if (this->is_ready() == false)
        co_return;

    if (this->get_state() == test_state::running)
        co_return;

    fb::logger::info("Bulletin test: All bots ready, notifying controller");
    this->set_state(test_state::ready);
    this->notify_ready();
    co_return;
}

} // namespace fb::bot::integration