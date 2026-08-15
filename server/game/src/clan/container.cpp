#include <fb/game/clan.h>
#include <fb/game/server.h>
#include <fb/config.h>
#include <fb/model/model.h>

using namespace fb::game;
namespace internal_resp = fb::protocol::internal::response;

clan::container::container(server& server) :
    lazy_container<clan>(server)
{ }

async::task<clan::container::entity_ptr> clan::container::fetch(uint32_t id)
{
    auto   world = fb::config<uint32_t>("world");
    auto&& resp =
        co_await this->_server.http.get<internal_resp::ClanDetails>("internal", std::format("/clan/{}/{}", world, id));

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
