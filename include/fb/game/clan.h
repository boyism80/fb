#ifndef __CLAN_H__
#define __CLAN_H__

#include <fb/game/character.h>
#include <fb/game/clan.member.h>
#include <fb/game/lazy_container.h>
#include <string_view>
#include <unordered_map>

namespace fb::protocol::internal::response {
class UpdatedClan;
}

namespace fb::game {

class clan : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    class container;

private:
    using character_ptr_t    = std::shared_ptr<fb::game::character>;
    using character_weak_ptr = std::weak_ptr<fb::game::character>;
    using member_map         = std::unordered_map<std::string, clan_member>;
    using character_map      = std::unordered_map<uint32_t, character_weak_ptr>;

private:
    server&                    _server;
    uint32_t                   _id;
    std::string                _name;
    std::optional<std::string> _title;
    member_map                 _members;
    character_map              _characters;

public:
    clan(server&                           server,
         uint32_t                          id,
         std::string_view                  name,
         const std::optional<std::string>& title,
         const member_map&                 members);
    clan(const clan&) = delete;
    clan(clan&&);
    ~clan() = default;

public:
    // clang-format off
    void                              update(std::string_view name, const std::optional<std::string>& title, const member_map& members);
    uint32_t                          id() const;
    const std::string&                name() const;
    const std::optional<std::string>& title() const;
    void                              title(const std::optional<std::string>& title);
    const member_map&                 members() const;
    clan_member*                      member(std::string_view name);
    const clan_member*                member(std::string_view name) const;
    bool                              contains(std::string_view name) const;
    void                              join(const clan_member& member);
    void                              leave(std::string_view member);
    void                              change_role(std::string_view member_name, CLAN_ROLE new_role);
    const character_map&              characters() const;
    void                              attach(character_weak_ptr ch);
    void                              detach(character_weak_ptr ch);
    std::vector<character_ptr_t>      nears(const fb::game::map& map, const fb::model::point16_t& position) const;
    // clang-format on
};

class clan::container : public lazy_container<clan>
{
protected:
    async::task<entity_ptr> fetch(uint32_t id) override;

public:
    explicit container(server& server);

    async::task<void> apply_updated(const fb::protocol::internal::response::UpdatedClan& resp);

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
};

} // namespace fb::game

#endif // !__CLAN_H__
