#ifndef __GROUP_H__
#define __GROUP_H__

#include <async/task.h>
#include <fb/game/character.h>
#include <fb/game/lazy_container.h>

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fb::game {

class group : public fb::lua::luable
{
public:
    LUA_PROTOTYPE

public:
    class container;

private:
    using character_ptr      = std::shared_ptr<character>;
    using character_weak_ptr = std::weak_ptr<character>;
    using character_map      = std::unordered_map<uint32_t, character_weak_ptr>;
    using character_vector   = std::vector<character_weak_ptr>;

    server&                  _server;
    uint64_t                 _id;
    std::string              _master;
    std::vector<std::string> _members;
    character_map            _active_members;
    uint64_t                 _revision = 0;

public:
    group(server&                         server,
          uint64_t                        id,
          std::string_view                master,
          const std::vector<std::string>& members,
          uint64_t                        revision);
    group(const group&) = delete;
    group(group&& g);
    ~group() = default;

public:
    // clang-format off
    void                       enter(std::weak_ptr<character> ch);
    void                       detach(std::weak_ptr<character> ch);
    void                       add_member(std::string_view name);
    void                       remove_member(std::string_view name);
    uint64_t                   id() const;
    const std::string&         master() const;
    std::vector<character_ptr> characters() const;
    std::vector<std::string>   members() const;
    uint64_t                   revision() const;
    void                       revision(uint64_t value);
    void                       reload(const group& fresh);
    character_vector           nears(const fb::game::map& map, const fb::model::point16_t& position) const;
    // clang-format on
};

class group::container : public lazy_container<group, 16, uint64_t>
{
protected:
    async::task<entity_ptr> fetch(uint64_t id) override;

private:
    async::task<void> reload(uint64_t id);

public:
    explicit container(server& server);

public:
    // clang-format off
    async::task<void> create(character& me, std::string_view target_name);
    async::task<void> destroy(character& me);
    async::task<void> handle_action(character& actor, std::string_view target_name);
    async::task<void> toggle_member(character& actor, std::string_view target_name);
    async::task<void> leave_member(character& leaver);
    async::task<void> broadcast(uint32_t world, uint64_t group_id, std::string_view message, MESSAGE_TYPE type);
    async::task<void> on_error(uint32_t error, std::string_view actor);
    async::task<void> on_create(std::string target, uint64_t group_id, std::string master, std::map<uint32_t, std::string> members, uint64_t revision);
    async::task<void> on_destroyed(std::string actor, uint64_t group_id, uint64_t revision);
    async::task<void> on_enter(std::string target, uint64_t group_id, std::optional<std::string> new_member, uint64_t revision);
    async::task<void> on_leave(std::string target, uint64_t group_id, std::optional<std::string> deleted_member, uint64_t revision);
    async::task<void> on_kick(std::string target, uint64_t group_id, std::optional<std::string> deleted_member, uint64_t revision);
    async::task<void> on_broadcast(uint64_t group_id, std::string message, uint8_t type);
    void              update_portraits(const group& g, std::vector<std::shared_ptr<character>> recipients = {});
    void              update_portraits(uint64_t group_id);
    void              update_portraits(character& ch);
    void              clear_portraits(character& ch);
    void              update_hp(character& source, bool include_max = false);
    // clang-format on

private:
    // clang-format off
    async::task<void>                                         remove_member(uint64_t group_id, std::string name, std::string message, std::string others_message, std::string log_name, uint64_t revision);
    async::task<void>                                         reset_member(character& ch, std::string message);
    async::task<void>                                         drop_stale_membership(character& ch, uint64_t group_id);
    async::task<void>                                         send_portraits(std::vector<std::string> names, std::string master, std::vector<std::shared_ptr<character>> online, std::vector<std::shared_ptr<character>> recipients);
    static fb::protocol::game::response::group_portrait_entry portrait_entry(const character& ch, bool leader);
    // clang-format on
};

} // namespace fb::game

#endif // !__GROUP_H__
