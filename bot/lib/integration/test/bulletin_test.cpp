#include <fb/bot/integration/bulletin_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>

using namespace std::chrono_literals;

namespace fb::bot::integration {

bulletin_test::bulletin_test(game_bot_controller& controller) :
    bot_integration_test(controller, 2)
{
    fb::logger::debug("Bulletin test constructed");
}

async::task<bool> bulletin_test::get_sections_scenario()
{
    auto bots = this->get_test_bots();
    if (bots.size() < 2)
        throw std::runtime_error("Not enough bots for this test, requires 2.");

    auto& bot1 = bots[0];
    auto& bot2 = bots[1];

    // Test getting bulletin sections
    fb::logger::info("Testing getting bulletin sections");
    bot1->chat("=== TESTING GETTING BULLETIN SECTIONS ===");
    auto sections = co_await this->get_sections(bot1);
    if (sections.empty())
    {
        fb::logger::fatal("Failed to get bulletin sections");
        bot1->chat("=== FAILED TO GET BULLETIN SECTIONS ===");
        this->set_state(test_state::failed);
        co_return false;
    }
    fb::logger::info("Successfully retrieved {} bulletin sections", sections.size());
    bot1->chat(std::format("=== SUCCESSFULLY RETRIEVED {} SECTIONS ===", sections.size()));

    // Test bulletin write functionality
    fb::logger::info("Testing bulletin write functionality");
    const std::string bot1_article_title    = "Bot1 Article Title";
    const std::string bot1_article_contents = "Bot1's test article contents.";
    const std::string bot2_article_title    = "Bot2 Article Title";
    const std::string bot2_article_contents = "Bot2's test article contents.";

    // First write attempt - should fail due to low level
    fb::logger::info("Testing bulletin write with low level (should fail)");
    if (co_await this->write(bot1, bot1_article_title, bot1_article_contents))
    {
        fb::logger::fatal("Bulletin write test should have failed with low level");
        bot1->chat("=== BULLETIN WRITE TEST FAILED - Should have failed with low level ===");
        this->set_state(test_state::failed);
        co_return false;
    }

    fb::logger::info("Bulletin write failed as expected with low level");
    bot1->chat("=== BULLETIN WRITE FAILED AS EXPECTED (LOW LEVEL) ===");

    // Change level to 10 for both bots
    fb::logger::info("Changing level to 10 for both bots");
    co_await bot1->change_level(10, DEFAULT_TIMEOUT);
    co_await bot2->change_level(10, DEFAULT_TIMEOUT);

    fb::logger::info("Level changed to 10, testing bulletin write again");
    bot1->chat("=== LEVEL CHANGED TO 10, TESTING BULLETIN WRITE AGAIN ===");

    // Bot1 writes an article
    if (!co_await this->write(bot1, bot1_article_title, bot1_article_contents))
    {
        fb::logger::fatal("Bulletin write test failed for bot1 with level 10");
        bot1->chat("=== BULLETIN WRITE TEST FAILED FOR BOT1 WITH LEVEL 10 ===");
        this->set_state(test_state::failed);
        co_return false;
    }
    fb::logger::info("Bot1 write test successful with level 10");
    bot1->chat("=== BOT1 WRITE TEST COMPLETED SUCCESSFULLY WITH LEVEL 10 ===");

    // Bot2 writes an article
    if (!co_await this->write(bot2, bot2_article_title, bot2_article_contents))
    {
        fb::logger::fatal("Bulletin write test failed for bot2 with level 10");
        bot2->chat("=== BULLETIN WRITE TEST FAILED FOR BOT2 WITH LEVEL 10 ===");
        this->set_state(test_state::failed);
        co_return false;
    }
    fb::logger::info("Bot2 write test successful with level 10");
    bot2->chat("=== BOT2 WRITE TEST COMPLETED SUCCESSFULLY WITH LEVEL 10 ===");

    // Test bulletin article list functionality to find our written article
    fb::logger::info("Testing bulletin article list functionality to find our article");
    bot1->chat("=== TESTING BULLETIN ARTICLE LIST TO FIND OUR ARTICLE ===");

    // Find our written article from section 1 (general section)
    auto  articles      = co_await this->get_articles(bot1, 1, 0x7FFF);
    auto& bot1_name     = bot1->name();
    auto  my_article_it = std::find_if(articles.begin(), articles.end(), [&](const auto& article) {
        return article.uname == bot1_name;
    });

    if (my_article_it == articles.end())
    {
        fb::logger::fatal("Failed to find our written article");
        bot1->chat("=== FAILED TO FIND OUR WRITTEN ARTICLE ===");
        this->set_state(test_state::failed);
        co_return false;
    }

    auto found_article_id = my_article_it->id;
    fb::logger::info("Found our written article - ID: {}", found_article_id);
    bot1->chat(std::format("=== FOUND OUR WRITTEN ARTICLE - ID: {} ===", found_article_id));

    // Test reading our article
    fb::logger::info("Testing bulletin article read functionality");
    bot1->chat("=== TESTING BULLETIN ARTICLE READ ===");

    if (!co_await this->read_article(bot1, 1, found_article_id, bot1_article_title, bot1_article_contents))
    {
        fb::logger::fatal("Failed to read our own article with correct content");
        bot1->chat("=== FAILED TO READ OUR OWN ARTICLE ===");
        this->set_state(test_state::failed);
        co_return false;
    }
    bot1->chat("=== BULLETIN ARTICLE READ TEST COMPLETED SUCCESSFULLY ===");

    // Test deleting our article
    fb::logger::info("Testing bulletin article delete functionality");
    bot1->chat("=== TESTING BULLETIN ARTICLE DELETE ===");

    if (!co_await this->delete_article(bot1, 1, found_article_id))
    {
        fb::logger::fatal("Failed to delete our own article");
        bot1->chat("=== FAILED TO DELETE OUR OWN ARTICLE ===");
        this->set_state(test_state::failed);
        co_return false;
    }
    fb::logger::info("Bulletin article delete test successful");
    bot1->chat("=== BULLETIN ARTICLE DELETE TEST COMPLETED SUCCESSFULLY ===");

    // Test deleting a non-existent article
    fb::logger::info("Testing deleting a non-existent article (should fail)");
    bot1->chat("=== TESTING DELETING NON-EXISTENT ARTICLE (SHOULD FAIL) ===");
    if (co_await this->delete_article(bot1, 1, found_article_id))
    {
        fb::logger::fatal("Deleting a non-existent article succeeded, but it should have failed.");
        bot1->chat("=== FAILED: DELETING NON-EXISTENT ARTICLE SUCCEEDED (UNEXPECTED) ===");
        this->set_state(test_state::failed);
        co_return false;
    }
    fb::logger::info("Successfully failed to delete non-existent article, as expected.");
    bot1->chat("=== SUCCESS: FAILED TO DELETE NON-EXISTENT ARTICLE AS EXPECTED ===");

    // Test reading and deleting another user's article
    fb::logger::info("Testing with another user's article");
    bot1->chat("=== TESTING WITH ANOTHER USER'S ARTICLE ===");

    // Find an article written by someone else
    articles               = co_await this->get_articles(bot1, 1, 0x7FFF);
    auto& bot2_name        = bot2->name();
    auto  other_article_it = std::find_if(articles.begin(), articles.end(), [&](const auto& article) {
        return article.uname == bot2_name;
    });

    if (other_article_it == articles.end())
    {
        fb::logger::warn("Could not find an article from another user to test. Skipping test.");
        bot1->chat("=== SKIPPING: NO OTHER USER ARTICLE FOUND ===");
    }
    else
    {
        uint16_t article_id_to_test = other_article_it->id;
        fb::logger::info("Found another user's article - ID: {}", article_id_to_test);
        bot1->chat(std::format("=== FOUND OTHER'S ARTICLE - ID: {} ===", article_id_to_test));

        // Read the article - should succeed
        fb::logger::info("Testing to read another user's article");
        bot1->chat("=== TESTING READ OTHER'S ARTICLE ===");
        if (!co_await this->read_article(bot1, 1, article_id_to_test, bot2_article_title, bot2_article_contents))
        {
            fb::logger::fatal("Failed to read other's article with correct content");
            bot1->chat("=== FAILED TO READ OTHER'S ARTICLE ===");
            this->set_state(test_state::failed);
            co_return false;
        }
        bot1->chat("=== READ OTHER'S ARTICLE SUCCESSFUL ===");

        // Try to delete it - should fail
        fb::logger::info("Testing to delete another user's article (should fail)");
        bot1->chat("=== TESTING DELETE OTHER'S ARTICLE (SHOULD FAIL) ===");
        if (co_await this->delete_article(bot1, 1, article_id_to_test))
        {
            fb::logger::fatal("Deleting another user's article succeeded, but it should have failed.");
            bot1->chat("=== FAILED: DELETING OTHER'S ARTICLE SUCCEEDED (UNEXPECTED) ===");
            this->set_state(test_state::failed);
            co_return false;
        }

        fb::logger::info("Successfully failed to delete another user's article, as expected.");
        bot1->chat("=== SUCCESS: FAILED TO DELETE OTHER'S ARTICLE AS EXPECTED ===");
    }

    // Mail test
    fb::logger::info("Testing mail functionality");
    const std::string mail_title    = "Mail Test Title";
    const std::string mail_contents = "This is a mail test.";

    fb::logger::info("Sending mail from {} to {}", bot1->name(), bot2->name());
    if (!co_await this->send_mail(bot1, bot2->name(), mail_title, mail_contents))
    {
        fb::logger::fatal("Failed to send mail");
        this->set_state(test_state::failed);
        co_return false;
    }

    // Let's wait a bit for mail to be processed
    co_await bot1->thread()->sleep(1s);

    fb::logger::info("Checking {}'s mailbox", bot2->name());
    auto&& mails   = co_await this->get_mails(bot2);
    auto   mail_it = std::find_if(mails.begin(), mails.end(), [&](const auto& mail) {
        return mail.sender_name == bot1_name;
    });

    if (mail_it == mails.end())
    {
        fb::logger::fatal("Mail not found in receiver's mailbox");
        this->set_state(test_state::failed);
        co_return false;
    }

    auto mail_id = mail_it->id;
    fb::logger::info("Mail found with ID: {}", mail_id);

    fb::logger::info("Reading mail {}", mail_id);
    if (!co_await this->read_mail(bot2, mail_id, mail_title, mail_contents))
    {
        fb::logger::fatal("Failed to read mail with correct content");
        this->set_state(test_state::failed);
        co_return false;
    }

    fb::logger::info("Deleting mail {}", mail_id);
    if (!co_await this->delete_mail(bot2, mail_id))
    {
        fb::logger::fatal("Failed to delete mail");
        this->set_state(test_state::failed);
        co_return false;
    }
    fb::logger::info("Mail deleted successfully");

    // Test deleting a non-existent mail
    fb::logger::info("Testing deleting a non-existent mail (should fail)");
    bot2->chat("=== TESTING DELETING NON-EXISTENT MAIL (SHOULD FAIL) ===");
    if (co_await this->delete_mail(bot2, mail_id))
    {
        fb::logger::fatal("Deleting a non-existent mail succeeded, but it should have failed.");
        bot2->chat("=== FAILED: DELETING NON-EXISTENT MAIL SUCCEEDED (UNEXPECTED) ===");
        this->set_state(test_state::failed);
        co_return false;
    }
    fb::logger::info("Successfully failed to delete non-existent mail, as expected.");
    bot2->chat("=== SUCCESS: FAILED TO DELETE NON-EXISTENT MAIL AS EXPECTED ===");

    fb::logger::info("Bulletin test completed successfully");

    this->set_state(test_state::completed);

    co_return true;
}

async::task<bool> bulletin_test::write(std::shared_ptr<fb::bot::game_bot> bot,
                                       const std::string&                 title,
                                       const std::string&                 contents)
{
    if (bot == nullptr)
        co_return false;

    // Send the request and wait for response
    auto&& resp = co_await bot->request<fb::bot::integration::bulletin_bot>(
        fb::protocol::game::request::bulletin(BULLETIN_ACTION::WRITE, 1, 0, 0, title, contents),
        DEFAULT_TIMEOUT);

    // Check if the response indicates success
    auto expected_message = _TEXT(MESSAGE_BULLETIN_WRITE);
    if (resp.type == fb::bot::integration::bulletin_bot::bulletin_type::message &&
        resp.message_text == expected_message)
        co_return true;
    else
        co_return false;
}

async::task<std::vector<fb::bot::bulletin>> bulletin_test::get_sections(std::shared_ptr<fb::bot::game_bot> bot)
{
    if (bot == nullptr)
        co_return {};

    // Send the request and wait for response
    auto&& resp = co_await bot->request<fb::bot::integration::bulletin_bot>(
        fb::protocol::game::request::bulletin(BULLETIN_ACTION::SECTIONS),
        [](auto& resp) -> bool {
            if (resp.type != fb::bot::integration::bulletin_bot::bulletin_type::sections)
                return false;

            return true;
        },
        DEFAULT_TIMEOUT);

    fb::logger::info("Bulletin sections retrieved successfully - Sections: {}", resp.bulletins.size());

    co_return resp.bulletins;
}

async::task<std::vector<fb::bot::integration::bulletin_bot::article_data>>
bulletin_test::get_articles(std::shared_ptr<fb::bot::game_bot> bot, uint16_t section, uint16_t offset)
{
    if (bot == nullptr)
        co_return {};

    // Send the request and wait for response
    auto&& resp = co_await bot->request<fb::bot::integration::bulletin_bot>(
        fb::protocol::game::request::bulletin(BULLETIN_ACTION::ARTICLES, section, 0, offset),
        [section](auto& resp) -> bool {
            if (resp.type != fb::bot::integration::bulletin_bot::bulletin_type::articles)
                return false;

            if (resp.articles_bulletin_id != section)
                return false;

            return true;
        },
        DEFAULT_TIMEOUT);

    fb::logger::info("Bulletin articles retrieved successfully - Section: {}, Name: {}, Articles: {}",
                     resp.articles_bulletin_id,
                     resp.articles_bulletin_name,
                     resp.articles.size());

    co_return resp.articles;
}

async::task<bool> bulletin_test::read_article(std::shared_ptr<fb::bot::game_bot> bot,
                                              uint16_t                           section,
                                              uint16_t                           article_id,
                                              const std::string&                 expected_title,
                                              const std::string&                 expected_contents)
{
    if (bot == nullptr || article_id == 0)
        co_return false;

    // Send the request and wait for response
    auto&& resp = co_await bot->request<fb::bot::integration::bulletin_bot>(
        fb::protocol::game::request::bulletin(BULLETIN_ACTION::ARTICLE, section, article_id),
        [article_id](auto& resp) -> bool {
            if (resp.type != fb::bot::integration::bulletin_bot::bulletin_type::article)
                return false;

            if (resp.article_id != article_id)
                return false;

            return true;
        },
        DEFAULT_TIMEOUT);

    fb::logger::info("Bulletin article read successfully - ID: {}, Title: {}, Author: {}",
                     resp.article_id,
                     resp.article_title,
                     resp.article_uname);

    if (resp.article_title != expected_title)
    {
        fb::logger::fatal("Article title mismatch: expected '{}', got '{}'", expected_title, resp.article_title);
        co_return false;
    }

    if (resp.article_contents != expected_contents)
    {
        fb::logger::fatal("Article contents mismatch: expected '{}', got '{}'",
                          expected_contents,
                          resp.article_contents);
        co_return false;
    }

    co_return true;
}

async::task<bool> bulletin_test::delete_article(std::shared_ptr<fb::bot::game_bot> bot,
                                                uint16_t                           section,
                                                uint16_t                           article_id)
{
    if (bot == nullptr || article_id == 0)
        co_return false;

    // Send the request and wait for response
    auto&& resp = co_await bot->request<fb::bot::integration::bulletin_bot>(
        fb::protocol::game::request::bulletin(BULLETIN_ACTION::DELETE, section, article_id),
        [](auto& resp) -> bool {
            return resp.type == fb::bot::integration::bulletin_bot::bulletin_type::message;
        },
        DEFAULT_TIMEOUT);

    if (resp.message_text == _TEXT(MESSAGE_BULLETIN_SUCCESS_DELETE))
    {
        fb::logger::info("Bulletin article deleted successfully - ID: {}", article_id);
        co_return true;
    }

    fb::logger::warn("Failed to delete article {} - Message: {}", article_id, resp.message_text);
    co_return false;
}

async::task<bool> bulletin_test::send_mail(std::shared_ptr<fb::bot::game_bot> bot,
                                           const std::string&                 to,
                                           const std::string&                 title,
                                           const std::string&                 contents)
{
    if (bot == nullptr)
        co_return false;

    // Send the request and wait for response
    auto&& resp = co_await bot->request<fb::bot::integration::bulletin_bot>(
        fb::protocol::game::request::bulletin(BULLETIN_ACTION::SEND_MAIL, 0, 0, 0, title, contents, to),
        [](auto& resp) -> bool {
            return resp.type == fb::bot::integration::bulletin_bot::bulletin_type::message_mail;
        },
        DEFAULT_TIMEOUT);

    if (resp.message_success)
    {
        fb::logger::info("Mail sent successfully to {}", to);
        co_return true;
    }

    fb::logger::warn("Failed to send mail to {} - Message: {}", to, resp.message_text);
    co_return false;
}

async::task<std::vector<fb::bot::integration::bulletin_bot::mail_data>>
bulletin_test::get_mails(std::shared_ptr<fb::bot::game_bot> bot)
{
    if (bot == nullptr)
        co_return {};

    // Send the request and wait for response
    auto&& resp = co_await bot->request<fb::bot::integration::bulletin_bot>(
        fb::protocol::game::request::bulletin(BULLETIN_ACTION::ARTICLES, 0xFFFF, 0, 0x7FFF), // section 0xFFFF for mail
        [](auto& resp) -> bool {
            return resp.type == fb::bot::integration::bulletin_bot::bulletin_type::mails;
        },
        DEFAULT_TIMEOUT);

    fb::logger::info("Mails retrieved successfully - Mails: {}", resp.mails.size());

    co_return resp.mails;
}

async::task<bool> bulletin_test::read_mail(std::shared_ptr<fb::bot::game_bot> bot,
                                           uint16_t                           mail_id,
                                           const std::string&                 expected_title,
                                           const std::string&                 expected_contents)
{
    if (bot == nullptr)
        co_return false;

    // Send the request and wait for response
    auto&& resp = co_await bot->request<fb::bot::integration::bulletin_bot>(
        fb::protocol::game::request::bulletin(BULLETIN_ACTION::ARTICLE, 0xFFFF, mail_id, 0),
        [mail_id](auto& resp) -> bool {
            if (resp.type != fb::bot::integration::bulletin_bot::bulletin_type::mail)
                return false;

            if (resp.mail_id != mail_id)
                return false;

            return true;
        },
        DEFAULT_TIMEOUT);

    if (resp.mail_title != expected_title)
    {
        fb::logger::fatal("Mail title mismatch: expected '{}', got '{}'", expected_title, resp.mail_title);
        co_return false;
    }

    if (resp.mail_contents != expected_contents)
    {
        fb::logger::fatal("Mail contents mismatch: expected '{}', got '{}'", expected_contents, resp.mail_contents);
        co_return false;
    }

    co_return true;
}

async::task<bool> bulletin_test::delete_mail(std::shared_ptr<fb::bot::game_bot> bot, uint16_t mail_id)
{
    if (bot == nullptr)
        co_return false;

    // Send the request and wait for response
    auto&& resp = co_await bot->request<fb::bot::integration::bulletin_bot>(
        fb::protocol::game::request::bulletin(BULLETIN_ACTION::DELETE, 0xFFFF, mail_id, 0),
        [](auto& resp) -> bool {
            return resp.type == fb::bot::integration::bulletin_bot::bulletin_type::message_mail;
        },
        DEFAULT_TIMEOUT);

    if (resp.message_text == _TEXT(MESSAGE_BULLETIN_SUCCESS_DELETE))
    {
        fb::logger::info("Mail deleted successfully - ID: {}", mail_id);
        co_return true;
    }

    fb::logger::warn("Failed to delete mail {} - Message: {}", mail_id, resp.message_text);
    co_return false;
}

generator<bot_integration_test::scenario_t> bulletin_test::on_generate_scenario()
{
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->get_sections_scenario();
    };
}

std::string bulletin_test::name() const
{
    return "Bulletin Test";
}

async::task<void> bulletin_test::on_initialize(game_bot_controller& controller)
{
    co_await super::on_initialize(controller);

    auto  bots = this->get_test_bots();
    auto& bot1 = bots[0];
    auto& bot2 = bots[1];
    co_await bot2->move(DIRECTION::RIGHT, 1, DEFAULT_INTERVAL);
    co_await bot2->thread()->switching();
    co_await bot2->thread()->sleep(DEFAULT_INTERVAL);
    co_await bot2->direction(DIRECTION::BOTTOM, DEFAULT_TIMEOUT);

    co_return;
}

async::task<void> bulletin_test::on_scenario_started(uint32_t scenario_index)
{
    co_return;
}

async::task<void> bulletin_test::on_scenario_finished(uint32_t scenario_index)
{
    co_return;
}

} // namespace fb::bot::integration