#ifndef __BOT_INTEGRATION_BULLETIN_TEST_H__
#define __BOT_INTEGRATION_BULLETIN_TEST_H__

#include <optional>
#include <fb/bot/integration/test_case.h>

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
     * @brief      Initializes the bulletin test and spawns required bots.
     *
     * @param[in]  controller  The game bot controller to use for spawning bots.
     *
     * @return     A task that completes when initialization is finished.
     */
    async::task<void> initialize(game_bot_controller& controller);

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
     * @brief      Finds an article written by the current bot.
     *
     * @param[in]  bot      The bot to use for finding the article.
     * @param[in]  section  The bulletin section ID to search in.
     * @param[in]  offset   The starting offset for pagination.
     * @return     A task that completes when article search finishes, returning the article ID if found,
     * or std::nullopt if no article written by the bot is found.
     */
    async::task<std::optional<uint16_t>> find_my_article(std::shared_ptr<fb::bot::game_bot> bot,
                                                         uint16_t                           section,
                                                         uint16_t                           offset);

    /**
     * @brief      Tests bulletin article read functionality.
     *
     * @param[in]  bot         The bot to use for the bulletin article read test.
     * @param[in]  section     The bulletin section ID containing the article.
     * @param[in]  article_id  The unique identifier of the article to read.
     * @return     A task that completes when bulletin article read test finishes, returning true if the response
     * contains valid article data.
     */
    async::task<bool> read_article(std::shared_ptr<fb::bot::game_bot> bot, uint16_t section, uint16_t article_id);

    /**
     * @brief      Tests bulletin article delete functionality.
     *
     * @param[in]  bot         The bot to use for the bulletin article delete test.
     * @param[in]  section     The bulletin section ID containing the article.
     * @param[in]  article_id  The unique identifier of the article to delete.
     * @return     A task that completes when bulletin article delete test finishes, returning true if the response
     * indicates successful deletion.
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
    std::string name() const override final
    {
        return "Bulletin Test";
    }

    /**
     * @brief      Checks if all spawned bots are ready for bulletin test.
     *
     * @return     True if all spawned bots are ready for bulletin test, false otherwise.
     */
    bool is_ready() const override final;

    /**
     * @brief      Called when a bot connects to the bulletin test.
     *
     * @param[in]  bot  The connected bot to store for testing.
     */
    void on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot) override final;

    /**
     * @brief      Called when a bot receives an object ID response.
     *
     * @param[in]  bot       The bot that received the object ID.
     * @param[in]  response  The object ID response containing the new ID.
     *
     * @return     An async task that completes when hook processing is finished.
     */
    async::task<void> on_hook_sequence(fb::bot::game_bot& bot, const fb::protocol::game::response::id& response);

    /**
     * @brief      Called when a bot receives a position response.
     *
     * @param[in]  bot       The bot that received the position.
     * @param[in]  response  The position response containing the new position.
     *
     * @return     An async task that completes when hook processing is finished.
     */
    async::task<void> on_hook_position(fb::bot::game_bot& bot, const fb::protocol::game::response::position& response);
};

} // namespace fb::bot::integration

#endif // __BOT_INTEGRATION_BULLETIN_TEST_H__