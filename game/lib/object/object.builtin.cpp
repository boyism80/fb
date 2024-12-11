#include <object.h>
#include <context.h>

using namespace fb::game;

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

    async::awaitable_then(
        ctx->send(*obj, fb::protocol::game::response::object::sound(*obj, SOUND(sound)), context::scope::PIVOT),
        [](auto result) {
        });
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
        async::awaitable_then(
            ctx->send(
                *obj,
                [obj](const auto& to) {
                    return std::unique_ptr<fb::protocol::base::header>(
                        new fb::protocol::game::response::session::show(static_cast<character&>(*obj), to));
                },
                context::scope::PIVOT),
            [](auto result) {
            });
    }
    else
    {
        async::awaitable_then(ctx->send(*obj, fb::protocol::game::response::object::show(*obj), context::scope::PIVOT),
                              [](auto result) {
                              });
    }

    if (obj->is(OBJECT_TYPE::CHARACTER))
    {
        auto session = static_cast<character*>(obj);
        async::awaitable_then(
            ctx->send(*obj, fb::protocol::game::response::session::position(*session), context::scope::SELF),
            [](auto result) {
            });
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

        async::awaitable_then(
            [=]() -> async::task<void> {
                std::ignore = co_await obj->direction(direction);
                co_await ctx->send(*obj, fb::protocol::game::response::object::direction(*obj), context::scope::PIVOT);
            }(),
            [](auto result) {
            });
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

    async::awaitable_then(
        ctx->send(*obj, fb::protocol::game::response::object::chat(*obj, type, sstream.str()), context::scope::PIVOT),
        [](auto result) {
        });
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
        async::awaitable_then(
            ctx->send(*obj, fb::protocol::game::response::message(message, MESSAGE_TYPE(type)), context::scope::SELF),
            [](auto result) {
            });
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
    auto buff    = async::awaitable_get(obj->buffs.push_back(*spell, seconds));
    if (buff == nullptr)
        thread->pushnil();
    else
        async::awaitable_then(ctx->send(*obj, fb::protocol::game::response::spell::buff(*buff), context::scope::SELF),
                              [](auto result) {
                              });

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
            thread->pushboolean(async::awaitable_get(obj->buffs.remove(*model)));
    }
    else if (thread->is_obj(2))
    {
        auto buff = thread->touserdata<fb::model::spell>(2);
        if (buff == nullptr)
            return 0;

        thread->pushboolean(async::awaitable_get(obj->buffs.remove(*buff)));
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
        async::awaitable_then(
            ctx->send(*obj, fb::protocol::game::response::object::effect(*obj, effect), context::scope::PIVOT),
            [](auto result) {
            });
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

        async::awaitable_get(obj->map(map, position));
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
        async::awaitable_get(item->map(obj->_map, obj->_position));
        thread->pushobject(item);

        async::awaitable_then(
            ctx->send(*item, fb::protocol::game::response::object::show(*item), context::scope::PIVOT),
            [](auto result) {
            });
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