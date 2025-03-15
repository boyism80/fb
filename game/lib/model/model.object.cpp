#include <fb/game/character.h>
#include <fb/game/context.h>
#include <fb/game/map.h>
#include <fb/game/mob.h>
#include <fb/model/model.h>
#include <fb/game/object.h>

uint8_t fb::model::object::dialog_look_type() const
{
    return this->look > 0xBFFF ? 0x02 : 0x01;
}

fb::model::enum_value::OBJECT_TYPE fb::model::object::what() const
{
    return fb::model::enum_value::OBJECT_TYPE::UNKNOWN;
}

bool fb::model::object::operator== (const fb::model::object& r) const
{
    return this == &r;
}

bool fb::model::object::operator!= (const fb::model::object& r) const
{
    return this != &r;
}

int fb::model::object::builtin_name(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::model::object>(1);
    if (object == nullptr)
        return 0;

    lua->pushstring(object->name);
    return 1;
}

int fb::model::object::builtin_look(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::model::object>(1);
    if (object == nullptr)
        return 0;

    lua->pushinteger(object->look);
    return 1;
}

int fb::model::object::builtin_color(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::model::object>(1);
    if (object == nullptr)
        return 0;

    lua->pushinteger(object->color);
    return 1;
}

int fb::model::object::builtin_dialog(lua_State* L)
{
    // Ex) obj:dialog(ch, "hello", true, true);

    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::model::object>(1);
    if (object == nullptr)
        return 0;

    auto argc = lua->argc();
    if (argc < 3)
        throw std::runtime_error("not enough parameters");

    auto ch = lua->touserdata<fb::game::character>(2);
    if (ch == nullptr)
        return 0;

    auto message     = lua->tostring(3);
    auto button_prev = lua->toboolean(4, false);
    auto button_next = lua->toboolean(5, false);

    auto listener = ch->get_listener<fb::game::character>();
    if (listener != nullptr)
        listener->on_dialog(*ch, *object, message, button_prev, button_next);

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}

int fb::model::object::builtin_list(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto obj     = lua->touserdata<fb::model::object>(1);
    if (obj == nullptr)
        return 0;

    auto ch = lua->touserdata<fb::game::character>(2);
    if (ch == nullptr)
        return 0;

    auto message       = lua->tostring(3);
    auto size          = lua->rawlen(4);
    auto button_prev   = lua->toboolean(5);
    auto custom_preset = (argc >= 6 && lua_type(L, 6) == LUA_TTABLE);
    auto preset        = fb::game::dialog::preset(*ch);
    if (custom_preset)
    {
        lua->pushstring("sex");
        if (lua_rawget(L, 6) == LUA_TNUMBER)
            preset.sex = static_cast<SEX>(lua->tointeger(-1));

        lua->pushstring("state");
        if (lua_rawget(L, 6) == LUA_TNUMBER)
            preset.state = static_cast<STATE>(lua->tointeger(-1));

        lua->pushstring("face");
        if (lua_rawget(L, 6) == LUA_TNUMBER)
            preset.face = lua->tointeger(-1);

        lua->pushstring("hair_color");
        if (lua_rawget(L, 6) == LUA_TNUMBER)
            preset.hair_color = lua->tointeger(-1);

        lua->pushstring("weapon");
        if (lua_rawget(L, 6) == LUA_TNUMBER)
            preset.weapon = lua->tointeger(-1);

        lua->pushstring("weapon_color");
        if (lua_rawget(L, 6) == LUA_TNUMBER)
            preset.weapon_color = lua->tointeger(-1);

        lua->pushstring("armor");
        if (lua_rawget(L, 6) == LUA_TNUMBER)
            preset.armor = lua->tointeger(-1);

        lua->pushstring("armor_color");
        if (lua_rawget(L, 6) == LUA_TNUMBER)
            preset.armor_color = lua->tointeger(-1);

        lua->pushstring("shield");
        if (lua_rawget(L, 6) == LUA_TNUMBER)
            preset.shield = lua->tointeger(-1);

        lua->pushstring("shield_color");
        if (lua_rawget(L, 6) == LUA_TNUMBER)
            preset.shield_color = lua->tointeger(-1);
    }

    auto menus = std::vector<std::string>();
    for (int i = 0; i < size; i++)
    {
        lua->rawgeti(4, i + 1);
        menus.push_back(lua->tostring(-1));
    }

    auto listener = ch->get_listener<fb::game::character>();
    if (listener != nullptr)
    {
        if (custom_preset)
            listener->on_dialog(*ch, *obj, message, menus, button_prev, preset);
        else
            listener->on_dialog(*ch, *obj, message, menus, button_prev);
    }

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}

int fb::model::object::builtin_input(lua_State* L)
{
    // Ex) obj::input(ch, "message")
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto obj     = lua->touserdata<fb::model::object>(1);
    if (obj == nullptr)
        return 0;

    auto ch = lua->touserdata<fb::game::character>(2);
    if (ch == nullptr)
        return 0;

    auto message = lua->tostring(3);

    auto argc     = lua->argc();
    auto listener = ch->get_listener<fb::game::character>();
    if (argc > 3)
    {
        auto message_top = lua->tostring(4);
        auto message_bot = lua->tostring(5);
        auto maxlen      = (uint8_t)lua->tointeger(6, 0xFF);
        auto prev        = lua->toboolean(7, false);

        if (listener != nullptr)
            listener->on_dialog(*ch, *obj, message, message_top, message_bot, maxlen, prev);
    }
    else
    {
        if (listener != nullptr)
            listener->on_dialog(*ch, *obj, message);
    }

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}

int fb::model::object::builtin_menu(lua_State* L)
{
    // Ex) obj::menu(ch, "hello", {"hello 1", "hello 2", "hello 3"})
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto obj     = lua->touserdata<fb::model::object>(1);
    if (obj == nullptr)
        return 0;

    auto ch = lua->touserdata<fb::game::character>(2);
    if (ch == nullptr)
        return 0;

    auto message = lua->tostring(3);

    // Read menu list
    auto size  = lua->rawlen(4);
    auto menus = std::vector<std::string>();
    for (int i = 0; i < size; i++)
    {
        lua->rawgeti(4, i + 1);
        menus.push_back(lua->tostring(-1));
    }

    auto listener = ch->get_listener<fb::game::character>();
    if (listener != nullptr)
        listener->on_dialog(*ch, *obj, message, menus);

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}

int fb::model::object::builtin_item(lua_State* L)
{
    // Ex) obj::menu(ch, "hello", {item1, item2, item3})
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto obj     = lua->touserdata<fb::model::object>(1);
    if (obj == nullptr)
        return 0;

    auto ch = lua->touserdata<fb::game::character>(2);
    if (ch == nullptr)
        return 0;

    auto message = lua->tostring(3);

    auto items = fb::game::dialog::item_pairs();
    lua->pushnil();
    while (lua->next(4))
    {
        // auto i = lua->tointeger(-2);
        auto item  = static_cast<fb::model::item*>(nullptr);
        auto price = uint32_t(0);

        { // get 1st field
            lua->pushinteger(1);
            lua_gettable(L, -2);
            switch (lua_type(L, -1))
            {
            case LUA_TSTRING:
                item = context->model.item.name2item(lua->tostring(-1));
                break;

            case LUA_TUSERDATA:
                item = lua->touserdata<fb::model::item>(-1);
                break;
            }
            lua->pop(1);
        }

        { // get 2nd field
            lua->pushinteger(2);
            lua_gettable(L, -2);
            switch (lua_type(L, -1))
            {
            case LUA_TNUMBER:
                price = lua->tointeger(-1);
                break;
            }
        }
        lua->pop(1);

        if (item == nullptr)
            continue;

        items.push_back({*item, price});
        lua->pop(1);
    }

    auto listener = ch->get_listener<fb::game::character>();
    if (listener != nullptr)
        listener->on_dialog(*ch, *obj, message, items);

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}

int fb::model::object::builtin_slot(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto obj     = lua->touserdata<fb::model::object>(1);
    if (obj == nullptr)
        return 0;

    auto ch = lua->touserdata<fb::game::character>(2);
    if (ch == nullptr)
        return 0;

    auto message = lua->tostring(3);
    auto slots   = std::vector<uint8_t>();
    auto size    = lua_rawlen(L, 4);
    for (int i = 0; i < size; i++)
    {
        lua->pushinteger(i + 1);
        lua_gettable(L, 4);
        slots.push_back(lua->tointeger(-1));
        lua->pop(1);
    }

    auto listener = ch->get_listener<fb::game::character>();
    if (listener != nullptr)
        listener->on_dialog(*ch, *obj, message, slots);

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}