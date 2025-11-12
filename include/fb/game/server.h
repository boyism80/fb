#ifndef __FB_GAME_H__
#define __FB_GAME_H__

#include <boost/algorithm/string/join.hpp>
#include <json/json.h>
#include <fstream>
#include <fb/acceptor.h>
#include <fb/game/protocol.h>
#include <fb/amqp.h>
#include <fb/game/listener.h>
#include <fb/game/thread_params.h>
#include <fb/game/map/container.h>
#include <fb/hash.h>
#include <fb/redis.h>
#include <fb/shard_container.h>
#include <fb/game/clan.h>
#include <fb/game/npc_interaction_handler.h>
#include <fb/game/system_mail.h>
#include <fb/locker.h>
#include <vector>

using namespace fb::protocol::internal;
using namespace fb::protocol::internal::request;
namespace fb_reqs       = fb::protocol::game::request;
namespace fb_resp       = fb::protocol::game::response;
namespace internal      = fb::protocol::internal;
namespace internal_resp = fb::protocol::internal::response;

REGISTER_RESPONSE(fb::protocol::internal::request::Shutdown, fb::protocol::internal::response::Shutdown)
REGISTER_RESPONSE(fb::protocol::internal::request::WriteArticle, fb::protocol::internal::response::WriteArticle)
REGISTER_RESPONSE(fb::protocol::internal::request::DeleteArticle, fb::protocol::internal::response::DeleteArticle)
REGISTER_RESPONSE(fb::protocol::internal::request::CreateClan, fb::protocol::internal::response::CreateClan)
REGISTER_RESPONSE(fb::protocol::internal::request::DestroyClan, fb::protocol::internal::response::DestroyClan)
REGISTER_RESPONSE(fb::protocol::internal::request::SetClanTitle, fb::protocol::internal::response::SetClanTitle)
REGISTER_RESPONSE(fb::protocol::internal::request::JoinClan, fb::protocol::internal::response::JoinClan)
REGISTER_RESPONSE(fb::protocol::internal::request::LeaveClan, fb::protocol::internal::response::LeaveClan)
REGISTER_RESPONSE(fb::protocol::internal::request::KickClan, fb::protocol::internal::response::KickClan)
REGISTER_RESPONSE(fb::protocol::internal::request::BroadcastClan, fb::protocol::internal::response::BroadcastClan)
REGISTER_RESPONSE(fb::protocol::internal::request::Logout, fb::protocol::internal::response::Logout)
REGISTER_RESPONSE(fb::protocol::internal::request::Save, fb::protocol::internal::response::Save)
REGISTER_RESPONSE(fb::protocol::internal::request::Broadcast, fb::protocol::internal::response::Broadcast)
REGISTER_RESPONSE(fb::protocol::internal::request::EnterGroup, fb::protocol::internal::response::EnterGroup)
REGISTER_RESPONSE(fb::protocol::internal::request::BroadcastGroup, fb::protocol::internal::response::BroadcastGroup)
REGISTER_RESPONSE(fb::protocol::internal::request::Login, fb::protocol::internal::response::Login)
REGISTER_RESPONSE(fb::protocol::internal::request::LeaveGroup, fb::protocol::internal::response::LeaveGroup)
REGISTER_RESPONSE(fb::protocol::internal::request::KickGroup, fb::protocol::internal::response::KickGroup)
REGISTER_RESPONSE(fb::protocol::internal::request::SetOption, fb::protocol::internal::response::SetOption)
REGISTER_RESPONSE(fb::protocol::internal::request::WriteMail, fb::protocol::internal::response::WriteMail)
REGISTER_RESPONSE(fb::protocol::internal::request::DeleteMail, fb::protocol::internal::response::DeleteMail)
REGISTER_RESPONSE(fb::protocol::internal::request::Whisper, fb::protocol::internal::response::Whisper)
REGISTER_RESPONSE(fb::protocol::internal::request::Transfer, fb::protocol::internal::response::Transfer)
REGISTER_RESPONSE(fb::protocol::internal::request::ChangeClanRole, fb::protocol::internal::response::ChangeClanRole)
REGISTER_RESPONSE(fb::protocol::internal::request::UpdateFriends, fb::protocol::internal::response::UpdateFriends)
REGISTER_RESPONSE(fb::protocol::internal::request::GetSystemMails, fb::protocol::internal::response::GetSystemMails)
REGISTER_RESPONSE(fb::protocol::internal::request::WriteSystemMail, fb::protocol::internal::response::WriteSystemMail)

namespace fb::game {

enum class scope
{
    PIVOT,
    GROUP,
    MAP,
    WORLD
};

class server : public fb::acceptor<fb::game::character>
{
public:
    LUA_PROTOTYPE

public:
    using object_set                   = std::unordered_map<const fb::game::object*, std::unique_ptr<fb::game::object>>;
    using protocol_generator           = std::function<std::unique_ptr<fb::protocol::header>(const fb::game::object&)>;
    using clan_ptr                     = std::shared_ptr<fb::game::clan>;
    using group_ptr                    = std::shared_ptr<fb::game::group>;
    using npc_interaction_handler_ptr  = std::unique_ptr<fb::game::npc_interaction_handler>;
    using npc_interaction_handler_list = std::vector<npc_interaction_handler_ptr>;

private:
    fb::model::datetime                  _time;
    npc_interaction_handler_list         _npc_interaction_handlers;
    fb::redis                            _redis;
    fb::locker<std::vector<system_mail>> _system_mails;

public:
    fb::game::listener_impl listener;

public:
    fb::model::model                                        model;
    fb::game::map_container                                 maps;
    fb::game::character::container                          characters;
    fb::sharded_container<clan_ptr, 16>                     clans;
    fb::sharded_container<group_ptr, 16>                    groups;
    fb::sharded_container<map::cache_bytes, 1024, uint64_t> map_update_cache;

public:
    server(boost::asio::io_context& io_context, uint16_t port);
    server(const server&) = delete;
    ~server();

public:
    async::task<void> upsert_group_then(uint32_t gid, const std::function<void(group_ptr&)>& fn);
    async::task<void> upsert_group_then(uint32_t gid, const std::string& master, const std::vector<std::string>& members, const std::function<void(group_ptr&)>& fn);
    void              update_clan(clan& clan, fb::protocol::internal::Clan& resp1, const std::vector<fb::protocol::internal::ClanMember>& resp2) const;
    async::task<void> upsert_clan_then(uint32_t id, std::function<async::task<void>(std::shared_ptr<fb::game::clan>&)> fn);

private:
    template <typename HandlerType> void bind_npc_interaction()
    {
        auto handler = std::make_unique<HandlerType>(*this);
        this->_npc_interaction_handlers.push_back(std::move(handler));
    }

public:
    void assert_whisper(const internal::response::Whisper& response) const;
    void assert_group(uint32_t error, const std::string& actor) const;
    void assert_clan(uint32_t error) const;
    void assert_mail(uint32_t error) const;

public:
    async::task<void> on_enter_group(const internal_resp::EnterGroup& resp);
    async::task<void> on_leave_group(const internal_resp::LeaveGroup& resp);
    async::task<void> on_kick_group(const internal_resp::KickGroup& resp);

public:
    void              on_broadcast(const internal_resp::Broadcast& resp);
    void              on_group_broadcast(const internal_resp::BroadcastGroup& resp);
    async::task<void> on_clan_broadcast(const internal_resp::BroadcastClan& resp);
    async::task<void> on_clan_title_changed(const internal_resp::SetClanTitle& resp);
    async::task<void> on_clan_join_member(const internal_resp::JoinClan& resp);
    async::task<void> on_clan_leave_member(const internal_resp::LeaveClan& resp);
    async::task<void> on_clan_kick_member(const internal_resp::KickClan& resp);
    void              on_write_mail(const internal_resp::WriteMail& resp);
    void              on_whisper(const internal_resp::Whisper& resp);
    async::task<void> on_clan_change_role(const internal_resp::ChangeClanRole& resp);

public:
    template <typename T, typename... Args> std::shared_ptr<T> make(Args&&... args)
    {
        auto ptr = std::make_shared<T>(*this, std::forward<Args>(args)...);
        if constexpr (std::is_base_of_v<fb::game::object, T>)
        {
            ptr->on_init();
        }
        return ptr;
    }

    void rezen_force();
    void rezen_force(const fb::game::map& map);

public:
    template <typename T> [[nodiscard]] async::task<void> destroy(T& obj, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT)
    {
        if constexpr (std::is_same_v<T, fb::game::object>)
        {
            auto shared_ptr = obj.template shared_from_this_as<fb::game::object>();
            std::ignore     = co_await shared_ptr->map(nullptr, fb::model::point16_t{0, 0}, destroy_type);
        }
        co_return;
    }

public:
    async::task<void>               send(fb::game::object& object, const fb::protocol::header& header, fb::game::scope scope, bool exclude_self = false, bool encrypt = true);
    [[nodiscard]] async::task<void> save(fb::game::character& ch);

public:
    virtual uint32_t                                        thread_id(const fb::socket<fb::game::character>& socket) const;
    fb::thread*                                             thread(const fb::game::map& map);
    const fb::model::datetime&                              time() const;
    [[nodiscard]] async::task<void>                         broadcast(const std::string& message, MESSAGE_TYPE type, BROADCAST_TYPE broadcast_type);
    [[nodiscard]] async::task<bool>                         create_group(character& me, const std::string& target);
    [[nodiscard]] async::task<void>                         leave_group(character& me);
    [[nodiscard]] async::task<void>                         kick_group_member(const group& group, const std::string& kicker, const std::string& target);
    [[nodiscard]] async::task<void>                         broadcast(const group& group, const std::string& message, MESSAGE_TYPE type);
    [[nodiscard]] async::task<void>                         create_clan(character& me, std::string name);
    [[nodiscard]] async::task<void>                         destroy_clan(character& me);
    [[nodiscard]] async::task<void>                         set_clan_title(uint32_t changer_uid, std::string title);
    [[nodiscard]] async::task<void>                         join_clan_member(character& inviter, character& invitee);
    [[nodiscard]] async::task<void>                         kick_clan_member(const clan& clan, const std::string& kicker, const std::string& target);
    [[nodiscard]] async::task<void>                         change_clan_member_role(const clan& clan, uint32_t changer_uid, const std::string& target, CLAN_ROLE role);
    [[nodiscard]] async::task<void>                         broadcast(const clan& clan, const std::string& message, MESSAGE_TYPE type);
    [[nodiscard]] async::task<internal_resp::WriteMail>     send_mail(const character& ch, const std::string& to, const std::string& title, const std::string& contents);
    [[nodiscard]] async::task<internal_resp::WriteMail>     send_mail(const std::string& sender, const std::string& to, const std::string& title, const std::string& contents);
    [[nodiscard]] async::task<internal_resp::GetMailList>   mail_list(const character& ch, uint16_t offset, uint16_t count);
    [[nodiscard]] async::task<internal_resp::GetMail>       read_mail(character& ch, uint16_t id);
    [[nodiscard]] async::task<internal_resp::DeleteMail>    delete_mail(character& ch, uint16_t id);
    [[nodiscard]] async::task<std::list<bulletin::article>> bulletin_list(uint16_t section, uint16_t offset);
    [[nodiscard]] async::task<bulletin::article>            read_bulletin(uint16_t section, uint16_t id);
    [[nodiscard]] async::task<void>                         write_bulletin(character& ch, uint16_t section, const std::string& title, const std::string& contents);
    [[nodiscard]] async::task<void>                         delete_bulletin(character& ch, uint16_t section, uint16_t id);
    [[nodiscard]] async::task<void>                         whisper(character& sender, std::string receiver_name, std::string message);
    [[nodiscard]] async::task<void>                         fetch_system_mails();
    std::vector<system_mail>                                get_system_mails() const;

protected:
    bool                                 decrypt_policy(uint8_t cmd) const override final;
    bool                                 assert_tps(const fb::socket<fb::game::character>& socket) const override final;
    void                                 handle_init_amqp(fb::amqp::socket& amqp) override final;
    [[nodiscard]] async::task<void>      handle_start() override final;
    [[nodiscard]] async::task<bool>      handle_connected(fb::socket<fb::game::character>& ch) override final;
    [[nodiscard]] async::task<bool>      handle_disconnected(fb::socket<fb::game::character>& ch) override final;
    std::shared_ptr<fb::game::character> handle_accepted(fb::socket<fb::game::character>& socket) override final;

protected:
    uint8_t id() const override final;
    Service service() const override final;

public:
    void                            update_status();
    void                            update_time();
    async::task<bool>               npc_interaction(character& ch, const std::string& message, const std::vector<std::shared_ptr<fb::game::npc>>& npcs);
    [[nodiscard]] async::task<void> leave_clan_member(const clan& clan, const std::string& name);
    [[nodiscard]] async::task<void> join_clan_member(const clan& clan, character& inviter, character& invitee);
};

} // namespace fb::game

template <typename T, typename... Args>
std::shared_ptr<T> fb::model::object::make(fb::game::server& server, Args&&... args) const
{
    return server.template make<T>(*this, std::forward<Args>(args)...);
}

#endif // !__FB_GAME_H__
