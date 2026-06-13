#ifndef __GROUP_H__
#define __GROUP_H__

#include <fb/game/character.h>
#include <fb/game/lazy_container.h>
#include <async/task.h>
#include <map>
#include <unordered_set>
#include <string_view>

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
    using character_set      = std::unordered_set<character_ptr>;
    using character_vector   = std::vector<character_weak_ptr>;

    server&                  _server;
    uint32_t                 _id;
    std::string              _master;
    std::vector<std::string> _members;
    character_set            _active_members;

public:
    group(server& server, uint32_t id, std::string_view master, const std::vector<std::string>& members);
    group(const group&) = delete;
    group(group&& g);
    ~group() = default;

public:
    void                     enter(std::weak_ptr<character> ch);
    void                     detach(std::weak_ptr<character> ch);
    void                     add_member(std::string_view name);
    void                     remove_member(std::string_view name);
    uint32_t                 id() const;
    const std::string&       master() const;
    character_set            characters() const;
    std::vector<std::string> members() const;
    character_vector         nears(const fb::game::map& map, const fb::model::point16_t& position) const;
};

class group::container : public lazy_container<group>
{
protected:
    async::task<entity_ptr> fetch(uint32_t id) override;

public:
    explicit container(server& server);

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

#endif // !__GROUP_H__
