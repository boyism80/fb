#include <fb/config.h>
#include <fb/game/clan.h>
#include <fb/game/server.h>
#include <fb/logger.h>
#include <fb/model/model.h>

#include <cstdint>
#include <exception>
#include <format>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace fb::game;
namespace internal_resp = fb::protocol::internal::response;

clan::container::container(server& server) :
    lazy_container<clan>(server)
{ }

async::task<clan::container::entity_ptr> clan::container::fetch(uint32_t id)
{
    auto&& resp =
        co_await this->_server.http.get<internal_resp::ClanDetails>("internal", std::format("/clan/id/{}", id));

    switch (static_cast<fb::model::enum_value::ERROR_CODE>(resp.error))
    {
    case fb::model::enum_value::ERROR_CODE::NONE:
    {
        auto members = std::unordered_map<std::string, clan_member>{};
        for (auto& member : resp.members)
        {
            members.insert({
                member.name,
                clan_member{member.name, static_cast<CLAN_ROLE>(member.role)}
            });
        }
        auto enemy_clan_ids =
            std::unordered_set<uint32_t>(resp.clan.enemy_clan_ids.begin(), resp.clan.enemy_clan_ids.end());
        co_return this->_server.make<fb::game::clan>(id,
                                                     resp.clan.name,
                                                     resp.clan.title,
                                                     members,
                                                     resp.clan.revision,
                                                     resp.clan.allied_clan_id,
                                                     enemy_clan_ids,
                                                     resp.clan.money);
    }

    case fb::model::enum_value::ERROR_CODE::NOT_FOUND_CLAN:
        co_return nullptr;

    default:
        throw std::runtime_error(std::format("cannot get clan (error : {})", resp.error));
    }
}

async::task<void> clan::container::reload(uint32_t id)
{
    auto* thread = this->_server.threads.current();
    auto  fresh  = entity_ptr{};
    try
    {
        fresh = co_await this->fetch(id);
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("clan reload failed (id: {}): {}", id, e.what());
    }

    if (thread != nullptr)
        co_await thread->switching();

    if (fresh == nullptr)
        co_return;

    auto guard = co_await this->enter_write(id);
    if (guard.holds_lock() == false || guard.value() == nullptr)
        co_return;

    if (guard.value()->revision() < fresh->revision())
        guard.value()->reload(*fresh);
}
