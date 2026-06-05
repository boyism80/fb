#ifndef __GROUP_CONTAINER_H__
#define __GROUP_CONTAINER_H__

#include <fb/shard_container.h>
#include <fb/game/group.h>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <map>
#include <cstdint>

namespace fb::game {

class server;
class character;

class group_container : public sharded_container<std::shared_ptr<group>, 16>
{
public:
    using group_ptr = std::shared_ptr<group>;
    using ensure_fn = std::function<async::task<void>(group_ptr&)>;

public:
    fb::game::server& server;

public:
    explicit group_container(fb::game::server& server);

    void detach(std::weak_ptr<character> weak, uint32_t group_id);

    async::task<void> ensure(uint32_t id, ensure_fn fn);
    async::task<void> create(character& me, std::string_view target_name);
    async::task<void> destroy(character& me);
    async::task<void> handle_action(character& actor, std::string_view target_name);
    async::task<void> toggle_member(character& actor, std::string_view target_name);
    async::task<void> leave_member(character& leaver);
    async::task<void> broadcast(uint32_t group_id, std::string_view message, MESSAGE_TYPE type);

    async::task<void> on_error(uint32_t error, std::string_view actor);
    async::task<void>
    on_create(std::string target, uint32_t group_id, std::string master, std::map<uint32_t, std::string> members);
    async::task<void> on_destroyed(std::string actor, uint32_t group_id);
    async::task<void> on_enter(std::string target, uint32_t group_id, std::optional<std::string> new_member);
    async::task<void> on_leave(std::string target, uint32_t group_id, std::optional<std::string> deleted_member);
    async::task<void> on_kick(std::string target, uint32_t group_id, std::optional<std::string> deleted_member);
    async::task<void> on_broadcast(uint32_t group_id, std::string message, uint8_t type);
};

} // namespace fb::game

#endif
