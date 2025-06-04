#include <fb/game/object.h>
#include <fb/game/context.h>
#include <fb/game/character.h>

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

int object::builtin_model(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    obj->assert_thread();

    auto& model = obj->based();
    lua->pushobject(model);
    return 1;
}

int object::builtin_id(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    obj->assert_thread();

    lua->pushinteger(obj->sequence());
    return 1;
}

int object::builtin_eq(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto me   = lua->touserdata<object>(1);
    if (me == nullptr)
        return 0;
    me->assert_thread();

    auto you = lua->touserdata<object>(2);
    if (you == nullptr)
        return 0;
    you->assert_thread();

    lua->pushboolean(me->sequence() == you->sequence());
    return 1;
}

int object::builtin_tostring(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto me  = lua->touserdata<object>(1);
    if (me == nullptr)
        return 0;
    me->assert_thread();

    lua->pushstring(me->name());
    return 1;
}

int object::builtin_name(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    obj->assert_thread();

    lua->pushstring(obj->name());
    return 1;
}

int object::builtin_sound(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto sound = static_cast<SOUND>(lua->tointeger(2));
    return ctx->builtin(*obj, lua, 0, [=]() {
        obj->sound(sound);
    });
}

int object::builtin_position(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto n = (argc == 1 ? 2 : 0);
    return ctx->builtin(*obj, lua, n, [=]() {
        if (argc == 1)
        {
            lua->pushinteger(obj->_position.x);
            lua->pushinteger(obj->_position.y);
        }
        else
        {
            uint16_t x, y;
            if (lua->is_table(2))
            {
                lua->rawgeti(2, 1);
                x = (uint16_t)lua->tointeger(-1);
                lua->remove(-1);

                lua->rawgeti(2, 2);
                y = (uint16_t)lua->tointeger(-1);
                lua->remove(-1);
            }
            else
            {
                x = (uint16_t)lua->tointeger(2);
                y = (uint16_t)lua->tointeger(3);
            }

            obj->position(x, y, true);
        }
    });
}

int object::builtin_front_position(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto step = lua->tointeger(2, 1);
    obj->assert_thread();

    return ctx->builtin(*obj, lua, 2, [=]() {
        auto position = obj->front_position(step);
        lua->pushinteger(position.x);
        lua->pushinteger(position.y);
    });
}

int object::builtin_direction(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto direction = static_cast<DIRECTION>(lua->tointeger(2));
    auto n         = (argc == 1 ? 1 : 0);
    return ctx->builtin(*obj, lua, n, [=]() {
        if (argc == 1)
            lua->pushinteger(obj->_direction);
        else
            obj->direction(direction);
    });
}

int object::builtin_chat(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto message  = lua->tostring(2);
    auto type     = lua->toenum(3, CHAT_TYPE::NORMAL);
    auto decorate = lua->toboolean(4, true);

    return ctx->builtin(*obj, lua, 0, [=]() {
        if (obj->is(OBJECT_TYPE::ITEM) == false)
            obj->chat(message, type, decorate);
    });
}

int object::builtin_buff(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto ctx  = lua->env<fb::game::context>("context");
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    if (ctx->alive(*obj) == false)
        return 0;

    const fb::model::spell* model = nullptr;
    switch (lua_type(L, 2))
    {
    case LUA_TUSERDATA:
        model = lua->touserdata<fb::model::spell>(2);
        break;

    case LUA_TSTRING:
        model = ctx->model.spell.name2spell(lua->tostring(2));
        break;

    default:
        return 0;
    }

    if (model == nullptr)
        return 0;

    auto seconds = (uint32_t)lua->tointeger(3);
    auto caster  = lua->touserdata<fb::game::object>(4);
    auto buff    = obj->buffs.push_back(*model, seconds, caster);
    if (buff == nullptr)
    {
        lua->pushnil();
        return 1;
    }

    obj->send(fb::protocol::game::response::spell_buff(*buff));
    return 1;
}

int object::builtin_unbuff(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    obj->assert_thread();

    if (lua->is_str(2))
    {
        auto name  = lua->tostring(2);
        auto model = ctx->model.spell.name2spell(name);
        if (model == nullptr)
            lua->pushboolean(false);
        else
            lua->pushboolean(obj->buffs.remove(*model));
    }
    else if (lua->is_userdata<fb::game::buff>(2))
    {
        auto buff = lua->touserdata<fb::game::buff>(2);
        if (buff == nullptr)
            return 0;

        lua->pushboolean(obj->buffs.remove(buff->model.id));
    }
    else if (lua->is_userdata<fb::model::spell>(2))
    {
        auto model = lua->touserdata<fb::model::spell>(2);
        if (model == nullptr)
            return 0;

        lua->pushboolean(obj->buffs.remove(model->id));
    }
    else
    {
        lua->pushboolean(false);
    }

    return 1;
}

int object::builtin_isbuff(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    obj->assert_thread();

    auto argc = lua->argc();
    for (int i = 1; i < argc; i++)
    {
        if (lua->is_str(i + 1))
        {
            auto name  = lua->tostring(i + 1);
            auto model = ctx->model.spell.name2spell(name);
            if (model == nullptr)
                continue;

            if (obj->buffs.contains(*model) == false)
                continue;

            lua->pushboolean(true);
            return 1;
        }

        if (lua->is_userdata<fb::game::buff>(i + 1))
        {
            auto buff = lua->touserdata<fb::game::buff>(i + 1);
            if (buff == nullptr)
                continue;

            if (obj->buffs.contains(buff->model.id) == false)
                continue;

            lua->pushboolean(true);
            return 1;
        }

        if (lua->is_userdata<fb::model::spell>(i + 1))
        {
            auto model = lua->touserdata<fb::model::spell>(i + 1);
            if (model == nullptr)
                continue;

            if (obj->buffs.contains(model->id) == false)
                continue;

            lua->pushboolean(true);
            return 1;
        }
    }

    lua->pushboolean(false);
    return 1;
}

int object::builtin_effect(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto effect = static_cast<uint8_t>(lua->tointeger(2));
    return ctx->builtin(*obj, lua, 0, [=]() {
        obj->effect(effect);
    });
}

int object::builtin_map(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto map      = (fb::game::map*)nullptr;
    auto position = std::optional<fb::model::point16_t>{};
    if (argc > 1)
    {
        if (lua->is_userdata<fb::game::map>(2))
        {
            map = lua->touserdata<fb::game::map>(2);
        }
        else if (lua->is_userdata<fb::model::map>(2))
        {
            auto model = lua->touserdata<fb::model::map>(2);
            if (ctx->maps.contains(model->id))
                map = &ctx->maps[model->id];
        }
        else if (lua->is_num(2))
        {
            auto id = lua->tointeger(2);
            if (ctx->maps.contains(id))
                map = &ctx->maps[id];
        }
        else if (lua->is_str(2))
        {
            map = ctx->maps.name2map(lua->tostring(2));
        }
        else
        {
        }

        if (map == nullptr)
        {
            lua->pushstring("올바르지 않은 맵입니다.");
            return 1;
        }

        if (lua->is_table(3))
        {
            lua->rawgeti(3, 1);
            auto x = (uint16_t)lua->tointeger(-1);
            lua->remove(-1);

            lua->rawgeti(3, 2);
            auto y = (uint16_t)lua->tointeger(-1);
            lua->remove(-1);

            position = fb::model::point16_t{x, y};
        }
        else if (lua->is_num(3) && lua->is_num(4))
        {
            auto x   = (uint16_t)lua->tointeger(3);
            auto y   = (uint16_t)lua->tointeger(4);
            position = fb::model::point16_t{x, y};
        }
        else
        {
        }
    }

    static auto static_func = [](object*                                    obj,
                                 fb::game::map*                             map,
                                 const std::optional<fb::model::point16_t>& position,
                                 fb::lua::context*                          lua) -> async::task<void> {
        if (position.has_value())
        {
            if (co_await obj->map(map, position.value()) == false)
                lua->pushboolean(false);
            else
                lua->pushboolean(true);
        }
        else
        {
            if (co_await obj->map(map) == false)
                lua->pushboolean(false);
            else
                lua->pushboolean(true);
        }
    };

    if (argc == 1)
    {
        return ctx->builtin(*obj, lua, 1, [=]() {
            auto map = obj->map();
            if (map == nullptr)
                lua->pushnil();
            else
                lua->pushobject(map);
        });
    }
    else
    {
        auto no_async = false;
        if (obj->_map == nullptr)
            no_async = false;
        else if (obj->_map->model.host != map->model.host)
            no_async = false;
        else if (obj->_map->thread() != map->thread())
            no_async = false;
        else
            no_async = true;

        if (no_async)
        {
            auto task = static_func(obj, map, position, lua);

            return ctx->builtin(*obj, lua, 1, [=]() {
                async::awaitable_get(static_func(obj, map, position, lua));
            });
        }
        else
        {
            return ctx->builtin_async(*obj, lua, 1, [=]() -> async::task<void> {
                co_await static_func(obj, map, position, lua);
            });
        }
    }
}

int object::builtin_mkitem(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    obj->assert_thread();

    auto name  = lua->tostring(2);
    auto model = ctx->model.item.name2item(name);
    if (model == nullptr)
    {
        lua->pushnil();
    }
    else
    {
        auto ctx  = lua->env<fb::game::context>("context");
        auto item = model->make(*ctx);
        item->map(obj->_map, obj->_position);
        lua->pushobject(item);
        ctx->send(*item, fb::protocol::game::response::update(*item), fb::game::scope::PIVOT);
    }

    return 1;
}

int object::builtin_sight_in(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    obj->assert_thread();

    auto filter = OBJECT_TYPE(lua->tointeger(2, (int)OBJECT_TYPE::UNKNOWN));

    lua->new_table();
    const auto& objects = obj->sight_in(filter);

    for (int i = 0; i < objects.size(); i++)
    {
        lua->pushobject(objects[i]);
        lua->rawseti(-2, uint64_t(i + 1));
    }

    return 1;
}

int object::builtin_nears(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    obj->assert_thread();
    auto map = obj->map();
    if (map == nullptr)
        return 0;

    auto visit  = std::set<uint32_t>();
    auto filter = OBJECT_TYPE(lua->tointeger(2, (int)OBJECT_TYPE::UNKNOWN));
    if (argc >= 3 && lua_type(L, 3) == LUA_TTABLE)
    {
        auto size   = lua->rawlen(3);
        auto points = std::vector<fb::model::point16_t>{};
        for (int i = 0; i < size; i++)
        {
            lua->rawgeti(3, i + 1);

            lua->rawgeti(-1, 1);
            auto x = lua->tointeger(-1);
            lua->remove(-1);

            lua->rawgeti(-1, 2);
            auto y = lua->tointeger(-1);
            lua->remove(-1);
            points.push_back(fb::model::point16_t(x, y));

            lua->remove(-1);
        }

        auto all = lua->toboolean(4, true);
        lua->new_table();
        const auto& objects = obj->nears(filter);
        for (int i = 0; i < objects.size(); i++)
        {
            auto matched = false;
            for (auto& point : points)
            {
                if (objects[i]->_position == point)
                {
                    matched = true;
                    break;
                }
            }

            if (!matched)
                continue;

            if (!all)
            {
                auto index = map->index(objects[i]->position());
                if (visit.contains(index))
                    continue;

                visit.insert(index);
            }

            lua->pushobject(objects[i]);
            lua->rawseti(-2, uint64_t(i + 1));
        }
    }
    else
    {
        auto width  = lua->tointeger(3, -1);
        auto height = lua->tointeger(4, -1);
        auto all    = lua->toboolean(5, true);

        lua->new_table();
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

            if (!all)
            {
                auto index = map->index(objects[i]->position());
                if (visit.contains(index))
                    continue;

                visit.insert(index);
            }

            lua->pushobject(objects[i]);
            lua->rawseti(-2, uint64_t(i + 1));
        }
    }

    return 1;
}

int object::builtin_front(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    obj->assert_thread();

    auto filter = lua->toenum(2, OBJECT_TYPE::UNKNOWN);
    auto front  = obj->forward(filter);
    if (front == nullptr)
        lua->pushnil();
    else
        lua->pushobject(front);

    return 1;
}

int object::builtin_is(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    return ctx->builtin(*obj, lua, 1, [=]() {
        auto type = lua->tointeger(2);
        lua->pushboolean(obj->is(OBJECT_TYPE(type)));
    });
}

int object::builtin_thread(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    obj->assert_thread();

    auto active_thread = obj->thread();
    if (active_thread == nullptr)
        lua->pushnil();
    else
        lua->pushobject(active_thread);

    return 1;
}

int object::builtin_ptr(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    obj->assert_thread();
    lua->pushinteger((uint64_t)(void*)obj);
    return 1;
}

int object::builtin_near(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    obj->assert_thread();
    if (obj->_map == nullptr)
    {
        lua->pushboolean(false);
        return 1;
    }

    auto you   = lua->touserdata<object>(2);
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
    lua->pushboolean(found);
    return 1;
}

int object::builtin_buffs(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    lua->new_table();
    auto i = 0;
    for (auto& [_, buff] : obj->buffs)
    {
        lua->pushinteger(i + 1);
        lua->pushobject(buff);
        lua_settable(L, -3);

        i++;
    }

    return 1;
}