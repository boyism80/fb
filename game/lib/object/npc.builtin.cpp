#include <fb/game/npc.h>
#include <fb/game/context.h>
#include <fb/game/lua.builtin.h>

int fb::game::npc::model::builtin_input(lua_State* lua)
{
    // Ex) npc::input(session, "message")
    return ::builtin_input<fb::game::npc::model>(lua);
}

int fb::game::npc::model::builtin_menu(lua_State* lua)
{
    // Ex) npc::menu(session, "hello", {"hello 1", "hello 2", "hello 3"})
    return ::builtin_menu<fb::game::npc::model>(lua);
}

int fb::game::npc::model::builtin_item(lua_State* lua)
{
    // Ex) npc::menu(session, "hello", {item1, item2, item3})
    return ::builtin_item<fb::game::npc::model>(lua);
}

fb::game::npc::npc(fb::game::context& context, const fb::game::npc::model* model) : 
    fb::game::object(context, model, fb::game::object::config())
{ }

fb::game::npc::npc(const npc& right) :
    object(right)
{ }

fb::game::npc::~npc()
{ }

int fb::game::npc::builtin_input(lua_State* lua)
{
    return ::builtin_input<npc>(lua);
}

int fb::game::npc::builtin_menu(lua_State* lua)
{
    return ::builtin_menu<npc>(lua);
}

int fb::game::npc::builtin_item(lua_State* lua)
{
    return ::builtin_item<npc>(lua);
}