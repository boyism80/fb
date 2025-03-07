#include <fb/model/model.h>
#include <fb/game/context.h>

int fb::model::npc::builtin_input(lua_State* L)
{
    // Ex) npc::input(ch, "message")
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto npc     = lua->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
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
        auto maxlen      = argc < 6 ? 0xFF : (int)lua->tointeger(6);
        auto prev        = argc < 7 ? false : lua->toboolean(7);

        if (listener != nullptr)
            listener->on_dialog(*ch, *npc, message, message_top, message_bot, maxlen, prev);
    }
    else
    {
        if (listener != nullptr)
            listener->on_dialog(*ch, *npc, message);
    }

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}

int fb::model::npc::builtin_menu(lua_State* L)
{
    // Ex) npc::menu(ch, "hello", {"hello 1", "hello 2", "hello 3"})
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto npc     = lua->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
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
        listener->on_dialog(*ch, *npc, message, menus);

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}

int fb::model::npc::builtin_list(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto npc     = lua->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
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
            listener->on_dialog(*ch, *npc, message, menus, button_prev, preset);
        else
            listener->on_dialog(*ch, *npc, message, menus, button_prev);
    }

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}

int fb::model::npc::builtin_item(lua_State* L)
{
    // Ex) npc::menu(ch, "hello", {item1, item2, item3})
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto npc     = lua->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
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
        listener->on_dialog(*ch, *npc, message, items);

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}

int fb::model::npc::builtin_slot(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto npc     = lua->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
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
        listener->on_dialog(*ch, *npc, message, slots);

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}

int fb::model::npc::builtin_sell(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto npc     = lua->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    lua->new_table();
    for (int i = 0; i < npc->sell.size(); i++)
    {
        lua->pushinteger(i + 1);
        lua->pushinteger(npc->sell[i]);
        lua_settable(L, -3);
    }
    return 1;
}

int fb::model::npc::builtin_sell_price(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto npc     = lua->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    auto name = lua->tostring(2);
    for (auto sell : npc->sell)
    {
        if (context->model.sell.contains(sell) == false)
            continue;

        for (auto& [_, x] : context->model.sell[sell])
        {
            auto& item = context->model.item[x.item];
            if (item.name != name)
                continue;

            lua->pushinteger(x.price.value_or(item.price));
            return 1;
        }
    }

    return 0;
}

int fb::model::npc::builtin_buy_price(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto npc     = lua->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    auto name = lua->tostring(2);
    if (npc->buy.has_value() == false)
        return 0;

    if (context->model.buy.contains(npc->buy.value()) == false)
        return 0;

    for (auto& [_, x] : context->model.buy[npc->buy.value()])
    {
        auto& item = context->model.item[x.item];
        if (item.name != name)
            continue;

        lua->pushinteger(x.price.value_or(item.price / 2));
        return 1;
    }

    return 0;
}

int fb::model::npc::builtin_buy(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto npc     = lua->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    if (npc->buy.has_value())
        lua->pushinteger(npc->buy.value());
    else
        lua->pushnil();

    return 1;
}

fb::model::npc* fb::model::__npc::name2npc(const std::string& name) const
{
    for (auto& npc : *this)
    {
        if (npc.second.name == name)
            return &npc.second;
    }

    return nullptr;
}