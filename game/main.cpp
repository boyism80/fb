#include "resource.h"
#include <stdio.h>
#include <iostream>
#include "module/socket/socket.h"
#include "module/config/config.h"
#include "module/console/console.h"
#include "module/leak.h"
#include "model/acceptor/acceptor.game.h"
#include "model/master/master.h"
#include "model/lua/lua.h"

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

int main(int argc, const char** argv)
{
    //_CrtSetBreakAlloc(157);

    auto& c = console::get();

#ifdef _WIN32
    ::SetConsoleIcon(IDI_BARAM);
    ::SetConsoleTitle(CONSOLE_TITLE);
#endif

    // Execute acceptor
    auto& config = fb::config();
    boost::asio::io_context io_service;
    auto acceptor = new fb::game::acceptor(io_service, config["port"].asInt(), config["delay"].asInt());

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
            sprintf(buffer, " * [%0.2lf%%] 맵 정보를 읽었습니다. (%s)", percentage, name.c_str());
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
            sprintf(buffer, " * [100%%] 총 %d개의 맵 정보를 읽었습니다.", count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return 0;
    }

    current_line =  current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_door("db/door.json", 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, " * [%0.2lf%%] 도어 정보를 읽었습니다. (%s)", percentage, name.c_str());
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
            sprintf(buffer, " * [100%%] 총 %d개의 도어 정보를 읽었습니다.", count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return 0;
    }

    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_spell("db/spell.json", 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, " * [%0.2lf%%] 스펠 정보를 읽었습니다. (%s)", percentage, name.c_str());
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
            sprintf(buffer, " * [100%%] 총 %d개의 스펠 정보를 읽었습니다.", count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return 0;
    }


    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_warp("db/warp.json", 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, " * [%0.2lf%%] 워프 정보를 읽었습니다. (%s)", percentage, name.c_str());
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
            sprintf(buffer, " * [100%%] 총 %d개의 워프 정보를 읽었습니다.", count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return 0;
    }

    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_items("db/item.json", 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, " * [%0.2lf%%] 아이템 정보를 읽었습니다. (%s)", percentage, name.c_str());
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
            sprintf(buffer, " * [100%%] 총 %d개의 아이템 정보를 읽었습니다.", count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return 0;
    }

    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_itemmix("db/itemmix.json", 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, " * [%0.2lf%%] 조합 정보를 읽었습니다. (%s)", percentage, name.c_str());
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
            sprintf(buffer, " * [100%%] 총 %d개의 조합 정보를 읽었습니다.", count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return 0;
    }

    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_npc("db/npc.json", 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, " * [%0.2lf%%] NPC 정보를 읽었습니다. (%s)", percentage, name.c_str());
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
            sprintf(buffer, " * [100%%] 총 %d개의 NPC 정보를 읽었습니다.", count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return 0;
    }

    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_mob("db/mob.json", 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, " * [%0.2lf%%] 몹 정보를 읽었습니다. (%s)", percentage, name.c_str());
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
            sprintf(buffer, " * [100%%] 총 %d개의 몹 정보를 읽었습니다.", count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return 0;
    }

    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_drop_item("db/item_drop.json", 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, " * [%0.2lf%%] 드롭 정보를 읽었습니다. (%s)", percentage, name.c_str());
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
            sprintf(buffer, " * [100%%] 총 %d개의 드롭 정보를 읽었습니다.", count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return 0;
    }

    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_npc_spawn("db/npc_spawn.json", acceptor, 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, " * [%0.2lf%%] NPC 스폰 정보를 읽었습니다. (%s)", percentage, name.c_str());
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
            sprintf(buffer, " * [100%%] 총 %d개의 NPC 스폰 정보를 읽었습니다.", count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return 0;
    }

    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_mob_spawn("db/mob_spawn.json", acceptor, 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, " * [%0.2lf%%] 몹 스폰 정보를 읽었습니다. (%s)", percentage, name.c_str());
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
            sprintf(buffer, " * [100%%] 총 %d개의 몹 스폰 정보를 읽었습니다.", count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return 0;
    }

    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_class("db/class.json", 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, " * [%0.2lf%%] 클래스 정보를 읽었습니다. (%s)", percentage, name.c_str());
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
            sprintf(buffer, " * [100%%] 총 %d개의 클래스 정보를 읽었습니다.", count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return 0;
    }

    current_line = current_line + current_error_line + 1;
    current_error_line = 0;
    if(master.load_board("db/board.json", 
        [&] (const std::string& name, double percentage)
        {
            sprintf(buffer, " * [%0.2lf%%] 게시판 정보를 읽었습니다. (%s)", percentage, name.c_str());
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
            sprintf(buffer, " * [100%%] 총 %d개의 게시판 정보를 읽었습니다.", count);
            c.puts(buffer, 0, current_line);
            clear(c, buffer, current_line);
        }) == false)
    {
        return 0;
    }
    io_service.run();

    // Release
    delete acceptor;
    fb::game::lua::release();
    fb::game::master::release();

    return 0;
}