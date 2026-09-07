#include <fb/game/server.h>
#include <fb/game/matchmaker.h>
#include <fb/game/builtin/matchmaker.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(matchmaker, "fb.game.matchmaker")
{"mu",                 builtin::matchmaker::builtin_mu},
{"sigma",              builtin::matchmaker::builtin_sigma},
{"enrolled",           builtin::matchmaker::builtin_enrolled},
{"registry_id",        builtin::matchmaker::builtin_registry_id},
{"pending_match_id",   builtin::matchmaker::builtin_pending_match_id},
{"register",           builtin::matchmaker::builtin_register},
{"unregister",         builtin::matchmaker::builtin_unregister},
{"confirm",            builtin::matchmaker::builtin_confirm},
{"decline",            builtin::matchmaker::builtin_decline},
END_LUA_EXTENSION; // clang-format on

int builtin::matchmaker::builtin_mu(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto mm = lua->touserdata<fb::game::matchmaker>(1);
    if (mm == nullptr)
        return 0;

    auto match_type = static_cast<uint32_t>(lua->tointeger(2));
    auto weak       = mm->owner.weak_from_this_as<fb::game::character>();
    auto mu         = std::make_shared<std::optional<double>>();
    auto builder    = lua->new_co_builder();
    builder.weak    = weak;
    builder.yield   = [=]() -> async::task<void> {
        auto self = weak.lock();
        if (self == nullptr)
            co_return;

        auto skill = self->matchmaker.get(match_type);
        if (skill.has_value())
            *mu = skill->mu;
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (mu->has_value())
            lua->pushnumber(mu->value());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
}

int builtin::matchmaker::builtin_sigma(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto mm = lua->touserdata<fb::game::matchmaker>(1);
    if (mm == nullptr)
        return 0;

    auto match_type = static_cast<uint32_t>(lua->tointeger(2));
    auto weak       = mm->owner.weak_from_this_as<fb::game::character>();
    auto sigma      = std::make_shared<std::optional<double>>();
    auto builder    = lua->new_co_builder();
    builder.weak    = weak;
    builder.yield   = [=]() -> async::task<void> {
        auto self = weak.lock();
        if (self == nullptr)
            co_return;

        auto skill = self->matchmaker.get(match_type);
        if (skill.has_value())
            *sigma = skill->sigma;
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (sigma->has_value())
            lua->pushnumber(sigma->value());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
}

int builtin::matchmaker::builtin_enrolled(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto mm = lua->touserdata<fb::game::matchmaker>(1);
    if (mm == nullptr)
        return 0;

    auto weak     = mm->owner.weak_from_this_as<fb::game::character>();
    auto enrolled = std::make_shared<bool>(false);
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto self = weak.lock();
        if (self == nullptr)
            co_return;

        *enrolled = self->matchmaker.enrolled();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*enrolled);
        co_return 1;
    };
    return builder.run();
}

int builtin::matchmaker::builtin_registry_id(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto mm = lua->touserdata<fb::game::matchmaker>(1);
    if (mm == nullptr)
        return 0;

    auto weak        = mm->owner.weak_from_this_as<fb::game::character>();
    auto registry_id = std::make_shared<std::optional<std::string>>();
    auto builder     = lua->new_co_builder();
    builder.weak     = weak;
    builder.yield    = [=]() -> async::task<void> {
        auto self = weak.lock();
        if (self == nullptr)
            co_return;

        auto id = self->matchmaker.registry_id();
        if (id.has_value())
            registry_id->emplace(id.value());
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (registry_id->has_value())
            lua->pushstring(registry_id->value());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
}

int builtin::matchmaker::builtin_pending_match_id(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto mm = lua->touserdata<fb::game::matchmaker>(1);
    if (mm == nullptr)
        return 0;

    auto weak             = mm->owner.weak_from_this_as<fb::game::character>();
    auto pending_match_id = std::make_shared<std::optional<std::string>>();
    auto builder          = lua->new_co_builder();
    builder.weak          = weak;
    builder.yield         = [=]() -> async::task<void> {
        auto self = weak.lock();
        if (self == nullptr)
            co_return;

        auto& id = self->matchmaker.pending_match_id();
        if (id.has_value())
            pending_match_id->emplace(*id);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (pending_match_id->has_value())
            lua->pushstring(pending_match_id->value().c_str());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
}

int builtin::matchmaker::builtin_register(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto mm = lua->touserdata<fb::game::matchmaker>(1);
    if (mm == nullptr)
        return 0;

    auto match_type = static_cast<uint32_t>(lua->tointeger(2));
    auto weak       = mm->owner.weak_from_this_as<fb::game::character>();
    auto error      = std::make_shared<std::optional<std::string>>();
    auto builder    = lua->new_co_builder();
    builder.weak    = weak;
    builder.yield   = [=]() -> async::task<void> {
        auto self = weak.lock();
        if (self == nullptr)
            co_return;

        try
        {
            co_await self->matchmaker.register_queue(match_type);
        }
        catch (const std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
            lua->pushstring(error->value().c_str());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
}

int builtin::matchmaker::builtin_unregister(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto mm = lua->touserdata<fb::game::matchmaker>(1);
    if (mm == nullptr)
        return 0;

    auto weak     = mm->owner.weak_from_this_as<fb::game::character>();
    auto error    = std::make_shared<std::optional<std::string>>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto self = weak.lock();
        if (self == nullptr)
            co_return;

        try
        {
            co_await self->matchmaker.unregister_queue(false);
        }
        catch (const std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
            lua->pushstring(error->value().c_str());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
}

int builtin::matchmaker::builtin_confirm(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto mm = lua->touserdata<fb::game::matchmaker>(1);
    if (mm == nullptr)
        return 0;

    auto weak     = mm->owner.weak_from_this_as<fb::game::character>();
    auto error    = std::make_shared<std::optional<std::string>>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto self = weak.lock();
        if (self == nullptr)
            co_return;

        auto& pending = self->matchmaker.pending_match_id();
        if (!pending.has_value() || pending->empty())
        {
            *error = "no pending match to confirm";
            co_return;
        }

        auto match_id = *pending;
        try
        {
            co_await self->matchmaker.confirm_queue(std::move(match_id), false);
        }
        catch (const std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
            lua->pushstring(error->value().c_str());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
}

int builtin::matchmaker::builtin_decline(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto mm = lua->touserdata<fb::game::matchmaker>(1);
    if (mm == nullptr)
        return 0;

    auto weak     = mm->owner.weak_from_this_as<fb::game::character>();
    auto error    = std::make_shared<std::optional<std::string>>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto self = weak.lock();
        if (self == nullptr)
            co_return;

        auto& pending = self->matchmaker.pending_match_id();
        if (!pending.has_value() || pending->empty())
        {
            *error = "no pending match to decline";
            co_return;
        }

        auto match_id = *pending;
        try
        {
            co_await self->matchmaker.decline_queue(std::move(match_id), false);
        }
        catch (const std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
            lua->pushstring(error->value().c_str());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
}
