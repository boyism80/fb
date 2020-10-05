#include "resource.h"
#include <stdio.h>
#include <iostream>
#include "module/socket/socket.h"
#include "module/config/config.h"
#include "module/leak.h"
#include "model/acceptor/acceptor.game.h"
#include "model/master/master.h"
#include "model/lua/lua.h"
#include "module/console/console.h"
#include "module/string/string.h"

void clear(console& c, char* buffer, int current_line)
{
    static int prev_size = 0;

    auto current_size = strlen(buffer);
    if(prev_size > current_size)
    {
        std::memset(buffer, ' ', prev_size - current_size);
        buffer[prev_size - current_size] = 0;
        c.puts(buffer, current_size, current_line);
    }

    prev_size = current_size;
}

bool load_db(console& c, fb::game::listener* listener)
{
    auto& config = fb::config::get();

    auto height = 8;
    c.box(0, 0, c.width()-1, height);

    auto header = "The Kingdom of the wind [GAME]";
    c.puts(header, (c.width()-1 - strlen(header)) / 2, 2);

    auto github = "https://github.com/boyism80/fb";
    c.puts(github, c.width()-1 - strlen(github) - 3, 4);

    auto madeby = "made by cshyeon";
    c.puts(madeby, c.width()-1 - strlen(madeby) - 3, 5);

    auto current_line = height;
    auto current_error_line = 0;
    char buffer[256] = {0,};

    auto& master = fb::game::master::get();
#if defined DEBUG | defined _DEBUG
    if(master.load_maps("db/map-temp.json", 
#else
    if(master.load_maps("db/map.json",
#endif
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, fb::game::message::assets::MAP_LOADED, percentage, name.c_str());
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            sprintf(buffer, "    - %s (%s)", error.c_str(), name.c_str());
            c.puts(buffer, 0, current_line + (++current_error_line));
        }, 
        [&] (uint32_t count)
        {
            sprintf(buffer, fb::game::message::assets::MAP_ALL_LOADED, count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return false;
    }

    current_line =  current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_door("db/door.json", 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, fb::game::message::assets::DOOR_LOADED, percentage, name.c_str());
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            sprintf(buffer, "    - %s (%s)", error.c_str(), name.c_str());
            c.puts(buffer, 0, current_line + (++current_error_line));
        }, 
        [&] (uint32_t count)
        {
            sprintf(buffer, fb::game::message::assets::DOOR_ALL_LOADED, count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return false;
    }

    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_spell("db/spell.json", 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, fb::game::message::assets::SPELL_LOADED, percentage, name.c_str());
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            sprintf(buffer, "    - %s (%s)", error.c_str(), name.c_str());
            c.puts(buffer, 0, current_line + (++current_error_line));
        }, 
        [&] (uint32_t count)
        {
            sprintf(buffer, fb::game::message::assets::SPELL_ALL_LOADED, count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return false;
    }


    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_warp("db/warp.json", 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, fb::game::message::assets::WARP_LOADED, percentage, name.c_str());
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            sprintf(buffer, "    - %s (%s)", error.c_str(), name.c_str());
            c.puts(buffer, 0, current_line + (++current_error_line));
        }, 
        [&] (uint32_t count)
        {
            sprintf(buffer, fb::game::message::assets::WARP_ALL_LOADED, count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return false;
    }

    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_items("db/item.json", 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, fb::game::message::assets::ITEM_LOADED, percentage, name.c_str());
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            sprintf(buffer, "    - %s (%s)", error.c_str(), name.c_str());
            c.puts(buffer, 0, current_line + (++current_error_line));
        }, 
        [&] (uint32_t count)
        {
            sprintf(buffer, fb::game::message::assets::ITEM_ALL_LOADED, count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return false;
    }

    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_itemmix("db/itemmix.json", 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, fb::game::message::assets::ITEM_MIX_LOADED, percentage, name.c_str());
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            sprintf(buffer, "    - %s (%s)", error.c_str(), name.c_str());
            c.puts(buffer, 0, current_line + (++current_error_line));
        }, 
        [&] (uint32_t count)
        {
            sprintf(buffer, fb::game::message::assets::ITEM_MIX_ALL_LOADED, count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return false;
    }

    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_npc("db/npc.json", 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, fb::game::message::assets::NPC_LOADED, percentage, name.c_str());
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            sprintf(buffer, "    - %s (%s)", error.c_str(), name.c_str());
            c.puts(buffer, 0, current_line + (++current_error_line));
        }, 
        [&] (uint32_t count)
        {
            sprintf(buffer, fb::game::message::assets::NPC_ALL_LOADED, count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return false;
    }

    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_mob("db/mob.json", 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, fb::game::message::assets::MOB_LOADED, percentage, name.c_str());
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            sprintf(buffer, "    - %s (%s)", error.c_str(), name.c_str());
            c.puts(buffer, 0, current_line + (++current_error_line));
        }, 
        [&] (uint32_t count)
        {
            sprintf(buffer, fb::game::message::assets::MOB_ALL_LOADED, count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return false;
    }

    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_drop_item("db/item_drop.json", 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, fb::game::message::assets::ITEM_LOADED, percentage, name.c_str());
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            sprintf(buffer, "    - %s (%s)", error.c_str(), name.c_str());
            c.puts(buffer, 0, current_line + (++current_error_line));
        }, 
        [&] (uint32_t count)
        {
            sprintf(buffer, fb::game::message::assets::DROP_ALL_LOADED, count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return false;
    }

    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_npc_spawn("db/npc_spawn.json", listener, 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, fb::game::message::assets::NPC_SPAWN_LOADED, percentage, name.c_str());
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            sprintf(buffer, "    - %s (%s)", error.c_str(), name.c_str());
            c.puts(buffer, 0, current_line + (++current_error_line));
        }, 
        [&] (uint32_t count)
        {
            sprintf(buffer, fb::game::message::assets::NPC_SPAWN_ALL_LOADED, count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return false;
    }

    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_mob_spawn("db/mob_spawn.json", listener, 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, fb::game::message::assets::MOB_SPAWN_LOADED, percentage, name.c_str());
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            sprintf(buffer, "    - %s (%s)", error.c_str(), name.c_str());
            c.puts(buffer, 0, current_line + (++current_error_line));
        }, 
        [&] (uint32_t count)
        {
            sprintf(buffer, fb::game::message::assets::MOB_SPAWN_ALL_LOADED, count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return false;
    }

    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_class("db/class.json", 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, fb::game::message::assets::CLASS_LOADED, percentage, name.c_str());
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            sprintf(buffer, "    - %s (%s)", error.c_str(), name.c_str());
            c.puts(buffer, 0, current_line + (++current_error_line));
        }, 
        [&] (uint32_t count)
        {
            sprintf(buffer, fb::game::message::assets::CLASS_ALL_LOADED, count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return false;
    }

    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_board("db/board.json", 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, fb::game::message::assets::BOARD_LOADED, percentage, name.c_str());
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }, 
        [&] (const std::string& name, const std::string& error)
        {
            sprintf(buffer, "    - %s (%s)", error.c_str(), name.c_str());
            c.puts(buffer, 0, current_line + (++current_error_line));
        }, 
        [&] (uint32_t count)
        {
            sprintf(buffer, fb::game::message::assets::BOARD_ALL_LOADED, count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return false;
    }

    return true;
}

int main(int argc, const char** argv)
{
    //_CrtSetBreakAlloc(157);

    auto& c = console::get();

#ifdef _WIN32
    ::SetConsoleIcon(IDI_BARAM);
    ::SetConsoleTitle(CONSOLE_TITLE);
#endif

    // Execute acceptor
    auto& config = fb::config::get();
    boost::asio::io_context io_service;
    auto acceptor = new fb::game::acceptor(io_service, config["port"].asInt(), config["delay"].asInt());

    load_db(c, acceptor);
    io_service.run();

    // Release
    delete acceptor;
    fb::game::lua::release();
    fb::game::master::release();
    c.release();
    return 0;
}