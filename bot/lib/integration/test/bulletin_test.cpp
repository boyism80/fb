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

    // Test bulletin article list functionality to find our written article
    fb::logger::info("Testing bulletin article list functionality to find our article");
    bot->chat("=== TESTING BULLETIN ARTICLE LIST TO FIND OUR ARTICLE ===");

    // Find our written article from section 1 (general section)
    auto found_article_id = co_await this->find_my_article(bot, 1, 0);
    if (!found_article_id.has_value())
    {
        fb::logger::fatal("Failed to find our written article");
        bot->chat("=== FAILED TO FIND OUR WRITTEN ARTICLE ===");
        this->set_state(test_state::failed);
        co_return false;
    }

    fb::logger::info("Found our written article - ID: {}", found_article_id.value());
    bot->chat(std::format("=== FOUND OUR WRITTEN ARTICLE - ID: {} ===", found_article_id.value()));

    // Test reading our article
    fb::logger::info("Testing bulletin article read functionality");
    bot->chat("=== TESTING BULLETIN ARTICLE READ ===");

    if (!co_await this->read_article(bot, 1, found_article_id.value()))
    {
        fb::logger::fatal("Bulletin article read test failed");
        bot->chat("=== BULLETIN ARTICLE READ TEST FAILED ===");
        this->set_state(test_state::failed);
        co_return false;
    }

    fb::logger::info("Bulletin article read test successful");
    bot->chat("=== BULLETIN ARTICLE READ TEST COMPLETED SUCCESSFULLY ===");

    // Test deleting our article
    fb::logger::info("Testing bulletin article delete functionality");
    bot->chat("=== TESTING BULLETIN ARTICLE DELETE ===");

    if (!co_await this->delete_article(bot, 1, found_article_id.value()))
    {
        fb::logger::fatal("Bulletin article delete test failed");
        bot->chat("=== BULLETIN ARTICLE DELETE TEST FAILED ===");
        this->set_state(test_state::failed);
        co_return false;
    }

    fb::logger::info("Bulletin article delete test successful");
    bot->chat("=== BULLETIN ARTICLE DELETE TEST COMPLETED SUCCESSFULLY ===");

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

async::task<std::optional<uint16_t>> bulletin_test::find_my_article(std::shared_ptr<fb::bot::game_bot> bot,
                                                                    uint16_t                           section,
                                                                    uint16_t                           offset)
{
    constexpr auto timeout = 10s;

    if (bot == nullptr)
        co_return std::nullopt;

    // Send the request and wait for response
    auto&& resp = co_await bot->request<fb::protocol::game::response::bulletin_articles>(
        fb::protocol::game::request::bulletin(BULLETIN_ACTION::ARTICLES, section, offset),
        timeout);

    // Check if the response contains valid data
    if (resp.bulletin_id == section && !resp.bulletin_name.empty())
    {
        fb::logger::info("Bulletin articles retrieved successfully - Section: {}, Name: {}, Articles: {}",
                         resp.bulletin_id,
                         resp.bulletin_name,
                         resp.articles.size());

        // Find articles written by this bot
        auto bot_name = bot->name();
        for (const auto& article : resp.articles)
        {
            fb::logger::debug("Article - ID: {}, Title: {}, Author: {}, Date: {}/{}",
                              article.id,
                              article.title,
                              article.uname,
                              article.month,
                              article.day);

            // Check if this article was written by our bot
            if (article.uname == bot_name)
            {
                fb::logger::info("Found article written by bot - ID: {}, Title: {}", article.id, article.title);
                co_return article.id;
            }
        }

        fb::logger::warn("No articles found written by bot: {}", bot_name);
        co_return std::nullopt;
    }
    else
    {
        fb::logger::fatal("Bulletin articles response invalid - Section: {}, Name: {}",
                          resp.bulletin_id,
                          resp.bulletin_name);
        co_return std::nullopt;
    }
}

async::task<bool> bulletin_test::read_article(std::shared_ptr<fb::bot::game_bot> bot,
                                              uint16_t                           section,
                                              uint16_t                           article_id)
{
    constexpr auto timeout = 10s;

    if (bot == nullptr || article_id == 0)
        co_return false;

    // Send the request and wait for response
    auto&& resp = co_await bot->request<fb::protocol::game::response::bulletin_article>(
        fb::protocol::game::request::bulletin(BULLETIN_ACTION::ARTICLE, section, article_id),
        timeout);

    // Check if the response contains valid article data
    if (resp.id == article_id && !resp.title.empty())
    {
        fb::logger::info("Bulletin article read successfully - ID: {}, Title: {}, Author: {}",
                         resp.id,
                         resp.title,
                         resp.uname);
        co_return true;
    }
    else
    {
        fb::logger::fatal("Bulletin article read failed - Expected ID: {}, Got ID: {}", article_id, resp.id);
        co_return false;
    }
}

async::task<bool> bulletin_test::delete_article(std::shared_ptr<fb::bot::game_bot> bot,
                                                uint16_t                           section,
                                                uint16_t                           article_id)
{
    constexpr auto timeout = 10s;

    if (bot == nullptr || article_id == 0)
        co_return false;

    // Send the request and wait for response
    auto&& resp = co_await bot->request<fb::protocol::game::response::bulletin_message>(
        fb::protocol::game::request::bulletin(BULLETIN_ACTION::DELETE, section, article_id),
        timeout);

    // Check if the response indicates success
    auto expected_message = _TEXT(MESSAGE_BULLETIN_SUCCESS_DELETE);
    if (resp.text == expected_message)
    {
        fb::logger::info("Bulletin article deleted successfully - ID: {}", article_id);
        co_return true;
    }
    else
    {
        fb::logger::fatal("Bulletin article delete failed - ID: {}, Response: {}", article_id, resp.text);
        co_return false;
    }
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