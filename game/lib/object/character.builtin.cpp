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
{"active",              character::builtin_active},
{"creature",            character::builtin_creature},
{"dialog",              character::builtin_dialog},
{"list",                character::builtin_list},
{"input",               character::builtin_input},
{"menu",                character::builtin_menu},
{"slot",                character::builtin_slot},
END_LUA_EXTENSION; // clang-format on

int character::builtin_look(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto sex = static_cast<SEX>(lua->tointeger(2));
    auto n   = (argc == 1 ? 1 : 0);
    return ctx->builtin(*ch, lua, n, [=]() {
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    if (lua->is_obj(2))
    {
        auto sequence = uint32_t{0xFFFFFFFD};
        auto model    = (const fb::model::object*)nullptr;
        if (lua->is_userdata<fb::game::object>(2))
        {
            auto obj = lua->touserdata<fb::game::object>(2);
            model    = &obj->based<fb::model::object>();
            sequence = obj->sequence();
        }
        else if (lua->is_userdata<fb::model::object>(2))
        {
            model = lua->touserdata<fb::model::object>(2);
        }
        else
        {
            return 0;
        }

        auto message = lua->tostring(3);
        auto items   = fb::game::dialog::item_pairs();
        if (lua->rawlen(4) > 0)
        {
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
                        item = ctx->model.item.name2item(lua->tostring(-1));
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
        }

        auto listener = ch->get_listener<fb::game::character>();
        if (listener != nullptr)
            listener->on_dialog(*ch, *model, message, items, sequence);

        if (ch->dialog != nullptr)
            ch->dialog->release();

        ch->dialog = lua;
        return lua->yield(1);
    }
    else
    {
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
}

int character::builtin_items(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;
    auto name  = lua->tostring(2);
    auto count = lua->tointeger(3, 1);
    auto store = lua->toboolean(4, true);

    if (store == false)
        return object::builtin_mkitem(L);

    auto model = ctx->model.item.name2item(name);
    if (model == nullptr)
    {
        lua->pushnil();
    }
    else
    {
        auto ctx  = lua->env<fb::game::context>("context");
        auto item = model->make(*ctx, count);
        auto slot = ch->items.add(item);
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

    auto ctx = lua->env<fb::game::context>("context");
    try
    {
        auto argc = lua->argc();
        auto ch   = lua->touserdata<character>(1);
        if (ch == nullptr || ctx->alive(*ch) == false)
            return 0;

        auto index       = uint8_t(0);
        auto count       = (uint8_t)lua->tointeger(3, 1);
        auto delete_attr = lua->toenum(4, ITEM_DELETE_TYPE::REMOVED);

        if (lua->is_userdata<fb::game::item>(2))
        {
            auto item = lua->touserdata<fb::game::item>(2);
            if (item == nullptr)
                return 0;

            index = ch->items.index(item->based<fb::model::item>());
        }
        else if (lua->is_userdata<fb::model::item>(2))
        {
            auto model = lua->touserdata<fb::model::item>(2);
            if (model == nullptr)
                return 0;

            index = ch->items.index(*model);
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

            auto model = ctx->model.item.name2item(name);
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto value = std::optional<uint16_t>{};
    if (argc >= 2 && lua_type(L, 2) == LUA_TNUMBER)
        value = static_cast<uint16_t>(lua->tointeger(2));

    static auto static_func = [](fb::lua::context* lua, character* ch, int argc, std::optional<uint16_t> value) {

    };

    auto n = (argc == 1 ? 1 : 0);
    return ctx->builtin(*ch, lua, n, [=]() {
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

        auto ctx = lua->env<fb::game::context>("context");
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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
            else if (lua->is_userdata<fb::game::item>(2))
            {
                auto  item  = lua->touserdata<fb::game::item>(2);
                auto& model = item->based<fb::model::item>();
                auto  found = std::find_if(deposited_items.cbegin(),
                                          deposited_items.cend(),
                                          [&model](fb::game::item* deposited_item) {
                                              return deposited_item->based<fb::model::item>() == model;
                                          });

                if (found == deposited_items.cend())
                    throw std::exception();

                lua->pushobject(*found);
            }
            else if (lua->is_userdata<fb::model::item>(2))
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto item  = lua->touserdata<fb::game::item>(2);
    auto count = lua->tointeger(3, 1);
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto  item            = lua->touserdata<fb::game::item>(2);
    auto  count           = lua->tointeger(3, 1);
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    if (argc < 2)
    {
        // 그룹 Lock scope는 스크립트 내의 다음 yield를
        // 만나기 전까지 유효
        ctx->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto name = lua->tostring(2);

    static auto fn =
        [](fb::game::context* ctx, fb::lua::context* lua, character& ch, const std::string& name) -> async::task<void> {
        lua->pushboolean(co_await ctx->create_group(ch, name));
        lua->resume(1);
    };

    ctx->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
        co_await fn(ctx, lua, *ch, name);
    });

    return lua->yield(1);
}

int character::builtin_clan(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    if (argc < 2)
    {
        // 그룹 Lock scope는 스크립트 내의 다음 yield를
        // 만나기 전까지 유효
        ctx->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto name = lua->tostring(2);

    static auto fn =
        [](fb::game::context* ctx, fb::lua::context* lua, character& ch, const std::string& name) -> async::task<void> {
        try
        {
            co_await ctx->create_clan(ch, name);
            lua->pushnil();
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
        }
        lua->resume(1);
    };

    ctx->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
        co_await fn(ctx, lua, *ch, name);
    });

    return lua->yield(1);
}

int character::builtin_destroy_clan(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    static auto fn = [](fb::game::context* ctx, fb::lua::context* lua, character& ch) -> async::task<void> {
        try
        {
            co_await ctx->destroy_clan(ch);
            lua->pushnil();
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
        }
        lua->resume(1);
    };

    ctx->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
        co_await fn(ctx, lua, *ch);
    });

    return lua->yield(1);
}

int character::builtin_traces(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    const fb::model::trace* model = nullptr;

    if (lua->is_num(2))
    {
        auto id = lua->tointeger(2);
        if (ctx->model.trace.contains(id) == false)
        {
            lua->pushnil();
            return 1;
        }

        model = &ctx->model.trace[id];
    }
    else if (lua->is_userdata<fb::game::trace>(2))
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

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

int character::builtin_whisper(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto to      = lua->tostring(2);
    auto message = lua->tostring(3);

    static auto fn = [](fb::game::context*   ctx,
                        fb::lua::context*    lua,
                        fb::game::character* ch,
                        const std::string&   to,
                        const std::string&   message) -> async::task<void> {
        try
        {
            co_await ctx->whisper(*ch, to, message);
            lua->pushnil();
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
        }

        lua->resume(1);
    };

    ctx->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
        co_await fn(ctx, lua, ch, to, message);
    });

    return lua->yield(1);
}

int character::builtin_assert_state(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto argc   = lua->argc();
    auto values = std::vector<STATE>{};
    for (int i = 1; i < argc; i++)
    {
        values.push_back(static_cast<STATE>(lua->tointeger(i + 1)));
    }

    try
    {
        ch->assert_state(values);
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto value = static_cast<NATION>(lua->tointeger(2));
    auto n     = (argc == 1 ? 1 : 0);
    return ctx->builtin(*ch, lua, n, [=]() {
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto weapon = lua->touserdata<fb::game::weapon>(2);
    auto n      = (argc == 1 ? 1 : 0);
    return ctx->builtin(*ch, lua, n, [=]() mutable {
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto n     = (argc == 1 ? 1 : 0);
    auto value = lua->tostring(2);
    return ctx->builtin(*ch, lua, n, [=]() {
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto items = std::vector<fb::game::item*>{};
    for (int i = 2; i <= argc; i++)
    {
        auto item = lua->touserdata<fb::game::item>(i);
        items.push_back(item);
    }

    return ctx->builtin(*ch, lua, 0, [=]() {
        ch->items.add(items, true);
    });
}

int character::builtin_weapon_damage(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto n     = (argc == 1 ? 1 : 0);
    auto value = lua->tointeger(2);

    return ctx->builtin(*ch, lua, n, [=]() {
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto value = lua->toboolean(2);
    auto n     = (argc == 1 ? 1 : 0);
    return ctx->builtin(*ch, lua, n, [=]() {
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto map = ch->map();
    if (map == nullptr)
    {
        lua->pushnil();
        return 1;
    }

    auto name  = lua->tostring(2);
    auto owned = true;
    auto model = ctx->model.mob.name2mob(name);
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

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto name  = lua->tostring(2);
    auto model = ctx->model.spell.name2spell(name);
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
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
    else if (lua->is_num(2))
    {
        auto slot = lua->tointeger(2);
        ch->spells.remove(slot);
    }
    else if (lua->is_str(2))
    {
        auto name = lua->tostring(2);
        auto slot = uint8_t{0xFF};
        for (int i = 0; i < CONTAINER_CAPACITY; i++)
        {
            auto spell = ch->spells[i];
            if (spell == nullptr)
                continue;

            if (spell->model.name == name)
            {
                slot = i;
                break;
            }
        }

        if (slot == 0xFF)
            return 0;

        ch->spells.remove(slot);
    }

    return 0;
}

int character::builtin_world(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto name = lua->tostring(2);
    for (auto& [id, world] : ctx->model.world)
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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto name    = lua->tostring(2, "func");
    auto new_lua = fb::lua::new_context(lua);
    if (new_lua == nullptr)
        return 0;

    new_lua->load("scripts/script.lua");
    new_lua->func(name);
    new_lua->pushobject(ch);
    std::ignore = new_lua->call(1);
    switch (new_lua->state())
    {
    case LUA_PENDING:
    case LUA_YIELD:
        return lua->yield(1);

    case LUA_OK:
        lua->pushboolean(true);
        return 1;

    default:
        lua->pushboolean(false);
        return 1;
    }
}

int character::builtin_ad(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto width  = lua->tointeger(2, 300);
    auto height = lua->tointeger(3, 120);
    auto url    = lua->tostring(4, "http://www.google.com");
    auto time   = lua->tointeger(5, 60);
    ch->send(fb::protocol::game::response::ad(width, height, url, time));
    return 0;
}

int character::builtin_web(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto type    = lua->tointeger(2, 0);
    auto url     = lua->tostring(3, "http://www.google.com");
    auto message = lua->tostring(4, "default message");
    ch->send(fb::protocol::game::response::web(type, url, message));
    return 0;
}

int character::builtin_delay(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto model = (const fb::model::spell*)nullptr;
    if (lua->is_userdata<fb::model::spell>(2))
    {
        model = lua->touserdata<fb::model::spell>(2);
    }
    else if (lua->is_userdata<fb::game::spell>(2))
    {
        auto spell = lua->touserdata<fb::game::spell>(2);
        if (spell != nullptr)
            model = &spell->model;
    }
    else if (lua->is_str(2))
    {
        model = ctx->model.spell.name2spell(lua->tostring(2));
    }

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

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto n     = (argc == 1 ? 1 : 0);
    auto value = lua_type(L, 2) == LUA_TNIL ? std::nullopt : std::optional<std::uint32_t>{lua->tointeger(2)};
    return ctx->builtin(*ch, lua, n, [=]() {
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

int character::builtin_active(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto item = lua->touserdata<fb::game::item>(2);
    if (item == nullptr)
        return 0;

    auto slot = ch->items.index(*item);
    if (slot == 0xFF)
        return 0;

    ch->items.active(slot);
    return 0;
}

int character::builtin_send_mail(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto to    = lua->tostring(2);
    auto title = std::string{"mail title"};
    if (argc >= 3 && lua->is_nil(3) == false)
        title = lua->tostring(3);

    auto contents = std::string{"mail contents"};
    if (argc >= 4 && lua->is_nil(4) == false)
        contents = lua->tostring(4);

    return ctx->builtin_async(*ch, lua, 0, [=]() -> async::task<void> {
        co_await ctx->send_mail(*ch, to, title, contents);
    });
}

int character::builtin_creature(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    return ctx->builtin(*ch, lua, 1, [=]() {
        lua->pushinteger(ch->creature());
    });
}

int fb::game::character::builtin_dialog(lua_State* L)
{
    // Ex) ch:dialog(obj, "hello", true, true);

    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto argc = lua->argc();
    if (argc < 3)
        throw std::runtime_error("not enough parameters");

    auto sequence = uint32_t{0xFFFFFFFD};
    auto model    = (const fb::model::object*)nullptr;
    if (lua->is_userdata<fb::game::object>(2))
    {
        auto obj = lua->touserdata<fb::game::object>(2);
        model    = &obj->based<fb::model::object>();
        sequence = obj->sequence();
    }
    else if (lua->is_userdata<fb::model::object>(2))
    {
        model = lua->touserdata<fb::model::object>(2);
    }
    else
    {
        return 0;
    }

    auto message     = lua->tostring(3);
    auto button_prev = lua->toboolean(4, false);
    auto button_next = lua->toboolean(5, false);

    auto listener = ch->get_listener<fb::game::character>();
    if (listener != nullptr)
        listener->on_dialog(*ch, *model, message, button_prev, button_next, sequence);

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}

int fb::game::character::builtin_list(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto sequence = uint32_t{0xFFFFFFFD};
    auto model    = (const fb::model::object*)nullptr;
    if (lua->is_userdata<fb::game::object>(2))
    {
        auto obj = lua->touserdata<fb::game::object>(2);
        model    = &obj->based<fb::model::object>();
        sequence = obj->sequence();
    }
    else if (lua->is_userdata<fb::model::object>(2))
    {
        model = lua->touserdata<fb::model::object>(2);
    }
    else
    {
        return 0;
    }

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
            listener->on_dialog(*ch, *model, message, menus, button_prev, preset, sequence);
        else
            listener->on_dialog(*ch, *model, message, menus, button_prev, sequence);
    }

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}

int fb::game::character::builtin_input(lua_State* L)
{
    // Ex) obj::input(ch, "message")
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto sequence = uint32_t{0xFFFFFFFD};
    auto model    = (const fb::model::object*)nullptr;
    if (lua->is_userdata<fb::game::object>(2))
    {
        auto obj = lua->touserdata<fb::game::object>(2);
        model    = &obj->based<fb::model::object>();
        sequence = obj->sequence();
    }
    else if (lua->is_userdata<fb::model::object>(2))
    {
        model = lua->touserdata<fb::model::object>(2);
    }
    else
    {
        return 0;
    }

    auto message  = lua->tostring(3);
    auto argc     = lua->argc();
    auto listener = ch->get_listener<fb::game::character>();
    if (argc > 3)
    {
        auto message_top = lua->tostring(4);
        auto message_bot = lua->tostring(5);
        auto maxlen      = (uint8_t)lua->tointeger(6, 0xFF);
        auto prev        = lua->toboolean(7, false);

        if (listener != nullptr)
            listener->on_dialog(*ch, *model, message, message_top, message_bot, maxlen, prev, sequence);
    }
    else
    {
        if (listener != nullptr)
            listener->on_dialog(*ch, *model, message, sequence);
    }

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}

int fb::game::character::builtin_menu(lua_State* L)
{
    // Ex) obj::menu(ch, "hello", {"hello 1", "hello 2", "hello 3"})
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto sequence = uint32_t{0xFFFFFFFD};
    auto model    = (const fb::model::object*)nullptr;
    if (lua->is_userdata<fb::game::object>(2))
    {
        auto obj = lua->touserdata<fb::game::object>(2);
        model    = &obj->based<fb::model::object>();
        sequence = obj->sequence();
    }
    else if (lua->is_userdata<fb::model::object>(2))
    {
        model = lua->touserdata<fb::model::object>(2);
    }
    else
    {
        return 0;
    }

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
        listener->on_dialog(*ch, *model, message, menus, sequence);

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}

int fb::game::character::builtin_slot(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto ch      = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto sequence = uint32_t{0xFFFFFFFD};
    auto model    = (const fb::model::object*)nullptr;
    if (lua->is_userdata<fb::game::object>(2))
    {
        auto obj = lua->touserdata<fb::game::object>(2);
        model    = &obj->based<fb::model::object>();
        sequence = obj->sequence();
    }
    else if (lua->is_userdata<fb::model::object>(2))
    {
        model = lua->touserdata<fb::model::object>(2);
    }
    else
    {
        return 0;
    }

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
        listener->on_dialog(*ch, *model, message, slots, sequence);

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}