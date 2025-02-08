#include <fb/game/object.h>
#include <fb/game/context.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(object, "fb.game.object")
{"model",               object::builtin_model},
{"__eq",                object::builtin_eq},
{"__tostring",          object::builtin_tostring},
{"id",                  object::builtin_id},
{"name",                object::builtin_name},
{"sound",               object::builtin_sound},
{"position",            object::builtin_position},
{"front_position",      object::builtin_front_position},
{"direction",           object::builtin_direction},
{"chat",                object::builtin_chat},
{"buff",                object::builtin_buff},
{"isbuff",              object::builtin_isbuff},
{"unbuff",              object::builtin_unbuff},
{"buffs",               object::builtin_buffs},
{"effect",              object::builtin_effect},
{"map",                 object::builtin_map},
{"mkitem",              object::builtin_mkitem},
{"sight_in",            object::builtin_sight_in},
{"nears",               object::builtin_nears},
{"front",               object::builtin_front},
{"is",                  object::builtin_is},
{"thread",              object::builtin_thread},
{"ptr",                 object::builtin_ptr},
{"near",                object::builtin_near},
END_LUA_EXTENSION; // clang-format on

int object::builtin_model(lua_State* lua)
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

int object::builtin_id(lua_State* lua)
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

int object::builtin_eq(lua_State* lua)
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

int object::builtin_tostring(lua_State* lua)
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

int object::builtin_name(lua_State* lua)
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

int object::builtin_sound(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    auto sound = static_cast<SOUND>(thread->tointeger(2));
    if (obj->thread() == ctx->threads.current())
    {
        obj->sound(sound);
        return 0;
    }
    else
    {
        ctx->threads.enqueue(*obj, [=](auto&) -> async::task<void> {
            obj->sound(sound);
            thread->resume(0);
            co_return;
        });

        return thread->yield(0);
    }
}

int object::builtin_position(lua_State* lua)
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
        if (obj->thread() == ctx->threads.current())
        {
            thread->pushinteger(obj->_position.x);
            thread->pushinteger(obj->_position.y);
            return 2;
        }
        else
        {
            ctx->threads.enqueue(*obj, [=](auto&) -> async::task<void> {
                thread->pushinteger(obj->_position.x);
                thread->pushinteger(obj->_position.y);
                thread->resume(2);
                co_return;
            });
            return thread->yield(2);
        }
    }

    static auto static_func = [](fb::lua::context* thread, object* obj) {
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

        obj->position(x, y, true);
    };
    if (obj->thread() == ctx->threads.current())
    {
        static_func(thread, obj);
        return 0;
    }
    else
    {
        ctx->threads.enqueue(*obj, [=](auto&) -> async::task<void> {
            static_func(thread, obj);
            thread->resume(0);
            co_return;
        });

        return thread->yield(0);
    }
}

int object::builtin_front_position(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx  = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto obj  = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    auto step = argc < 2 ? 1 : thread->tointeger(2);
    obj->assert_thread();

    if (obj->thread() == ctx->threads.current())
    {
        auto position = obj->front_position(step);
        thread->pushinteger(position.x);
        thread->pushinteger(position.y);
        return 2;
    }
    else
    {
        ctx->threads.enqueue(*obj, [=](auto&) -> async::task<void> {
            auto position = obj->front_position(step);
            thread->pushinteger(position.x);
            thread->pushinteger(position.y);
            thread->resume(2);
            co_return;
        });
        return thread->yield(2);
    }
}

int object::builtin_direction(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx  = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto obj  = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        if (obj->thread() == ctx->threads.current())
        {
            thread->pushinteger(obj->_direction);
            return 1;
        }
        else
        {
            ctx->threads.enqueue(*obj, [=](auto&) -> async::task<void> {
                thread->pushinteger(obj->_direction);
                thread->resume(1);
                co_return;
            });
            return thread->yield(1);
        }
    }
    else
    {
        auto direction = static_cast<DIRECTION>(thread->tointeger(2));
        if (obj->thread() == ctx->threads.current())
        {
            obj->direction(direction);
            return 0;
        }
        else
        {
            ctx->threads.enqueue(*obj, [=](auto&) -> async::task<void> {
                obj->direction(direction);
                thread->resume(0);
                co_return;
            });
            return thread->yield(0);
        }
    }
}

int object::builtin_chat(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx  = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto obj  = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    auto message  = thread->tostring(2);
    auto type     = argc < 3 ? CHAT_TYPE::NORMAL : CHAT_TYPE(thread->tointeger(3));
    auto decorate = argc < 4 ? true : thread->toboolean(4);

    static auto static_func = [](const std::string& message, CHAT_TYPE type, bool decorate, object* obj) {
        auto sstream = std::stringstream{};
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

        if (obj->is(OBJECT_TYPE::ITEM) == false)
            obj->chat(sstream.str(), type);
    };

    if (obj->thread() == ctx->threads.current())
    {
        static_func(message, type, decorate, obj);
        return 0;
    }
    else
    {
        ctx->threads.enqueue(*obj, [=](auto&) -> async::task<void> {
            static_func(message, type, decorate, obj);
            thread->resume(0);
            co_return;
        });
        return thread->yield(0);
    }
}

int object::builtin_buff(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto argc = thread->argc();
    auto ctx  = thread->env<fb::game::context>("context");
    auto obj  = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    if (ctx->alive(*obj) == false)
        return 0;

    const fb::model::spell* model = nullptr;
    switch (lua_type(lua, 2))
    {
    case LUA_TUSERDATA:
        model = &thread->touserdata<fb::game::spell>(2)->model;
        break;

    case LUA_TSTRING:
        model = ctx->model.spell.name2spell(thread->tostring(2));
        break;

    default:
        return 0;
    }

    if (model == nullptr)
        return 0;

    auto seconds = (uint32_t)thread->tointeger(3);
    auto caster  = argc >= 4 ? thread->touserdata<fb::game::object>(4) : nullptr;
    auto buff    = obj->buffs.push_back(*model, seconds, caster);
    if (buff == nullptr)
    {
        thread->pushnil();
        return 1;
    }

    obj->send(fb::protocol::game::response::spell_buff(*buff));
    return 1;
}

int object::builtin_unbuff(lua_State* lua)
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
        auto buff = thread->touserdata<fb::game::buff>(2);
        if (buff == nullptr)
            return 0;

        thread->pushboolean(obj->buffs.remove(buff->model.id));
    }
    else
    {
        thread->pushboolean(false);
    }

    return 1;
}

int object::builtin_isbuff(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    auto argc = thread->argc();
    for (int i = 1; i < argc; i++)
    {
        if (thread->is_str(i + 1))
        {
            auto name  = thread->tostring(i + 1);
            auto model = ctx->model.spell.name2spell(name);
            if (model == nullptr)
                continue;

            if (obj->buffs.contains(*model) == false)
                continue;

            thread->pushboolean(true);
            return 1;
        }

        if (thread->is_obj(i + 1))
        {
            auto buff = thread->touserdata<fb::game::buff>(i + 1);
            if (buff == nullptr)
                continue;

            if (obj->buffs.contains(buff->model.id) == false)
                continue;

            thread->pushboolean(true);
            return 1;
        }
    }

    thread->pushboolean(false);
    return 1;
}

int object::builtin_effect(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    auto effect = static_cast<uint8_t>(thread->tointeger(2));
    if (obj->thread() == ctx->threads.current())
    {
        obj->effect(effect);
        return 0;
    }
    else
    {
        ctx->threads.enqueue(*obj, [=](auto&) -> async::task<void> {
            obj->effect(effect);
            thread->resume(0);
            co_return;
        });
        return thread->yield(0);
    }
}

int object::builtin_map(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx  = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto obj  = thread->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        if (obj->thread() == ctx->threads.current())
        {
            auto map = obj->map();
            if (map == nullptr)
                thread->pushnil();
            else
                thread->pushobject(map);
            return 1;
        }
        else
        {
            static auto static_func = [](fb::game::context* context, fb::lua::context* thread, object* obj) {
                auto map = obj->map();
                if (map == nullptr)
                    thread->pushnil();
                else
                    thread->pushobject(map);
                thread->resume(1);
            };

            ctx->threads.enqueue(*obj, [=](auto&) -> async::task<void> {
                static_func(ctx, thread, obj);
                co_return;
            });
            return thread->yield(1);
        }
    }

    try
    {
        static auto static_func = [](fb::game::context*                  context,
                                     fb::lua::context*                   thread,
                                     fb::game::object*                   obj,
                                     fb::game::map*                      map,
                                     std::optional<fb::model::point16_t> position) -> async::task<void> {
            try
            {
                if (position.has_value())
                {
                    if (co_await obj->map(map, position.value()) == false)
                        throw std::runtime_error(_TEXT(MESSAGE_NOT_READY_GAME_SERVER));
                }
                else
                {
                    if (co_await obj->map(map) == false)
                        throw std::runtime_error(_TEXT(MESSAGE_NOT_READY_GAME_SERVER));
                }

                thread->pushnil();
            }
            catch (std::exception& e)
            {
                thread->pushstring(e.what());
            }

            thread->resume(1);
        };

        fb::game::map* map = nullptr;
        if (thread->is_obj(2))
        {
            map = thread->touserdata<fb::game::map>(2);
            if (map == nullptr)
                throw std::runtime_error("올바르지 않은 맵입니다.");
        }
        else if (thread->is_num(2))
        {
            auto id = thread->tointeger(2);
            if (ctx->maps.contains(id) == false)
                throw std::runtime_error("올바르지 않은 맵입니다.");
            map = &ctx->maps[id];
        }
        else if (thread->is_str(2))
        {
            map = ctx->maps.name2map(thread->tostring(2));
            if (map == nullptr)
                throw std::runtime_error("올바르지 않은 맵입니다.");
        }
        else
        {
            throw std::runtime_error("올바르지 않은 맵입니다.");
        }

        auto position = std::optional<fb::model::point16_t>{};
        if (thread->is_table(3))
        {
            thread->rawgeti(3, 1);
            auto x = (uint16_t)thread->tointeger(-1);
            thread->remove(-1);

            thread->rawgeti(3, 2);
            auto y = (uint16_t)thread->tointeger(-1);
            thread->remove(-1);

            position = fb::model::point16_t{x, y};
        }
        else if (thread->is_num(3) && thread->is_num(4))
        {
            auto x   = (uint16_t)thread->tointeger(3);
            auto y   = (uint16_t)thread->tointeger(4);
            position = fb::model::point16_t{x, y};
        }
        else
        {
        }

        ctx->threads.enqueue(*obj, [=](auto&) -> async::task<void> {
            co_await static_func(ctx, thread, obj, map, position);
        });
        return thread->yield(1);
    }
    catch (std::exception& e)
    {
        thread->pushstring(e.what());
        return 1;
    }
}

int object::builtin_mkitem(lua_State* lua)
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
        ctx->send(*item, fb::protocol::game::response::update(*item), context::scope::PIVOT);
    }

    return 1;
}

int object::builtin_sight_in(lua_State* lua)
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
    const auto& objects = obj->sight_in(filter);

    for (int i = 0; i < objects.size(); i++)
    {
        thread->pushobject(objects[i]);
        thread->rawseti(-2, uint64_t(i + 1));
    }

    return 1;
}

int object::builtin_nears(lua_State* lua)
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
    auto width  = argc < 3 ? -1 : thread->tointeger(3);
    auto height = argc < 4 ? -1 : thread->tointeger(4);

    thread->new_table();
    const auto& objects = obj->nears(filter);
    for (int i = 0; i < objects.size(); i++)
    {
        if (width != -1 && height != -1)
        {
            if (objects[i]->_position.x < obj->_position.x - width ||
                objects[i]->_position.x > obj->_position.x + width)
                continue;

            if (objects[i]->_position.y < obj->_position.y - height ||
                objects[i]->_position.y > obj->_position.y + height)
                continue;
        }

        thread->pushobject(objects[i]);
        thread->rawseti(-2, uint64_t(i + 1));
    }

    return 1;
}

int object::builtin_front(lua_State* lua)
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

int object::builtin_is(lua_State* lua)
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

int object::builtin_thread(lua_State* lua)
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

int object::builtin_ptr(lua_State* lua)
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

int object::builtin_near(lua_State* lua)
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

int object::builtin_buffs(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto obj     = thread->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    thread->new_table();
    auto i = 0;
    for (auto& [_, buff] : obj->buffs)
    {
        thread->pushinteger(i + 1);
        thread->pushobject(buff);
        lua_settable(lua, -3);

        i++;
    }

    return 1;
}