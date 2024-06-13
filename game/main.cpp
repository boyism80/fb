#include "resource.h"
#include <fb/game/context.h>
#include <fb/core/leak.h>
#include <fb/core/mst.h>

bool load_db(fb::console& c, fb::game::context& context)
{
    auto& config = fb::config::get();

    auto height = 8;
    c.box(c.width()-1, height);

    auto header = "The Kingdom of the wind [GAME]";
    c.cursor((c.width()-1 - strlen(header)) / 2, 2).put(header);

    auto github = "https://github.com/boyism80/fb";
    c.cursor(c.width()-1 - strlen(github) - 3, 4).put(github);

    auto madeby = "made by cshyeon";
    c.cursor(c.width()-1 - strlen(madeby) - 3, 5).put(madeby);

    c.cursor(0, height + 1);

    if(fb::game::old_model::doors.load
    (
        config["table"]["door"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.put(fb::game::message::assets::DOOR_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
        {
            c.comment("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.put(fb::game::message::assets::DOOR_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    c.next();
    if(fb::game::old_model::regex.load
    (
        config["table"]["regex"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.put(fb::game::message::assets::REGEX_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
        {
            c.comment("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.put(fb::game::message::assets::REGEX_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    c.next();
    if(fb::game::old_model::maps.load
    (
        config["table"]["map"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.put(fb::game::message::assets::MAP_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
        {
            c.comment("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.put(fb::game::message::assets::MAP_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    c.next();
    if(fb::game::old_model::worlds.load
    (
        config["table"]["world"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.put(fb::game::message::assets::WORLD_MAP_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
        {
            c.comment("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.put(fb::game::message::assets::WORLD_MAP_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    c.next();
    if(fb::game::old_model::spells.load
    (
        config["table"]["spell"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.put(fb::game::message::assets::SPELL_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
        {
            c.comment("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.put(fb::game::message::assets::SPELL_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }


    c.next();
    if(fb::game::old_model::maps.load_warps
    (
        config["table"]["warp"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.put(fb::game::message::assets::WARP_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
        {
            c.comment("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.put(fb::game::message::assets::WARP_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    c.next();
    if(fb::game::old_model::items.load
    (
        config["table"]["item"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.put(fb::game::message::assets::ITEM_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
        {
            c.comment("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.put(fb::game::message::assets::ITEM_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    c.next();
    if(fb::game::old_model::mixes.load
    (
        config["table"]["item mix"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.put(fb::game::message::assets::ITEM_MIX_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
        {
            c.comment("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.put(fb::game::message::assets::ITEM_MIX_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    c.next();
    if(fb::game::old_model::sell.load
    (
        config["table"]["sell"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.put(fb::game::message::assets::SELL_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
        {
            c.comment("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.put(fb::game::message::assets::SELL_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    c.next();
    if(fb::game::old_model::buy.load
    (
        config["table"]["buy"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.put(fb::game::message::assets::BUY_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
        {
            c.comment("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.put(fb::game::message::assets::BUY_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    c.next();
    if(fb::game::old_model::npcs.load
    (
        config["table"]["npc"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.put(fb::game::message::assets::NPC_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
        {
            c.comment("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.put(fb::game::message::assets::NPC_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    c.next();
    if(fb::game::old_model::mobs.load
    (
        config["table"]["mob"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.put(fb::game::message::assets::MOB_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
        {
            c.comment("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.put(fb::game::message::assets::MOB_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    c.next();
    if(fb::game::old_model::mobs.load_drops
    (
        config["table"]["item drop"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.put(fb::game::message::assets::ITEM_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
        {
            if(name.size() > 0)
                c.comment("    - %s (%s)", error.c_str(), name.c_str());
            else
                c.comment("    - %s", error.c_str());
            c.trim();
        }, 
        [&] (uint32_t count)
        {
            c.put(fb::game::message::assets::DROP_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    c.next();
    if(fb::game::old_model::npcs.load_spawn
    (
        config["table"]["npc spawn"].asString(),
        context, 
        [&] (const auto& name, auto percentage)
        {
            c.put(fb::game::message::assets::NPC_SPAWN_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
        {
            c.comment("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.put(fb::game::message::assets::NPC_SPAWN_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    c.next();
    if(fb::game::old_model::mobs.load_spawn
    (
        config["table"]["mob spawn"].asString(),
        context, 
        [&] (const auto& name, auto percentage)
        {
            c.put(fb::game::message::assets::MOB_SPAWN_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
        {
            c.comment("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.put(fb::game::message::assets::MOB_SPAWN_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    c.next();
    if(fb::game::old_model::classes.load
    (
        config["table"]["class"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.put(fb::game::message::assets::CLASS_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
        {
            c.comment("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.put(fb::game::message::assets::CLASS_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

    c.next();
    if(fb::game::old_model::boards.load
    (
        config["table"]["board"].asString(),
        [&] (const auto& name, auto percentage)
        {
            c.put(fb::game::message::assets::BOARD_LOADED, percentage, name.c_str());
        }, 
        [&] (const auto& name, const auto& error)
        {
            c.comment("    - %s (%s)", error.c_str(), name.c_str());
        }, 
        [&] (uint32_t count)
        {
            c.put(fb::game::message::assets::BOARD_ALL_LOADED, count);
        }) == false)
    {
        return false;
    }

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
        auto context = std::make_unique<fb::game::context>(io_context, config["port"].asInt());

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