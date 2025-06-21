#include <fb/game/character.h>
#include <fb/game/context.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(character, "fb.game.character")
{"__eq",                object::builtin::builtin_eq},
{"look",                character::builtin::builtin_look},
{"color",               character::builtin::builtin_color},
{"sex",                 character::builtin::builtin_sex},
{"money",               character::builtin::builtin_money},
{"exp",                 character::builtin::builtin_exp},
{"item",                character::builtin::builtin_item},
{"items",               character::builtin::builtin_items},
{"equipments",          character::builtin::builtin_equipments},
{"dropitem",            character::builtin::builtin_item_drop},
{"mkitem",              character::builtin::builtin_mkitem},
{"rmitem",              character::builtin::builtin_rmitem},
{"state",               character::builtin::builtin_state},
{"disguise",            character::builtin::builtin_disguise},
{"class",               character::builtin::builtin_class},
{"promotion",           character::builtin::builtin_promotion},
{"level",               character::builtin::builtin_level},
{"assert",              character::builtin::builtin_assert},
{"deposited_money",     character::builtin::builtin_deposited_money},
{"stored_item",         character::builtin::builtin_stored_item},
{"store_item",          character::builtin::builtin_store_item},
{"retrieve_item",       character::builtin::builtin_retrieve_item},
{"group",               character::builtin::builtin_group},
{"create_group",        character::builtin::builtin_create_group},
{"clan",                character::builtin::builtin_clan},
{"create_clan",         character::builtin::builtin_create_clan},
{"destroy_clan",        character::builtin::builtin_destroy_clan},
{"achievements",        character::builtin::builtin_achievements},
{"achievement",         character::builtin::builtin_achievement},
{"push_achievement",    character::builtin::builtin_push_achievement},
{"erase_achievement",   character::builtin::builtin_erase_achievement},
{"whisper",             character::builtin::builtin_whisper},
{"send_mail",           character::builtin::builtin_send_mail},
{"nation",              character::builtin::builtin_nation},
{"weapon",              character::builtin::builtin_weapon},
{"title",               character::builtin::builtin_title},
{"gain",                character::builtin::builtin_gain},
{"weapon_damage",       character::builtin::builtin_weapon_damage},
{"detect",              character::builtin::builtin_detect},
{"spawn_mob",           character::builtin::builtin_spawn_mob},
{"spawned_mobs",        character::builtin::builtin_spawned_mobs},
{"base_hp",             character::builtin::builtin_base_hp},
{"base_mp",             character::builtin::builtin_base_mp},
{"base_str",            character::builtin::builtin_base_str},
{"base_dex",            character::builtin::builtin_base_dex},
{"base_int",            character::builtin::builtin_base_int},
{"base_dam",            character::builtin::builtin_base_dam},
{"base_hit",            character::builtin::builtin_base_hit},
{"armor_color",         character::builtin::builtin_armor_color},
{"mkspell",             character::builtin::builtin_mkspell},
{"rmspell",             character::builtin::builtin_rmspell},
{"world",               character::builtin::builtin_world},
{"script",              character::builtin::builtin_script},
{"ad",                  character::builtin::builtin_ad},
{"web",                 character::builtin::builtin_web},
{"birthday",            character::builtin::builtin_birthday},
{"active",              character::builtin::builtin_active},
{"super_hide",          character::builtin::builtin_super_hide},
{"creature",            character::builtin::builtin_creature},
{"teleport",            character::builtin::builtin_teleport},
{"dialog",              character::builtin::builtin_dialog},
{"list",                character::builtin::builtin_list},
{"input",               character::builtin::builtin_input},
{"menu",                character::builtin::builtin_menu},
{"slot",                character::builtin::builtin_slot},
END_LUA_EXTENSION; // clang-format on

int character::builtin::builtin_look(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto look = ch->look();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(look);
                return 1;
            });
        });
    }
    else
    {
        auto value = (uint16_t)lua->tointeger(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->look(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_color(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto color = ch->color();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(color);
                return 1;
            });
        });
    }
    else
    {
        auto value = (uint8_t)lua->tointeger(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->color(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_sex(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto sex = ch->sex();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(sex);
                return 1;
            });
        });
    }
    else
    {
        auto sex = static_cast<SEX>(lua->tointeger(2));
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->sex(sex);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_money(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto money = ch->money();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(money);
                return 1;
            });
        });
    }
    else
    {
        auto value = (uint32_t)lua->tointeger(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->money(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_exp(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto exp = ch->exp();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(exp);
                return 1;
            });
        });
    }
    else
    {
        auto value = (uint32_t)lua->tointeger(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->exp(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_item(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    if (lua->is_number(2))
    {
        auto index = (uint8_t)lua->tointeger(2);
        auto item  = ch->items[index];
        if (item == nullptr)
            lua->pushnil();
        else
            lua->pushobject(item);
        return 1;
    }

    if (lua->is_string(2))
    {
        auto name = lua->tostring(2);
        auto item = ch->items.find(name);
        if (item == nullptr)
            lua->pushnil();
        else
            lua->pushobject(item);
        return 1;
    }

    if (lua->is_userdata<fb::model::item>(2))
    {
        auto model = lua->touserdata<fb::model::item>(2);
        auto item  = ch->items.find(*model);
        if (item == nullptr)
            lua->pushnil();
        else
            lua->pushobject(item);
        return 1;
    }

    // item dialog
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
                if (lua->is_string(-1))
                {
                    item = ctx->model.item.name2item(lua->tostring(-1));
                }
                else if (lua->is_userdata<fb::model::item>(-1))
                {
                    item = lua->touserdata<fb::model::item>(-1);
                }
                else
                {
                }
                lua->pop(1);
            }

            { // get 2nd field
                lua->pushinteger(2);
                lua_gettable(L, -2);
                if (lua->is_number(-1))
                {
                    price = lua->tointeger(-1);
                }
                else
                {
                }
            }
            lua->pop(1);

            if (item == nullptr)
                continue;

            items.push_back({*item, price});
            lua->pop(1);
        }
    }

    ch->listener.on_dialog(*ch, *model, message, items, sequence);
    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}

int character::builtin::builtin_items(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    return lua->ensure_yield(*ctx, *ch, [=]() {
        auto buffer = std::vector<std::pair<uint8_t, fb::game::item*>>();
        for (int i = 0; i < CONTAINER_CAPACITY; i++)
        {
            if (ch->items[i] == nullptr)
                continue;

            buffer.push_back({i, ch->items[i]});
        }

        return lua->ensure_resume(*ctx, *ch, [=]() {
            lua->new_table();
            for (auto& [index, item] : buffer)
            {
                lua->pushinteger(index + 1);
                lua->pushobject(item);
                lua_settable(L, -3);
            }
            return 1;
        });
    });
}

int character::builtin::builtin_equipments(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    return lua->ensure_yield(*ctx, *ch, [=]() {
        auto buffer = std::vector<std::pair<EQUIPMENT_PARTS, fb::game::item*>>();
        for (auto& [parts, equipment] : ch->items.equipments())
        {
            if (equipment == nullptr)
                continue;

            buffer.push_back({parts, equipment});
        }

        return lua->ensure_resume(*ctx, *ch, [=]() {
            lua->new_table();
            for (auto& [parts, equipment] : buffer)
            {
                lua->pushinteger(parts);
                lua->pushobject(equipment);
                lua_settable(L, -3);
            }
            return 1;
        });
    });
}

int character::builtin::builtin_item_drop(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto index    = static_cast<uint8_t>(lua->tointeger(2));
    auto drop_all = lua->toboolean(3);

    return lua->ensure_yield(*ctx, *ch, [=]() {
        auto dropped = ch->items.drop(index - 1, drop_all ? 1 : -1);

        return lua->ensure_resume(*ctx, *ch, [=]() {
            if (dropped != nullptr)
                lua->pushobject(dropped);
            else
                lua->pushnil();
            return 1;
        });
    });
}

int character::builtin::builtin_mkitem(lua_State* L)
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
        return object::builtin::builtin_mkitem(L);

    return lua->ensure_yield(*ctx, *ch, [=]() {
        auto model = ctx->model.item.name2item(name);
        auto item  = static_cast<fb::game::item*>(nullptr);
        auto slot  = static_cast<uint8_t>(0xFF);

        if (model != nullptr)
        {
            item = model->make(*ctx, count);
            slot = ch->items.add(item);
        }

        return lua->ensure_resume(*ctx, *ch, [=]() {
            if (model == nullptr || slot == 0xFF)
                lua->pushnil();
            else
                lua->pushobject(ch->items[slot]);
            return 1;
        });
    });
}

int character::builtin::builtin_rmitem(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto count       = static_cast<uint8_t>(lua->tointeger(3, 1));
    auto delete_attr = lua->toenum(4, ITEM_DELETE_TYPE::REMOVED);

    if (lua->is_userdata<fb::game::item>(2))
    {
        auto item = lua->touserdata<fb::game::item>(2);
        if (item == nullptr)
            return 0;

        return lua->ensure_yield(*ctx, *ch, [=]() {
            try
            {
                auto index   = ch->items.index(item->based<fb::model::item>());
                auto dropped = ch->items.remove(index, count, delete_attr);
                if (dropped != nullptr)
                    std::ignore = dropped->destroy();
            }
            catch (...)
            { }

            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
    else if (lua->is_userdata<fb::model::item>(2))
    {
        auto model = lua->touserdata<fb::model::item>(2);
        if (model == nullptr)
            return 0;

        return lua->ensure_yield(*ctx, *ch, [=]() {
            try
            {
                auto index   = ch->items.index(*model);
                auto dropped = ch->items.remove(index, count, delete_attr);
                if (dropped != nullptr)
                    std::ignore = dropped->destroy();
            }
            catch (...)
            { }

            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
    else if (lua->is_number(2))
    {
        auto raw_index = lua->tointeger(2);

        return lua->ensure_yield(*ctx, *ch, [=]() {
            try
            {
                auto index   = static_cast<uint8_t>(raw_index) - 1;
                auto dropped = ch->items.remove(index, count, delete_attr);
                if (dropped != nullptr)
                    std::ignore = dropped->destroy();
            }
            catch (...)
            { }

            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
    else if (lua->is_string(2))
    {
        auto name = lua->tostring(2);
        if (name.empty())
            return 0;

        return lua->ensure_yield(*ctx, *ch, [=]() {
            try
            {
                auto model = ctx->model.item.name2item(name);
                if (model == nullptr)
                    return lua->ensure_resume(*ctx, *ch, [=]() {
                        return 0;
                    });

                auto index   = ch->items.index(*model);
                auto dropped = ch->items.remove(index, count, delete_attr);
                if (dropped != nullptr)
                    std::ignore = dropped->destroy();
            }
            catch (...)
            { }

            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
    else
    {
        return 0;
    }
}

int character::builtin::builtin_state(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto state = ch->state();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(state);
                return 1;
            });
        });
    }
    else
    {
        auto value = STATE(lua->tointeger(2));
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->state(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_disguise(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto disguise = ch->disguise();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(disguise.value());
                return 1;
            });
        });
    }
    else if (lua->is_nil(2))
    {
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->undisguise();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
    else if (lua->is_number(2))
    {
        auto value = lua->tointeger(2);

        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->disguise(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
    else
    {
        return 0;
    }
}

int character::builtin::builtin_class(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto cls = ch->cls();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(cls);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->cls(static_cast<CLASS>(value));
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_promotion(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto promotion = ch->promotion();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(promotion);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->promotion(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_level(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto level = ch->level();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(level);
                return 1;
            });
        });
    }
    else
    {
        auto level = std::max(0, std::min((int)lua->tointeger(2), 255));
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->level(level);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_assert(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    auto size = lua->rawlen(2);

    if (argc == 1)
    {
        lua->pushboolean(true);
        return 1;
    }

    auto values = std::vector<STATE>();
    if (lua->is_table(2))
    {
        for (int i = 0; i < size; i++)
        {
            lua->rawgeti(2, i + 1);
            if (lua->is_number(-1))
                values.push_back((STATE)lua->tointeger(-1));
        }
    }
    else
    {
        for (auto i = 1; i < argc; i++)
        {
            if (lua->is_number(i + 1))
                values.push_back(static_cast<STATE>(lua->tointeger(i + 1)));
        }
    }

    return lua->ensure_yield(*ctx, *ch, [=]() {
        try
        {
            ch->assert_state(values);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        }
        catch (std::runtime_error& e)
        {
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushstring(e.what());
                return 1;
            });
        }
    });

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

int character::builtin::builtin_role(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto role = ch->role();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(role);
                return 1;
            });
        });
    }
    else
    {
        auto value = static_cast<ROLE>(lua->tointeger(2));
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->role(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_deposited_money(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto deposited = ch->items.deposited();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(deposited);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->items.deposited(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_stored_item(lua_State* L)
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
        // Get all stored items
        return lua->ensure_yield(*ctx, *ch, [=]() {
            const auto& stored_items = ch->items.stored();

            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->new_table();
                for (int i = 0; i < stored_items.size(); i++)
                {
                    lua->pushinteger(i + 1);
                    lua->pushobject(stored_items.at(i));
                    lua_settable(L, -3);
                }
                return 1;
            });
        });
    }
    else if (lua->is_number(2))
    {
        // Search by index
        auto index = lua->tointeger(2);

        return lua->ensure_yield(*ctx, *ch, [=]() {
            const auto& stored_items = ch->items.stored();
            auto        stored_item  = static_cast<fb::game::item*>(nullptr);

            if (index <= stored_items.size() - 1)
                stored_item = stored_items.at(index);

            return lua->ensure_resume(*ctx, *ch, [=]() {
                if (stored_item == nullptr)
                    lua->pushnil();
                else
                    lua->pushobject(stored_item);
                return 1;
            });
        });
    }
    else if (lua->is_string(2))
    {
        // Search by name
        auto name = lua->tostring(2);

        return lua->ensure_yield(*ctx, *ch, [=]() {
            const auto& stored_items = ch->items.stored();
            auto found = std::find_if(stored_items.cbegin(), stored_items.cend(), [&name](fb::game::item* stored_item) {
                return stored_item->based<fb::model::item>().name == name;
            });

            auto stored_item = (found != stored_items.cend()) ? *found : nullptr;

            return lua->ensure_resume(*ctx, *ch, [=]() {
                if (stored_item == nullptr)
                    lua->pushnil();
                else
                    lua->pushobject(stored_item);
                return 1;
            });
        });
    }
    else if (lua->is_userdata<fb::game::item>(2))
    {
        // Search by game item
        auto item = lua->touserdata<fb::game::item>(2);

        return lua->ensure_yield(*ctx, *ch, [=]() {
            const auto& stored_items = ch->items.stored();
            auto&       model        = item->based<fb::model::item>();
            auto        found =
                std::find_if(stored_items.cbegin(), stored_items.cend(), [&model](fb::game::item* stored_item) {
                    return stored_item->based<fb::model::item>() == model;
                });

            auto stored_item = (found != stored_items.cend()) ? *found : nullptr;

            return lua->ensure_resume(*ctx, *ch, [=]() {
                if (stored_item == nullptr)
                    lua->pushnil();
                else
                    lua->pushobject(stored_item);
                return 1;
            });
        });
    }
    else if (lua->is_userdata<fb::model::item>(2))
    {
        // Search by model item
        auto model = lua->touserdata<fb::model::item>(2);

        return lua->ensure_yield(*ctx, *ch, [=]() {
            const auto& stored_items = ch->items.stored();
            auto found = std::find_if(stored_items.cbegin(), stored_items.cend(), [model](fb::game::item* stored_item) {
                return stored_item->based<fb::model::item>() == *model;
            });

            auto stored_item = (found != stored_items.cend()) ? *found : nullptr;

            return lua->ensure_resume(*ctx, *ch, [=]() {
                if (stored_item == nullptr)
                    lua->pushnil();
                else
                    lua->pushobject(stored_item);
                return 1;
            });
        });
    }
    else
    {
        return 0;
    }
}

int character::builtin::builtin_store_item(lua_State* L)
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

    return lua->ensure_yield(*ctx, *ch, [=]() {
        auto success = false;
        if (index != 0xFF)
            success = ch->items.store(index, count);

        return lua->ensure_resume(*ctx, *ch, [=]() {
            lua->pushboolean(success);
            return 1;
        });
    });
}

int character::builtin::builtin_retrieve_item(lua_State* L)
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

    return lua->ensure_yield(*ctx, *ch, [=]() {
        auto& stored_items = ch->items.stored();
        auto  found        = std::find(stored_items.cbegin(), stored_items.cend(), item);
        auto  returned     = static_cast<fb::game::item*>(nullptr);

        if (found != stored_items.cend())
        {
            auto index = std::distance(stored_items.cbegin(), found);
            returned   = ch->items.retrieve(index, count);
        }

        return lua->ensure_resume(*ctx, *ch, [=]() {
            if (returned == nullptr)
                lua->pushnil();
            else
                lua->pushobject(returned);
            return 1;
        });
    });
}

int character::builtin::builtin_group(lua_State* L)
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
        static auto fn = [](fb::game::context* ctx, character* ch, fb::lua::context* lua) -> async::task<void> {
            auto group_ptr = ch->_group;
            if (group_ptr == nullptr)
            {
                co_await lua->switching();
                lua->pushnil();
                lua->resume(1);
            }
            else
            {
                group_ptr->read([=](const auto& group) {
                    async::awaitable_then(lua->switching(), [lua, &group](auto result) {
                        result();
                        lua->pushobject(group);
                        lua->resume(1);
                    });
                });
            }
            co_return;
        };

        ctx->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
            co_await fn(ctx, ch, lua);
        });
        return lua->yield(1);
    }
    else if (lua->is_function(2))
    {
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto group_ptr = ch->_group;

            return lua->ensure_resume(*ctx, *ch, [=]() {
                static auto static_func = [](fb::lua::context* ctx) {
                    lua_call(*ctx, 2, LUA_MULTRET);
                    ctx->remove(-ctx->argc());
                    return ctx->argc();
                };

                lua->pushobject(ch);
                if (group_ptr == nullptr)
                {
                    lua->pushnil();
                    return static_func(lua);
                }
                else
                {
                    return group_ptr->template read<int>([=](const auto& group) {
                        lua->pushobject(group);
                        return static_func(lua);
                    });
                }
            });
        });
    }
    else
    {
        return 0;
    }
}

int character::builtin::builtin_create_group(lua_State* L)
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
        [](fb::game::context* ctx, character* ch, fb::lua::context* lua, std::string target) -> async::task<void> {
        try
        {
            co_await ctx->create_group(*ch, target);
            co_await lua->switching();
            lua->pushboolean(true);
            lua->resume(1);
        }
        catch (std::exception& e)
        {
            async::awaitable_then(lua->switching(), [lua](auto result) {
                result();
                lua->pushboolean(false);
                lua->resume(1);
            });
        }
    };

    ctx->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
        co_await fn(ctx, ch, lua, name);
    });

    return lua->yield(1);
}

int character::builtin::builtin_clan(lua_State* L)
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
        static auto fn = [](fb::game::context* ctx, character* ch, fb::lua::context* lua) -> async::task<void> {
            auto clan_ptr = ch->_clan;
            if (clan_ptr == nullptr)
            {
                co_await lua->switching();
                lua->pushnil();
                lua->resume(1);
            }
            else
            {
                async::awaitable_then(lua->switching(), [lua, clan_ptr](auto result) {
                    result();
                    clan_ptr->read([=](const auto& clan) {
                        lua->pushobject(clan);
                        lua->resume(1);
                    });
                });
            }
            co_return;
        };

        ctx->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
            co_await fn(ctx, ch, lua);
        });
        return lua->yield(1);
    }
    else if (lua->is_function(2))
    {
        static auto fn = [](fb::game::context* ctx, character* ch, fb::lua::context* lua) -> async::task<void> {
            auto clan_ptr = ch->_clan;
            if (clan_ptr == nullptr)
            {
                co_await lua->switching();
                lua->pushnil();
                lua->resume(1);
            }
            else
            {
                async::awaitable_then(lua->switching(), [lua, clan_ptr](auto result) {
                    result();
                    clan_ptr->read([=](const auto& clan) {
                        lua->pushobject(clan);
                        lua->resume(1);
                    });
                });
            }
        };

        ctx->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
            co_await fn(ctx, ch, lua);
        });

        return lua->yield(1);
    }
    else
    {
        return 0;
    }
}

int character::builtin::builtin_create_clan(lua_State* L)
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
        [](fb::game::context* ctx, character* ch, fb::lua::context* lua, const std::string& name) -> async::task<void> {
        try
        {
            co_await ctx->create_clan(*ch, name);
            co_await lua->switching();
            lua->pushnil();
            lua->resume(1);
        }
        catch (std::exception& e)
        {
            auto what = std::string(e.what());
            async::awaitable_then(lua->switching(), [lua, what](auto result) {
                result();
                lua->pushstring(what.c_str());
                lua->resume(1);
            });
        }
        co_return;
    };

    ctx->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
        co_await fn(ctx, ch, lua, name);
        co_return;
    });

    return lua->yield(1);
}

int character::builtin::builtin_destroy_clan(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    // Static function to isolate async operation and ensure parameter lifetime safety
    static auto static_func = [](fb::game::context* ctx, character* ch, fb::lua::context* lua) -> async::task<bool> {
        try
        {
            co_await ctx->destroy_clan(*ch);
            co_await lua->switching();
            lua->pushnil();
            lua->resume(1);
            co_return true;
        }
        catch (std::exception& e)
        {
            auto what = std::string(e.what());
            async::awaitable_then(lua->switching(), [lua, what](auto result) {
                result();
                lua->pushstring(what.c_str());
                lua->resume(1);
            });
            co_return false;
        }
    };

    ctx->threads.enqueue(*ch, [ctx, ch, lua](auto&) -> async::task<void> {
        // Call static_func with explicit parameters for lifetime safety
        co_await static_func(ctx, ch, lua);
    });

    return lua->yield(1);
}

int character::builtin::builtin_achievements(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    return lua->ensure_yield(*ctx, *ch, [=]() {
        auto buffer = std::vector<std::pair<uint32_t, fb::game::achievement*>>();
        for (auto& [id, achievement] : ch->achievements)
        {
            buffer.push_back({id, achievement.get()});
        }

        return lua->ensure_resume(*ctx, *ch, [=]() {
            lua->new_table();
            auto i = 0;
            for (auto& [id, achievement] : buffer)
            {
                lua->pushinteger(i + 1);
                lua->pushobject(*achievement);
                lua_settable(L, -3);
                i++;
            }
            return 1;
        });
    });

    return 1;
}

int character::builtin::builtin_achievement(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto i = lua->tointeger(2);
    return lua->ensure_yield(*ctx, *ch, [=]() {
        auto achievement = static_cast<fb::game::achievement*>(nullptr);
        if (ch->achievements.contains(i))
            achievement = ch->achievements.at(i).get();

        return lua->ensure_resume(*ctx, *ch, [=]() {
            if (achievement == nullptr)
                lua->pushnil();
            else
                lua->pushobject(*achievement);

            return 1;
        });
    });
}

int character::builtin::builtin_push_achievement(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto ctx  = lua->env<fb::game::context>("context");
    auto ch   = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto model = static_cast<const fb::model::achievement*>(nullptr);
    if (lua->is_number(2))
    {
        auto id = lua->tointeger(2);
        if (!ctx->model.achievement.contains(id))
            return 0;

        model = &ctx->model.achievement[id];
    }
    else if (lua->is_userdata<fb::game::achievement>(2))
    {
        model = (const fb::model::achievement*)(lua->touserdata<fb::game::achievement>(2));
    }
    else
    {
        return 0;
    }

    auto text = std::optional<std::string>{std::nullopt};
    if (lua->is_string(3))
        text = lua->tostring(3);

    auto icon = std::optional<uint8_t>{std::nullopt};
    if (lua->is_number(4))
        icon = lua->tointeger(4);

    auto color = std::optional<uint16_t>{std::nullopt};
    if (lua->is_number(5))
        color = lua->tointeger(5);

    return lua->ensure_yield(*ctx, *ch, [=]() {
        auto already_has = ch->achievements.contains(model->id);
        if (!already_has)
            ch->achievements.insert({model->id, std::make_unique<fb::game::achievement>(*model, text, icon, color)});
        else
            ch->achievements[model->id] = std::make_unique<fb::game::achievement>(*model, text, icon, color);

        return lua->ensure_resume(*ctx, *ch, [=]() {
            if (already_has)
                lua->pushnil();
            else
                lua->pushobject(*ch->achievements[model->id]);
            return 1;
        });
    });
}

int character::builtin::builtin_erase_achievement(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto i = lua->tointeger(2);
    return lua->ensure_yield(*ctx, *ch, [=]() {
        auto success = false;
        if (ch->achievements.contains(i))
        {
            ch->achievements.erase(i);
            success = true;
        }

        return lua->ensure_resume(*ctx, *ch, [=]() {
            lua->pushboolean(success);
            return 1;
        });
    });
}

int character::builtin::builtin_whisper(lua_State* L)
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

int character::builtin::builtin_nation(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto nation = ch->nation();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(static_cast<uint8_t>(nation));
                return 1;
            });
        });
    }
    else
    {
        auto value = static_cast<NATION>(lua->tointeger(2));
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->nation(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_weapon(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto weapon = ch->items.weapon();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                if (weapon == nullptr)
                    lua->pushnil();
                else
                    lua->pushobject(weapon);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->touserdata<fb::game::weapon>(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->items.weapon(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_title(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto title = ch->title();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushstring(title);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tostring(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->title(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_gain(lua_State* L)
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

    return lua->ensure_yield(*ctx, *ch, [=]() {
        ch->items.add(items, true);
        return lua->ensure_resume(*ctx, *ch, [=]() {
            return 0;
        });
    });
}

int character::builtin::builtin_weapon_damage(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto weapon_damage = ch->weapon_damage();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(weapon_damage);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->weapon_damage(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_detect(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto detect = ch->detect();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushboolean(detect);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->toboolean(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->detect(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_spawn_mob(lua_State* L)
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
    if (argc > 3)
    {
        if (lua->is_table(3))
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
    }

    return lua->ensure_yield(*ctx, *ch, [=]() mutable {
        if (argc < 3)
        {
            x = ch->x();
            y = ch->y();
        }

        auto mob = ch->spawn_mob(*model, fb::model::point16_t(x, y), owned);
        return lua->ensure_resume(*ctx, *ch, [=]() {
            if (mob == nullptr)
                lua->pushnil();
            else
                lua->pushobject(mob);
            return 1;
        });
    });
}

int character::builtin::builtin_spawned_mobs(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    return lua->ensure_yield(*ctx, *ch, [=]() {
        auto& spawned_mobs = ch->spawned_mobs();

        return lua->ensure_resume(*ctx, *ch, [=]() {
            lua->new_table();
            for (int i = 0; i < spawned_mobs.size(); i++)
            {
                lua->pushinteger(i + 1);
                lua->pushobject(spawned_mobs[i]);
                lua_settable(L, -3);
            }

            return 1;
        });
    });
}

int character::builtin::builtin_base_hp(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto base_hp = ch->base_hp();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(base_hp);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->base_hp(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_base_mp(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto base_mp = ch->base_mp();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(base_mp);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->base_mp(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_base_str(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto base_str = ch->base_str();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(base_str);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->base_str(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_base_dex(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto base_dex = ch->base_dex();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(base_dex);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->base_dex(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_base_int(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto base_int = ch->base_int();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(base_int);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->base_int(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_base_dam(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto base_dam = ch->base_dam();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(base_dam);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->base_dam(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_base_hit(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto base_hit = ch->base_hit();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushinteger(base_hit);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->base_hit(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_armor_color(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto result = std::optional<uint16_t>{};
            if (ch->_armor_color.has_value())
                result = ch->_armor_color.value();

            return lua->ensure_resume(*ctx, *ch, [=]() {
                if (result.has_value())
                    lua->pushinteger(result.value());
                else
                    lua->pushnil();
                return 1;
            });
        });
    }
    else
    {
        auto value = std::optional<uint16_t>{};
        if (lua->is_number(2))
            value = lua->tointeger(2);

        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->armor_color(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_mkspell(lua_State* L)
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

    return lua->ensure_yield(*ctx, *ch, [=]() {
        auto slot = ch->spells.add(*model);
        if (slot == 0xFF)
            return 0;

        return lua->ensure_resume(*ctx, *ch, [=]() {
            lua->pushinteger(slot);
            return 1;
        });
    });
}

int character::builtin::builtin_rmspell(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            for (int i = 0; i < CONTAINER_CAPACITY; i++)
            {
                auto spell = ch->spells[i];
                if (spell == nullptr)
                    continue;

                ch->spells.remove(i);
            }

            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
    else if (lua->is_number(2))
    {
        auto slot = lua->tointeger(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->spells.remove(slot);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
    else if (lua->is_string(2))
    {
        return lua->ensure_yield(*ctx, *ch, [=]() {
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

            if (slot != 0xFF)
                ch->spells.remove(slot);

            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
    else
    {
        return 0;
    }
}

int character::builtin::builtin_world(lua_State* L)
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
                return lua->ensure_yield(*ctx, *ch, [=]() {
                    ch->show_world_map(id, index);
                    return lua->ensure_resume(*ctx, *ch, [=]() {
                        lua->pushboolean(true);
                        return 1;
                    });
                });
            }
        }
    }

    lua->pushboolean(false);
    return 1;
}

int character::builtin::builtin_script(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto file = lua->tostring(2, "script.lua");
    auto func = lua->tostring(3, "func");

    if (ch->thread() == ctx->threads.current())
    {
        auto new_lua = fb::lua::new_context(lua);
        if (new_lua == nullptr)
            return 0;

        new_lua->load(std::format("scripts/{}", file));
        new_lua->func(func);
        new_lua->pushobject(ch);
        lua_xmove(L, *new_lua, argc - 3);

        auto n      = 0;
        std::ignore = new_lua->call(argc - 2, true, &n);
        switch (new_lua->state())
        {
        case LUA_PENDING:
        case LUA_YIELD:
            return lua->yield(0);

        case LUA_OK:
            return n;

        default:
            return 0;
        }
    }
    else
    {
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto new_lua = fb::lua::new_context(lua);
            if (new_lua == nullptr)
                return 0;

            new_lua->load(std::format("scripts/{}", file));
            new_lua->func(func);
            new_lua->pushobject(ch);
            lua_xmove(L, *new_lua, argc - 3);
            std::ignore = new_lua->call(argc - 2);
            return lua->yield(0);
        });
    }
}

int character::builtin::builtin_ad(lua_State* L)
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

int character::builtin::builtin_web(lua_State* L)
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

int character::builtin::builtin_delay(lua_State* L)
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
    else if (lua->is_string(2))
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

int character::builtin::builtin_birthday(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto result = std::optional<std::uint32_t>{};
            if (ch->birthday().has_value())
                result = ch->birthday().value();

            return lua->ensure_resume(*ctx, *ch, [=]() {
                if (result.has_value())
                    lua->pushinteger(result.value());
                else
                    lua->pushnil();
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->birthday(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_active(lua_State* L)
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

int character::builtin::builtin_super_hide(lua_State* L)
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
        return lua->ensure_yield(*ctx, *ch, [=]() {
            auto result = ch->super_hide();
            return lua->ensure_resume(*ctx, *ch, [=]() {
                lua->pushboolean(result);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->toboolean(2);
        return lua->ensure_yield(*ctx, *ch, [=]() {
            ch->super_hide(value);
            return lua->ensure_resume(*ctx, *ch, [=]() {
                return 0;
            });
        });
    }
}

int character::builtin::builtin_send_mail(lua_State* L)
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

    return lua->ensure_yield(*ctx, *ch, [=]() {
        std::ignore = ctx->send_mail(*ch, to, title, contents);
        return lua->ensure_resume(*ctx, *ch, [=]() {
            return 0;
        });
    });
}

int character::builtin::builtin_creature(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto ch   = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    return lua->ensure_yield(*ctx, *ch, [=]() {
        auto creature = ch->creature();
        return lua->ensure_resume(*ctx, *ch, [=]() {
            lua->pushinteger(creature);
            return 1;
        });
    });
}

int fb::game::character::builtin::builtin_teleport(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto ch  = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr || ctx->alive(*ch) == false)
        return 0;

    auto target = lua->touserdata<fb::game::character>(2);
    if (target == nullptr || ctx->alive(*target) == false)
        return 0;

    if (ch->thread() == target->thread())
    {
        auto map    = target->map();
        std::ignore = ch->map(map, target->position());
        lua->pushboolean(true);
        return 1;
    }
    else
    {
        ctx->threads.enqueue(*target, [=](auto& thread) -> async::task<void> {
            auto map = target->map();
            if (map == nullptr)
            {
                lua->release();
                co_return;
            }
            auto position = target->position();

            ctx->threads.enqueue(*ch, [=](auto& thread) -> async::task<void> {
                async::awaitable_then(ch->map(map, position), [=](auto result) {
                    auto success = result();
                    lua->ensure_resume(*ctx, *ch, [=]() {
                        lua->pushboolean(success);
                        return 1;
                    });
                });
                co_return;
            });
        });

        lua->yield(0);
        return 0;
    }
}

int fb::game::character::builtin::builtin_dialog(lua_State* L)
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

    ch->listener.on_dialog(*ch, *model, message, button_prev, button_next, sequence);

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}

int fb::game::character::builtin::builtin_list(lua_State* L)
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

    if (custom_preset)
        ch->listener.on_dialog(*ch, *model, message, menus, button_prev, preset, sequence);
    else
        ch->listener.on_dialog(*ch, *model, message, menus, button_prev, sequence);

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}

int fb::game::character::builtin::builtin_input(lua_State* L)
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

    auto message = lua->tostring(3);
    auto argc    = lua->argc();
    if (argc > 3)
    {
        auto message_top = lua->tostring(4);
        auto message_bot = lua->tostring(5);
        auto maxlen      = (uint8_t)lua->tointeger(6, 0xFF);
        auto prev        = lua->toboolean(7, false);

        ch->listener.on_dialog(*ch, *model, message, message_top, message_bot, maxlen, prev, sequence);
    }
    else
    {
        ch->listener.on_dialog(*ch, *model, message, sequence);
    }

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}

int fb::game::character::builtin::builtin_menu(lua_State* L)
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

    ch->listener.on_dialog(*ch, *model, message, menus, sequence);

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}

int fb::game::character::builtin::builtin_slot(lua_State* L)
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

    ch->listener.on_dialog(*ch, *model, message, slots, sequence);

    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}