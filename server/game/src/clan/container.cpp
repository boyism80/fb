#include <fb/config.h>
#include <fb/game/clan.h>
#include <fb/game/server.h>
#include <fb/model/model.h>

#include <cstdint>
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

async::task<clan::container::fetched> clan::container::fetch(uint32_t id)
{
    auto&& resp =
        co_await this->_server.http.get<internal_resp::ClanDetails>("internal", std::format("/clan/id/{}", id));
    auto at = version_time(resp.updated_date);

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
        auto row_at = resp.clan.updated_date.empty() ? at : version_time(resp.clan.updated_date);
        co_return fetched{this->_server.make<fb::game::clan>(id,
                                                             resp.clan.name,
                                                             resp.clan.title,
                                                             members,
                                                             row_at,
                                                             resp.clan.allied_clan_id,
                                                             enemy_clan_ids,
                                                             resp.clan.money),
                          at};
    }

    case fb::model::enum_value::ERROR_CODE::NOT_FOUND_CLAN:
        co_return fetched{nullptr, at};

    default:
        throw std::runtime_error(std::format("cannot get clan (error : {})", resp.error));
    }
}

clan::container::entity_ptr clan::container::merge_fetched(uint32_t                         id,
                                                           const std::optional<entity_ptr>& current,
                                                           const fetched&                   incoming)
{
    if (current.has_value() && *current != nullptr)
    {
        if ((*current)->updated_date() >= incoming.at)
            return *current;
    }
    else
    {
        if (this->recorded(id, incoming.at))
            return current.has_value() ? *current : nullptr;
    }

    this->note(id, incoming.at);
    return incoming.entity;
}

bool clan::container::recorded(uint32_t id, const fb::model::datetime& at) const
{
    auto _  = std::lock_guard(this->_updated_lock);
    auto it = this->_updated_at.find(id);
    return it != this->_updated_at.end() && it->second >= at;
}

bool clan::container::accept(uint32_t id, const fb::model::datetime& at) const
{
    return this->recorded(id, at) == false;
}

void clan::container::note(uint32_t id, const fb::model::datetime& at)
{
    auto _  = std::lock_guard(this->_updated_lock);
    auto it = this->_updated_at.find(id);
    if (it == this->_updated_at.end())
        this->_updated_at.emplace(id, at);
    else if (at > it->second)
        it->second = at;
}

async::task<clan::container::guard> clan::container::open(uint32_t id, const fb::model::datetime& at)
{
    {
        auto held = co_await this->enter_write(id);
        if (held.holds_lock() == false)
            co_return guard{};
        if (held.value() != nullptr)
            co_return std::move(held);
        if (this->accept(id, at) == false)
            co_return guard{};
    }

    this->erase(id);
    co_return co_await this->enter_write(id);
}
