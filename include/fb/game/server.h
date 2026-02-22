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
#include <fb/game/system_mail.h>
#include <fb/game/storage.h>
#include <fb/game/channel/system_mail_channel.h>
#include <fb/game/channel/storage_pending_channel.h>
#include <fb/log_collector.h>
#include <fb/locker.h>
#include <vector>
#include <memory>
#include <string_view>
#include <unordered_map>

REGISTER_RESPONSE(fb::protocol::internal::request::Shutdown, fb::protocol::internal::response::Shutdown)
REGISTER_RESPONSE(fb::protocol::internal::request::Heartbeat, fb::protocol::internal::response::Heartbeat)
REGISTER_RESPONSE(fb::protocol::internal::request::WriteArticle, fb::protocol::internal::response::WriteArticle)
REGISTER_RESPONSE(fb::protocol::internal::request::DeleteArticle, fb::protocol::internal::response::DeleteArticle)
REGISTER_RESPONSE(fb::protocol::internal::request::CreateClan, fb::protocol::internal::response::ClanDetails)
REGISTER_RESPONSE(fb::protocol::internal::request::DestroyClan, fb::protocol::internal::response::DestroyClan)
REGISTER_RESPONSE(fb::protocol::internal::request::SetClanTitle, fb::protocol::internal::response::UpdatedClan)
REGISTER_RESPONSE(fb::protocol::internal::request::JoinClan, fb::protocol::internal::response::UpdatedClan)
REGISTER_RESPONSE(fb::protocol::internal::request::LeaveClan, fb::protocol::internal::response::UpdatedClan)
REGISTER_RESPONSE(fb::protocol::internal::request::KickClan, fb::protocol::internal::response::UpdatedClan)
REGISTER_RESPONSE(fb::protocol::internal::request::ChangeClanRole, fb::protocol::internal::response::UpdatedClan)
REGISTER_RESPONSE(fb::protocol::internal::request::BroadcastClan, fb::protocol::internal::response::BroadcastClan)
REGISTER_RESPONSE(fb::protocol::internal::request::Logout, fb::protocol::internal::response::Logout)
REGISTER_RESPONSE(fb::protocol::internal::request::Save, fb::protocol::internal::response::Save)
REGISTER_RESPONSE(fb::protocol::internal::request::Broadcast, fb::protocol::internal::response::Broadcast)
REGISTER_RESPONSE(fb::protocol::internal::request::CreateGroup, fb::protocol::internal::response::GroupDetails)
REGISTER_RESPONSE(fb::protocol::internal::request::EnterGroup, fb::protocol::internal::response::UpdatedGroup)
REGISTER_RESPONSE(fb::protocol::internal::request::LeaveGroup, fb::protocol::internal::response::UpdatedGroup)
REGISTER_RESPONSE(fb::protocol::internal::request::KickGroup, fb::protocol::internal::response::UpdatedGroup)
REGISTER_RESPONSE(fb::protocol::internal::request::DestroyGroup, fb::protocol::internal::response::DestroyGroup)
REGISTER_RESPONSE(fb::protocol::internal::request::BroadcastGroup, fb::protocol::internal::response::BroadcastGroup)
REGISTER_RESPONSE(fb::protocol::internal::request::Login, fb::protocol::internal::response::Login)
REGISTER_RESPONSE(fb::protocol::internal::request::SetOption, fb::protocol::internal::response::SetOption)
REGISTER_RESPONSE(fb::protocol::internal::request::WriteMail, fb::protocol::internal::response::WriteMail)
REGISTER_RESPONSE(fb::protocol::internal::request::DeleteMail, fb::protocol::internal::response::DeleteMail)
REGISTER_RESPONSE(fb::protocol::internal::request::Whisper, fb::protocol::internal::response::Whisper)
REGISTER_RESPONSE(fb::protocol::internal::request::Transfer, fb::protocol::internal::response::Transfer)
REGISTER_RESPONSE(fb::protocol::internal::request::UpdateFriends, fb::protocol::internal::response::UpdateFriends)
REGISTER_RESPONSE(fb::protocol::internal::request::GetStoragePending,
                  fb::protocol::internal::response::GetStoragePending)
REGISTER_RESPONSE(fb::protocol::internal::request::WriteSystemMail, fb::protocol::internal::response::WriteSystemMail)
REGISTER_RESPONSE(fb::protocol::internal::request::Ban, fb::protocol::internal::response::Ban)
REGISTER_RESPONSE(fb::protocol::internal::request::Unban, fb::protocol::internal::response::Unban)
REGISTER_RESPONSE(fb::protocol::internal::request::SetExpMultiplier, fb::protocol::internal::response::SetExpMultiplier)
REGISTER_RESPONSE(fb::protocol::internal::request::SetDropRateMultiplier,
                  fb::protocol::internal::response::SetDropRateMultiplier)
REGISTER_RESPONSE(fb::protocol::marketplace::request::List, fb::protocol::marketplace::response::List)
REGISTER_RESPONSE(fb::protocol::marketplace::request::Cancel, fb::protocol::marketplace::response::Cancel)
REGISTER_RESPONSE(fb::protocol::marketplace::request::Purchase, fb::protocol::marketplace::response::Purchase)
REGISTER_RESPONSE(fb::protocol::marketplace::request::Search, fb::protocol::marketplace::response::Search)
REGISTER_RESPONSE(fb::protocol::marketplace::request::GetListings, fb::protocol::marketplace::response::GetListings)
REGISTER_RESPONSE(fb::protocol::marketplace::request::GetPurchases, fb::protocol::marketplace::response::GetPurchases)

namespace fb::game {

namespace internal      = fb::protocol::internal;
namespace internal_reqs = fb::protocol::internal::request;
namespace internal_resp = fb::protocol::internal::response;

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
    using object_set         = std::unordered_map<const fb::game::object*, std::unique_ptr<fb::game::object>>;
    using protocol_generator = std::function<std::unique_ptr<fb::protocol::header>(const fb::game::object&)>;
    using clan_ptr           = std::shared_ptr<fb::game::clan>;
    using group_ptr          = std::shared_ptr<fb::game::group>;
    using ensure_group_fn    = std::function<async::task<void>(group_ptr&)>;
    using ensure_clan_fn     = std::function<async::task<void>(clan_ptr&)>;

private:
    fb::model::datetime                               _time;
    double                                            _exp_multiplier;
    double                                            _drop_rate_multiplier;
    std::unordered_map<uint32_t, fb::model::datetime> _scheduled_tasks;

public:
    fb::log_collector                                        log;
    system_mail_channel                                      system_mail;
    storage_pending_channel                                  storage_pending;
    listener_impl                                            listener;
    map_container                                            maps;
    fb::locker<character::container>                         characters;
    fb::sharded_container<clan_ptr, 16>                      clans;
    fb::sharded_container<group_ptr, 16>                     groups;
    fb::sharded_container<map::cache_bytes, 1024, uint64_t>  map_update_cache;
    fb::locker<std::unordered_map<std::string, Json::Value>> globals;

public:
    server(boost::asio::io_context& io_context, uint16_t port);
    server(const server&) = delete;
    server(server&&)      = delete;
    ~server();

public:
    void assert_whisper(const internal_resp::Whisper& response) const;
    void assert_group(uint32_t error, std::string_view actor) const;
    void assert_clan(uint32_t error) const;
    void assert_mail(uint32_t error) const;

public:
    async::task<void> on_broadcast(const internal_resp::Broadcast& resp);
    async::task<void> on_group_broadcast(const internal_resp::BroadcastGroup& resp);
    async::task<void> on_clan_broadcast(const internal_resp::BroadcastClan& resp);
    async::task<void> on_create_clan(const internal_resp::ClanDetails& resp);
    async::task<void> on_destroyed_clan(const internal_resp::DestroyClan& resp);
    async::task<void> on_updated_clan(const internal_resp::UpdatedClan& resp);
    async::task<void> on_create_group(const internal_resp::GroupDetails& resp);
    async::task<void> on_destroyed_group(const internal_resp::DestroyGroup& resp);
    async::task<void> on_updated_group(const internal_resp::UpdatedGroup& resp);
    async::task<void> on_write_mail(const internal_resp::WriteMail& resp);
    async::task<void> on_whisper(const internal_resp::Whisper& resp);

public:
    // clang-format off
    template <typename T, typename... Args> std::shared_ptr<T> make(Args&&... args)
    {
        auto ptr = std::make_shared<T>(*this, std::forward<Args>(args)...);
        if constexpr (std::is_base_of_v<object, T>)
        {
            ptr->on_init();
        }
        return ptr;
    }

    template <typename T> async::task<void> destroy(T& obj, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT)
    {
        if constexpr (std::is_same_v<T, object>)
        {
            auto shared_ptr = obj.template shared_from_this_as<object>();
            std::ignore     = co_await shared_ptr->map(nullptr, fb::model::point16_t{0, 0}, destroy_type);
        }
        co_return;
    }
    // clang-format on

protected:
    uint8_t           id() const override final;
    internal::Service service() const override final;
    bool              decrypt_policy(uint8_t cmd) const override final;
    bool              assert_tps(const fb::socket<character>& socket) const override final;
    void              on_init_amqp(fb::amqp::socket& amqp) override final;
    async::task<void> on_start() override final;
    async::task<bool> on_connected(fb::socket<character>& ch) override final;
    async::task<bool> on_disconnected(fb::socket<character>& ch) override final;

public:
    // clang-format off
    async::task<void>                          send(object& obj, const fb::protocol::header& header, fb::game::scope scope, bool exclude_self = false, bool encrypt = true);
    async::task<void>                          save(character& ch);
    void                                       save();
    virtual uint32_t                           thread_id(const fb::socket<character>& socket) const;
    fb::thread*                                thread(const fb::game::map& map);
    const fb::model::datetime&                 time() const;
    async::task<void>                          broadcast(std::string_view message, MESSAGE_TYPE type, BROADCAST_TYPE broadcast_type);
    async::task<void>                          create_group(character& me, std::string_view target_name);
    async::task<void>                          destroy_group(character& me);
    async::task<void>                          handle_group_action(character& actor, std::string_view target_name);
    async::task<void>                          toggle_group_member(character& actor, std::string_view target_name);
    async::task<void>                          leave_group_member(character& leaver);
    async::task<void>                          broadcast_group(uint32_t group_id, std::string_view message, MESSAGE_TYPE type);
    async::task<void>                          create_clan(character& me, std::string_view name);
    async::task<void>                          destroy_clan(character& me);
    async::task<void>                          join_clan_member(character& inviter, std::string_view target_name);
    async::task<void>                          leave_clan_member(character& leaver);
    async::task<void>                          kick_clan_member(character& kicker, std::string_view target_name);
    async::task<void>                          change_clan_role(character& changer, std::string_view target_name, CLAN_ROLE role);
    async::task<void>                          set_clan_title(character& changer, std::string_view title);
    async::task<void>                          broadcast_clan(uint32_t clan_id, std::string_view message, MESSAGE_TYPE type);
    async::task<internal_resp::WriteMail>      send_mail(const character& ch, std::string_view to, std::string_view title, std::string_view contents);
    async::task<internal_resp::GetMailList>    mail_list(const character& ch, uint16_t offset, uint16_t count);
    async::task<internal_resp::GetMail>        read_mail(character& ch, uint16_t id);
    async::task<internal_resp::DeleteMail>     delete_mail(character& ch, uint16_t id);
    async::task<std::list<bulletin::article>>  bulletin_list(uint16_t section, uint16_t offset);
    async::task<bulletin::article>             read_bulletin(uint16_t section, uint16_t id);
    async::task<void>                          write_bulletin(character& ch, uint16_t section, std::string_view title, std::string_view contents);
    async::task<void>                          delete_bulletin(character& ch, uint16_t section, uint16_t id);
    async::task<void>                          whisper(character& sender, std::string receiver_name, std::string message);
    async::task<internal_resp::Ban>            ban(std::string_view name, std::string_view reason, const std::optional<uint32_t>& days);
    async::task<internal_resp::Unban>          unban(std::string_view name);
    async::task<void>                          update_status();
    void                                       update_time();
    void                                       rezen_force();
    void                                       rezen_force(const fb::game::map& map);
    async::task<void>                          ensure_group(uint32_t id, ensure_group_fn fn);
    void                                       update_clan(clan& clan, internal::Clan& clan_dto, const std::vector<internal::ClanMember>& members_dto) const;
    async::task<void>                          ensure_clan(uint32_t id, ensure_clan_fn fn);
    double                                     exp_multiplier() const;
    void                                       exp_multiplier(double value);
    double                                     drop_rate_multiplier() const;
    void                                       drop_rate_multiplier(double value);
    void                                       initialize_schedules();
    std::unordered_map<uint32_t, fb::model::datetime>& scheduled_tasks();
    // clang-format on
};

} // namespace fb::game

template <typename T, typename... Args>
std::shared_ptr<T> fb::model::object::make(fb::game::server& server, Args&&... args) const
{
    return server.template make<T>(*this, std::forward<Args>(args)...);
}

#endif // !__FB_GAME_H__
