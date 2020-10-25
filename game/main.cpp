#include "resource.h"
#include "module/socket/socket.h"
#include "module/config/config.h"
#include "module/leak.h"
#include "model/acceptor/acceptor.game.h"
#include "module/string/string.h"
#include "protocol/internal.h"
#include "model/lua/lua.h"
#include "module/console/console.h"
#include "model/table/table.game.h"

bool load_db(fb::console& c, fb::game::listener* listener)
{
    auto& config = fb::config::get();

    auto height = 8;
    c.box(0, 0, c.width()-1, height);

    auto header = "The Kingdom of the wind [GAME]";
    c.move((c.width()-1 - strlen(header)) / 2, 2).puts(header);

    auto github = "https://github.com/boyism80/fb";
    c.move(c.width()-1 - strlen(github) - 3, 4).puts(github);

    auto madeby = "made by cshyeon";
    c.move(c.width()-1 - strlen(madeby) - 3, 5).puts(madeby);

    c.current_line(height + 1);
    auto current_line = height;

    if(fb::game::table::doors.load
    (
        config["table"]["door"].asString(),
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::DOOR_LOADED, percentage, name.c_str());
            c.clear(n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::DOOR_ALL_LOADED, count);
            c.clear(n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();

    if(fb::game::table::maps.load
    (
        config["table"]["map"].asString(),
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::MAP_LOADED, percentage, name.c_str());
            c.clear(n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::MAP_ALL_LOADED, count);
            c.clear(n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();
    if(fb::game::table::spells.load
    (
        config["table"]["spell"].asString(),
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::SPELL_LOADED, percentage, name.c_str());
            c.clear(n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::SPELL_ALL_LOADED, count);
            c.clear(n, current_line);
        }) == false)
    {
        return false;
    }


    current_line = c.current_line();
    c.next();
    if(fb::game::table::maps.load_warps
    (
        config["table"]["warp"].asString(),
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::WARP_LOADED, percentage, name.c_str());
            c.clear(n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::WARP_ALL_LOADED, count);
            c.clear(n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();
    if(fb::game::table::items.load
    (
        config["table"]["item"].asString(),
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::ITEM_LOADED, percentage, name.c_str());
            c.clear(n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::ITEM_ALL_LOADED, count);
            c.clear(n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();
    if(fb::game::table::mixes.load
    (
        config["table"]["item mix"].asString(),
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::ITEM_MIX_LOADED, percentage, name.c_str());
            c.clear(n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::ITEM_MIX_ALL_LOADED, count);
            c.clear(n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();
    if(fb::game::table::npcs.load
    (
        config["table"]["npc"].asString(),
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::NPC_LOADED, percentage, name.c_str());
            c.clear(n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::NPC_ALL_LOADED, count);
            c.clear(n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();
    if(fb::game::table::mobs.load
    (
        config["table"]["mob"].asString(),
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::MOB_LOADED, percentage, name.c_str());
            c.clear(n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::MOB_ALL_LOADED, count);
            c.clear(n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();
    if(fb::game::table::mobs.load_drops
    (
        config["table"]["item drop"].asString(),
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::ITEM_LOADED, percentage, name.c_str());
            c.clear(n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            if(name.size() > 0)
                c.puts("    - %s (%s)", error.c_str(), name.c_str());
            else
                c.puts("    - %s", error.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::DROP_ALL_LOADED, count);
            c.clear(n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();
    if(fb::game::table::npcs.load_spawn
    (
        config["table"]["npc spawn"].asString(),
        listener, 
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::NPC_SPAWN_LOADED, percentage, name.c_str());
            c.clear(n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::NPC_SPAWN_ALL_LOADED, count);
            c.clear(n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();
    if(fb::game::table::mobs.load_spawn
    (
        config["table"]["mob spawn"].asString(),
        listener, 
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::MOB_SPAWN_LOADED, percentage, name.c_str());
            c.clear(n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::MOB_SPAWN_ALL_LOADED, count);
            c.clear(n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();
    if(fb::game::table::classes.load
    (
        config["table"]["class"].asString(),
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::CLASS_LOADED, percentage, name.c_str());
            c.clear(n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::CLASS_ALL_LOADED, count);
            c.clear(n, current_line);
        }) == false)
    {
        return false;
    }

    return true;
}

int main(int argc, const char** argv)
{
    //_CrtSetBreakAlloc(157);
#ifdef _WIN32
    ::SetConsoleIcon(IDI_BARAM);
    ::SetConsoleTitle(CONSOLE_TITLE);
#endif

    auto& c = fb::console::get();

    // Execute acceptor
    auto& config = fb::config::get();
    boost::asio::io_context io_service;

    const auto connection = INTERNAL_CONNECTION
    {
        config["internal"]["ip"].asString(),
        (uint16_t)config["internal"]["port"].asInt(),
        [&] (fb::base::socket<>& socket, bool success)
        {
            if(success)
                socket.send(fb::protocol::internal::request::subscribe(config["id"].asString()));
        },
        [&] ()
        {
            // on disconnected
        }
    };
    auto acceptor = new fb::game::acceptor
    (
        io_service, 
        config["port"].asInt(), 
        config["delay"].asInt(),
        connection
    );

    load_db(c, acceptor);
    io_service.run();

    // Release
    delete acceptor;
    fb::game::lua::release();
    fb::console::release();
    return 0;
}