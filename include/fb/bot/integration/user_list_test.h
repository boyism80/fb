#ifndef __USER_LIST_TEST_H__
#define __USER_LIST_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/game/protocol.h>

namespace fb::bot::integration {

/**
 * @brief      Integration test for user list functionality.
 *
 *             This test class focuses on testing user list protocol
 *             including user data serialization/deserialization,
 *             user count handling, and user information display.
 */
class user_list_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_initialize(game_bot_controller& controller) override final;

public:
    /**
     * @brief      Constructs a new user list test instance.
     *
     * @param      controller  Reference to the game bot controller.
     */
    user_list_test(game_bot_controller& controller);

    /**
     * @brief      Destroys the user list test instance.
     */
    ~user_list_test() = default;

    /**
     * @brief      Gets the test name.
     *
     * @return     "User List Test" as the identifier.
     */
    std::string name() const override final;

private:
    /**
     * @brief      Tests user list scenario 1.
     *
     * @return     An async task that completes with true if test passed, false otherwise.
     */
    async::task<bool> test_scenario_1();
};

} // namespace fb::bot::integration

#endif // __USER_LIST_TEST_H__