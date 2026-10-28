#include <fb/config.h>
#include <fb/game/group.h>
#include <fb/game/server.h>
#include <fb/logger.h>
#include <fb/model/model.h>
#include <macro.h>

#include <algorithm>
#include <cstdint>
#include <exception>
#include <format>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

using namespace fb::game;
namespace internal_resp = fb::protocol::internal::response;

group::container::container(server& server) :
    lazy_container<group, 16, uint64_t>(server)
{ }

async::task<group::container::entity_ptr> group::container::fetch(uint64_t id)
{
    auto&& resp =
        co_await this->_server.http.get<internal_resp::GroupDetails>("internal", std::format("/group/id/{}", id));

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
        co_return this->_server.make<fb::game::group>(id, resp.group.master, members, resp.group.revision);
    }

    case fb::model::enum_value::ERROR_CODE::GROUP_NOT_FOUND:
        co_return nullptr;

    default:
        throw std::runtime_error(std::format("cannot get group (error : {})", resp.error));
    }
}

async::task<void> group::container::reload(uint64_t id)
{
    auto* thread = this->_server.threads.current();
    auto  fresh  = entity_ptr{};
    auto  failed = false;
    try
    {
        fresh = co_await this->fetch(id);
    }
    catch (const std::exception& e)
    {
        failed = true;
        fb::logger::warn("group reload failed (id: {}): {}", id, e.what());
    }

    if (thread != nullptr)
        co_await thread->switching();

    if (fresh == nullptr)
    {
        if (failed == false)
            co_await this->on_destroyed(std::string{}, id, std::numeric_limits<uint64_t>::max());
        co_return;
    }

    auto removed = std::vector<std::shared_ptr<character>>{};
    auto joining = std::vector<std::shared_ptr<character>>{};
    {
        auto guard = co_await this->enter_write(id);
        if (guard.holds_lock() == false || guard.value() == nullptr)
            co_return;
        if (guard.value()->revision() >= fresh->revision())
            co_return;

        removed     = guard.value()->reload(*fresh);
        auto active = guard.value()->characters();
        for (const auto& name : guard.value()->members())
        {
            auto ch = this->_server.characters.find(name);
            if (ch != nullptr && std::find(active.begin(), active.end(), ch) == active.end())
                joining.push_back(ch);
        }
    }

    co_await this->_server.characters.foreach_async(
        [this, id](auto& ch) -> async::task<void> {
            if (ch->group.id() == id)
                co_await this->reset_member(*ch, _TEXT(MESSAGE_GROUP_LEFT_SUCCESS));
        },
        removed);
    co_await this->_server.characters.foreach_async(
        [this, id](auto& ch) -> async::task<void> {
            std::ignore = co_await this->enter_member(*ch, id);
        },
        joining);

    if (removed.empty() == false || joining.empty() == false)
        this->update_portraits(id);
}
