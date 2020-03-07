#include "builtin_function.h"
#include "fb_game.h"

using namespace fb::game;

int fb::game::acceptor::builtin_name2mob(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");

    auto name = lua_tostring(lua, 1);
    auto mob = db::name2mob(name);

    if(mob == nullptr)  { lua_pushnil(lua); }
    else                { mob->new_lua<fb::game::mob::core>(lua); }
    return 1;
}

int fb::game::acceptor::builtin_name2npc(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");

    auto name = lua_tostring(lua, 1);
    auto npc = db::name2npc(name);
    
    if(npc == nullptr)  { lua_pushnil(lua); }
    else                { npc->new_lua<fb::game::npc::core>(lua); }
    return 1;
}

int fb::game::acceptor::builtin_name2item(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");

    auto name = lua_tostring(lua, 1);
    auto item = db::name2item(name);

    if(item == nullptr) { lua_pushnil(lua); }
    else                { item->new_lua<fb::game::npc::core>(lua); }
    return 1;
}
