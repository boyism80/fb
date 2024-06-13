#include <fb/game/session.h>
#include <fb/game/context.h>
#include <fb/game/old_model.h>

int fb::game::session::builtin_look(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->look());
        return 1;
    }
    else
    {
        auto value = (uint16_t)thread->tointeger(2);
        session->look(value);
        return 0;
    }
}

int fb::game::session::builtin_color(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->color());
        return 1;
    }
    else
    {
        auto value = (uint8_t)thread->tointeger(2);
        session->color(value);
        return 0;
    }
}

int fb::game::session::builtin_money(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->money());
        return 1;
    }
    else
    {
        auto value = (uint32_t)thread->tointeger(2);
        session->money(value);
        return 0;
    }
}

int fb::game::session::builtin_exp(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->experience());
        return 1;
    }
    else
    {
        auto value = (uint32_t)thread->tointeger(2);
        session->experience(value);
        return 0;
    }
}

int fb::game::session::builtin_base_hp(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->base_hp());
        return 1;
    }
    else
    {
        auto value = (uint32_t)thread->tointeger(2);
        session->base_hp(value);
        return 0;
    }
}

int fb::game::session::builtin_base_mp(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->base_mp());
        return 1;
    }
    else
    {
        auto value = (uint32_t)thread->tointeger(2);
        session->base_mp(value);
        return 0;
    }
}

int fb::game::session::builtin_strength(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->strength());
        return 1;
    }
    else
    {
        auto value = (uint8_t)thread->tointeger(2);
        session->strength(value);
        return 0;
    }
}

int fb::game::session::builtin_dexterity(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->dexteritry());
        return 1;
    }
    else
    {
        auto value = (uint8_t)thread->tointeger(2);
        session->dexteritry(value);
        return 0;
    }
}

int fb::game::session::builtin_intelligence(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->intelligence());
        return 1;
    }
    else
    {
        auto value = (uint8_t)thread->tointeger(2);
        session->intelligence(value);
        return 0;
    }
}

int fb::game::session::builtin_item(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;
    auto item = (fb::game::item*)nullptr;
    if(lua_isnumber(lua, 2))
    {
        auto index = (uint8_t)thread->tointeger(2);
        item = session->items[index];
    }
    else if(lua_isstring(lua, 2))
    {
        auto name = thread->tostring(2);
        item = session->items.find(name);
    }

    if(item == nullptr)
        thread->pushnil();
    else
        thread->pushobject(item);

    return 1;
}

int fb::game::session::builtin_items(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    thread->new_table();
    for(int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        if(session->items[i] == nullptr)
            continue;

        thread->pushinteger(i + 1);
        thread->pushobject(session->items[i]);
        lua_settable(lua, -3);
    }

    return 1;
}

int fb::game::session::builtin_item_drop(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;
    auto index = (uint8_t)thread->tointeger(2);
    auto drop_all = thread->toboolean(3);

    auto dropped = session->items.drop(index - 1, drop_all ? 1 : -1);
    if(dropped != nullptr)
        thread->pushobject(dropped);
    else
        thread->pushnil();

    return 1;
}

int fb::game::session::builtin_mkitem(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;
    auto name = thread->tostring(2);
    auto count = argc < 3 ? 1 : thread->tointeger(3);
    auto store = argc < 4 ? true : thread->toboolean(4);

    if(store == false)
        return object::builtin_mkitem(lua);

    auto model = fb::game::old_model::items.name2item(name);
    if(model == nullptr)
    {
        thread->pushnil();
    }
    else
    {
        auto context = thread->env<fb::game::context>("context");
        auto item = model->make(*context, count);
        auto slot = session->items.add(item);
        if(slot == 0xFF)
        {
            thread->pushnil();
            return 1;
        }

        thread->pushobject(session->items[slot]);
    }

    return 1;
}

int fb::game::session::builtin_rmitem(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    try
    {
        auto argc = thread->argc();
        auto session = thread->touserdata<fb::game::session>(1);
        if(session == nullptr || context->exists(*session) == false)
            return 0;
        auto index = uint8_t(0);
        auto count = argc < 3 ? 1 : (int)thread->tointeger(3);
        auto delete_attr = argc < 4 ? fb::game::item::DELETE_TYPE::REMOVED : (fb::game::item::DELETE_TYPE)thread->tointeger(4);

        if(lua_isuserdata(lua, 2))
        {
            auto item = thread->touserdata<fb::game::item>(2);
            if(item == nullptr)
                return 0;
            if(item == nullptr)
                throw std::exception();

            index = session->items.index(item->based<fb::game::item>());
        }
        else if(lua_isnumber(lua, 2))
        {
            index = (uint8_t)thread->tointeger(2) - 1;
        }
        else if(lua_isstring(lua, 2))
        {
            auto name = thread->tostring(2);
            if(name.empty())
                throw std::exception();

            auto model = fb::game::old_model::items.name2item(name);
            if(model == nullptr)
                throw std::exception();

            index = session->items.index(model);
        }
        else
        {
            throw std::exception();
        }


        auto dropped = session->items.remove(index, count, delete_attr);
        if(dropped != nullptr)
            dropped->destroy();
    }
    catch(...)
    { }

    return 0;
}

int fb::game::session::builtin_state(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;
    if(argc == 1)
    {
        thread->pushinteger(session->state());
        return 1;
    }
    else
    {
        auto value = fb::game::STATE_TYPE(thread->tointeger(2));
        session->state(value);
        return 0;
    }
}

int fb::game::session::builtin_disguise(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->disguise().value());
        return 1;
    }
    else if(luaL_checkinteger(lua, 2))
    {
        session->disguise((uint16_t)thread->tointeger(2));
        return 0;
    }
    else
    {
        session->undisguise();
        return 0;
    }
}

int fb::game::session::builtin_class(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        auto cls = session->_class;
        auto promotion = session->_promotion;

        auto cls_name = fb::game::old_model::classes.class2name(cls, promotion);
        if(cls_name == nullptr)
        {
            thread->pushnil();
        }
        else
        {
            thread->pushstring(*cls_name);
        }
    }
    else
    {
        auto cls_name = thread->tostring(2);
        uint8_t cls, promotion;
        if(fb::game::old_model::classes.name2class(cls_name, &cls, &promotion) == false)
        {
            thread->pushboolean(false);
        }
        else
        {
            auto context = thread->env<fb::game::context>("context");
            context->send(*session, fb::protocol::game::response::session::id(*session), context::scope::SELF);
            context->send(*session, fb::protocol::game::response::session::state(*session, STATE_LEVEL::LEVEL_MAX), context::scope::SELF);
            thread->pushboolean(true);
        }
    }

    return 1;
}

int fb::game::session::builtin_level(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->level());
        return 1;
    }
    else
    {
        auto level = std::max(0, std::min((int)thread->tointeger(2), 255));
        session->level(level);

        auto context = thread->env<fb::game::context>("context");
        context->send(*session, fb::protocol::game::response::session::state(*session, STATE_LEVEL::LEVEL_MAX), context::scope::SELF);
        return 0;
    }
}

int fb::game::session::builtin_group(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    auto group = session->group();
    if(group == nullptr)
        thread->pushnil();
    else
        thread->pushobject(group);

    return 1;
}

int fb::game::session::builtin_assert(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    auto size = thread->rawlen(2);
    std::vector<fb::game::STATE_TYPE> values;
    for(int i = 0; i < size; i++)
    {
        thread->rawgeti(2, i+1);
        values.push_back((fb::game::STATE_TYPE)thread->tointeger(-1));
    }

    try
    {
        session->assert_state(values);
        return 0;
    }
    catch(std::runtime_error& e)
    {
        thread->pushstring(e.what());
        return 1;
    }
}

int fb::game::session::builtin_admin(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushboolean(session->admin());
        return 1;
    }
    else
    {
        session->admin(thread->toboolean(2));
        return 0;
    }
}

int fb::game::session::builtin_deposited_money(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->deposited_money());
        return 1;
    }
    else
    {
        session->deposited_money(thread->tointeger(2));
        return 0;
    }
}

int fb::game::session::builtin_deposited_item(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    const auto& deposited_items = session->deposited_items();
    if(argc == 1)
    {
        thread->new_table();
        for (int i = 0; i < deposited_items.size(); i++)
        {
            thread->pushinteger(i + 1);
            thread->pushobject(deposited_items.at(i));
            lua_settable(lua, -3);
        }
    }
    else if(argc >= 2)
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
                auto name = thread->tostring(2);
                auto found = std::find_if(deposited_items.cbegin(), deposited_items.cend(), [&name](fb::game::item* deposited_item)
                {
                    return deposited_item->based<fb::game::item>()->name == name;
                });

                if (found == deposited_items.cend())
                    throw std::exception();

                thread->pushobject(*found);
            }
            else if (thread->is_obj(2))
            {
                auto model = thread->touserdata<fb::game::item::model>(2);
                auto found = std::find_if(deposited_items.cbegin(), deposited_items.cend(), [model](fb::game::item* deposited_item) 
                {
                    return deposited_item->based<fb::game::item>() == model;
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

int fb::game::session::builtin_deposit_item(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    auto item = thread->touserdata<fb::game::item>(2);
    auto count = argc >= 3 ? thread->tointeger(3) : 1;
    auto index = session->items.index(*item);
    if (index == 0xFF)
    {
        thread->pushboolean(false);
    }
    else
    {
        thread->pushboolean(session->deposit_item(index, count));
    }

    return 1;
}

int fb::game::session::builtin_withdraw_item(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    auto item = thread->touserdata<fb::game::item>(2);
    auto count = argc >= 3 ? thread->tointeger(3) : 1;
    auto& deposited_items = session->deposited_items();
    auto found = std::find(deposited_items.cbegin(), deposited_items.cend(), item);
    if (found == deposited_items.cend())
    {
        thread->pushnil();
    }
    else
    {
        auto index = std::distance(deposited_items.cbegin(), found);
        auto returned = session->withdraw_item(index, count);
        thread->pushobject(returned);
    }
    
    return 1;
}