#include "model/acceptor/acceptor.game.h"
#include "builtin/builtin_function.h"

using namespace fb::game;

int fb::game::acceptor::builtin_sleep(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    if(thread == nullptr)
        return 0;

    auto ms = (uint32_t)thread->tointeger(1);
    thread->pending(true);

    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    acceptor->threads().dispatch
    (
        [lua] ()
        {
            auto thread = lua::thread::get(*lua);
            thread->pending(false);
            thread->resume(0);
        },
        std::chrono::milliseconds(ms)
    );
    return thread->yield(0);
}

int fb::game::acceptor::builtin_name2mob(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    if(thread == nullptr)
        return 0;

    auto name = thread->tostring(1);
    auto mob = fb::game::table::mobs.name2mob(name);

    if(mob == nullptr)  { thread->pushnil(); }
    else                { mob->to_lua(lua); }
    return 1;
}

int fb::game::acceptor::builtin_name2npc(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    if(thread == nullptr)
        return 0;

    auto name = thread->tostring(1);
    auto npc = fb::game::table::npcs.name2npc(name);
    
    if(npc == nullptr)  { thread->pushnil(); }
    else                { npc->to_lua(lua); }
    return 1;
}

int fb::game::acceptor::builtin_name2map(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    if(thread == nullptr)
        return 0;

    auto name = thread->tostring(1);
    auto map = fb::game::table::maps.name2map(name);

    if(map == nullptr)  { thread->pushnil(); }
    else                { map->to_lua(lua); }
    return 1;
}

int fb::game::acceptor::builtin_name2item(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    if(thread == nullptr)
        return 0;

    auto name = thread->tostring(1);
    auto item = fb::game::table::items.name2item(name);

    if(item == nullptr) { thread->pushnil(); }
    else                { item->to_lua(lua); }
    return 1;
}

int fb::game::acceptor::builtin_timer(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    if(thread == nullptr)
        return 0;

    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto value = (uint32_t)thread->tointeger(1);
    auto decrease = thread->toboolean(2);

    acceptor->send(fb::protocol::game::response::timer(value, decrease ? timer::DECREASE : timer::INCREASE));
    return 0;
}

int fb::game::acceptor::builtin_weather(lua_State* lua)
{
    auto thread = lua::thread::get(*lua);
    if(thread == nullptr)
        return 0;

    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto value = (uint32_t)thread->tointeger(1);

    acceptor->send(fb::protocol::game::response::weather(weather::type(value)));
    return 0;
}