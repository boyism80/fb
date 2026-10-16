#include <fb/config.h>
#include <fb/game/group.h>
#include <fb/game/server.h>
#include <fb/model/model.h>

#include <cstdint>
#include <format>
#include <stdexcept>
#include <string>
#include <vector>

using namespace fb::game;
namespace internal_resp = fb::protocol::internal::response;

group::container::container(server& server) :
    lazy_container<group>(server)
{ }

async::task<group::container::fetched> group::container::fetch(uint32_t id)
{
    auto&& resp =
        co_await this->_server.http.get<internal_resp::GroupDetails>("internal", std::format("/group/id/{}", id));
    auto at = version_time(resp.updated_date);

    switch (static_cast<fb::model::enum_value::ERROR_CODE>(resp.error))
    {
    case fb::model::enum_value::ERROR_CODE::NONE:
    {
        auto members = std::vector<std::string>{};
        members.reserve(resp.members.size());
        for (auto& member : resp.members)
        {
            members.push_back(member.name);
        }
        auto row_at = resp.group.updated_date.empty() ? at : version_time(resp.group.updated_date);
        co_return fetched{this->_server.make<fb::game::group>(id, resp.group.master, members, row_at), at};
    }

    case fb::model::enum_value::ERROR_CODE::GROUP_NOT_FOUND:
        co_return fetched{nullptr, at};

    default:
        throw std::runtime_error(std::format("cannot get group (error : {})", resp.error));
    }
}

group::container::entity_ptr group::container::merge_fetched(uint32_t                         id,
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

bool group::container::recorded(uint32_t id, const fb::model::datetime& at) const
{
    auto _  = std::lock_guard(this->_updated_lock);
    auto it = this->_updated_at.find(id);
    return it != this->_updated_at.end() && it->second >= at;
}

bool group::container::accept(uint32_t id, const fb::model::datetime& at) const
{
    return this->recorded(id, at) == false;
}

void group::container::note(uint32_t id, const fb::model::datetime& at)
{
    auto _  = std::lock_guard(this->_updated_lock);
    auto it = this->_updated_at.find(id);
    if (it == this->_updated_at.end())
        this->_updated_at.emplace(id, at);
    else if (at > it->second)
        it->second = at;
}

async::task<group::container::guard> group::container::open(uint32_t id, const fb::model::datetime& at)
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
