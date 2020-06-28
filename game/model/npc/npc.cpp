#include <model/npc/npc.h>
#include <model/acceptor/acceptor.game.h>
#include <model/listener/listener.h>
#include <builtin/builtin_function.h>

using namespace fb::game;

fb::game::npc::master::master(const std::string& name, uint16_t look, uint8_t color) : 
    fb::game::object::master(name, look, color)
{}

fb::game::npc::master::~master()
{}

fb::game::object* fb::game::npc::master::make(listener* listener) const
{
    return new npc(this, listener);
}

fb::ostream fb::game::npc::master::make_dialog_stream(const std::string& message, const std::vector<std::string>& menus, fb::game::map* map, dialog::interaction interaction) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x2F)
        .write_u8(0x01)
        .write_u8(interaction)
        .write_u32(map ? map->id() : 0x01)
        .write_u8(this->dialog_look_type())
        .write_u8(0x01)
        .write_u16(this->look())
        .write_u8(this->color())
        .write_u8(this->dialog_look_type())
        .write_u16(this->look())
        .write_u8(this->color())
        .write(message, true);

    ostream.write_u16((uint16_t)menus.size());
    for(int i = 0; i < menus.size(); i++)
    {
        ostream.write(menus[i])
            .write_u16(i);
    }

    ostream.write_u8(0x00);
    return ostream;
}

fb::ostream fb::game::npc::master::make_dialog_stream(const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::map* map, dialog::interaction interaction) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x2F)
        .write_u8(0x05)
        .write_u8(interaction)
        .write_u32(map ? map->id() : 0x01)
        .write_u8(this->dialog_look_type())
        .write_u8(0x01)
        .write_u16(this->look())
        .write_u8(this->color())
        .write_u8(this->dialog_look_type())
        .write_u16(this->look())
        .write_u8(this->color())
        .write(message, true)
        .write_u16(0xFFFF)
        .write_u8((uint8_t)item_slots.size());

    for(auto item_slot : item_slots)
        ostream.write_u8(item_slot);
    ostream.write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::npc::master::make_dialog_stream(const std::string& message, const std::vector<fb::game::item::master*>& cores, fb::game::map* map, uint16_t pursuit, dialog::interaction interaction) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x2F)
        .write_u8(0x04)
        .write_u8(interaction)
        .write_u32(map ? map->id() : 0x01)
        .write_u8(this->dialog_look_type())
        .write_u8(0x01)
        .write_u16(this->look())
        .write_u8(this->color())
        .write_u8(this->dialog_look_type())
        .write_u16(this->look())
        .write_u8(this->color())
        .write(message, true)
        .write_u16(pursuit)
        .write_u16((uint16_t)cores.size());

    for(int i = 0; i < cores.size(); i++)
    {
        const auto          item = cores[i];

        ostream.write_u16(item->look())
            .write_u8(item->color())
            .write_u32(item->price())
            .write(item->name())
            .write(item->desc());
    }

    ostream.write_u8(0x00);
    return ostream;
}

fb::ostream fb::game::npc::master::make_input_dialog_stream(const std::string& message, fb::game::map* map, dialog::interaction interaction) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x2F)
        .write_u8(0x03)
        .write_u8(interaction)
        .write_u32(map ? map->id() : 0x01)
        .write_u8(this->dialog_look_type())
        .write_u8(0x01)
        .write_u16(this->look())
        .write_u8(this->color())
        .write_u8(this->dialog_look_type())
        .write_u16(this->look())
        .write_u8(this->color())
        .write(message, true)
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::npc::master::make_input_dialog_stream(const std::string& message, const std::string& top, const std::string& bottom, int maxlen, bool prev, fb::game::map* map, dialog::interaction interaction) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x30)
        .write_u8(0x04)
        .write_u8(interaction)
        .write_u32(map ? map->id() : 0x01)
        .write_u8(this->dialog_look_type())
        .write_u8(0x01)
        .write_u16(this->look())
        .write_u8(this->color())
        .write_u8(this->dialog_look_type())
        .write_u16(this->look())
        .write_u8(this->color())
        .write_u32(0x00000001)
        .write_u8(prev)
        .write_u8(0x00)
        .write(message, true)
        .write(top, false)
        .write_u8(maxlen)
        .write(bottom, false)
        .write_u8(0x00);

    return ostream;
}

int fb::game::npc::master::builtin_input_dialog(lua_State* lua)
{
    // Ex) npc::input_dialog(session, "message")
    return ::builtin_input_dialog<npc::master>(lua);
}

int fb::game::npc::master::builtin_menu_dialog(lua_State* lua)
{
    // Ex) npc::menu_dialog(session, "hello", {"hello 1", "hello 2", "hello 3"})
    return ::builtin_menu_dialog<npc::master>(lua);
}

int fb::game::npc::master::builtin_item_dialog(lua_State* lua)
{
    // Ex) npc::menu_dialog(session, "hello", {item1, item2, item3})
    return ::builtin_item_dialog<npc::master>(lua);
}

fb::game::npc::npc(const master* master, listener* listener) : 
    fb::game::object(master, listener),
    _listener(listener)
{
}

fb::game::npc::npc(const npc& right) :
    object(right)
{
}

fb::game::npc::~npc()
{
}

const std::string& fb::game::npc::script() const
{
    return this->_script;
}

void fb::game::npc::script(const std::string& value)
{
    this->_script = value;
}

fb::game::npc* fb::game::npc::make()
{
    return new npc(*this);
}

fb::ostream fb::game::npc::make_dialog_stream(const std::string& message, const std::vector<std::string>& menus) const
{
    return this->based<npc::master>()->make_dialog_stream(message, menus, this->_map);
}

fb::ostream fb::game::npc::make_dialog_stream(const std::string& message, const std::vector<uint8_t>& item_slots) const
{
    return this->based<npc::master>()->make_dialog_stream(message, item_slots, this->_map);
}

fb::ostream fb::game::npc::make_dialog_stream(const std::string& message, const std::vector<item::master*>& cores, fb::game::map* map) const
{
    return this->based<npc::master>()->make_dialog_stream(message, cores, map);
}

fb::ostream fb::game::npc::make_input_dialog_stream(const std::string& message, fb::game::map* map) const
{
    return this->based<npc::master>()->make_input_dialog_stream(message, this->_map);
}

fb::ostream fb::game::npc::make_input_dialog_stream(const std::string& message, const std::string& top, const std::string& bottom, int maxlen, bool prev) const
{
    return this->based<npc::master>()->make_input_dialog_stream(message, top, bottom, maxlen, prev, this->_map);
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