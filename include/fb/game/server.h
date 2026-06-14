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
#include <fb/game/map.h>
#include <fb/hash.h>
#include <fb/game/group.h>
#include <fb/game/clan.h>
#include <fb/game/service/mail.h>
#include <fb/game/service/bulletin.h>
#include <fb/game/service/system_storage.h>
#include <fb/game/service/system_mail.h>
#include <fb/game/service/schedule.h>
#include <fb/game/service/property.h>
#include <fb/game/storage.h>
#include <fb/game/system_storage_box.h>
#include <fb/log_collector.h>
#include <fb/synchronized.h>
#include <vector>
#include <memory>
#include <mutex>
#include <string_view>
#include <unordered_map>

// clang-format off
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
REGISTER_RESPONSE(fb::protocol::internal::request::SaveBatch, fb::protocol::internal::response::BatchSave)
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
REGISTER_RESPONSE(fb::protocol::internal::request::WriteMails, fb::protocol::internal::response::WriteMails)
REGISTER_RESPONSE(fb::protocol::internal::request::DeliverSystemMail, fb::protocol::internal::response::DeliverSystemMail)
REGISTER_RESPONSE(fb::protocol::internal::request::DeleteMail, fb::protocol::internal::response::DeleteMail)
REGISTER_RESPONSE(fb::protocol::internal::request::Whisper, fb::protocol::internal::response::Whisper)
REGISTER_RESPONSE(fb::protocol::internal::request::Transfer, fb::protocol::internal::response::Transfer)
REGISTER_RESPONSE(fb::protocol::internal::request::UpdateFriends, fb::protocol::internal::response::UpdateFriends)
REGISTER_RESPONSE(fb::protocol::internal::request::WriteSystemStorageBox, fb::protocol::internal::response::WriteSystemStorageBox)
REGISTER_RESPONSE(fb::protocol::internal::request::WriteSystemMail, fb::protocol::internal::response::WriteSystemMail)
REGISTER_RESPONSE(fb::protocol::internal::request::Ban, fb::protocol::internal::response::Ban)
REGISTER_RESPONSE(fb::protocol::internal::request::Unban, fb::protocol::internal::response::Unban)
REGISTER_RESPONSE(fb::protocol::internal::request::SetExpMultiplier, fb::protocol::internal::response::SetExpMultiplier)
REGISTER_RESPONSE(fb::protocol::internal::request::SetDropRateMultiplier, fb::protocol::internal::response::SetDropRateMultiplier)
REGISTER_RESPONSE(fb::protocol::internal::request::SetDateTime, fb::protocol::internal::response::SetDateTime)
REGISTER_RESPONSE(fb::protocol::marketplace::request::List, fb::protocol::marketplace::response::List)
REGISTER_RESPONSE(fb::protocol::marketplace::request::Cancel, fb::protocol::marketplace::response::Cancel)
REGISTER_RESPONSE(fb::protocol::marketplace::request::Purchase, fb::protocol::marketplace::response::Purchase)
REGISTER_RESPONSE(fb::protocol::marketplace::request::Search, fb::protocol::marketplace::response::Search)
REGISTER_RESPONSE(fb::protocol::marketplace::request::GetListings, fb::protocol::marketplace::response::GetListings)
REGISTER_RESPONSE(fb::protocol::marketplace::request::GetPurchases, fb::protocol::marketplace::response::GetPurchases)
// clang-format on

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
    friend class character::container;

public:
    LUA_PROTOTYPE

public:
    using object_set         = std::unordered_map<const fb::game::object*, std::unique_ptr<fb::game::object>>;
    using protocol_generator = std::function<std::unique_ptr<fb::protocol::header>(const fb::game::object&)>;

private:
    fb::model::datetime _time;
    double              _exp_multiplier;
    double              _drop_rate_multiplier;

public:
    fb::log_collector                      log;
    listener_impl                          listener;
    fb::synchronized<character::container> characters;
    map::container                         maps;
    clan::container                        clans;
    group::container                       groups;
    service::mail                          mail;
    service::bulletin                      bulletin;
    service::system_storage                system_storage;
    service::system_mail                   system_mail;
    service::schedule                      schedules;
    service::property                      property;

public:
    server(boost::asio::io_context& io_context, uint16_t port);
    server(const server&) = delete;
    server(server&&)      = delete;
    ~server();

private:
    internal::SavePayload save_payload(const character& ch) const;
    async::task<void>     set_saved_before_shutdown_on_all();

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
    // clang-format off
    uint8_t                                     id() const override final;
    internal::Service                           service() const override final;
    bool                                        decrypt_policy(uint8_t opcode) const override final;
    bool                                        assert_tps(const fb::socket<character>& socket) const override final;
    void                                        on_init_amqp(fb::amqp::socket& amqp) override final;
    async::task<void>                           on_start() override final;
    async::task<void>                           on_exit() override final;
    async::task<bool>                           on_connected(fb::socket<character>& ch) override final;
    async::task<bool>                           on_disconnected(fb::socket<character>& ch) override final;
    // clang-format on

public:
    // clang-format off
    async::task<void>                           send(object& obj, const fb::protocol::header& header, fb::game::scope scope, bool exclude_self = false, bool encrypt = true);
    async::task<void>                           save();
    async::task<void>                           save(character& ch);
    void                                        sync_time();
    async::task<internal_resp::Ban>             ban(std::string_view name, std::string_view reason, const std::optional<uint32_t>& days);
    async::task<internal_resp::Unban>           unban(std::string_view name);

public:
    virtual uint32_t                            thread_id(const fb::socket<character>& socket) const;
    const fb::model::datetime&                  time() const;
    async::task<void>                           update_status();
    double                                      exp_multiplier() const;
    void                                        exp_multiplier(double value);
    double                                      drop_rate_multiplier() const;
    void                                        drop_rate_multiplier(double value);
    // clang-format on
};

} // namespace fb::game

template <typename T, typename... Args>
std::shared_ptr<T> fb::model::object::make(fb::game::server& server, Args&&... args) const
{
    return server.template make<T>(*this, std::forward<Args>(args)...);
}

#endif // !__FB_GAME_H__
