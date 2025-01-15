#include <fb/game/character.h>
#include <fb/game/context.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(character, "fb.game.character")
{"__eq",                fb::game::object::builtin_eq},
{"look",                character::builtin_look},
{"color",               character::builtin_color},
{"money",               character::builtin_money},
{"exp",                 character::builtin_exp},
{"base_hp",             character::builtin_base_hp},
{"base_mp",             character::builtin_base_mp},
{"str",                 character::builtin_str},
{"dex",                 character::builtin_dex},
{"int",                 character::builtin_int},
{"item",                character::builtin_item},
{"items",               character::builtin_items},
{"dropitem",            character::builtin_item_drop},
{"mkitem",              character::builtin_mkitem},
{"rmitem",              character::builtin_rmitem},
{"state",               character::builtin_state},
{"disguise",            character::builtin_disguise},
{"class",               character::builtin_class},
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
{"message",             character::builtin_message},
{"buff_str",            character::builtin_buff_str},
{"buff_dex",            character::builtin_buff_dex},
{"buff_int",            character::builtin_buff_int},
END_LUA_EXTENSION; // clang-format on

int character::builtin_look(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
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

int character::builtin_color(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
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

int character::builtin_money(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
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

int character::builtin_exp(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
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

int character::builtin_base_hp(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
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

int character::builtin_base_mp(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
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

int character::builtin_str(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        if (ch->matched_thread())
        {
            thread->pushinteger(ch->strength());
            return 1;
        }
        else
        {
            static auto static_func = [](fb::lua::context* thread, character* ch) {
                thread->pushinteger(ch->strength());
                thread->resume(1);
            };

            context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
                static_func(thread, ch);
                co_return;
            });

            return thread->yield(1);
        }
    }
    else
    {
        auto value = thread->tointeger(2);
        if (ch->matched_thread())
        {
            ch->strength(value);
            return 0;
        }
        else
        {
            static auto static_func = [](fb::lua::context* thread, character* ch, uint8_t value) {
                ch->strength(value);
                thread->resume(0);
            };

            context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
                static_func(thread, ch, value);
                co_return;
            });
            return thread->yield(0);
        }
    }
}

int character::builtin_dex(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        if (ch->matched_thread())
        {
            thread->pushinteger(ch->dexterity());
            return 1;
        }
        else
        {
            static auto static_func = [](fb::lua::context* thread, character* ch) {
                thread->pushinteger(ch->dexterity());
                thread->resume(1);
            };

            context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
                static_func(thread, ch);
                co_return;
            });

            return thread->yield(1);
        }
    }
    else
    {
        auto value = thread->tointeger(2);
        if (ch->matched_thread())
        {
            ch->dexterity(value);
            return 0;
        }
        else
        {
            static auto static_func = [](fb::lua::context* thread, character* ch, uint8_t value) {
                ch->dexterity(value);
                thread->resume(0);
            };

            context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
                static_func(thread, ch, value);
                co_return;
            });
            return thread->yield(0);
        }
    }
}

int character::builtin_int(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        if (ch->matched_thread())
        {
            thread->pushinteger(ch->intelligence());
            return 1;
        }
        else
        {
            static auto static_func = [](fb::lua::context* thread, character* ch) {
                thread->pushinteger(ch->intelligence());
                thread->resume(1);
            };

            context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
                static_func(thread, ch);
                co_return;
            });

            return thread->yield(1);
        }
    }
    else
    {
        auto value = thread->tointeger(2);
        if (ch->matched_thread())
        {
            ch->intelligence(value);
            return 0;
        }
        else
        {
            static auto static_func = [](fb::lua::context* thread, character* ch, uint8_t value) {
                ch->intelligence(value);
                thread->resume(0);
            };

            context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
                static_func(thread, ch, value);
                co_return;
            });
            return thread->yield(0);
        }
    }
}

int character::builtin_item(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto ch      = thread->touserdata<character>(1);
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

int character::builtin_items(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto ch      = thread->touserdata<character>(1);
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

int character::builtin_item_drop(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto ch      = thread->touserdata<character>(1);
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

int character::builtin_mkitem(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
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

int character::builtin_rmitem(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    try
    {
        auto argc = thread->argc();
        auto ch   = thread->touserdata<character>(1);
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

int character::builtin_state(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
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

int character::builtin_disguise(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    auto value = std::optional<uint16_t>{};
    if (argc >= 2 && lua_type(lua, 2) == LUA_TNUMBER)
        value = static_cast<uint16_t>(thread->tointeger(2));

    static auto static_func = [](fb::lua::context* thread, character* ch, int argc, std::optional<uint16_t> value) {
        if (argc == 1)
        {
            thread->pushinteger(ch->disguise().value());
            return 1;
        }
        else if (value.has_value())
        {
            ch->disguise(value.value());
            return 0;
        }
        else
        {
            ch->undisguise();
            return 0;
        }
    };

    if (ch->matched_thread())
    {
        return static_func(thread, ch, argc, value);
    }
    else
    {
        context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
            auto n = static_func(thread, ch, argc, value);
            if (n == 0)
                thread->pushnil();

            thread->resume(1);
            co_return;
        });
        return thread->yield(1);
    }
}

int character::builtin_class(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
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

            ch->send(fb::protocol::game::response::id(*ch));
            ch->send(fb::protocol::game::response::update_internal(*ch, STATE_LEVEL::LEVEL_MAX));
            thread->pushboolean(true);
        }
    }

    return 1;
}

int character::builtin_level(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
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
        ch->send(fb::protocol::game::response::update_internal(*ch, STATE_LEVEL::LEVEL_MAX));
        return 0;
    }
}

int character::builtin_assert(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto               argc = thread->argc();
    auto               ch   = thread->touserdata<character>(1);
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

int character::builtin_admin(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
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

int character::builtin_deposited_money(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
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

int character::builtin_deposited_item(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
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

int character::builtin_deposit_item(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
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

int character::builtin_withdraw_item(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
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

int character::builtin_group(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    if (argc < 2)
    {
        // 그룹 Lock scope는 스크립트 내의 다음 yield를
        // 만나기 전까지 유효
        context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
            if (ch->_group == nullptr)
            {
                thread->pushnil();
                thread->resume(1);
            }
            else
            {
                ch->_group->lock([=](auto& group) {
                    thread->pushobject(group);
                    thread->resume(1);
                });
            }
            co_return;
        });
        return thread->yield(1);
    }
    else if (lua_type(lua, 2) == LUA_TFUNCTION)
    {
        static auto static_func = [](fb::lua::context* ctx) {
            lua_call(*ctx, 2, LUA_MULTRET);

            ctx->remove(-ctx->argc());
            return ctx->argc();
        };

        thread->pushobject(ch);
        if (ch->_group == nullptr)
        {
            thread->pushnil();
            return static_func(thread);
        }
        else
        {
            return ch->_group->template lock<uint32_t>([=](auto& group) {
                thread->pushobject(group);
                return static_func(thread);
            });
        }
    }
    else
    {
        return 0;
    }
}

int character::builtin_create_group(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    auto name = thread->tostring(2);

    static auto fn = [](fb::game::context* context,
                        fb::lua::context*  thread,
                        character&         me,
                        const std::string& name) -> async::task<void> {
        thread->pushboolean(co_await context->create_group(me, name));
        thread->resume(1);
    };

    context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
        co_await fn(context, thread, *ch, name);
    });

    return thread->yield(1);
}

int character::builtin_clan(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    if (argc < 2)
    {
        // 그룹 Lock scope는 스크립트 내의 다음 yield를
        // 만나기 전까지 유효
        context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
            if (ch->_clan == nullptr)
            {
                thread->pushnil();
                thread->resume(1);
            }
            else
            {
                ch->_clan->lock([=](auto& clan) {
                    thread->pushobject(clan);
                    thread->resume(1);
                });
            }
            co_return;
        });
        return thread->yield(1);
    }
    else if (lua_type(lua, 2) == LUA_TFUNCTION)
    {
        static auto static_func = [](fb::lua::context* ctx) {
            lua_call(*ctx, 2, LUA_MULTRET);

            ctx->remove(-ctx->argc());
            return ctx->argc();
        };

        thread->pushobject(ch);
        if (ch->_clan == nullptr)
        {
            thread->pushnil();
            return static_func(thread);
        }
        else
        {
            return ch->_clan->template lock<uint32_t>([=](auto& clan) {
                thread->pushobject(clan);
                return static_func(thread);
            });
        }
    }
    else
    {
        return 0;
    }
}

int character::builtin_create_clan(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    auto name = thread->tostring(2);

    static auto fn = [](fb::game::context* context,
                        fb::lua::context*  thread,
                        character&         me,
                        const std::string& name) -> async::task<void> {
        try
        {
            co_await context->create_clan(me, name);
            thread->pushnil();
        }
        catch (std::exception& e)
        {
            thread->pushstring(e.what());
        }
        thread->resume(1);
    };

    context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
        co_await fn(context, thread, *ch, name);
    });

    return thread->yield(1);
}

int character::builtin_destroy_clan(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    static auto fn = [](fb::game::context* context, fb::lua::context* thread, character& me) -> async::task<void> {
        try
        {
            co_await context->destroy_clan(me);
            thread->pushnil();
        }
        catch (std::exception& e)
        {
            thread->pushstring(e.what());
        }
        thread->resume(1);
    };

    context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
        co_await fn(context, thread, *ch);
    });

    return thread->yield(1);
}

int character::builtin_traces(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto ch      = thread->touserdata<character>(1);
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

int character::builtin_trace(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto ch      = thread->touserdata<character>(1);
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

int character::builtin_push_trace(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto ch      = thread->touserdata<character>(1);
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

int character::builtin_erase_trace(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto ch      = thread->touserdata<character>(1);
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

int character::builtin_switch_context(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto me      = thread->touserdata<character>(1);
    if (me == nullptr)
        return 0;

    auto you = thread->touserdata<character>(2);
    if (you == nullptr)
    {
        thread->pushboolean(false);
        return 1;
    }

    thread->pushboolean(me->dialog.switch_context(you->dialog));
    return 1;
}

int character::builtin_whisper(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto me      = thread->touserdata<character>(1);
    if (me == nullptr)
        return 0;

    auto to      = thread->tostring(2);
    auto message = thread->tostring(3);

    static auto fn = [](fb::game::context*   context,
                        fb::lua::context*    thread,
                        fb::game::character* ch,
                        const std::string&   to,
                        const std::string&   message) -> async::task<void> {
        try
        {
            co_await context->whisper(*ch, to, message);
            thread->pushnil();
        }
        catch (std::exception& e)
        {
            thread->pushstring(e.what());
        }

        thread->resume(1);
    };

    context->threads.enqueue(*me, [=](auto&) -> async::task<void> {
        co_await fn(context, thread, me, to, message);
    });

    return thread->yield(1);
}

int character::builtin_send_mail(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto me      = thread->touserdata<character>(1);
    if (me == nullptr)
        return 0;

    auto to       = thread->tostring(2);
    auto title    = thread->tostring(3);
    auto contents = thread->tostring(4);

    static auto fn = [](fb::game::context*   context,
                        fb::lua::context*    thread,
                        fb::game::character* ch,
                        const std::string&   to,
                        const std::string&   title,
                        const std::string&   contents) -> async::task<void> {
        try
        {
            co_await context->send_mail(*ch, to, title, contents);
            thread->pushnil();
        }
        catch (std::exception& e)
        {
            thread->pushstring(e.what());
        }

        thread->resume(1);
    };

    context->threads.enqueue(*me, [=](auto&) -> async::task<void> {
        co_await fn(context, thread, me, to, title, contents);
    });

    return thread->yield(1);
}

int character::builtin_assert_state(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto me = thread->touserdata<character>(1);
    if (me == nullptr)
        return 0;

    auto argc   = thread->argc();
    auto values = std::vector<STATE>{};
    for (int i = 1; i < argc; i++)
    {
        values.push_back(static_cast<STATE>(thread->tointeger(i + 1)));
    }

    try
    {
        me->assert_state(values);
        thread->pushnil();
        return 1;
    }
    catch (std::exception& e)
    {
        thread->pushstring(e.what());
        return 1;
    }
}

int character::builtin_message(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx  = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto ch   = thread->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    auto message = thread->tostring(2);
    auto type    = argc < 3 ? MESSAGE_TYPE::STATE : static_cast<MESSAGE_TYPE>(thread->tointeger(3));

    if (ch->thread() == ctx->threads.current())
    {
        ch->message(message, type);
        return 0;
    }
    else
    {
        ctx->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
            ch->message(message, type);
            thread->resume(0);
            co_return;
        });

        return thread->yield(0);
    }
}

int fb::game::character::builtin_buff_str(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        if (ch->matched_thread())
        {
            thread->pushinteger(ch->buff_str());
            return 1;
        }
        else
        {
            static auto static_func = [](fb::lua::context* thread, character* ch) {
                thread->pushinteger(ch->buff_str());
                thread->resume(1);
            };

            context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
                static_func(thread, ch);
                co_return;
            });

            return thread->yield(1);
        }
    }
    else
    {
        auto value = thread->tointeger(2);
        if (ch->matched_thread())
        {
            ch->buff_str(value);
            return 0;
        }
        else
        {
            static auto static_func = [](fb::lua::context* thread, character* ch, uint8_t value) {
                ch->buff_str(value);
                thread->resume(0);
            };

            context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
                static_func(thread, ch, value);
                co_return;
            });
            return thread->yield(0);
        }
    }
}

int fb::game::character::builtin_buff_dex(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        if (ch->matched_thread())
        {
            thread->pushinteger(ch->buff_dex());
            return 1;
        }
        else
        {
            static auto static_func = [](fb::lua::context* thread, character* ch) {
                thread->pushinteger(ch->buff_dex());
                thread->resume(1);
            };

            context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
                static_func(thread, ch);
                co_return;
            });

            return thread->yield(1);
        }
    }
    else
    {
        auto value = thread->tointeger(2);
        if (ch->matched_thread())
        {
            ch->buff_dex(value);
            return 0;
        }
        else
        {
            static auto static_func = [](fb::lua::context* thread, character* ch, uint8_t value) {
                ch->buff_dex(value);
                thread->resume(0);
            };

            context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
                static_func(thread, ch, value);
                co_return;
            });
            return thread->yield(0);
        }
    }
}

int fb::game::character::builtin_buff_int(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto ch      = thread->touserdata<character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        if (ch->matched_thread())
        {
            thread->pushinteger(ch->buff_int());
            return 1;
        }
        else
        {
            static auto static_func = [](fb::lua::context* thread, character* ch) {
                thread->pushinteger(ch->buff_int());
                thread->resume(1);
            };

            context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
                static_func(thread, ch);
                co_return;
            });

            return thread->yield(1);
        }
    }
    else
    {
        auto value = thread->tointeger(2);
        if (ch->matched_thread())
        {
            ch->buff_int(value);
            return 0;
        }
        else
        {
            static auto static_func = [](fb::lua::context* thread, character* ch, uint8_t value) {
                ch->buff_int(value);
                thread->resume(0);
            };

            context->threads.enqueue(*ch, [=](auto&) -> async::task<void> {
                static_func(thread, ch, value);
                co_return;
            });
            return thread->yield(0);
        }
    }
}
