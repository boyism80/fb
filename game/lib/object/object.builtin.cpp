#include <fb/game/object.h>
#include <fb/game/context.h>
#include <fb/game/character.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(object, "fb.game.object")
{"model",               object::builtin::builtin_model},
{"__eq",                object::builtin::builtin_eq},
{"__tostring",          object::builtin::builtin_tostring},
{"id",                  object::builtin::builtin_id},
{"name",                object::builtin::builtin_name},
{"sound",               object::builtin::builtin_sound},
{"position",            object::builtin::builtin_position},
{"front_position",      object::builtin::builtin_front_position},
{"direction",           object::builtin::builtin_direction},
{"chat",                object::builtin::builtin_chat},
{"buff",                object::builtin::builtin_buff},
{"isbuff",              object::builtin::builtin_isbuff},
{"unbuff",              object::builtin::builtin_unbuff},
{"buffs",               object::builtin::builtin_buffs},
{"effect",              object::builtin::builtin_effect},
{"map",                 object::builtin::builtin_map},
{"mkitem",              object::builtin::builtin_mkitem},
{"sight_in",            object::builtin::builtin_sight_in},
{"nears",               object::builtin::builtin_nears},
{"front",               object::builtin::builtin_front},
{"is",                  object::builtin::builtin_is},
{"thread",              object::builtin::builtin_thread},
{"near",                object::builtin::builtin_near},
{"hidden",              object::builtin::builtin_hidden},
END_LUA_EXTENSION; // clang-format on

int object::builtin::builtin_model(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    auto& model = obj->based();
    lua->pushobject(model);
    return 1;
}

int object::builtin::builtin_id(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    lua->pushinteger(obj->sequence());
    return 1;
}

int object::builtin::builtin_eq(lua_State* L)
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

int object::builtin::builtin_tostring(lua_State* L)
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

int object::builtin::builtin_name(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    lua->pushstring(obj->name());
    return 1;
}

int object::builtin::builtin_sound(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    auto sound = static_cast<SOUND>(lua->tointeger(2));
    auto weak  = obj->weak_from_this_as<object>();
    return lua->ensure_yield(*ctx, weak, [=]() {
        obj->sound(sound);
        return lua->ensure_resume(*ctx, weak, [=]() {
            return 0;
        });
    });
}

int object::builtin::builtin_position(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this_as<object>();
        return lua->ensure_yield(*ctx, weak, [=]() {
            lua->pushinteger(obj->_position.x);
            lua->pushinteger(obj->_position.y);
            return lua->ensure_resume(*ctx, weak, [=]() {
                return 2;
            });
        });
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

        auto weak = obj->weak_from_this_as<object>();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->position(x, y, true);
            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int object::builtin::builtin_front_position(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    auto step = lua->tointeger(2, 1);
    obj->assert_thread();

    auto weak = obj->weak_from_this_as<object>();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto position = obj->front_position(step);
        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushinteger(position.x);
            lua->pushinteger(position.y);
            return 2;
        });
    });
}

int object::builtin::builtin_direction(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this_as<object>();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto direction = obj->_direction;
            return lua->ensure_resume(*ctx, weak, [=]() {
                lua->pushinteger(direction);
                return 1;
            });
        });
    }
    else
    {
        auto direction = static_cast<DIRECTION>(lua->tointeger(2));
        auto weak      = obj->weak_from_this_as<object>();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->direction(direction);
            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int object::builtin::builtin_chat(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    auto message  = lua->tostring(2);
    auto type     = lua->toenum(3, CHAT_TYPE::NORMAL);
    auto decorate = lua->toboolean(4, true);

    auto weak = obj->weak_from_this_as<object>();
    return lua->ensure_yield(*ctx, weak, [=]() {
        if (obj->is(OBJECT_TYPE::ITEM) == false)
            obj->chat(message, type, decorate);
        return lua->ensure_resume(*ctx, weak, [=]() {
            return 0;
        });
    });
}

int object::builtin::builtin_buff(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto ctx  = lua->env<fb::game::context>("context");
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    const fb::model::spell* model = nullptr;
    if (lua->is_userdata<fb::model::spell>(2))
    {
        model = lua->touserdata<fb::model::spell>(2);
    }
    else if (lua->is_string(2))
    {
        model = ctx->model.spell.name2spell(lua->tostring(2));
    }
    else
    {
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

int object::builtin::builtin_unbuff(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    if (lua->is_string(2))
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

int object::builtin::builtin_isbuff(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    auto argc = lua->argc();
    for (int i = 1; i < argc; i++)
    {
        if (lua->is_string(i + 1))
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

int object::builtin::builtin_effect(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    auto effect = static_cast<uint8_t>(lua->tointeger(2));
    auto weak   = obj->weak_from_this_as<object>();
    return lua->ensure_yield(*ctx, weak, [=]() {
        obj->effect(effect);
        return lua->ensure_resume(*ctx, weak, [=]() {
            return 0;
        });
    });
}

int object::builtin::builtin_map(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    auto map      = std::shared_ptr<fb::game::map>(nullptr);
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
                map = ctx->maps[model->id];
        }
        else if (lua->is_number(2))
        {
            auto id = lua->tointeger(2);
            if (ctx->maps.contains(id))
                map = ctx->maps[id];
        }
        else if (lua->is_string(2))
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
        else if (lua->is_number(3) && lua->is_number(4))
        {
            auto x   = (uint16_t)lua->tointeger(3);
            auto y   = (uint16_t)lua->tointeger(4);
            position = fb::model::point16_t{x, y};
        }
        else
        {
        }
    }

    static auto static_func = [](std::weak_ptr<object>                      weak,
                                 std::shared_ptr<fb::game::map>             map,
                                 const std::optional<fb::model::point16_t>& position) -> async::task<bool> {
        auto shared = weak.lock();
        if (shared == nullptr)
            co_return false;

        if (position.has_value())
            co_return co_await shared->map(map, position.value());
        else
            co_return co_await shared->map(map);
    };

    if (argc == 1)
    {
        auto weak = obj->weak_from_this_as<object>();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto map = obj->map();
            return lua->ensure_resume(*ctx, weak, [=]() {
                if (map == nullptr)
                    lua->pushnil();
                else
                    lua->pushobject(map);
                return 1;
            });
        });
    }
    else
    {
        auto weak = obj->weak_from_this_as<object>();
        ctx->threads.enqueue(weak, [=](auto& thread) -> async::task<void> {
            async::awaitable_then(static_func(weak, map, position), [=](auto result) {
                auto success = result();
                lua->ensure_resume(
                    *ctx,
                    weak,
                    [=]() {
                        lua->pushboolean(success);
                        return 1;
                    },
                    true);
            });
            co_return;
        });

        return lua->yield(0);
    }
}

int object::builtin::builtin_mkitem(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<object>(1);
    if (obj == nullptr)
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

int object::builtin::builtin_sight_in(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr)
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

int object::builtin::builtin_nears(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();
    auto map = obj->map();
    if (map == nullptr)
        return 0;

    auto visit  = std::set<uint32_t>();
    auto filter = OBJECT_TYPE(lua->tointeger(2, (int)OBJECT_TYPE::UNKNOWN));
    if (argc >= 3 && lua->is_table(3))
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

int object::builtin::builtin_front(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr)
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

int object::builtin::builtin_is(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this_as<object>();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto type = lua->tointeger(2);
        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushboolean(obj->is(OBJECT_TYPE(type)));
            return 1;
        });
    });
}

int object::builtin::builtin_thread(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    auto active_thread = obj->thread();
    if (active_thread == nullptr)
        lua->pushnil();
    else
        lua->pushobject(active_thread);

    return 1;
}

int object::builtin::builtin_near(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<object>(1);
    if (obj == nullptr)
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

int object::builtin::builtin_buffs(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
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

int object::builtin::builtin_hidden(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto me  = lua->touserdata<fb::game::object>(1);
    if (me == nullptr)
        return 0;

    auto you = lua->touserdata<fb::game::object>(2);
    if (you == nullptr)
        return 0;

    lua->pushboolean(me->hidden(*you));
    return 1;
}