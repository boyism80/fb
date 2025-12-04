#ifndef __CLAN_H__
#define __CLAN_H__

#include <fb/game/character.h>
#include <fb/game/clan.member.h>

namespace fb::game {

class clan : public lua::luable
{
public:
    LUA_PROTOTYPE

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
    clan(server& server, uint32_t id, const std::string& name, const std::optional<std::string>& title, const member_map& members);
    clan(const clan&) = delete;
    clan(clan&&);
    ~clan() = default;

public:
    void                              update(const std::string& name, const std::optional<std::string>& title, const member_map& members);
    uint32_t                          id() const;
    const std::string&                name() const;
    const std::optional<std::string>& title() const;
    void                              title(const std::optional<std::string>& title);
    const member_map&                 members() const;
    clan_member*                      member(const std::string& name);
    void                              join(const clan_member& member);
    void                              leave(const std::string& member);
    void                              change_role(const std::string& member_name, CLAN_ROLE new_role);
    const character_map&              characters() const;
    void                              attach(character_weak_ptr ch);
    void                              detach(character_weak_ptr ch);
    std::vector<character_ptr_t>      nears(const fb::game::map& map, const fb::model::point16_t& position) const;
};

} // namespace fb::game

#endif // !__CLAN_H__
