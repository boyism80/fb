#include <fb/game/npc.h>
#include <fb/game/context.h>
#include <fb/game/lua.builtin.h>

int fb::game::npc::master::builtin_input_dialog(lua_State* lua)
{
    // Ex) npc::input_dialog(session, "message")
    return ::builtin_input_dialog<fb::game::npc::master>(lua);
}

int fb::game::npc::master::builtin_menu_dialog(lua_State* lua)
{
    // Ex) npc::menu_dialog(session, "hello", {"hello 1", "hello 2", "hello 3"})
    return ::builtin_menu_dialog<fb::game::npc::master>(lua);
}

int fb::game::npc::master::builtin_item_dialog(lua_State* lua)
{
    // Ex) npc::menu_dialog(session, "hello", {item1, item2, item3})
    return ::builtin_item_dialog<fb::game::npc::master>(lua);
}

fb::game::npc::npc(fb::game::context& context, const fb::game::npc::master* master) : 
    fb::game::object(context, master, fb::game::object::config())
{ }

fb::game::npc::npc(const npc& right) :
    object(right)
{ }

fb::game::npc::~npc()
{ }

int fb::game::npc::builtin_input_dialog(lua_State* lua)
{
    return ::builtin_input_dialog<npc>(lua);
}

int fb::game::npc::builtin_menu_dialog(lua_State* lua)
{
    return ::builtin_menu_dialog<npc>(lua);
}

int fb::game::npc::builtin_item_dialog(lua_State* lua)
{
    return ::builtin_item_dialog<npc>(lua);
}