#include "resource.h"
#include <fb/game/context.h>
#include <fb/core/leak.h>

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
    if(fb::game::model::doors.load
    (
        config["table"]["door"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::DOOR_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
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
    if(fb::game::model::maps.load
    (
        config["table"]["map"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::MAP_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
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
    if(fb::game::model::worlds.load
    (
        config["table"]["world"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::WORLD_MAP_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
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
    if(fb::game::model::spells.load
    (
        config["table"]["spell"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::SPELL_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
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
    if(fb::game::model::maps.load_warps
    (
        config["table"]["warp"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::WARP_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
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
    if(fb::game::model::items.load
    (
        config["table"]["item"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::ITEM_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
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
    if(fb::game::model::mixes.load
    (
        config["table"]["item mix"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::ITEM_MIX_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
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
    if(fb::game::model::npcs.load
    (
        config["table"]["npc"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::NPC_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
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
    if(fb::game::model::mobs.load
    (
        config["table"]["mob"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::MOB_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
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
    if(fb::game::model::mobs.load_drops
    (
        config["table"]["item drop"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::ITEM_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
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
    if(fb::game::model::npcs.load_spawn
    (
        config["table"]["npc spawn"].asString(),
        context, 
        [&] (const auto& name, auto percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::NPC_SPAWN_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
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
    if(fb::game::model::mobs.load_spawn
    (
        config["table"]["mob spawn"].asString(),
        context, 
        [&] (const auto& name, auto percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::MOB_SPAWN_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
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
    if(fb::game::model::classes.load
    (
        config["table"]["class"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.cursor(0, pivot)
             .puts(fb::game::message::assets::CLASS_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
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

        boost::asio::io_context io_context;
        auto& config = fb::config::get();
        auto context = std::make_unique<fb::game::context>
        (
            io_context, 
            config["port"].asInt(), 
            std::chrono::seconds(config["delay"].asInt())
        );
        
        load_db(c, *context);
        
        int count = fb::config::get()["thread"].isNull() ? std::thread::hardware_concurrency() : fb::config::get()["thread"].asInt();
        context->run(count);
        while (context->running())
        {
            std::this_thread::sleep_for(100ms);
        }
    }
    catch(std::exception& e)
    {
        fb::logger::fatal(e.what());
    }

    // Release
    return 0;
}