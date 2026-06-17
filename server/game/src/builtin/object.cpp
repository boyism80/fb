#include <fb/game/builtin/object.h>
#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/game/appearance.h>
#include <async/awaitable_then.h>
#include <async/propagation.h>

using namespace fb::game;
using table = fb::model::table;

// clang-format off
IMPLEMENT_LUA_EXTENSION(object, "fb.game.object")
{"model",               builtin::object::builtin_model},
{"__eq",                builtin::object::builtin_eq},
{"__tostring",          builtin::object::builtin_tostring},
{"destroy",             builtin::object::builtin_destroy},
{"oid",                 builtin::object::builtin_oid},
{"name",                builtin::object::builtin_name},
{"sound",               builtin::object::builtin_sound},
{"position",            builtin::object::builtin_position},
{"front_position",      builtin::object::builtin_front_position},
{"direction",           builtin::object::builtin_direction},
{"chat",                builtin::object::builtin_chat},
{"buff",                builtin::object::builtin_buff},
{"isbuff",              builtin::object::builtin_isbuff},
{"unbuff",              builtin::object::builtin_unbuff},
{"buffs",               builtin::object::builtin_buffs},
{"effect",              builtin::object::builtin_effect},
{"map",                 builtin::object::builtin_map},
{"mkitem",              builtin::object::builtin_mkitem},
{"sight_in",            builtin::object::builtin_sight_in},
{"nears",               builtin::object::builtin_nears},
{"front",               builtin::object::builtin_front},
{"is",                  builtin::object::builtin_is},
{"thread",              builtin::object::builtin_thread},
{"near",                builtin::object::builtin_near},
{"hidden",              builtin::object::builtin_hidden},
{"script",              builtin::object::builtin_script},
{"appearance",          builtin::object::builtin_appearance},
END_LUA_EXTENSION; // clang-format on

int builtin::object::builtin_model(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    auto model_ptr = std::make_shared<const fb::model::object*>();
    auto weak      = obj->weak_from_this_as<fb::game::object>();
    auto builder   = lua->new_co_builder(*server);
    builder.weak   = weak;
    builder.yield  = [=]() -> async::task<void> {
        *model_ptr = &obj->based();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushobject(**model_ptr);
        co_return 1;
    };
    return builder.run();
}

int builtin::object::builtin_oid(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    auto oid      = std::make_shared<uint32_t>();
    auto weak     = obj->weak_from_this_as<fb::game::object>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *oid = obj->oid();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*oid);
        co_return 1;
    };
    return builder.run();
}

int builtin::object::builtin_eq(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto me     = lua->touserdata<fb::game::object>(1);
    if (me == nullptr)
        return 0;

    auto you = lua->touserdata<fb::game::object>(2);
    if (you == nullptr)
        return 0;

    auto result   = std::make_shared<bool>();
    auto weak     = me->weak_from_this_as<fb::game::object>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *result = me->oid() == you->oid();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
}

int builtin::object::builtin_tostring(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto me     = lua->touserdata<fb::game::object>(1);
    if (me == nullptr)
        return 0;

    auto name     = std::make_shared<std::string>();
    auto weak     = me->weak_from_this_as<fb::game::object>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *name = me->name();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushstring(*name);
        co_return 1;
    };
    return builder.run();
}

int builtin::object::builtin_destroy(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    auto weak     = obj->weak_from_this_as<fb::game::object>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        std::ignore = obj->destroy();
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::object::builtin_name(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    auto name     = std::make_shared<std::string>();
    auto weak     = obj->weak_from_this_as<fb::game::object>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *name = obj->name();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushstring(*name);
        co_return 1;
    };
    return builder.run();
}

int builtin::object::builtin_sound(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    auto sound    = static_cast<SOUND>(lua->tointeger(2));
    auto weak     = obj->weak_from_this_as<fb::game::object>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        obj->sound(sound);
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::object::builtin_position(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto position = std::make_shared<fb::model::point16_t>();
        auto weak     = obj->weak_from_this_as<fb::game::object>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *position = obj->position();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(position->x);
            lua->pushinteger(position->y);
            co_return 2;
        };
        return builder.run();
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

        auto weak     = obj->weak_from_this_as<fb::game::object>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->position(x, y, true);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::object::builtin_front_position(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    auto step     = lua->tointeger(2, 1);
    auto position = std::make_shared<fb::model::point16_t>();
    auto weak     = obj->weak_from_this_as<fb::game::object>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *position = obj->front_position(step);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(position->x);
        lua->pushinteger(position->y);
        co_return 2;
    };
    return builder.run();
}

int builtin::object::builtin_direction(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto direction = std::make_shared<DIRECTION>();
        auto weak      = obj->weak_from_this_as<fb::game::object>();
        auto builder   = lua->new_co_builder(*server);
        builder.weak   = weak;
        builder.yield  = [=]() -> async::task<void> {
            *direction = obj->direction();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(static_cast<uint8_t>(*direction));
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto direction = static_cast<DIRECTION>(lua->tointeger(2));
        auto weak      = obj->weak_from_this_as<fb::game::object>();
        auto builder   = lua->new_co_builder(*server);
        builder.weak   = weak;
        builder.yield  = [=]() -> async::task<void> {
            obj->direction(direction);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::object::builtin_chat(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    auto message  = lua->tostring(2);
    auto type     = lua->toenum(3, CHAT_TYPE::NORMAL);
    auto decorate = lua->toboolean(4, true);

    auto weak     = obj->weak_from_this_as<fb::game::object>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        if (obj->is(OBJECT_TYPE::ITEM) == false)
            obj->chat(message, type, decorate);
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::object::builtin_buff(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc   = lua->argc();
    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    const fb::model::spell* model = nullptr;
    if (lua->is_userdata<fb::model::spell>(2))
    {
        model = lua->touserdata<fb::model::spell>(2);
    }
    else if (lua->is_string(2))
    {
        model = table::spell.name2spell(lua->tostring(2));
    }
    else
    {
    }

    if (model == nullptr)
        return 0;

    auto seconds = (uint32_t)lua->tointeger(3);
    auto caster  = lua->touserdata<fb::game::object>(4);

    auto buff_holder = std::make_shared<std::shared_ptr<fb::game::buff>>();
    auto weak        = obj->weak_from_this_as<fb::game::object>();
    auto builder     = lua->new_co_builder(*server);
    builder.weak     = weak;
    builder.yield    = [=]() -> async::task<void> {
        auto buff = obj->buffs.push_back(*model, seconds, caster);
        if (buff != nullptr)
        {
            obj->send(fb::protocol::game::response::spell_buff(*buff));
            *buff_holder = buff;
        }
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*buff_holder == nullptr)
            lua->pushnil();
        else
            lua->pushobject(*buff_holder);
        co_return 1;
    };
    return builder.run();
}

int builtin::object::builtin_unbuff(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    enum class unbuff_mode
    {
        NONE,
        BY_NAME,
        BY_BUFF,
        BY_SPELL,
    };

    auto mode     = unbuff_mode::NONE;
    auto name     = std::string();
    auto spell_id = uint32_t(0);
    auto result   = std::make_shared<bool>(false);
    auto weak     = obj->weak_from_this_as<fb::game::object>();
    auto builder  = lua->new_co_builder(*server);

    if (lua->is_string(2))
    {
        mode = unbuff_mode::BY_NAME;
        name = lua->tostring(2);
    }
    else if (lua->is_userdata<fb::game::buff>(2))
    {
        auto buff = lua->touserdata<fb::game::buff>(2);
        if (buff == nullptr)
            return 0;
        mode     = unbuff_mode::BY_BUFF;
        spell_id = buff->model.id;
    }
    else if (lua->is_userdata<fb::model::spell>(2))
    {
        auto model = lua->touserdata<fb::model::spell>(2);
        if (model == nullptr)
            return 0;
        mode     = unbuff_mode::BY_SPELL;
        spell_id = model->id;
    }

    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        if (mode == unbuff_mode::BY_NAME)
        {
            auto model = table::spell.name2spell(name);
            *result    = model != nullptr && obj->buffs.remove(*model);
        }
        else if (mode == unbuff_mode::BY_BUFF || mode == unbuff_mode::BY_SPELL)
        {
            *result = obj->buffs.remove(spell_id);
        }
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
}

int builtin::object::builtin_isbuff(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    struct buff_check
    {
        enum class type_t
        {
            NAME,
            BUFF,
            SPELL,
        } type;
        std::string name;
        uint32_t    spell_id = 0;
    };

    auto checks = std::vector<buff_check>();
    auto argc   = lua->argc();
    for (int i = 1; i < argc; i++)
    {
        if (lua->is_string(i + 1))
        {
            checks.push_back({buff_check::type_t::NAME, lua->tostring(i + 1), 0});
        }
        else if (lua->is_userdata<fb::game::buff>(i + 1))
        {
            auto buff = lua->touserdata<fb::game::buff>(i + 1);
            if (buff == nullptr)
                continue;
            checks.push_back({buff_check::type_t::BUFF, "", buff->model.id});
        }
        else if (lua->is_userdata<fb::model::spell>(i + 1))
        {
            auto model = lua->touserdata<fb::model::spell>(i + 1);
            if (model == nullptr)
                continue;
            checks.push_back({buff_check::type_t::SPELL, "", model->id});
        }
    }

    auto found    = std::make_shared<bool>(false);
    auto weak     = obj->weak_from_this_as<fb::game::object>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        for (const auto& check : checks)
        {
            if (check.type == buff_check::type_t::NAME)
            {
                auto model = table::spell.name2spell(check.name);
                if (model != nullptr && obj->buffs.contains(*model))
                {
                    *found = true;
                    break;
                }
            }
            else if (obj->buffs.contains(check.spell_id))
            {
                *found = true;
                break;
            }
        }
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*found);
        co_return 1;
    };
    return builder.run();
}

int builtin::object::builtin_effect(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    auto effect   = static_cast<uint8_t>(lua->tointeger(2));
    auto weak     = obj->weak_from_this_as<fb::game::object>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        obj->effect(effect);
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::object::builtin_map(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto map_holder = std::make_shared<std::shared_ptr<fb::game::map>>();
        auto weak       = obj->weak_from_this_as<fb::game::object>();
        auto builder    = lua->new_co_builder(*server);
        builder.weak    = weak;
        builder.yield   = [=]() -> async::task<void> {
            *map_holder = obj->map();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            if (*map_holder == nullptr)
                lua->pushnil();
            else
                lua->pushobject(*map_holder);
            co_return 1;
        };
        return builder.run();
    }

    struct lua_map_callback_ref
    {
        fb::lua::context* lua = nullptr;
        int               ref = LUA_NOREF;

        ~lua_map_callback_ref()
        {
            this->release();
        }

        void release()
        {
            if (this->lua != nullptr && this->ref != LUA_NOREF)
            {
                luaL_unref(*this->lua, LUA_REGISTRYINDEX, this->ref);
                this->ref = LUA_NOREF;
            }
        }
    };

    const auto make_lua_map_callback = [](fb::lua::context*                     lua_ctx,
                                          std::shared_ptr<lua_map_callback_ref> holder,
                                          std::weak_ptr<fb::game::object>       object_weak) -> map_callback {
        return [lua_ctx, holder, object_weak]() -> async::task<bool> {
            auto shared = object_weak.lock();
            if (shared == nullptr)
                co_return false;

            co_await lua_ctx->switching();

            if (holder->ref == LUA_NOREF)
                co_return false;

            lua_rawgeti(*lua_ctx, LUA_REGISTRYINDEX, holder->ref);
            holder->release();

            if (lua_isfunction(*lua_ctx, -1) == false)
            {
                lua_pop(*lua_ctx, 1);
                co_return false;
            }

            lua_ctx->pushobject(*shared);
            if (lua_pcall(*lua_ctx, 1, 0, 0) != LUA_OK)
            {
                const char* raw = lua_tostring(*lua_ctx, -1);
                if (raw != nullptr)
                    fb::logger::warn("lua map callback error : {}", raw);
                lua_pop(*lua_ctx, 1);
                co_return false;
            }
            co_return true;
        };
    };

    const auto is_position_table = [](fb::lua::context* lua_ctx, int index) -> bool {
        if (lua_ctx->is_table(index) == false)
            return false;

        lua_ctx->rawgeti(index, 1);
        auto has_x = lua_ctx->is_number(-1);
        lua_ctx->remove(-1);
        if (has_x == false)
            return false;

        lua_ctx->rawgeti(index, 2);
        auto has_y = lua_ctx->is_number(-1);
        lua_ctx->remove(-1);
        return has_y;
    };

    const auto parse_map_position_table = [](fb::lua::context* lua_ctx, int index) -> fb::model::point16_t {
        lua_ctx->rawgeti(index, 1);
        auto x = (uint16_t)lua_ctx->tointeger(-1);
        lua_ctx->remove(-1);

        lua_ctx->rawgeti(index, 2);
        auto y = (uint16_t)lua_ctx->tointeger(-1);
        lua_ctx->remove(-1);

        return fb::model::point16_t{x, y};
    };

    const auto parse_map_option_table = [&](fb::lua::context*                      lua_ctx,
                                            int                                    index,
                                            map_options&                           opts,
                                            std::shared_ptr<lua_map_callback_ref>& callback_ref,
                                            std::weak_ptr<fb::game::object>        object_weak) {
        if (lua_ctx->is_table(index) == false)
            return;

        ::lua_getfield(*lua_ctx, index, "callback");
        if (lua_ctx->is_function(-1))
        {
            ::lua_pushvalue(*lua_ctx, -1);
            callback_ref      = std::make_shared<lua_map_callback_ref>();
            callback_ref->lua = lua_ctx;
            callback_ref->ref = ::luaL_ref(*lua_ctx, LUA_REGISTRYINDEX);
            opts.callback     = make_lua_map_callback(lua_ctx, callback_ref, object_weak);
        }
        ::lua_pop(*lua_ctx, 1);

        ::lua_getfield(*lua_ctx, index, "notify");
        if (lua_ctx->is_nil(-1) == false)
            opts.notify = lua_ctx->toboolean(-1);
        ::lua_pop(*lua_ctx, 1);
    };

    auto map             = std::shared_ptr<fb::game::map>(nullptr);
    auto position        = std::optional<fb::model::point16_t>{};
    auto options         = map_options{};
    auto weak            = obj->weak_from_this_as<fb::game::object>();
    auto callback_holder = std::shared_ptr<lua_map_callback_ref>{nullptr};

    if (lua->is_userdata<fb::game::map>(2))
    {
        map = lua->touserdata<fb::game::map>(2);
    }
    else if (lua->is_userdata<fb::model::map>(2))
    {
        auto model = lua->touserdata<fb::model::map>(2);
        if (server->maps.contains(model->id))
            map = server->maps[model->id];
    }
    else if (lua->is_number(2))
    {
        auto id = lua->tointeger(2);
        if (server->maps.contains(id))
            map = server->maps[id];
    }
    else if (lua->is_string(2))
    {
        map = server->maps.name2map(lua->tostring(2));
    }

    if (map == nullptr)
    {
        lua->pushstring(_TEXT(MESSAGE_MAP_INVALID));
        return 1;
    }

    auto offset = 3;
    if (argc >= offset)
    {
        if (is_position_table(lua, offset))
        {
            position  = parse_map_position_table(lua, offset);
            offset   += 1;
        }
        else if (lua->is_number(offset) && lua->is_number(offset + 1))
        {
            auto x    = (uint16_t)lua->tointeger(offset);
            auto y    = (uint16_t)lua->tointeger(offset + 1);
            position  = fb::model::point16_t{x, y};
            offset   += 2;
        }
        else if (lua->is_nil(offset))
        {
            offset += 1;
        }
    }

    if (argc >= offset && lua->is_table(offset))
        parse_map_option_table(lua, offset, options, callback_holder, weak);

    if (position.has_value() == false)
        position = map->model.spawn_position();

    auto success_holder = std::make_shared<bool>(false);
    auto builder        = lua->new_co_builder(*server);
    builder.weak        = weak;
    builder.yield       = [=]() -> async::task<void> {
        auto shared = weak.lock();
        if (shared == nullptr)
        {
            if (callback_holder != nullptr)
                callback_holder->release();
            co_return;
        }

        *success_holder = co_await shared->map(map, position, std::move(options));
        if (callback_holder != nullptr && *success_holder == false)
            callback_holder->release();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*success_holder);
        co_return 1;
    };
    return builder.run();
}

int builtin::object::builtin_mkitem(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    auto name  = lua->tostring(2);
    auto model = table::item.name2item(name);
    if (model == nullptr)
    {
        lua->pushnil();
        return 1;
    }

    auto item_holder = std::make_shared<std::shared_ptr<fb::game::item>>();
    auto weak        = obj->weak_from_this_as<fb::game::object>();
    auto builder     = lua->new_co_builder(*server);
    builder.weak     = weak;
    builder.yield    = [=]() -> async::task<void> {
        auto item = model->make(*server);
        item->map(obj->map(), obj->position());
        std::ignore  = server->send(*item, fb::protocol::game::response::update(*item), fb::game::scope::PIVOT);
        *item_holder = item;
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushobject(*item_holder);
        co_return 1;
    };
    return builder.run();
}

int builtin::object::builtin_sight_in(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    auto filter   = static_cast<OBJECT_TYPE>(lua->tointeger(2, (int)OBJECT_TYPE::OBJECT));
    auto objects  = std::make_shared<std::vector<fb::game::object::object_ptr>>();
    auto weak     = obj->weak_from_this_as<fb::game::object>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        for (const auto& entry : obj->sight_in(filter))
            objects->push_back(entry);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->new_table();
        for (size_t i = 0; i < objects->size(); i++)
        {
            lua->pushobject((*objects)[i]);
            lua->rawseti(-2, static_cast<int64_t>(i + 1));
        }
        co_return 1;
    };
    return builder.run();
}

int builtin::object::builtin_nears(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    auto filter = static_cast<OBJECT_TYPE>(lua->tointeger(2, (int)OBJECT_TYPE::OBJECT));

    struct nears_params
    {
        bool                              use_points = false;
        std::vector<fb::model::point16_t> points;
        int                               width  = -1;
        int                               height = -1;
        bool                              all    = true;
    };

    auto params = nears_params{};
    if (argc >= 3 && lua->is_table(3))
    {
        params.use_points = true;
        auto size         = lua->rawlen(3);
        for (int i = 0; i < size; i++)
        {
            lua->rawgeti(3, i + 1);

            lua->rawgeti(-1, 1);
            auto x = lua->tointeger(-1);
            lua->remove(-1);

            lua->rawgeti(-1, 2);
            auto y = lua->tointeger(-1);
            lua->remove(-1);
            params.points.push_back(fb::model::point16_t(x, y));

            lua->remove(-1);
        }
        params.all = lua->toboolean(4, true);
    }
    else
    {
        params.width  = lua->tointeger(3, -1);
        params.height = lua->tointeger(4, -1);
        params.all    = lua->toboolean(5, true);
    }

    auto objects  = std::make_shared<std::vector<fb::game::object::object_ptr>>();
    auto weak     = obj->weak_from_this_as<fb::game::object>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto map = obj->map();
        if (map == nullptr)
            co_return;

        auto        visit = std::set<uint32_t>();
        const auto& nears = obj->nears(filter);
        for (size_t i = 0; i < nears.size(); i++)
        {
            if (params.use_points)
            {
                auto matched = false;
                for (auto& point : params.points)
                {
                    if (nears[i]->position() == point)
                    {
                        matched = true;
                        break;
                    }
                }
                if (!matched)
                    continue;
            }
            else if (params.width != -1 && params.height != -1)
            {
                if (nears[i]->x() < obj->x() - params.width || nears[i]->x() > obj->x() + params.width)
                    continue;
                if (nears[i]->y() < obj->y() - params.height || nears[i]->y() > obj->y() + params.height)
                    continue;
            }

            if (!params.all)
            {
                auto index = map->index(nears[i]->position());
                if (visit.contains(index))
                    continue;
                visit.insert(index);
            }

            objects->push_back(nears[i]);
        }
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->new_table();
        for (size_t i = 0; i < objects->size(); i++)
        {
            lua->pushobject((*objects)[i]);
            lua->rawseti(-2, static_cast<int64_t>(i + 1));
        }
        co_return 1;
    };
    return builder.run();
}

int builtin::object::builtin_front(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    auto filter   = lua->toenum(2, OBJECT_TYPE::OBJECT);
    auto front    = std::make_shared<fb::game::object::object_ptr>();
    auto weak     = obj->weak_from_this_as<fb::game::object>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *front = obj->forward(filter);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*front == nullptr)
            lua->pushnil();
        else
            lua->pushobject(*front);
        co_return 1;
    };
    return builder.run();
}

int builtin::object::builtin_is(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    auto type     = lua->toenum(2, OBJECT_TYPE::OBJECT);
    auto matched  = std::make_shared<bool>();
    auto weak     = obj->weak_from_this_as<fb::game::object>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *matched = obj->is(type);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*matched);
        co_return 1;
    };
    return builder.run();
}

int builtin::object::builtin_thread(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    auto thread_holder = std::make_shared<fb::thread*>();
    auto weak          = obj->weak_from_this_as<fb::game::object>();
    auto builder       = lua->new_co_builder(*server);
    builder.weak       = weak;
    builder.yield      = [=]() -> async::task<void> {
        *thread_holder = obj->thread();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*thread_holder == nullptr)
            lua->pushnil();
        else
            lua->pushobject(*thread_holder);
        co_return 1;
    };
    return builder.run();
}

int builtin::object::builtin_near(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    auto you      = lua->touserdata<fb::game::object>(2);
    auto found    = std::make_shared<bool>(false);
    auto weak     = obj->weak_from_this_as<fb::game::object>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto map = obj->map();
        if (map == nullptr)
            co_return;

        auto nears = map->nears(obj->position());
        for (const auto& x : nears)
        {
            if (you == x)
            {
                *found = true;
                break;
            }
        }
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*found);
        co_return 1;
    };
    return builder.run();
}

int builtin::object::builtin_buffs(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    auto buffs    = std::make_shared<std::vector<std::shared_ptr<fb::game::buff>>>();
    auto weak     = obj->weak_from_this_as<fb::game::object>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        for (auto& [_, buff] : obj->buffs)
            buffs->push_back(buff);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->new_table();
        for (size_t i = 0; i < buffs->size(); i++)
        {
            lua->pushinteger(static_cast<int>(i + 1));
            lua->pushobject((*buffs)[i]);
            lua_settable(L, -3);
        }
        co_return 1;
    };
    return builder.run();
}

int builtin::object::builtin_hidden(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto me     = lua->touserdata<fb::game::object>(1);
    if (me == nullptr)
        return 0;

    auto you = lua->touserdata<fb::game::object>(2);
    if (you == nullptr)
        return 0;

    auto result   = std::make_shared<bool>();
    auto weak     = me->weak_from_this_as<fb::game::object>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *result = me->hidden(*you);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
}

int builtin::object::builtin_script(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    auto file = lua->tostring(2, "scripts/script.lua");
    auto func = lua->tostring(3, "func");
    auto weak = obj->weak_from_this_as<fb::game::object>();

    struct script_child
    {
        fb::lua::context* ctx = nullptr;
    };

    auto child_holder   = std::make_shared<script_child>();
    auto retc_holder    = std::make_shared<int>(0);
    auto obj_holder     = std::make_shared<std::shared_ptr<fb::game::object>>();
    auto on_same_thread = std::make_shared<bool>(obj->thread() == server->threads.current());

    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto shared = weak.lock();
        if (shared == nullptr)
            co_return;

        *obj_holder = shared;

        auto new_lua = fb::lua::new_context(lua,
                                            {
                                                .auto_release       = false,
                                                .auto_resume_parent = false,
                                            });
        if (new_lua == nullptr)
            co_return;

        if (*on_same_thread)
            new_lua->load(file);
        else
            new_lua->load(std::format("scripts/{}", file));

        new_lua->func(func);
        new_lua->pushobject(*obj_holder);
        lua_xmove(*lua, *new_lua, argc - 3);

        child_holder->ctx = new_lua;
        co_await new_lua->call(argc - 2, retc_holder.get());
    };
    builder.resume = [=]() -> async::task<int> {
        auto new_lua = child_holder->ctx;
        if (new_lua == nullptr)
            co_return 0;

        auto n = *retc_holder;
        lua_xmove(*new_lua, *lua, n);
        new_lua->release();
        child_holder->ctx = nullptr;
        co_return n;
    };
    return builder.run();
}

int builtin::object::builtin_appearance(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::object>(1);
    if (obj == nullptr)
        return 0;

    auto appearance_holder = std::make_shared<std::shared_ptr<fb::game::appearance>>();
    auto weak              = obj->weak_from_this_as<fb::game::object>();
    auto builder           = lua->new_co_builder(*server);
    builder.weak           = weak;
    builder.yield          = [=]() -> async::task<void> {
        *appearance_holder = obj->appearance();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        (*appearance_holder)->to_lua(lua);
        co_return 1;
    };
    return builder.run();
}