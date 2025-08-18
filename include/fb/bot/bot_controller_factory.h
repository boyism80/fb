#ifndef FB_BOT_BOT_CONTROLLER_FACTORY_H
#define FB_BOT_BOT_CONTROLLER_FACTORY_H

#include <memory>
#include <fb/bot/container.h>
#include <fb/bot/gateway_controller.h>
#include <fb/bot/login_controller.h>
#include <fb/bot/game_controller.h>
#include <fb/bot/load/gateway_controller.h>
#include <fb/bot/load/login_controller.h>
#include <fb/bot/load/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>
#include <fb/bot/integration/login_controller.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/config.h>
#include <fb/bot/test_mode.h>

template <fb::bot::test_mode Mode>
class bot_controller_factory;

template <>
class bot_controller_factory<fb::bot::test_mode::LOAD_TEST>
{
public:
    static std::shared_ptr<fb::bot::gateway_bot_controller> create_gateway_controller(fb::bot::bot_container& container)
    {
        return std::make_shared<fb::bot::load::gateway_bot_controller>(container);
    }

    static std::shared_ptr<fb::bot::login_bot_controller> create_login_controller(fb::bot::bot_container& container)
    {
        return std::make_shared<fb::bot::load::login_bot_controller>(container);
    }

    static std::shared_ptr<fb::bot::game_bot_controller> create_game_controller(fb::bot::bot_container& container)
    {
        return std::make_shared<fb::bot::load::game_bot_controller>(container);
    }

    static uint32_t get_thread_count()
    {
        return fb::config<uint32_t>("thread:logic");
    }

    static uint32_t get_io_size()
    {
        return fb::config<uint32_t>("io_size");
    }

    static bool should_create_display_thread()
    {
        return true;
    }
};

template <>
class bot_controller_factory<fb::bot::test_mode::INTEGRATION_TEST>
{
public:
    static std::shared_ptr<fb::bot::gateway_bot_controller> create_gateway_controller(fb::bot::bot_container& container)
    {
        return std::make_shared<fb::bot::integration::gateway_bot_controller>(container);
    }

    static std::shared_ptr<fb::bot::login_bot_controller> create_login_controller(fb::bot::bot_container& container)
    {
        return std::make_shared<fb::bot::integration::login_bot_controller>(container);
    }

    static std::shared_ptr<fb::bot::game_bot_controller> create_game_controller(fb::bot::bot_container& container)
    {
        return std::make_shared<fb::bot::integration::game_bot_controller>(container);
    }

    static uint32_t get_thread_count()
    {
        return 1;
    }

    static uint32_t get_io_size()
    {
        return 1;
    }

    static bool should_create_display_thread()
    {
        return false;
    }
};

#endif // FB_BOT_BOT_CONTROLLER_FACTORY_H
