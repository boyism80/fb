#ifndef __BOT_INTEGRATION_BULLETIN_TEST_H__
#define __BOT_INTEGRATION_BULLETIN_TEST_H__

#include <optional>
#include <fb/bot/integration/test_case.h>
#include <fb/bot/integration/bulletin_bot.h>

namespace fb::bot::integration {

/**
 * @brief      Bulletin integration test implementation.
 *
 *             Tests bulletin board functionality.
 */
class bulletin_test : public bot_integration_test
{
public:
    /**
     * @brief      Constructs a new bulletin test with controller reference.
     *
     * @param[in]  controller  Reference to the parent game bot controller.
     */
    bulletin_test(game_bot_controller& controller);

    /**
     * @brief      Executes the bulletin test.
     *
     * @return     A task that completes when bulletin test finishes, returning true on success.
     */
    async::task<bool> execute();

    /**
     * @brief      Tests bulletin write functionality.
     *
     * @param[in]  bot       The bot to use for the bulletin write test.
     * @param[in]  title     The title of the bulletin post.
     * @param[in]  contents  The contents of the bulletin post.
     * @return     A task that completes when bulletin write test finishes, returning true if the response message
     * matches expected success message.
     */
    async::task<bool> write(std::shared_ptr<fb::bot::game_bot> bot,
                            const std::string&                 title,
                            const std::string&                 contents);

    /**
     * @brief      Gets a list of bulletin sections.
     *
     * @param[in]  bot      The bot to use for getting sections.
     * @return     A task that completes with a vector of bulletin sections.
     */
    async::task<std::vector<fb::bot::bulletin>> get_sections(std::shared_ptr<fb::bot::game_bot> bot);

    /**
     * @brief      Gets a list of articles from a bulletin section.
     *
     * @param[in]  bot      The bot to use for finding articles.
     * @param[in]  section  The bulletin section ID to search in.
     * @param[in]  offset   The starting offset for pagination.
     * @return     A task that completes with a vector of article data.
     */
    async::task<std::vector<fb::bot::integration::bulletin_bot::article_data>>
    get_articles(std::shared_ptr<fb::bot::game_bot> bot, uint16_t section, uint16_t offset);

    /**
     * @brief      Sends a mail to another user.
     *
     * @param[in]  bot       The bot sending the mail.
     * @param[in]  to        The name of the recipient.
     * @param[in]  title     The title of the mail.
     * @param[in]  contents  The contents of the mail.
     * @return     A task that completes with true if the mail was sent successfully.
     */
    async::task<bool> send_mail(std::shared_ptr<fb::bot::game_bot> bot,
                                const std::string&                 to,
                                const std::string&                 title,
                                const std::string&                 contents);

    /**
     * @brief      Gets a list of mails from the mail box.
     *
     * @param[in]  bot      The bot to use for getting mails.
     * @return     A task that completes with a vector of mail data.
     */
    async::task<std::vector<fb::bot::integration::bulletin_bot::mail_data>>
    get_mails(std::shared_ptr<fb::bot::game_bot> bot);

    /**
     * @brief      Tests mail read functionality.
     *
     * @param[in]  bot       The bot to use for the mail read test.
     * @param[in]  mail_id   The unique identifier of the mail to read.
     * @param[in]  expected_title The expected title of the mail.
     * @param[in]  expected_contents The expected contents of the mail.
     * @return     A task that completes with true if the mail contents match.
     */
    async::task<bool> read_mail(std::shared_ptr<fb::bot::game_bot> bot,
                                uint16_t                           mail_id,
                                const std::string&                 expected_title,
                                const std::string&                 expected_contents);

    /**
     * @brief      Tests mail delete functionality.
     *
     * @param[in]  bot       The bot to use for the mail delete test.
     * @param[in]  mail_id   The unique identifier of the mail to delete.
     * @return     A task that completes with true if the mail was deleted successfully.
     */
    async::task<bool> delete_mail(std::shared_ptr<fb::bot::game_bot> bot, uint16_t mail_id);

    /**
     * @brief      Tests bulletin article read functionality.
     *
     * @param[in]  bot         The bot to use for the bulletin article read test.
     * @param[in]  section     The bulletin section ID containing the article.
     * @param[in]  article_id  The unique identifier of the article to read.
     * @param[in]  expected_title The expected title of the article.
     * @param[in]  expected_contents The expected contents of the article.
     * @return     A task that completes with true if the article contents match.
     */
    async::task<bool> read_article(std::shared_ptr<fb::bot::game_bot> bot,
                                   uint16_t                           section,
                                   uint16_t                           article_id,
                                   const std::string&                 expected_title,
                                   const std::string&                 expected_contents);

    /**
     * @brief      Tests bulletin article delete functionality.
     *
     * @param[in]  bot         The bot to use for the bulletin article delete test.
     * @param[in]  section     The bulletin section ID containing the article.
     * @param[in]  article_id  The unique identifier of the article to delete.
     * @return     A task that completes when bulletin article delete test finishes, returning true on success.
     */
    async::task<bool> delete_article(std::shared_ptr<fb::bot::game_bot> bot, uint16_t section, uint16_t article_id);

    /**
     * @brief      Resets the bulletin test state to initial conditions.
     */
    void reset() override final;

    /**
     * @brief      Gets the test name.
     *
     * @return     "Bulletin Test" as the identifier.
     */
    std::string name() const override final;
};

} // namespace fb::bot::integration

#endif // __BOT_INTEGRATION_BULLETIN_TEST_H__