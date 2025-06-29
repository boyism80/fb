#include <fb/bot/integration/test_case.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/logger.h>

namespace fb::bot::integration {

// Base test class implementation
void bot_integration_test::cleanup()
{
    fb::logger::info("Test cleanup completed (base implementation)");
}

} // namespace fb::bot::integration