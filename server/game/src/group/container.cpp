#include <fb/game/group.h>
#include <fb/game/server.h>
#include <fb/config.h>
#include <fb/model/model.h>

using namespace fb::game;
namespace internal_resp = fb::protocol::internal::response;

group::container::container(server& server) :
    lazy_container<group>(server)
{ }

async::task<group::container::entity_ptr> group::container::fetch(uint32_t id)
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
        co_return this->_server.make<fb::game::group>(id, resp.group.master, members);
    }

    case fb::model::enum_value::ERROR_CODE::GROUP_NOT_FOUND:
        co_return nullptr;

    default:
        throw std::runtime_error(std::format("cannot get group (error : {})", resp.error));
    }
}
