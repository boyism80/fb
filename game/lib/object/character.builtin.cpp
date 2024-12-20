#include <character.h>
#include <context.h>

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::character, "fb.game.character")
{"__eq",                fb::game::object::builtin_eq},
{"look",                fb::game::character::builtin_look},
{"color",               fb::game::character::builtin_color},
{"money",               fb::game::character::builtin_money},
{"exp",                 fb::game::character::builtin_exp},
{"base_hp",             fb::game::character::builtin_base_hp},
{"base_mp",             fb::game::character::builtin_base_mp},
{"str",                 fb::game::character::builtin_strength},
{"dex",                 fb::game::character::builtin_dexterity},
{"int",                 fb::game::character::builtin_intelligence},
{"item",                fb::game::character::builtin_item},
{"items",               fb::game::character::builtin_items},
{"dropitem",            fb::game::character::builtin_item_drop},
{"mkitem",              fb::game::character::builtin_mkitem},
{"rmitem",              fb::game::character::builtin_rmitem},
{"state",               fb::game::character::builtin_state},
{"disguise",            fb::game::character::builtin_disguise},
{"class",               fb::game::character::builtin_class},
{"level",               fb::game::character::builtin_level},
{"assert",              fb::game::character::builtin_assert},
{"deposited_money",     fb::game::character::builtin_deposited_money},
{"deposited_item",      fb::game::character::builtin_deposited_item},
{"deposit_item",        fb::game::character::builtin_deposit_item},
{"withdraw_item",       fb::game::character::builtin_withdraw_item},
{"group",               fb::game::character::builtin_group},
{"traces",              fb::game::character::builtin_traces},
{"trace",               fb::game::character::builtin_trace},
{"push_trace",          fb::game::character::builtin_push_trace},
{"erase_trace",         fb::game::character::builtin_erase_trace},
END_LUA_EXTENSION; // clang-format on

int fb::game::character::builtin_look(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        thread->pushinteger(ch->look());
        return 1;
    }
    else
    {
        auto value = (uint16_t)thread->tointeger(2);
        ch->look(value);
        return 0;
    }
}

int fb::game::character::builtin_color(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        thread->pushinteger(ch->color());
        return 1;
    }
    else
    {
        auto value = (uint8_t)thread->tointeger(2);
        ch->color(value);
        return 0;
    }
}

int fb::game::character::builtin_money(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        thread->pushinteger(ch->money());
        return 1;
    }
    else
    {
        auto value = (uint32_t)thread->tointeger(2);
        ch->money(value);
        return 0;
    }
}

int fb::game::character::builtin_exp(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        thread->pushinteger(ch->exp());
        return 1;
    }
    else
    {
        auto value = (uint32_t)thread->tointeger(2);
        ch->exp(value);
        return 0;
    }
}

int fb::game::character::builtin_base_hp(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        thread->pushinteger(ch->base_hp());
        return 1;
    }
    else
    {
        auto value = (uint32_t)thread->tointeger(2);
        ch->base_hp(value);
        return 0;
    }
}

int fb::game::character::builtin_base_mp(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        thread->pushinteger(ch->base_mp());
        return 1;
    }
    else
    {
        auto value = (uint32_t)thread->tointeger(2);
        ch->base_mp(value);
        return 0;
    }
}

int fb::game::character::builtin_strength(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        thread->pushinteger(ch->strength());
        return 1;
    }
    else
    {
        auto value = (uint8_t)thread->tointeger(2);
        ch->strength(value);
        return 0;
    }
}

int fb::game::character::builtin_dexterity(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        thread->pushinteger(ch->dexteritry());
        return 1;
    }
    else
    {
        auto value = (uint8_t)thread->tointeger(2);
        ch->dexteritry(value);
        return 0;
    }
}

int fb::game::character::builtin_intelligence(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        thread->pushinteger(ch->intelligence());
        return 1;
    }
    else
    {
        auto value = (uint8_t)thread->tointeger(2);
        ch->intelligence(value);
        return 0;
    }
}

int fb::game::character::builtin_item(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;
    auto item = (fb::game::item*)nullptr;
    if (lua_isnumber(lua, 2))
    {
        auto index = (uint8_t)thread->tointeger(2);
        item       = ch->items[index];
    }
    else if (lua_isstring(lua, 2))
    {
        auto name = thread->tostring(2);
        item      = ch->items.find(name);
    }

    if (item == nullptr)
        thread->pushnil();
    else
        thread->pushobject(item);

    return 1;
}

int fb::game::character::builtin_items(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    thread->new_table();
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        if (ch->items[i] == nullptr)
            continue;

        thread->pushinteger(i + 1);
        thread->pushobject(ch->items[i]);
        lua_settable(lua, -3);
    }

    return 1;
}

int fb::game::character::builtin_item_drop(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;
    auto index    = (uint8_t)thread->tointeger(2);
    auto drop_all = thread->toboolean(3);

    auto dropped = ch->items.drop(index - 1, drop_all ? 1 : -1);
    if (dropped != nullptr)
        thread->pushobject(dropped);
    else
        thread->pushnil();

    return 1;
}

int fb::game::character::builtin_mkitem(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;
    auto name  = thread->tostring(2);
    auto count = argc < 3 ? 1 : thread->tointeger(3);
    auto store = argc < 4 ? true : thread->toboolean(4);

    if (store == false)
        return object::builtin_mkitem(lua);

    auto model = context->model.item.name2item(name);
    if (model == nullptr)
    {
        thread->pushnil();
    }
    else
    {
        auto context = thread->env<fb::game::context>("context");
        auto item    = model->make(*context, count);
        auto slot    = ch->items.add(item);
        if (slot == 0xFF)
        {
            thread->pushnil();
            return 1;
        }

        thread->pushobject(ch->items[slot]);
    }

    return 1;
}

int fb::game::character::builtin_rmitem(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    try
    {
        auto argc = thread->argc();
        auto ch   = thread->touserdata<fb::game::character>(1);
        if (ch == nullptr)
            return 0;
        auto index       = uint8_t(0);
        auto count       = argc < 3 ? 1 : (int)thread->tointeger(3);
        auto delete_attr = argc < 4 ? ITEM_DELETE_TYPE::REMOVED : (ITEM_DELETE_TYPE)thread->tointeger(4);

        if (lua_isuserdata(lua, 2))
        {
            auto item = thread->touserdata<fb::game::item>(2);
            if (item == nullptr)
                return 0;
            if (item == nullptr)
                throw std::exception();

            index = ch->items.index(item->based<fb::model::item>());
        }
        else if (lua_isnumber(lua, 2))
        {
            index = (uint8_t)thread->tointeger(2) - 1;
        }
        else if (lua_isstring(lua, 2))
        {
            auto name = thread->tostring(2);
            if (name.empty())
                throw std::exception();

            auto model = context->model.item.name2item(name);
            if (model == nullptr)
                throw std::exception();

            index = ch->items.index(*model);
        }
        else
        {
            throw std::exception();
        }

        auto dropped = ch->items.remove(index, count, delete_attr);
        if (dropped != nullptr)
            std::ignore = dropped->destroy();
    }
    catch (...)
    { }

    return 0;
}

int fb::game::character::builtin_state(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;
    if (argc == 1)
    {
        thread->pushinteger(ch->state());
        return 1;
    }
    else
    {
        auto value = STATE(thread->tointeger(2));
        ch->state(value);
        return 0;
    }
}

int fb::game::character::builtin_disguise(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        thread->pushinteger(ch->disguise().value());
        return 1;
    }
    else if (luaL_checkinteger(lua, 2))
    {
        ch->disguise((uint16_t)thread->tointeger(2));
        return 0;
    }
    else
    {
        ch->undisguise();
        return 0;
    }
}

int fb::game::character::builtin_class(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto cls       = ch->_class;
        auto promotion = ch->_promotion;

        auto promo_model = context->model.promotion(cls, promotion);
        if (promo_model == nullptr)
        {
            thread->pushnil();
        }
        else
        {
            thread->pushstring(promo_model->name);
        }
    }
    else
    {
        auto cls_name  = thread->tostring(2);
        auto cls       = CLASS::NONE;
        auto promotion = uint8_t(0);
        if (context->model.promotion.name2class(cls_name, cls, promotion) == false)
        {
            thread->pushboolean(false);
        }
        else
        {
            auto context = thread->env<fb::game::context>("context");

            context->send(*ch, fb::protocol::game::response::character::id(*ch), context::scope::SELF);
            context->send(*ch,
                          fb::protocol::game::response::character::state(*ch, STATE_LEVEL::LEVEL_MAX),
                          context::scope::SELF);
            thread->pushboolean(true);
        }
    }

    return 1;
}

int fb::game::character::builtin_level(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        thread->pushinteger(ch->level());
        return 1;
    }
    else
    {
        auto level = std::max(0, std::min((int)thread->tointeger(2), 255));
        ch->level(level);

        auto context = thread->env<fb::game::context>("context");
        context->send(*ch,
                      fb::protocol::game::response::character::state(*ch, STATE_LEVEL::LEVEL_MAX),
                      context::scope::SELF);
        return 0;
    }
}

int fb::game::character::builtin_assert(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto               argc = thread->argc();
    auto               ch   = thread->touserdata<fb::game::character>(1);
    auto               size = thread->rawlen(2);
    std::vector<STATE> values;
    for (int i = 0; i < size; i++)
    {
        thread->rawgeti(2, i + 1);
        values.push_back((STATE)thread->tointeger(-1));
    }

    try
    {
        ch->assert_state(values);
        return 0;
    }
    catch (std::runtime_error& e)
    {
        thread->pushstring(e.what());
        return 1;
    }
}

int fb::game::character::builtin_admin(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        thread->pushboolean(ch->admin());
        return 1;
    }
    else
    {
        ch->admin(thread->toboolean(2));
        return 0;
    }
}

int fb::game::character::builtin_deposited_money(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        thread->pushinteger(ch->deposited_money());
        return 1;
    }
    else
    {
        ch->deposited_money(thread->tointeger(2));
        return 0;
    }
}

int fb::game::character::builtin_deposited_item(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    const auto& deposited_items = ch->deposited_items();
    if (argc == 1)
    {
        thread->new_table();
        for (int i = 0; i < deposited_items.size(); i++)
        {
            thread->pushinteger(i + 1);
            thread->pushobject(deposited_items.at(i));
            lua_settable(lua, -3);
        }
    }
    else if (argc >= 2)
    {
        try
        {
            if (thread->is_num(2))
            {
                auto index = thread->tointeger(2);
                if (index > deposited_items.size() - 1)
                    throw std::exception();

                auto deposited_item = deposited_items.at(index);
                thread->pushobject(deposited_item);
            }
            else if (thread->is_str(2))
            {
                auto name  = thread->tostring(2);
                auto found = std::find_if(deposited_items.cbegin(),
                                          deposited_items.cend(),
                                          [&name](fb::game::item* deposited_item) {
                                              return deposited_item->based<fb::model::item>().name == name;
                                          });

                if (found == deposited_items.cend())
                    throw std::exception();

                thread->pushobject(*found);
            }
            else if (thread->is_obj(2))
            {
                auto model = thread->touserdata<fb::model::item>(2);
                auto found = std::find_if(deposited_items.cbegin(),
                                          deposited_items.cend(),
                                          [model](fb::game::item* deposited_item) {
                                              return deposited_item->based<fb::model::item>() == *model;
                                          });

                if (found == deposited_items.cend())
                    throw std::exception();

                thread->pushobject(*found);
            }
            else
            {
                throw std::exception();
            }
        }
        catch (std::exception&)
        {
            thread->pushnil();
        }
    }
    else
    {
        thread->pushnil();
    }

    return 1;
}

int fb::game::character::builtin_deposit_item(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto item  = thread->touserdata<fb::game::item>(2);
    auto count = argc >= 3 ? thread->tointeger(3) : 1;
    auto index = ch->items.index(*item);
    if (index == 0xFF)
    {
        thread->pushboolean(false);
    }
    else
    {
        thread->pushboolean(ch->deposit_item(index, count));
    }

    return 1;
}

int fb::game::character::builtin_withdraw_item(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto  item            = thread->touserdata<fb::game::item>(2);
    auto  count           = argc >= 3 ? thread->tointeger(3) : 1;
    auto& deposited_items = ch->deposited_items();
    auto  found           = std::find(deposited_items.cbegin(), deposited_items.cend(), item);
    if (found == deposited_items.cend())
    {
        thread->pushnil();
    }
    else
    {
        auto index    = std::distance(deposited_items.cbegin(), found);
        auto returned = ch->withdraw_item(index, count);
        thread->pushobject(returned);
    }

    return 1;
}

int fb::game::character::builtin_group(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    static auto func = [](fb::lua::context* ctx) {
        lua_call(*ctx, 2, LUA_MULTRET);

        ctx->remove(-ctx->argc());
        return ctx->argc();
    };

    thread->pushobject(ch);
    if (ch->_group == nullptr)
    {
        thread->pushnil();
        return func(thread);
    }
    else
    {
        return ch->_group->template lock<uint32_t>([=](auto& group) {
            thread->pushobject(group);
            return func(thread);
        });
    }
}

int fb::game::character::builtin_traces(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    thread->new_table();
    int i = 0;
    for (auto& [id, trace] : ch->traces)
    {
        thread->pushinteger(i + 1);
        thread->pushobject(*trace);
        lua_settable(lua, -3);

        i++;
    }
    return 1;
}

int fb::game::character::builtin_trace(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto i = thread->tointeger(2);
    if (ch->traces.contains(i) == false)
    {
        thread->pushnil();
    }
    else
    {
        thread->pushobject(*ch->traces.at(i));
    }
    return 1;
}

int fb::game::character::builtin_push_trace(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    const fb::model::trace* model = nullptr;

    if (lua_isinteger(lua, 2))
    {
        auto id = thread->tointeger(2);
        if (context->model.trace.contains(id) == false)
        {
            thread->pushnil();
            return 1;
        }

        model = &context->model.trace[id];
    }
    else if (lua_isuserdata(lua, 2))
    {
        model = (const fb::model::trace*)thread->touserdata<fb::game::trace>(2);
    }

    if (ch->traces.contains(model->id))
    {
        thread->pushnil();
        return 1;
    }

    auto text = thread->argc() < 3 ? std::optional<std::string>{std::nullopt} : thread->tostring(3);
    ch->traces.insert({model->id, std::make_unique<fb::game::trace>(*model, text)});
    thread->pushobject(*ch->traces[model->id]);
    return 1;
}

int fb::game::character::builtin_erase_trace(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto ch      = thread->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto i = thread->tointeger(2);
    if (ch->traces.contains(i) == false)
    {
        thread->pushboolean(false);
    }
    else
    {
        ch->traces.erase(i);
        thread->pushboolean(true);
    }
    return 1;
}
