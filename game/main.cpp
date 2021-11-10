#include "resource.h"
#include "module/db/db.h"
#include "module/socket/socket.h"
#include "module/config/config.h"
#include "module/leak.h"
#include "model/context/context.game.h"
#include "module/string/string.h"
#include "protocol/internal.h"
#include "model/lua/lua.h"
#include "module/console/console.h"
#include "model/table/table.game.h"

bool load_db(fb::console& c, fb::game::context& context)
{
    auto& config = fb::config::get();

    auto height = 8;
    c.box(0, 0, c.width()-1, height);

    auto header = "The Kingdom of the wind [GAME]";
    c.cursor((c.width()-1 - strlen(header)) / 2, 2).put(header);

    auto github = "https://github.com/boyism80/fb";
    c.cursor(c.width()-1 - strlen(github) - 3, 4).put(github);

    auto madeby = "made by cshyeon";
    c.cursor(c.width()-1 - strlen(madeby) - 3, 5).put(madeby);

    auto pivot = height + 1;
    auto stack = 0;
    if(fb::game::table::doors.load
    (
        config["table"]["door"].asString(),
        [&] (const std::string& name, double percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::DOOR_LOADED, percentage, name.c_str());
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.cursor(0, pivot + (++stack))
             .puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::DOOR_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    pivot += (stack + 1);
    stack = 0;
    if(fb::game::table::maps.load
    (
        config["table"]["map"].asString(),
        [&] (const std::string& name, double percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::MAP_LOADED, percentage, name.c_str());
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.cursor(0, pivot + (++stack))
             .puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::MAP_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    pivot += (stack + 1);
    stack = 0;
    if(fb::game::table::worlds.load
    (
        config["table"]["world"].asString(),
        [&] (const std::string& name, double percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::WORLD_MAP_LOADED, percentage, name.c_str());
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.cursor(0, pivot + (++stack))
             .puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::WORLD_MAP_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    pivot += (stack + 1);
    stack = 0;
    if(fb::game::table::spells.load
    (
        config["table"]["spell"].asString(),
        [&] (const std::string& name, double percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::SPELL_LOADED, percentage, name.c_str());
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.cursor(0, pivot + (++stack))
             .puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::SPELL_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }


    pivot += (stack + 1);
    stack = 0;
    if(fb::game::table::maps.load_warps
    (
        config["table"]["warp"].asString(),
        [&] (const std::string& name, double percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::WARP_LOADED, percentage, name.c_str());
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.cursor(0, pivot + (++stack))
             .puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::WARP_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    pivot += (stack + 1);
    stack = 0;
    if(fb::game::table::items.load
    (
        config["table"]["item"].asString(),
        [&] (const std::string& name, double percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::ITEM_LOADED, percentage, name.c_str());
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.cursor(0, pivot + (++stack))
             .puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::ITEM_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    pivot += (stack + 1);
    stack = 0;
    if(fb::game::table::mixes.load
    (
        config["table"]["item mix"].asString(),
        [&] (const std::string& name, double percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::ITEM_MIX_LOADED, percentage, name.c_str());
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.cursor(0, pivot + (++stack))
             .puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::ITEM_MIX_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    pivot += (stack + 1);
    stack = 0;
    if(fb::game::table::npcs.load
    (
        config["table"]["npc"].asString(),
        [&] (const std::string& name, double percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::NPC_LOADED, percentage, name.c_str());
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.cursor(0, pivot + (++stack))
             .puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::NPC_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    pivot += (stack + 1);
    stack = 0;
    if(fb::game::table::mobs.load
    (
        config["table"]["mob"].asString(),
        [&] (const std::string& name, double percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::MOB_LOADED, percentage, name.c_str());
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.cursor(0, pivot + (++stack))
             .puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::MOB_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    pivot += (stack + 1);
    stack = 0;
    if(fb::game::table::mobs.load_drops
    (
        config["table"]["item drop"].asString(),
        [&] (const std::string& name, double percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::ITEM_LOADED, percentage, name.c_str());
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.cursor(0, pivot + (++stack));

            if(name.size() > 0)
                c.puts("    - %s (%s)", error.c_str(), name.c_str());
            else
                c.puts("    - %s", error.c_str());
            c.trim();
        }, 
        [&] (uint32_t count)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::DROP_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    pivot += (stack + 1);
    stack = 0;
    if(fb::game::table::npcs.load_spawn
    (
        config["table"]["npc spawn"].asString(),
        context, 
        [&] (const std::string& name, double percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::NPC_SPAWN_LOADED, percentage, name.c_str());
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.cursor(0, pivot + (++stack))
             .puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::NPC_SPAWN_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    pivot += (stack + 1);
    stack = 0;
    if(fb::game::table::mobs.load_spawn
    (
        config["table"]["mob spawn"].asString(),
        context, 
        [&] (const std::string& name, double percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::MOB_SPAWN_LOADED, percentage, name.c_str());
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.cursor(0, pivot + (++stack))
             .puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::MOB_SPAWN_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    pivot += (stack + 1);
    stack = 0;
    if(fb::game::table::classes.load
    (
        config["table"]["class"].asString(),
        [&] (const std::string& name, double percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::CLASS_LOADED, percentage, name.c_str());
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.cursor(0, pivot + (++stack))
             .puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::CLASS_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    pivot += (stack + 1);
    c.cursor(0, pivot);
    return true;
}

int main(int argc, const char** argv)
{
    auto& c = fb::console::get();

    try
    {
        //_CrtSetBreakAlloc(12722647);

#ifdef _WIN32
        ::SetConsoleIcon(IDI_BARAM);
        ::SetConsoleTitle(CONSOLE_TITLE);
#endif

        const char* env = std::getenv("KINGDOM_OF_WIND_ENVIRONMENT");
        if(env == nullptr)
            env = "dev";
        auto& config = fb::config::get(env);

        boost::asio::io_context io_context;
        fb::db::bind(io_context);

        const auto connection = INTERNAL_CONNECTION
        {
            config["internal"]["ip"].asString(),
            (uint16_t)config["internal"]["port"].asInt(),
            [&] (fb::base::socket<>& socket, bool success)
            {
                if(success)
                {
                    socket.send(fb::protocol::internal::request::subscribe(config["id"].asString(), fb::protocol::internal::services::GAME, (uint8_t)config["group"].asUInt()));
                }
                else
                {
                    auto& c = fb::console::get();
                    auto t = std::time(nullptr);
                    auto tm = *std::localtime(&t);

                    std::ostringstream sstream;
                    sstream << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
                    c.puts(" * [ERROR] Failed connect to internal server. (%s)", sstream.str().c_str());
                }
            },
            [&] ()
            {
                auto& c = fb::console::get();
                auto t = std::time(nullptr);
                auto tm = *std::localtime(&t);

                std::ostringstream sstream;
                sstream << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
                c.puts(" * [ERROR] Internal connection has disconnected. (%s)", sstream.str().c_str());
            }
        };

        auto context = std::make_unique<fb::game::context>
        (
            io_context, 
            config["port"].asInt(), 
            std::chrono::seconds(config["delay"].asInt()),
            connection
        );

        load_db(c, *context);

        boost::asio::signal_set signal(io_context, SIGINT, SIGTERM);
        signal.async_wait
        (
            [&context](const boost::system::error_code& error, int signal)
            {
                context.get()->exit();
            }
        );

        io_context.run();
    }
    catch(std::exception& e)
    {
        c.puts(e.what());
    }

    // Release
    return 0;
}