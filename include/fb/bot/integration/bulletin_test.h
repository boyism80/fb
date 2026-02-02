#ifndef __BOT_INTEGRATION_BULLETIN_TEST_H__
#define __BOT_INTEGRATION_BULLETIN_TEST_H__

#include <optional>
#include <fb/bot/integration/test_case.h>
#include <fb/bot/integration/bulletin_bot.h>
#include <string_view>

namespace fb::bot::integration {

class bulletin_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

public:
    bulletin_test(game_bot_controller& controller);

private:
    async::task<bool> get_sections_scenario();
    async::task<bool> bulletin_write_scenario();
    async::task<bool> bulletin_read_scenario();
    async::task<bool> bulletin_delete_scenario();
    async::task<bool> mail_scenario();

protected:
    async::task<void> on_initialize(game_bot_controller& controller) override final;

    generator<scenario_t> on_generate_scenario() override final;

    async::task<void> on_scenario_started(uint32_t scenario_index) override final;

    async::task<void> on_scenario_finished(uint32_t scenario_index) override final;

    async::task<bool> write(std::shared_ptr<game_bot> bot, std::string_view title, std::string_view contents);

    async::task<std::vector<fb::bot::bulletin>> get_sections(std::shared_ptr<game_bot> bot);
    async::task<std::vector<fb::bot::integration::bulletin_bot::article_data>>
    get_articles(std::shared_ptr<game_bot> bot, uint16_t section, uint16_t offset);

    async::task<bool> send_mail(std::shared_ptr<game_bot> bot,
                                std::string_view        to,
                                std::string_view        title,
                                std::string_view        contents);

    async::task<std::vector<fb::bot::integration::bulletin_bot::mail_data>> get_mails(std::shared_ptr<game_bot> bot);

    async::task<bool> read_mail(std::shared_ptr<game_bot> bot,
                                uint16_t                  mail_id,
                                std::string_view        expected_title,
                                std::string_view        expected_contents);

    async::task<bool> delete_mail(std::shared_ptr<game_bot> bot, uint16_t mail_id);
    async::task<bool> read_article(std::shared_ptr<game_bot> bot,
                                   uint16_t                  section,
                                   uint16_t                  article_id,
                                   std::string_view        expected_title,
                                   std::string_view        expected_contents);

    async::task<bool> delete_article(std::shared_ptr<game_bot> bot, uint16_t section, uint16_t article_id);
    std::string       name() const override final;
};

} // namespace fb::bot::integration

#endif // __BOT_INTEGRATION_BULLETIN_TEST_H__