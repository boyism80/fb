#include "resource.h"
#include "module/socket/socket.h"
#include "module/config/config.h"
#include "module/leak.h"
#include "model/acceptor/acceptor.game.h"
#include "model/master/master.h"
#include "model/lua/lua.h"
#include "module/console/console.h"
#include "module/string/string.h"
#include "protocol/internal.h"

void clear(fb::console& c, uint32_t x, int y)
{
    static int prev = 0;
    static char buffer[256];
    
    if(prev > x)
    {
        auto len = c.width() - x;
        std::memset(buffer, ' ', len);
        buffer[len] = 0;
        c.move(x, y).puts(buffer);
    }

    prev = x;
}

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

    auto& master = fb::game::master::get();

    if(master.load_door("db/door.json", 
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::DOOR_LOADED, percentage, name.c_str());
            clear(c, n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::DOOR_ALL_LOADED, count);
            clear(c, n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();
#if defined DEBUG | defined _DEBUG
    if(master.load_maps("db/map-temp.json", 
#else
    if(master.load_maps("db/map.json",
#endif
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::MAP_LOADED, percentage, name.c_str());
            clear(c, n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::MAP_ALL_LOADED, count);
            clear(c, n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();
    if(master.load_spell("db/spell.json", 
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::SPELL_LOADED, percentage, name.c_str());
            clear(c, n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::SPELL_ALL_LOADED, count);
            clear(c, n, current_line);
        }) == false)
    {
        return false;
    }


    current_line = c.current_line();
    c.next();
    if(master.load_warp("db/warp.json", 
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::WARP_LOADED, percentage, name.c_str());
            clear(c, n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::WARP_ALL_LOADED, count);
            clear(c, n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();
    if(master.load_items("db/item.json", 
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::ITEM_LOADED, percentage, name.c_str());
            clear(c, n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::ITEM_ALL_LOADED, count);
            clear(c, n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();
    if(master.load_itemmix("db/itemmix.json", 
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::ITEM_MIX_LOADED, percentage, name.c_str());
            clear(c, n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::ITEM_MIX_ALL_LOADED, count);
            clear(c, n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();
    if(master.load_npc("db/npc.json", 
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::NPC_LOADED, percentage, name.c_str());
            clear(c, n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::NPC_ALL_LOADED, count);
            clear(c, n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();
    if(master.load_mob("db/mob.json", 
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::MOB_LOADED, percentage, name.c_str());
            clear(c, n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::MOB_ALL_LOADED, count);
            clear(c, n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();
    if(master.load_drop_item("db/item_drop.json", 
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::ITEM_LOADED, percentage, name.c_str());
            clear(c, n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::DROP_ALL_LOADED, count);
            clear(c, n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();
    if(master.load_npc_spawn("db/npc_spawn.json", listener, 
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::NPC_SPAWN_LOADED, percentage, name.c_str());
            clear(c, n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::NPC_SPAWN_ALL_LOADED, count);
            clear(c, n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();
    if(master.load_mob_spawn("db/mob_spawn.json", listener, 
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::MOB_SPAWN_LOADED, percentage, name.c_str());
            clear(c, n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::MOB_SPAWN_ALL_LOADED, count);
            clear(c, n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();
    if(master.load_class("db/class.json", 
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::CLASS_LOADED, percentage, name.c_str());
            clear(c, n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::CLASS_ALL_LOADED, count);
            clear(c, n, current_line);
        }) == false)
    {
        return false;
    }

    current_line = c.current_line();
    c.next();
    if(master.load_board("db/board.json", 
        [&] (const std::string& name, double percentage)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::BOARD_LOADED, percentage, name.c_str());
            clear(c, n, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            c.puts("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            auto n = c.move(0, current_line)
                      .puts(fb::game::message::assets::BOARD_ALL_LOADED, count);
            clear(c, n, current_line);
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
            {
                for(auto x : config["id"])
                    socket.send(fb::protocol::internal::request::subscribe(x.asString()));
            }
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
    fb::game::master::release();
    fb::console::release();
    return 0;
}