#include <fb/game/builtin/matchmaker.h>

#include <fb/game/matchmaker.h>
#include <fb/game/server.h>

#include <cstdint>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <utility>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(matchmaker, "fb.game.matchmaker")
{"mu",                 builtin::matchmaker::builtin_mu},
{"sigma",              builtin::matchmaker::builtin_sigma},
{"queued",             builtin::matchmaker::builtin_queued},
{"ticket_id",          builtin::matchmaker::builtin_ticket_id},
{"pending_match_id",   builtin::matchmaker::builtin_pending_match_id},
{"enqueue",            builtin::matchmaker::builtin_enqueue},
{"dequeue",            builtin::matchmaker::builtin_dequeue},
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

int builtin::matchmaker::builtin_queued(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto mm = lua->touserdata<fb::game::matchmaker>(1);
    if (mm == nullptr)
        return 0;

    auto weak     = mm->owner.weak_from_this_as<fb::game::character>();
    auto queued   = std::make_shared<bool>(false);
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto self = weak.lock();
        if (self == nullptr)
            co_return;

        *queued = self->matchmaker.queued();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*queued);
        co_return 1;
    };
    return builder.run();
}

int builtin::matchmaker::builtin_ticket_id(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto mm = lua->touserdata<fb::game::matchmaker>(1);
    if (mm == nullptr)
        return 0;

    auto weak      = mm->owner.weak_from_this_as<fb::game::character>();
    auto ticket_id = std::make_shared<std::optional<uint64_t>>();
    auto builder   = lua->new_co_builder();
    builder.weak   = weak;
    builder.yield  = [=]() -> async::task<void> {
        auto self = weak.lock();
        if (self == nullptr)
            co_return;

        *ticket_id = self->matchmaker.ticket_id();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (ticket_id->has_value())
            lua->pushinteger(static_cast<lua_Integer>(ticket_id->value()));
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
    auto pending_match_id = std::make_shared<std::optional<uint64_t>>();
    auto builder          = lua->new_co_builder();
    builder.weak          = weak;
    builder.yield         = [=]() -> async::task<void> {
        auto self = weak.lock();
        if (self == nullptr)
            co_return;

        *pending_match_id = self->matchmaker.pending_match_id();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (pending_match_id->has_value())
            lua->pushinteger(static_cast<lua_Integer>(pending_match_id->value()));
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
}

int builtin::matchmaker::builtin_enqueue(lua_State* L)
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
            co_await self->matchmaker.enqueue(match_type);
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

int builtin::matchmaker::builtin_dequeue(lua_State* L)
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
            co_await self->matchmaker.dequeue(fb::game::matchmaker::initiator::USER);
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

        auto pending = self->matchmaker.pending_match_id();
        if (pending.has_value() == false)
        {
            *error = "no pending match to confirm";
            co_return;
        }

        try
        {
            co_await self->matchmaker.confirm(pending.value());
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

        auto pending = self->matchmaker.pending_match_id();
        if (pending.has_value() == false)
        {
            *error = "no pending match to decline";
            co_return;
        }

        try
        {
            co_await self->matchmaker.decline(pending.value(), fb::game::matchmaker::initiator::USER);
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
