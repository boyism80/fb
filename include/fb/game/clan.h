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
    using member_map    = std::unordered_map<std::string, clan_member>;
    using character_map = std::unordered_map<uint32_t, std::weak_ptr<fb::game::character>>;

private:
    server&                    _server;
    uint32_t                   _id;
    std::string                _name;
    std::optional<std::string> _title;
    member_map                 _members;
    character_map              _characters;

public:
    clan(server& server, uint32_t id, const std::string& name, const std::optional<std::string>& title, const std::unordered_map<std::string, clan_member>& members);
    clan(const clan&) = delete;
    clan(clan&&);
    ~clan() = default;

public:
    void                                                update(const std::string& name, const std::optional<std::string>& title, const std::vector<clan_member>& members);
    uint32_t                                            id() const;
    const std::string&                                  name() const;
    const std::optional<std::string>&                   title() const;
    void                                                title(const std::optional<std::string>& title);
    const std::unordered_map<std::string, clan_member>& members() const;
    clan_member*                                        member(const std::string& name);
    void                                                join(const clan_member& member);
    void                                                leave(const std::string& member);
    void                                                change_role(const std::string& member_name, CLAN_ROLE new_role);
    const std::unordered_map<uint32_t, std::weak_ptr<fb::game::character>>& characters() const;
    void                                                                    attach_character(std::weak_ptr<character> ch);
    void                                                                    detach(std::weak_ptr<character> ch);
    std::vector<std::shared_ptr<fb::game::character>>                       nears(const fb::game::map& map, const fb::model::point16_t& position) const;
};

} // namespace fb::game

#endif // !__CLAN_H__
