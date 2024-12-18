#include <object.h>
#include <context.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::object, "fb.game.object")
{"model",               fb::game::object::builtin_model},
{"__eq",                fb::game::object::builtin_eq},
{"__tostring",          fb::game::object::builtin_tostring},
{"id",                  fb::game::object::builtin_id},
{"name",                fb::game::object::builtin_name},
{"sound",               fb::game::object::builtin_sound},
{"position",            fb::game::object::builtin_position},
{"direction",           fb::game::object::builtin_direction},
{"chat",                fb::game::object::builtin_chat},
{"message",             fb::game::object::builtin_message},
{"buff",                fb::game::object::builtin_buff},
{"isbuff",              fb::game::object::builtin_isbuff},
{"unbuff",              fb::game::object::builtin_unbuff},
{"effect",              fb::game::object::builtin_effect},
{"map",                 fb::game::object::builtin_map},
{"mkitem",              fb::game::object::builtin_mkitem},
{"showings",            fb::game::object::builtin_showings},
{"showns",              fb::game::object::builtin_showns},
{"front",               fb::game::object::builtin_front},
{"is",                  fb::game::object::builtin_is},
{"thread",              fb::game::object::builtin_thread},
{"ptr",                 fb::game::object::builtin_ptr},
{"near",                fb::game::object::builtin_near},
END_LUA_EXTENSION; // clang-format on

int fb::game::object::builtin_model(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    auto& model = obj->based();
    thread->pushobject(model);
    return 1;
}

int fb::game::object::builtin_id(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    thread->pushinteger(obj->sequence());
    return 1;
}

int fb::game::object::builtin_eq(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx  = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto me   = thread->touserdata<object>(1);
    if (me == nullptr)
        return 0;
    me->assert_thread();

    auto you = thread->touserdata<object>(2);
    if (you == nullptr)
        return 0;
    you->assert_thread();

    thread->pushboolean(me->sequence() == you->sequence());
    return 1;
}

int fb::game::object::builtin_tostring(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx = thread->env<fb::game::context>("context");
    auto me  = thread->touserdata<object>(1);
    if (me == nullptr)
        return 0;
    me->assert_thread();

    thread->pushstring(me->name());
    return 1;
}

int fb::game::object::builtin_name(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    thread->pushstring(obj->name());
    return 1;
}

int fb::game::object::builtin_sound(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    auto sound = thread->tointeger(2);

    ctx->send(*obj, fb::protocol::game::response::object::sound(*obj, SOUND(sound)), context::scope::PIVOT);
    thread->pushinteger(-1);
    return 1;
}

int fb::game::object::builtin_position(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx  = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto obj  = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    if (argc == 1)
    {
        thread->pushinteger(obj->_position.x);
        thread->pushinteger(obj->_position.y);
        return 2;
    }

    uint16_t x, y;
    if (thread->is_table(2))
    {
        thread->rawgeti(2, 1);
        x = (uint16_t)thread->tointeger(-1);
        thread->remove(-1);

        thread->rawgeti(2, 2);
        y = (uint16_t)thread->tointeger(-1);
        thread->remove(-1);
    }
    else
    {
        x = (uint16_t)thread->tointeger(2);
        y = (uint16_t)thread->tointeger(3);
    }

    std::vector<object*> shows, hides, showings, hiddens;
    obj->position(x, y, true);

    if (obj->is(OBJECT_TYPE::CHARACTER))
    {
        ctx->send(
            *obj,
            [obj](const auto& to) {
                return std::unique_ptr<fb::protocol::base::header>(
                    new fb::protocol::game::response::character::show(static_cast<character&>(*obj), to));
            },
            context::scope::PIVOT);
    }
    else
    {
        ctx->send(*obj, fb::protocol::game::response::object::show(*obj), context::scope::PIVOT);
    }

    if (obj->is(OBJECT_TYPE::CHARACTER))
    {
        auto session = static_cast<character*>(obj);
        ctx->send(*obj, fb::protocol::game::response::character::position(*session), context::scope::SELF);
    }

    return 0;
}

int fb::game::object::builtin_direction(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx  = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto obj  = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    if (argc == 1)
    {
        thread->pushinteger(obj->_direction);
        return 1;
    }
    else
    {
        auto direction = DIRECTION(thread->tointeger(2));
        auto ctx       = thread->env<fb::game::context>("context");

        std::ignore = obj->direction(direction);
        ctx->send(*obj, fb::protocol::game::response::object::direction(*obj), context::scope::PIVOT);
        return 0;
    }
}

int fb::game::object::builtin_chat(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx  = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto obj  = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    if (obj->is(OBJECT_TYPE::ITEM))
        return 0;

    obj->assert_thread();

    auto message  = thread->tostring(2);
    auto type     = argc < 3 ? CHAT_TYPE::NORMAL : CHAT_TYPE(thread->tointeger(3));
    auto decorate = argc < 4 ? true : thread->toboolean(4);

    std::stringstream sstream;
    if (decorate)
    {
        if (type == CHAT_TYPE::SHOUT)
            sstream << obj->name() << "! " << message;
        else
            sstream << obj->name() << ": " << message;
    }
    else
    {
        sstream << message;
    }

    obj->chat(message, type);
    return 0;
}

int fb::game::object::builtin_message(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx  = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto obj  = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    auto message = thread->tostring(2);
    auto type    = argc < 3 ? static_cast<int>(MESSAGE_TYPE::STATE) : thread->tointeger(3);

    if (obj->is(OBJECT_TYPE::CHARACTER))
    {
        ctx->send(*obj, fb::protocol::game::response::message(message, MESSAGE_TYPE(type)), context::scope::SELF);
    }

    return 0;
}

int fb::game::object::builtin_buff(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    auto spell = thread->touserdata<fb::model::spell>(2);
    if (spell == nullptr)
        return 0;

    auto seconds = (uint32_t)thread->tointeger(3);
    auto buff    = obj->buffs.push_back(*spell, seconds);
    if (buff == nullptr)
        thread->pushnil();
    else
        ctx->send(*obj, fb::protocol::game::response::spell::buff(*buff), context::scope::SELF);

    return 1;
}

int fb::game::object::builtin_unbuff(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    if (thread->is_str(2))
    {
        auto name  = thread->tostring(2);
        auto model = ctx->model.spell.name2spell(name);
        if (model == nullptr)
            thread->pushboolean(false);
        else
            thread->pushboolean(obj->buffs.remove(*model));
    }
    else if (thread->is_obj(2))
    {
        auto buff = thread->touserdata<fb::model::spell>(2);
        if (buff == nullptr)
            return 0;

        thread->pushboolean(obj->buffs.remove(*buff));
    }
    else
    {
        thread->pushboolean(false);
    }

    return 1;
}

int fb::game::object::builtin_isbuff(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    if (thread->is_str(2))
    {
        auto name  = thread->tostring(2);
        auto model = ctx->model.spell.name2spell(name);
        if (model == nullptr)
            thread->pushboolean(false);
        else
            thread->pushboolean(obj->buffs.contains(*model));
    }
    else if (thread->is_obj(2))
    {
        auto buff = thread->touserdata<fb::model::spell>(2);
        if (buff == nullptr)
            return 0;

        thread->pushboolean(obj->buffs.contains(*buff));
    }
    else
    {
        thread->pushboolean(false);
    }
    return 1;
}

int fb::game::object::builtin_effect(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    auto effect = (uint8_t)thread->tointeger(2);

    if (obj->is(OBJECT_TYPE::ITEM) == false)
    {
        ctx->send(*obj, fb::protocol::game::response::object::effect(*obj, effect), context::scope::PIVOT);
    }
    return 0;
}

int fb::game::object::builtin_map(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx = thread->env<fb::game::context>("context");
    try
    {
        auto argc = thread->argc();
        auto obj  = thread->touserdata<object>(1);
        if (obj == nullptr)
            return 0;

        if (argc == 1)
        {
            auto map = obj->map();
            if (map == nullptr)
                thread->pushnil();
            else
                thread->pushobject(map);
            return 1;
        }

        fb::game::map* map = nullptr;
        if (thread->is_obj(2))
        {
            map = thread->touserdata<fb::game::map>(2);
            if (map == nullptr)
                return 0;

            if (map == nullptr)
                throw std::exception();
        }
        else if (thread->is_str(2))
        {
            map = ctx->maps.name2map(thread->tostring(2));
            if (map == nullptr)
                throw std::exception();
        }
        else
        {
            throw std::exception();
        }

        point16_t position;
        if (thread->is_table(3))
        {
            thread->rawgeti(3, 1);
            position.x = (uint16_t)thread->tointeger(-1);
            thread->remove(-1);

            thread->rawgeti(3, 2);
            position.y = (uint16_t)thread->tointeger(-1);
            thread->remove(-1);
        }
        else if (thread->is_num(3) && thread->is_num(4))
        {
            position.x = (uint16_t)thread->tointeger(3);
            position.y = (uint16_t)thread->tointeger(4);
        }
        else
        {
            throw std::exception();
        }

        obj->map(map, position);
    }
    catch (...)
    { }

    return 0;
}

int fb::game::object::builtin_mkitem(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    auto name  = thread->tostring(2);
    auto model = ctx->model.item.name2item(name);
    if (model == nullptr)
    {
        thread->pushnil();
    }
    else
    {
        auto ctx  = thread->env<fb::game::context>("context");
        auto item = model->make(*ctx);
        item->map(obj->_map, obj->_position);
        thread->pushobject(item);
        ctx->send(*item, fb::protocol::game::response::object::show(*item), context::scope::PIVOT);
    }

    return 1;
}

int fb::game::object::builtin_showings(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx  = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto obj  = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    auto filter = argc < 2 ? OBJECT_TYPE::UNKNOWN : OBJECT_TYPE(thread->tointeger(2));

    thread->new_table();
    const auto& objects = obj->showns(filter);

    for (int i = 0; i < objects.size(); i++)
    {
        thread->pushobject(objects[i]);
        thread->rawseti(-2, uint64_t(i + 1));
    }

    return 1;
}

int fb::game::object::builtin_showns(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx  = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto obj  = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    auto filter = argc < 2 ? OBJECT_TYPE::UNKNOWN : OBJECT_TYPE(thread->tointeger(2));

    thread->new_table();
    const auto& objects = obj->showings(filter);

    for (int i = 0; i < objects.size(); i++)
    {
        thread->pushobject(objects[i]);
        thread->rawseti(-2, uint64_t(i + 1));
    }

    return 1;
}

int fb::game::object::builtin_front(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx  = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto obj  = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    auto filter = argc < 2 ? OBJECT_TYPE::UNKNOWN : OBJECT_TYPE(thread->tointeger(2));
    auto front  = obj->forward(filter);
    if (front == nullptr)
        thread->pushnil();
    else
        thread->pushobject(front);

    return 1;
}

int fb::game::object::builtin_is(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx  = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto obj  = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    auto type = thread->tointeger(2);
    thread->pushboolean(obj->is(OBJECT_TYPE(type)));
    return 1;
}

int fb::game::object::builtin_thread(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx  = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto obj  = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    auto active_thread = obj->thread();
    if (active_thread == nullptr)
        thread->pushnil();
    else
        thread->pushobject(active_thread);

    return 1;
}

int fb::game::object::builtin_ptr(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx  = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto obj  = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();
    thread->pushinteger((uint64_t)(void*)obj);
    return 1;
}

int fb::game::object::builtin_near(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx  = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto obj  = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();
    if (obj->_map == nullptr)
    {
        thread->pushboolean(false);
        return 1;
    }

    auto you   = thread->touserdata<object>(2);
    auto nears = obj->_map->nears(obj->_position);
    auto found = false;
    for (auto x : nears)
    {
        if (you == x)
        {
            found = true;
            break;
        }
    }
    thread->pushboolean(found);
    return 1;
}