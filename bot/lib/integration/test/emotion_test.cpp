#include <fb/bot/integration/emotion_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

emotion_test::emotion_test(game_bot_controller& controller) :
    bot_integration_test(controller, 1) // Use only 1 bot
{ }

async::task<void> emotion_test::on_initialize(game_bot_controller& controller)
{
    co_await this->super::on_initialize(controller);
}

generator<bot_integration_test::scenario_t> emotion_test::on_generate_scenario()
{
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_emotion();
    };

    co_return;
}

std::string emotion_test::name() const
{
    return "Emotion Test";
}

async::task<bool> emotion_test::test_emotion()
{
    fb::logger::debug("Starting emotion packet handler test");

    auto  bots   = this->get_test_bots();
    auto& bot    = bots.front();
    auto  passed = true;

    auto emotions = std::vector<uint8_t>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0xFE, 0xFF};
    for (auto emotion_value : emotions)
    {
        try
        {
            bot->chat(std::format("Testing emotion value: {}", emotion_value));

            // Send emotion request and wait for action response
            auto&& resp = co_await bot->request<fb::protocol::game::response::action>(
                fb::protocol::game::request::emotion(emotion_value),
                [emotion_value, oid = bot->oid()](auto& resp) -> bool {
                    // Verify that the action corresponds to the emotion value
                    // ACTION::EMOTION + emotion_value should be the expected action
                    if (resp.oid != oid)
                        return false;

                    auto expected_action = static_cast<ACTION>(
                        static_cast<uint8_t>(static_cast<uint8_t>(ACTION::EMOTION) + emotion_value));
                    return resp.value == expected_action;
                },
                DEFAULT_TIMEOUT);

            bot->chat(std::format("Emotion {} verified - Action: {}", emotion_value, static_cast<uint8_t>(resp.value)));
        }
        catch (const std::exception& e)
        {
            fb::logger::fatal("Emotion test failed for value {}: {}", emotion_value, e.what());
            passed = false;
        }
    }

    co_return passed;
}