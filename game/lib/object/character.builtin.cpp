#include <fb/game/character.h>
#include <fb/game/context.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(character, "fb.game.character")
{"__eq",                fb::game::object::builtin_eq},
{"look",                character::builtin_look},
{"color",               character::builtin_color},
{"sex",                 character::builtin_sex},
{"money",               character::builtin_money},
{"exp",                 character::builtin_exp},
{"str",                 character::builtin_str},
{"dex",                 character::builtin_dex},
{"int",                 character::builtin_intelligence},
{"item",                character::builtin_item},
{"items",               character::builtin_items},
{"equipments",          character::builtin_equipments},
{"dropitem",            character::builtin_item_drop},
{"mkitem",              character::builtin_mkitem},
{"rmitem",              character::builtin_rmitem},
{"state",               character::builtin_state},
{"disguise",            character::builtin_disguise},
{"class",               character::builtin_class},
{"promotion",           character::builtin_promotion},
{"level",               character::builtin_level},
{"assert",              character::builtin_assert},
{"deposited_money",     character::builtin_deposited_money},
{"deposited_item",      character::builtin_deposited_item},
{"deposit_item",        character::builtin_deposit_item},
{"withdraw_item",       character::builtin_withdraw_item},
{"group",               character::builtin_group},
{"create_group",        character::builtin_create_group},
{"clan",                character::builtin_clan},
{"create_clan",         character::builtin_create_clan},
{"destroy_clan",        character::builtin_destroy_clan},
{"traces",              character::builtin_traces},
{"trace",               character::builtin_trace},
{"push_trace",          character::builtin_push_trace},
{"erase_trace",         character::builtin_erase_trace},
{"switch_context",      character::builtin_switch_context},
{"whisper",             character::builtin_whisper},
{"send_mail",           character::builtin_send_mail},
{"assert_state",        character::builtin_assert_state},
{"nation",              character::builtin_nation},
{"weapon",              character::builtin_weapon},
{"title",               character::builtin_title},
{"gain",                character::builtin_gain},
{"weapon_damage",       character::builtin_weapon_damage},
{"detect",              character::builtin_detect},
{"spawn_mob",           character::builtin_spawn_mob},
{"spawned_mobs",        character::builtin_spawned_mobs},
{"base_hp",             character::builtin_base_hp},
{"base_mp",             character::builtin_base_mp},
{"base_str",            character::builtin_base_str},
{"base_dex",            character::builtin_base_dex},
{"base_int",            character::builtin_base_int},
{"base_dam",            character::builtin_base_dam},
{"base_hit",            character::builtin_base_hit},
{"armor_color",         character::builtin_armor_color},
{"mkspell",             character::builtin_mkspell},
{"rmspell",             character::builtin_rmspell},
{"world",               character::builtin_world},
{"script",              character::builtin_script},
{"ad",                  character::builtin_ad},
{"web",                 character::builtin_web},
{"birthday",            character::builtin_birthday},
END_LUA_EXTENSION; // clang-format on

int character::builtin_look(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        lua->pushinteger(ch->look());
        return 1;
    }
    else
    {
        auto value = (uint16_t)lua->tointeger(2);
        ch->look(value);
        return 0;
    }
}

int character::builtin_color(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        lua->pushinteger(ch->color());
        return 1;
    }
    else
    {
        auto value = (uint8_t)lua->tointeger(2);
        ch->color(value);
        return 0;
    }
}

int character::builtin_sex(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    auto sex = static_cast<SEX>(lua->tointeger(2));
    auto n   = (argc == 1 ? 1 : 0);
    return context->builtin(*ch, lua, n, [=]() {
        if (argc == 1)
            lua->pushinteger(ch->sex());
        else
            ch->sex(sex);
    });
}

int character::builtin_money(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        lua->pushinteger(ch->money());
        return 1;
    }
    else
    {
        auto value = (uint32_t)lua->tointeger(2);
        ch->money(value);
        return 0;
    }
}

int character::builtin_exp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        lua->pushinteger(ch->exp());
        return 1;
    }
    else
    {
        auto value = (uint32_t)lua->tointeger(2);
        ch->exp(value);
        return 0;
    }
}

int character::builtin_item(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;
    auto item = (fb::game::item*)nullptr;
    if (lua->is_num(2))
    {
        auto index = (uint8_t)lua->tointeger(2);
        item       = ch->items[index];
    }
    else if (lua->is_str(2))
    {
        auto name = lua->tostring(2);
        item      = ch->items.find(name);
    }

    if (item == nullptr)
        lua->pushnil();
    else
        lua->pushobject(item);

    return 1;
}

int character::builtin_items(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    lua->new_table();
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        if (ch->items[i] == nullptr)
            continue;

        lua->pushinteger(i + 1);
        lua->pushobject(ch->items[i]);
        lua_settable(L, -3);
    }

    return 1;
}

int character::builtin_equipments(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    lua->new_table();
    for (auto& [parts, equipment] : ch->items.equipments())
    {
        if (equipment == nullptr)
            continue;

        lua->pushinteger(parts);
        lua->pushobject(equipment);
        lua_settable(L, -3);
    }

    return 1;
}

int character::builtin_item_drop(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;
    auto index    = (uint8_t)lua->tointeger(2);
    auto drop_all = lua->toboolean(3);

    auto dropped = ch->items.drop(index - 1, drop_all ? 1 : -1);
    if (dropped != nullptr)
        lua->pushobject(dropped);
    else
        lua->pushnil();

    return 1;
}

int character::builtin_mkitem(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;
    auto name  = lua->tostring(2);
    auto count = argc < 3 ? 1 : lua->tointeger(3);
    auto store = argc < 4 ? true : lua->toboolean(4);

    if (store == false)
        return object::builtin_mkitem(L);

    auto model = context->model.item.name2item(name);
    if (model == nullptr)
    {
        lua->pushnil();
    }
    else
    {
        auto context = lua->env<fb::game::context>("context");
        auto item    = model->make(*context, count);
        auto slot    = ch->items.add(item);
        if (slot == 0xFF)
        {
            lua->pushnil();
            return 1;
        }

        lua->pushobject(ch->items[slot]);
    }

    return 1;
}

int character::builtin_rmitem(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    try
    {
        auto argc = lua->argc();
        auto ch   = lua->touserdata<character>(1);
        if (ch == nullptr)
            return 0;
        auto index       = uint8_t(0);
        auto count       = argc < 3 ? 1 : (int)lua->tointeger(3);
        auto delete_attr = argc < 4 ? ITEM_DELETE_TYPE::REMOVED : (ITEM_DELETE_TYPE)lua->tointeger(4);

        if (lua->is_obj(2))
        {
            auto item = lua->touserdata<fb::game::item>(2);
            if (item == nullptr)
                return 0;
            if (item == nullptr)
                throw std::exception();

            index = ch->items.index(item->based<fb::model::item>());
        }
        else if (lua->is_num(2))
        {
            index = (uint8_t)lua->tointeger(2) - 1;
        }
        else if (lua->is_str(2))
        {
            auto name = lua->tostring(2);
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

int character::builtin_state(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;
    if (argc == 1)
    {
        lua->pushinteger(ch->state());
        return 1;
    }
    else
    {
        auto value = STATE(lua->tointeger(2));
        ch->state(value);
        return 0;
    }
}

int character::builtin_disguise(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    auto value = std::optional<uint16_t>{};
    if (argc >= 2 && lua_type(L, 2) == LUA_TNUMBER)
        value = static_cast<uint16_t>(lua->tointeger(2));

    static auto static_func = [](fb::lua::context* lua, character* ch, int argc, std::optional<uint16_t> value) {

    };

    auto n = (argc == 1 ? 1 : 0);
    return context->builtin(*ch, lua, n, [=]() {
        if (argc == 1)
            lua->pushinteger(ch->disguise().value());
        else if (value.has_value())
            ch->disguise(value.value());
        else
            ch->undisguise();
    });
}

int character::builtin_class(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        lua->pushinteger(ch->_class);
        return 1;
    }
    else
    {
        auto value = lua->tointeger(2);
        ch->cls(static_cast<CLASS>(value));
        return 0;
    }
}

int character::builtin_promotion(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        lua->pushinteger(ch->_promotion);
        return 1;
    }
    else
    {
        auto value = lua->tointeger(2);
        ch->promotion(value);
        return 0;
    }
}

int character::builtin_level(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        lua->pushinteger(ch->level());
        return 1;
    }
    else
    {
        auto level = std::max(0, std::min((int)lua->tointeger(2), 255));
        ch->level(level);

        auto context = lua->env<fb::game::context>("context");
        ch->send(fb::protocol::game::response::update_internal(*ch, STATE_LEVEL::LEVEL_MAX));
        return 0;
    }
}

int character::builtin_assert(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto               argc = lua->argc();
    auto               ch   = lua->touserdata<character>(1);
    auto               size = lua->rawlen(2);
    std::vector<STATE> values;
    for (int i = 0; i < size; i++)
    {
        lua->rawgeti(2, i + 1);
        values.push_back((STATE)lua->tointeger(-1));
    }

    try
    {
        ch->assert_state(values);
        return 0;
    }
    catch (std::runtime_error& e)
    {
        lua->pushstring(e.what());
        return 1;
    }
}

int character::builtin_admin(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        lua->pushboolean(ch->admin());
        return 1;
    }
    else
    {
        ch->admin(lua->toboolean(2));
        return 0;
    }
}

int character::builtin_deposited_money(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        lua->pushinteger(ch->deposited_money());
        return 1;
    }
    else
    {
        ch->deposited_money(lua->tointeger(2));
        return 0;
    }
}

int character::builtin_deposited_item(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    const auto& deposited_items = ch->deposited_items();
    if (argc == 1)
    {
        lua->new_table();
        for (int i = 0; i < deposited_items.size(); i++)
        {
            lua->pushinteger(i + 1);
            lua->pushobject(deposited_items.at(i));
            lua_settable(L, -3);
        }
    }
    else if (argc >= 2)
    {
        try
        {
            if (lua->is_num(2))
            {
                auto index = lua->tointeger(2);
                if (index > deposited_items.size() - 1)
                    throw std::exception();

                auto deposited_item = deposited_items.at(index);
                lua->pushobject(deposited_item);
            }
            else if (lua->is_str(2))
            {
                auto name  = lua->tostring(2);
                auto found = std::find_if(deposited_items.cbegin(),
                                          deposited_items.cend(),
                                          [&name](fb::game::item* deposited_item) {
                                              return deposited_item->based<fb::model::item>().name == name;
                                          });

                if (found == deposited_items.cend())
                    throw std::exception();

                lua->pushobject(*found);
            }
            else if (lua->is_obj(2))
            {
                auto model = lua->touserdata<fb::model::item>(2);
                auto found = std::find_if(deposited_items.cbegin(),
                                          deposited_items.cend(),
                                          [model](fb::game::item* deposited_item) {
                                              return deposited_item->based<fb::model::item>() == *model;
                                          });

                if (found == deposited_items.cend())
                    throw std::exception();

                lua->pushobject(*found);
            }
            else
            {
                throw std::exception();
            }
        }
        catch (std::exception&)
        {
            lua->pushnil();
        }
    }
    else
    {
        lua->pushnil();
    }

    return 1;
}

int character::builtin_deposit_item(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    auto item  = lua->touserdata<fb::game::item>(2);
    auto count = argc >= 3 ? lua->tointeger(3) : 1;
    auto index = ch->items.index(*item);
    if (index == 0xFF)
    {
        lua->pushboolean(false);
    }
    else
    {
        lua->pushboolean(ch->deposit_item(index, count));
    }

    return 1;
}

int character::builtin_withdraw_item(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    auto  item            = lua->touserdata<fb::game::item>(2);
    auto  count           = argc >= 3 ? lua->tointeger(3) : 1;
    auto& deposited_items = ch->deposited_items();
    auto  found           = std::find(deposited_items.cbegin(), deposited_items.cend(), item);
    if (found == deposited_items.cend())
    {
        lua->pushnil();
    }
    else
    {
        auto index    = std::distance(deposited_items.cbegin(), found);
        auto returned = ch->withdraw_item(index, count);
        lua->pushobject(returned);
    }

    return 1;
}

int character::builtin_group(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    if (argc < 2)
    {
        // 그룹 Lock scope는 스크립트 내의 다음 yield를
        // 만나기 전까지 유효
        context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
            if (ch->_group == nullptr)
            {
                lua->pushnil();
                lua->resume(1);
            }
            else
            {
                ch->_group->lock([=](auto& group) {
                    lua->pushobject(group);
                    lua->resume(1);
                });
            }
            co_return;
        });
        return lua->yield(1);
    }
    else if (lua_type(L, 2) == LUA_TFUNCTION)
    {
        static auto static_func = [](fb::lua::context* ctx) {
            lua_call(*ctx, 2, LUA_MULTRET);

            ctx->remove(-ctx->argc());
            return ctx->argc();
        };

        lua->pushobject(ch);
        if (ch->_group == nullptr)
        {
            lua->pushnil();
            return static_func(lua);
        }
        else
        {
            return ch->_group->template lock<uint32_t>([=](auto& group) {
                lua->pushobject(group);
                return static_func(lua);
            });
        }
    }
    else
    {
        return 0;
    }
}

int character::builtin_create_group(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    auto name = lua->tostring(2);

    static auto fn = [](fb::game::context* context,
                        fb::lua::context*  lua,
                        character&         me,
                        const std::string& name) -> async::task<void> {
        lua->pushboolean(co_await context->create_group(me, name));
        lua->resume(1);
    };

    context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
        co_await fn(context, lua, *ch, name);
    });

    return lua->yield(1);
}

int character::builtin_clan(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    if (argc < 2)
    {
        // 그룹 Lock scope는 스크립트 내의 다음 yield를
        // 만나기 전까지 유효
        context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
            if (ch->_clan == nullptr)
            {
                lua->pushnil();
                lua->resume(1);
            }
            else
            {
                ch->_clan->lock([=](auto& clan) {
                    lua->pushobject(clan);
                    lua->resume(1);
                });
            }
            co_return;
        });
        return lua->yield(1);
    }
    else if (lua_type(L, 2) == LUA_TFUNCTION)
    {
        static auto static_func = [](fb::lua::context* ctx) {
            lua_call(*ctx, 2, LUA_MULTRET);

            ctx->remove(-ctx->argc());
            return ctx->argc();
        };

        lua->pushobject(ch);
        if (ch->_clan == nullptr)
        {
            lua->pushnil();
            return static_func(lua);
        }
        else
        {
            return ch->_clan->template lock<uint32_t>([=](auto& clan) {
                lua->pushobject(clan);
                return static_func(lua);
            });
        }
    }
    else
    {
        return 0;
    }
}

int character::builtin_create_clan(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    auto name = lua->tostring(2);

    static auto fn = [](fb::game::context* context,
                        fb::lua::context*  lua,
                        character&         me,
                        const std::string& name) -> async::task<void> {
        try
        {
            co_await context->create_clan(me, name);
            lua->pushnil();
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
        }
        lua->resume(1);
    };

    context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
        co_await fn(context, lua, *ch, name);
    });

    return lua->yield(1);
}

int character::builtin_destroy_clan(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    static auto fn = [](fb::game::context* context, fb::lua::context* lua, character& me) -> async::task<void> {
        try
        {
            co_await context->destroy_clan(me);
            lua->pushnil();
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
        }
        lua->resume(1);
    };

    context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
        co_await fn(context, lua, *ch);
    });

    return lua->yield(1);
}

int character::builtin_traces(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    lua->new_table();
    int i = 0;
    for (auto& [id, trace] : ch->traces)
    {
        lua->pushinteger(i + 1);
        lua->pushobject(*trace);
        lua_settable(L, -3);

        i++;
    }
    return 1;
}

int character::builtin_trace(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    auto i = lua->tointeger(2);
    if (ch->traces.contains(i) == false)
    {
        lua->pushnil();
    }
    else
    {
        lua->pushobject(*ch->traces.at(i));
    }
    return 1;
}

int character::builtin_push_trace(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    const fb::model::trace* model = nullptr;

    if (lua->is_num(2))
    {
        auto id = lua->tointeger(2);
        if (context->model.trace.contains(id) == false)
        {
            lua->pushnil();
            return 1;
        }

        model = &context->model.trace[id];
    }
    else if (lua->is_obj(2))
    {
        model = (const fb::model::trace*)lua->touserdata<fb::game::trace>(2);
    }

    if (ch->traces.contains(model->id))
    {
        lua->pushnil();
        return 1;
    }

    auto text = lua->argc() < 3 ? std::optional<std::string>{std::nullopt} : lua->tostring(3);
    ch->traces.insert({model->id, std::make_unique<fb::game::trace>(*model, text)});
    lua->pushobject(*ch->traces[model->id]);
    return 1;
}

int character::builtin_erase_trace(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    auto i = lua->tointeger(2);
    if (ch->traces.contains(i) == false)
    {
        lua->pushboolean(false);
    }
    else
    {
        ch->traces.erase(i);
        lua->pushboolean(true);
    }
    return 1;
}

int character::builtin_switch_context(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto me      = lua->touserdata<character>(1);
    if (me == nullptr)
        return 0;

    auto you = lua->touserdata<character>(2);
    if (you == nullptr)
    {
        lua->pushboolean(false);
        return 1;
    }

    lua->pushboolean(me->dialog.switch_context(you->dialog));
    return 1;
}

int character::builtin_whisper(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto me      = lua->touserdata<character>(1);
    if (me == nullptr)
        return 0;

    auto to      = lua->tostring(2);
    auto message = lua->tostring(3);

    static auto fn = [](fb::game::context*   context,
                        fb::lua::context*    lua,
                        fb::game::character* ch,
                        const std::string&   to,
                        const std::string&   message) -> async::task<void> {
        try
        {
            co_await context->whisper(*ch, to, message);
            lua->pushnil();
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
        }

        lua->resume(1);
    };

    context->threads.enqueue(*me, [=](auto&) -> async::task<void> {
        co_await fn(context, lua, me, to, message);
    });

    return lua->yield(1);
}

int character::builtin_assert_state(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto me = lua->touserdata<character>(1);
    if (me == nullptr)
        return 0;

    auto argc   = lua->argc();
    auto values = std::vector<STATE>{};
    for (int i = 1; i < argc; i++)
    {
        values.push_back(static_cast<STATE>(lua->tointeger(i + 1)));
    }

    try
    {
        me->assert_state(values);
        lua->pushnil();
        return 1;
    }
    catch (std::exception& e)
    {
        lua->pushstring(e.what());
        return 1;
    }
}

int character::builtin_nation(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    auto value = static_cast<NATION>(lua->tointeger(2));
    auto n     = (argc == 1 ? 1 : 0);
    return context->builtin(*ch, lua, n, [=]() {
        if (argc == 1)
            lua->pushinteger(static_cast<uint8_t>(ch->nation()));
        else
            ch->nation(value);
    });
}

int character::builtin_weapon(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    auto weapon = lua->touserdata<fb::game::weapon>(2);
    auto n      = (argc == 1 ? 1 : 0);
    return context->builtin(*ch, lua, n, [=]() mutable {
        if (argc == 1)
        {
            weapon = ch->items.weapon();
            if (weapon == nullptr)
                lua->pushnil();
            else
                lua->pushobject(weapon);
        }
        else
        {
            ch->items.weapon(weapon);
        }
    });
}

int character::builtin_title(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    auto n     = (argc == 1 ? 1 : 0);
    auto value = lua->tostring(2);
    return context->builtin(*ch, lua, n, [=]() {
        if (argc == 1)
            lua->pushstring(ch->title());
        else
            ch->title(value);
    });
}

int character::builtin_gain(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    auto items = std::vector<fb::game::item*>{};
    for (int i = 2; i <= argc; i++)
    {
        auto item = lua->touserdata<fb::game::item>(i);
        items.push_back(item);
    }

    return context->builtin(*ch, lua, 0, [=]() {
        ch->items.add(items, true);
    });
}

int character::builtin_weapon_damage(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    auto n     = (argc == 1 ? 1 : 0);
    auto value = lua->tointeger(2);

    return context->builtin(*ch, lua, n, [=]() {
        if (argc == 1)
            lua->pushinteger(ch->weapon_damage());
        else
            ch->weapon_damage(value);
    });
}

int character::builtin_detect(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    auto value = lua->toboolean(2);
    auto n     = (argc == 1 ? 1 : 0);
    return context->builtin(*ch, lua, n, [=]() {
        if (argc == 1)
            lua->pushboolean(ch->detect());
        else
            ch->detect(value);
    });
}

int character::builtin_spawn_mob(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    auto map = ch->map();
    if (map == nullptr)
    {
        lua->pushnil();
        return 1;
    }

    auto name  = lua->tostring(2);
    auto owned = true;
    auto model = context->model.mob.name2mob(name);
    if (model == nullptr)
    {
        lua->pushnil();
        return 1;
    }

    uint16_t x, y;
    if (argc < 3)
    {
        x = ch->x();
        y = ch->y();
    }
    else if (lua->is_table(3))
    {
        if (argc >= 4)
            owned = lua->toboolean(4);

        lua->rawgeti(3, 1);
        x = (uint16_t)lua->tointeger(-1);
        lua->remove(-1);

        lua->rawgeti(3, 2);
        y = (uint16_t)lua->tointeger(-1);
        lua->remove(-1);
    }
    else
    {
        if (argc >= 5)
            owned = lua->toboolean(5);

        x = (uint16_t)lua->tointeger(3);
        y = (uint16_t)lua->tointeger(4);
    }

    auto mob = ch->spawn_mob(*model, fb::model::point16_t(x, y), owned);
    if (mob == nullptr)
        lua->pushnil();
    else
        lua->pushobject(mob);
    return 1;
}

int character::builtin_spawned_mobs(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto ch      = lua->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    lua->new_table();
    auto i = 0;
    for (auto mob : ch->spawned_mobs())
    {
        lua->pushinteger(i + 1);
        lua->pushobject(mob);
        lua_settable(L, -3);

        i++;
    }

    return 1;
}

int character::builtin_base_hp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        lua->pushinteger(ch->base_hp());
        return 1;
    }
    else
    {
        ch->base_hp(lua->tointeger(2));
        return 0;
    }
}

int character::builtin_base_mp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        lua->pushinteger(ch->base_mp());
        return 1;
    }
    else
    {
        ch->base_mp(lua->tointeger(2));
        return 0;
    }
}

int character::builtin_base_str(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        lua->pushinteger(ch->base_str());
        return 1;
    }
    else
    {
        ch->base_str(lua->tointeger(2));
        return 0;
    }
}

int character::builtin_base_dex(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        lua->pushinteger(ch->base_dex());
        return 1;
    }
    else
    {
        ch->base_dex(lua->tointeger(2));
        return 0;
    }
}

int character::builtin_base_int(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        lua->pushinteger(ch->base_int());
        return 1;
    }
    else
    {
        ch->base_int(lua->tointeger(2));
        return 0;
    }
}

int character::builtin_base_dam(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        lua->pushinteger(ch->base_dam());
        return 1;
    }
    else
    {
        ch->base_dam(lua->tointeger(2));
        return 0;
    }
}

int character::builtin_base_hit(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        lua->pushinteger(ch->base_hit());
        return 1;
    }
    else
    {
        ch->base_hit(lua->tointeger(2));
        return 0;
    }
}

int character::builtin_armor_color(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        if (ch->_armor_color.has_value())
            lua->pushinteger(ch->_armor_color.value());
        else
            lua->pushnil();
        return 1;
    }
    else
    {
        switch (lua_type(L, 2))
        {
        case LUA_TNIL:
            ch->armor_color(std::nullopt);
            break;

        default:
            ch->armor_color(lua->tointeger(2));
            break;
        }
        return 0;
    }
}

int character::builtin_mkspell(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto name  = lua->tostring(2);
    auto model = context->model.spell.name2spell(name);
    if (model == nullptr)
        return 0;

    auto slot = ch->spells.add(*model);
    if (slot == 0xFF)
        return 0;

    lua->pushinteger(slot);
    return 1;
}

int character::builtin_rmspell(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        for (int i = 0; i < CONTAINER_CAPACITY; i++)
        {
            auto spell = ch->spells[i];
            if (spell == nullptr)
                continue;

            ch->spells.remove(i);
        }
    }
    else
    {
        auto slot = lua->tointeger(2);
        ch->spells.remove(slot);
    }

    return 0;
}

int character::builtin_world(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto name = lua->tostring(2);
    for (auto& [id, world] : context->model.world)
    {
        for (auto& [index, point] : world)
        {
            if (point.name == name)
            {
                ch->show_world_map(id, index);
                lua->pushboolean(true);
                return 1;
            }
        }
    }

    lua->pushboolean(false);
    return 1;
}

int character::builtin_script(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto name = std::string{"func"};
    if (argc >= 2 && lua_type(L, 2) == LUA_TSTRING)
        name = lua->tostring(2);

    auto new_lua = ch->dialog.new_context();
    new_lua->load("scripts/script.lua");
    new_lua->func(name);
    new_lua->pushobject(ch);
    async::awaitable_then(new_lua->call(1), [](auto result) {
        try
        {
            result();
        }
        catch (std::exception& e)
        {
            fb::logger::fatal(e.what());
        }
    });
    return 0;
}

int character::builtin_ad(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto width = 300;
    if (argc >= 2 && lua->is_nil(2) == false)
        width = lua->tointeger(2);

    auto height = 120;
    if (argc >= 3 && lua->is_nil(3) == false)
        height = lua->tointeger(3);

    auto url = std::string{"http://www.google.com"};
    if (argc >= 4 && lua->is_nil(4) == false)
        url = lua->tostring(4);

    auto time = 60;
    if (argc >= 5 && lua->is_nil(5) == false)
        time = lua->tointeger(5);

    ch->send(fb::protocol::game::response::ad(width, height, url, time));
    return 0;
}

int character::builtin_web(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto type = 0;
    if (argc >= 2 && lua->is_nil(2) == false)
        type = lua->tointeger(2);

    auto url = std::string{"http://www.google.com"};
    if (argc >= 3 && lua->is_nil(3) == false)
        url = lua->tostring(3);

    auto message = std::string{"default message"};
    if (argc >= 4 && lua->is_nil(4) == false)
        message = lua->tostring(4);

    ch->send(fb::protocol::game::response::web(type, url, message));
    return 0;
}

int character::builtin_delay(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto model = (fb::model::spell*)nullptr;
    if (lua->is_obj(2))
        model = lua->touserdata<fb::model::spell>(2);
    else if (lua->is_str(2))
        model = context->model.spell.name2spell(lua->tostring(2));

    if (model == nullptr)
        return 0;

    auto value = lua->tointeger(3);
    auto spell = ch->spells.find(*model);
    if (spell == nullptr)
        return 0;

    spell->delay(value);
    return 0;
}

int character::builtin_birthday(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto n     = (argc == 1 ? 1 : 0);
    auto value = lua_type(L, 2) == LUA_TNIL ? std::nullopt : std::optional<std::uint32_t>{lua->tointeger(2)};
    return context->builtin(*ch, lua, n, [=]() {
        if (argc == 1)
        {
            auto& birthday = ch->birthday();
            if (birthday.has_value())
                lua->pushinteger(birthday.value());
            else
                lua->pushnil();
        }
        else
        {
            ch->birthday(value);
        }
    });
}

int character::builtin_send_mail(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto to    = lua->tostring(2);
    auto title = std::string{"mail title"};
    if (argc >= 3 && lua->is_nil(3) == false)
        title = lua->tostring(3);

    auto contents = std::string{"mail contents"};
    if (argc >= 4 && lua->is_nil(4) == false)
        contents = lua->tostring(4);

    return context->builtin_async(*ch, lua, 0, [=]() -> async::task<void> {
        co_await context->send_mail(*ch, to, title, contents);
    });
}