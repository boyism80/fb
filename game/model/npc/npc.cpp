#include "model/npc/npc.h"
#include "model/acceptor/acceptor.game.h"
#include "builtin/builtin_function.h"

using namespace fb::game;

fb::game::npc::master::master(const std::string& name, uint16_t look, uint8_t color, const std::string& script) : 
    fb::game::object::master(name, look, color),
    script(script)
{ }

fb::game::npc::master::~master()
{ }

fb::game::object* fb::game::npc::master::make(listener* listener) const
{
    return new npc(this, listener);
}

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

fb::game::npc::npc(const fb::game::npc::master* master, listener* listener) : 
    fb::game::object(master, listener)
{ }

fb::game::npc::npc(const npc& right) :
    object(right)
{ }

fb::game::npc::~npc()
{ }

fb::game::npc* fb::game::npc::make()
{
    return new npc(*this);
}

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