#ifndef __CLAN_CONTAINER_H__
#define __CLAN_CONTAINER_H__

#include <fb/shard_container.h>
#include <fb/game/clan.h>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <cstdint>

namespace fb::protocol::internal::response {
class UpdatedClan;
}

namespace fb::game {

class server;
class character;

class clan_container : public sharded_container<std::shared_ptr<clan>, 16>
{
public:
    using clan_ptr  = std::shared_ptr<clan>;
    using ensure_fn = std::function<async::task<void>(clan_ptr&)>;

public:
    fb::game::server& server;

public:
    explicit clan_container(fb::game::server& server);

    void detach(std::weak_ptr<character> weak, uint32_t clan_id);

    async::task<void> ensure(uint32_t id, ensure_fn fn);
    async::task<void> create(character& me, std::string_view name);
    async::task<void> destroy(character& me);
    async::task<void> join_member(character& inviter, std::string_view target_name);
    async::task<void> leave_member(character& leaver);
    async::task<void> kick_member(character& kicker, std::string_view target_name);
    async::task<void> change_role(character& changer, std::string_view target_name, CLAN_ROLE role);
    async::task<void> set_title(character& changer, std::string_view title);
    async::task<void> broadcast(uint32_t clan_id, std::string_view message, MESSAGE_TYPE type);

    async::task<void> on_error(uint32_t error);
    async::task<void> on_create(uint32_t                                     clan_id,
                                std::string                                  name,
                                std::string                                  title,
                                std::unordered_map<std::string, clan_member> members);
    async::task<void> on_destroyed(uint32_t clan_id, std::string clan_name);
    async::task<void> on_broadcast(uint32_t clan_id, std::string message, uint8_t type);
    async::task<void> on_set_title(uint32_t clan_id, std::optional<std::string> new_title);
    async::task<void> on_join(uint32_t clan_id, std::optional<std::string> new_member, CLAN_ROLE role);
    async::task<void> on_leave(uint32_t clan_id, std::optional<std::string> deleted_member);
    async::task<void> on_kick(uint32_t clan_id, std::optional<std::string> deleted_member);
    async::task<void> on_change_role(uint32_t                   clan_id,
                                     std::optional<uint32_t>    target_uid,
                                     std::optional<std::string> target_name,
                                     std::optional<uint32_t>    old_role,
                                     std::optional<uint32_t>    new_role);

private:
    static async::task<void> apply_updated(clan_container&                                      container,
                                           const fb::protocol::internal::response::UpdatedClan& resp);
};

} // namespace fb::game

#endif
