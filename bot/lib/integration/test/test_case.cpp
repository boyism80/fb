#include <fb/bot/integration/test_case.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/logger.h>
#include <fb/game/protocol.h>
#include <format>

namespace fb::bot::integration {

void bot_integration_test::notify_ready()
{
    this->_controller.notify_test_ready();
}

} // namespace fb::bot::integration