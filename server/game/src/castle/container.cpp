#include <fb/game/castle.h>
#include <fb/game/server.h>
#include <fb/config.h>
#include <fb/logger.h>
#include <fb/model/model.h>

using namespace fb::game;
namespace internal_resp = fb::protocol::internal::response;

castle::container::container(server& server) :
    lazy_container<castle>(server)
{ }

async::task<castle::container::entity_ptr> castle::container::fetch(uint32_t id)
{
    auto   world = fb::config<uint32_t>("world");
    auto&& resp =
        co_await this->_server.http.get<internal_resp::CastleList>("internal", std::format("/castle/{}", world));

    if (static_cast<fb::model::enum_value::ERROR_CODE>(resp.error) != fb::model::enum_value::ERROR_CODE::NONE)
    {
        fb::logger::warn("castle: failed to load castle list (error : {})", resp.error);
        co_return this->_server.make<fb::game::castle>(static_cast<fb::model::enum_value::DIVINE_BEAST>(id),
                                                       std::nullopt);
    }

    for (auto& entry : resp.castles)
    {
        if (entry.divine_beast != id)
            continue;

        co_return this->_server.make<fb::game::castle>(
            static_cast<fb::model::enum_value::DIVINE_BEAST>(entry.divine_beast),
            entry.owner_clan_id);
    }

    fb::logger::warn("castle: no row found for divine_beast {}, defaulting to ownerless", id);
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
        std::ignore = co_await this->ensure(static_cast<uint32_t>(divine_beast));
    }
}

async::task<void> castle::container::on_updated(const internal_resp::UpdatedCastle& resp)
{
    if (static_cast<fb::model::enum_value::ERROR_CODE>(resp.error) != fb::model::enum_value::ERROR_CODE::NONE)
        co_return;

    auto guard = co_await this->ensure(resp.divine_beast);
    if (guard.value() != nullptr)
        guard.value()->owner_clan_id(resp.owner_clan_id);
}

async::task<void> castle::container::end_siege(fb::model::enum_value::DIVINE_BEAST divine_beast,
                                               const std::optional<uint32_t>&      winner_clan_id)
{
    auto id    = static_cast<uint32_t>(divine_beast);
    auto guard = co_await this->ensure(id);
    if (guard.value() == nullptr)
        co_return;

    auto& entity         = guard.value();
    auto  previous_owner = entity->owner_clan_id();
    entity->clear_siege();

    if (winner_clan_id.has_value() == false)
        co_return;

    if (previous_owner.has_value() && previous_owner.value() == winner_clan_id.value())
        co_return;

    auto   world = fb::config<uint32_t>("world");
    auto&& resp =
        co_await this->_server.http.post("internal",
                                         "/castle/owner",
                                         fb::protocol::internal::request::SetCastleOwner{world,
                                                                                         fb::config<uint32_t>("host"),
                                                                                         static_cast<uint8_t>(id),
                                                                                         winner_clan_id});

    if (static_cast<fb::model::enum_value::ERROR_CODE>(resp.error) != fb::model::enum_value::ERROR_CODE::NONE)
    {
        fb::logger::warn("castle: failed to persist owner change for divine_beast {} (error : {})", id, resp.error);
        co_return;
    }

    entity->owner_clan_id(resp.owner_clan_id);
}
