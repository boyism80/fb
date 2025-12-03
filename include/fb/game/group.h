#ifndef __GROUP_H__
#define __GROUP_H__

#include <fb/game/character.h>
#include <async/task.h>
#include <unordered_set>

namespace fb::game {

class group : public fb::lua::luable
{
public:
    LUA_PROTOTYPE

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
    group(server& server, uint32_t id, const std::string& master, const std::vector<std::string>& members);
    group(const group&) = delete;
    group(group&& g);
    ~group() = default;

public:
    void                     enter(std::weak_ptr<character> ch);
    void                     detach(std::weak_ptr<character> ch);
    async::task<void>        update(const std::string& master, const std::vector<std::string>& members);
    uint32_t                 id() const;
    const std::string&       master() const;
    character_set            characters() const;
    std::vector<std::string> members() const;
    character_vector         nears(const fb::game::map& map, const fb::model::point16_t& position) const;

public:
    async::task<void> enter_member(character& inviter, const std::string& target_name);
    async::task<void> leave_member(character& leaver);
    async::task<void> kick_member(character& kicker, const std::string& target_name);
    async::task<void> broadcast(const std::string& message, MESSAGE_TYPE type);
};

} // namespace fb::game

#endif // !__GROUP_H__
