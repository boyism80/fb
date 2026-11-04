#include <fb/config.h>
#include <fb/game/castle.h>
#include <fb/game/server.h>
#include <fb/http_client.h>
#include <fb/logger.h>
#include <fb/model/model.h>
#include <macro.h>

#include <cstdint>
#include <exception>
#include <format>
#include <memory>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

using namespace fb::game;
namespace internal_resp = fb::protocol::internal::response;

castle::container::container(server& server) :
    lazy_container<castle>(server)
{ }

async::task<castle::container::entity_ptr> castle::container::fetch(uint32_t id)
{
    // A process without a world (cross server) has no castle ownership, so no castle exists here rather than an
    // ownerless one.
    auto world = fb::config<std::optional<uint32_t>>("world");
    if (!world)
        co_return nullptr;

    auto&& resp =
        co_await this->_server.http.get<internal_resp::CastleList>("internal", std::format("/castle/{}", *world));

    // An unknown owner must not be cached as no owner; a failed fetch is not cached and is retried.
    if (static_cast<fb::model::enum_value::ERROR_CODE>(resp.error) != fb::model::enum_value::ERROR_CODE::NONE)
        throw std::runtime_error(std::format("castle: failed to load castle list (error : {})", resp.error));

    for (auto& entry : resp.castles)
    {
        if (entry.divine_beast != id)
            continue;

        auto castle =
            this->_server.make<fb::game::castle>(static_cast<fb::model::enum_value::DIVINE_BEAST>(entry.divine_beast),
                                                 entry.owner_clan_id);
        castle->apply(entry.revision,
                      entry.owner_clan_id,
                      entry.siege_id,
                      entry.siege_remaining,
                      entry.siege_active_remaining,
                      entry.siege_ended);
        co_return castle;
    }

    co_return this->_server.make<fb::game::castle>(static_cast<fb::model::enum_value::DIVINE_BEAST>(id), std::nullopt);
}

async::task<void> castle::container::load_all()
{
    // clang-format off
    static constexpr fb::model::enum_value::DIVINE_BEAST ALL[] = {
        fb::model::enum_value::DIVINE_BEAST::VERMILION_BIRD,
        fb::model::enum_value::DIVINE_BEAST::WHITE_TIGER,
        fb::model::enum_value::DIVINE_BEAST::BLACK_TORTOISE,
        fb::model::enum_value::DIVINE_BEAST::AZURE_DRAGON,
    };
    // clang-format on

    for (auto divine_beast : ALL)
    {
        std::ignore = co_await this->enter_write(static_cast<uint32_t>(divine_beast));
    }
}

template <typename Row> async::task<void> castle::container::reflect(const Row& row)
{
    auto previous = std::optional<uint32_t>{};
    {
        auto guard = co_await this->enter_write(row.divine_beast);
        if (guard.value() == nullptr)
            co_return;

        previous = guard.value()->owner_clan_id();
        if (guard.value()->apply(row.revision,
                                 row.owner_clan_id,
                                 row.siege_id,
                                 row.siege_remaining,
                                 row.siege_active_remaining,
                                 row.siege_ended) == false)
            co_return;
    }

    if (previous != row.owner_clan_id)
        this->on_owner_changed(static_cast<fb::model::enum_value::DIVINE_BEAST>(row.divine_beast), previous);
}

async::task<void> castle::container::on_updated(const internal_resp::UpdatedCastle& resp)
{
    if (static_cast<fb::model::enum_value::ERROR_CODE>(resp.error) != fb::model::enum_value::ERROR_CODE::NONE)
        co_return;

    co_await this->reflect(resp);
}

async::task<void> castle::container::refresh()
{
    auto world = fb::config<std::optional<uint32_t>>("world");
    if (!world)
        co_return;

    auto resp = internal_resp::CastleList{};
    try
    {
        resp =
            co_await this->_server.http.get<internal_resp::CastleList>("internal", std::format("/castle/{}", *world));
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("castle refresh failed: {}", e.what());
        co_return;
    }

    if (static_cast<fb::model::enum_value::ERROR_CODE>(resp.error) != fb::model::enum_value::ERROR_CODE::NONE)
    {
        fb::logger::warn("castle refresh failed (error : {})", resp.error);
        co_return;
    }

    for (const auto& entry : resp.castles)
    {
        co_await this->reflect(entry);
    }
}

void castle::container::on_owner_changed(fb::model::enum_value::DIVINE_BEAST divine_beast,
                                         const std::optional<uint32_t>&      previous)
{
    if (previous.has_value() == false)
        return;

    auto members = std::vector<std::shared_ptr<character>>{};
    {
        auto guard = this->_server.clans.try_enter_read(previous.value());
        if (guard.has_value() == false || guard->value() == nullptr)
            return;

        members = guard->value()->online_characters();
    }

    this->_server.characters.foreach_enqueue(
        [this, divine_beast](auto& ch) -> async::task<void> {
            auto lua = this->_server.lua.open("scripts/interaction.lua", "on_castle_lost");
            if (lua)
            {
                lua->pushobject(ch);
                lua->pushinteger(static_cast<int>(divine_beast));
                std::ignore = co_await lua->call(2);
            }
        },
        members);
}

async::task<castle::siege_start_result> castle::container::start_siege(fb::model::enum_value::DIVINE_BEAST divine_beast,
                                                                       uint64_t                            siege_key)
{
    auto world = fb::config<std::optional<uint32_t>>("world");
    if (!world)
        throw std::runtime_error("castle: no world to start a siege in");

    auto id   = static_cast<uint32_t>(divine_beast);
    auto resp = internal_resp::UpdatedCastle{};
    try
    {
        resp = co_await this->_server.http.post(
            "internal",
            "/castle/siege/start",
            fb::protocol::internal::request::StartCastleSiege{*world,
                                                              fb::config<uint32_t>("id"),
                                                              static_cast<uint8_t>(id),
                                                              siege_key});
    }
    catch (const fb::http_error& e)
    {
        fb::logger::warn("castle {} siege start request failed: {}", id, e.what());
        throw std::runtime_error(_TEXT(MESSAGE_REQUEST_UNCONFIRMED));
    }

    if (static_cast<fb::model::enum_value::ERROR_CODE>(resp.error) != fb::model::enum_value::ERROR_CODE::NONE)
        throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), resp.error));

    co_await this->reflect(resp);

    auto result = castle::siege_start_result{};
    {
        auto guard = co_await this->enter_write(id);
        if (guard.value() == nullptr)
            co_return result;

        result.active    = guard.value()->siege_active();
        result.remaining = guard.value()->siege_remaining_seconds().value_or(0);
    }

    result.started = resp.siege_started;
    co_return result;
}

async::task<bool> castle::container::finish_siege(fb::model::enum_value::DIVINE_BEAST divine_beast)
{
    auto world = fb::config<std::optional<uint32_t>>("world");
    if (!world)
        co_return false;

    auto id   = static_cast<uint32_t>(divine_beast);
    auto resp = internal_resp::UpdatedCastle{};
    try
    {
        resp = co_await this->_server.http.post(
            "internal",
            "/castle/siege/finish",
            fb::protocol::internal::request::FinishCastleSiege{*world,
                                                               fb::config<uint32_t>("id"),
                                                               static_cast<uint8_t>(id)});
    }
    catch (const fb::http_error& e)
    {
        fb::logger::warn("castle {} siege finish request failed: {}", id, e.what());
        throw std::runtime_error(_TEXT(MESSAGE_REQUEST_UNCONFIRMED));
    }

    if (static_cast<fb::model::enum_value::ERROR_CODE>(resp.error) != fb::model::enum_value::ERROR_CODE::NONE)
        throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), resp.error));

    co_await this->reflect(resp);

    auto guard = co_await this->enter_write(id);
    if (guard.value() == nullptr)
        co_return false;

    co_return guard.value()->siege_pending();
}

// The siege stays pending until internal records its end, so a failed save can be retried with the same winner.
async::task<castle::siege_end_result> castle::container::end_siege(fb::model::enum_value::DIVINE_BEAST divine_beast,
                                                                   const std::optional<uint32_t>&      winner_clan_id)
{
    auto id       = static_cast<uint32_t>(divine_beast);
    auto winner   = winner_clan_id;
    auto siege_id = uint64_t{0};
    {
        auto guard = co_await this->enter_write(id);
        if (guard.value() == nullptr)
            co_return siege_end_result::NOT_ACTIVE;

        // Checking and claiming the end under one lock lets only one of a timer and a GM command end the siege.
        auto& castle = guard.value();
        if (castle->siege_pending() == false || castle->siege_ending())
            co_return siege_end_result::NOT_ACTIVE;

        if (castle->siege_winner().has_value())
            winner = castle->siege_winner();

        siege_id = castle->siege_id().value();
        castle->siege_ending(true);
    }

    auto world = fb::config<std::optional<uint32_t>>("world");
    auto resp  = std::optional<internal_resp::UpdatedCastle>{};
    try
    {
        if (world)
        {
            auto&& updated = co_await this->_server.http.post(
                "internal",
                "/castle/siege/end",
                fb::protocol::internal::request::EndCastleSiege{*world,
                                                                fb::config<uint32_t>("id"),
                                                                static_cast<uint8_t>(id),
                                                                siege_id,
                                                                winner});
            if (static_cast<fb::model::enum_value::ERROR_CODE>(updated.error) ==
                fb::model::enum_value::ERROR_CODE::NONE)
                resp = updated;
            else
                fb::logger::warn("castle: failed to end siege {} for divine_beast {} (error : {})",
                                 siege_id,
                                 id,
                                 updated.error);
        }
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("castle: failed to end siege {} for divine_beast {} ({})", siege_id, id, e.what());
    }

    if (resp.has_value())
        co_await this->reflect(resp.value());

    auto guard = co_await this->enter_write(id);
    if (guard.value() == nullptr)
        co_return siege_end_result::NOT_ACTIVE;

    guard.value()->siege_ending(false);
    if (resp.has_value() == false)
    {
        guard.value()->siege_winner(winner);
        co_return siege_end_result::SAVE_FAILED;
    }

    co_return siege_end_result::ENDED;
}
