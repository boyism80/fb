#include <fb/game/server.h>
#include <fb/encoding.h>
#include <json/json.h>

using namespace fb::game;

void server::assert_clan(uint32_t error) const
{
    switch (static_cast<ERROR_CODE>(error))
    {
    case ERROR_CODE::NONE:
        return;

    case ERROR_CODE::CLAN_NAME_ALREADY_EXISTS:
        throw std::runtime_error(std::format("클랜명이 이미 존재함"));

    default:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), error));
    }
}

async::task<void> server::upsert_clan_then(uint32_t id, std::function<async::task<void>(std::shared_ptr<fb::game::clan>&)> fn)
{
    co_await this->clans.async_write(
        id,
        [this, id, fn](auto& clan) -> async::task<void> {
            co_await fn(clan);
        },
        [=, this]() -> async::task<std::shared_ptr<fb::game::clan>> {
            auto&& resp = co_await this->http.get<internal_resp::GetClan>("internal", std::format("/clan/{}", id));
            switch (static_cast<ERROR_CODE>(resp.error))
            {
            case ERROR_CODE::NONE:
            {
                auto members = std::unordered_map<std::string, clan_member>{};
                for (auto& member : resp.members)
                {
                    members.insert({
                        member.name,
                        clan_member{member.name, static_cast<CLAN_ROLE>(member.role)}
                    });
                }
                co_return std::make_shared<fb::game::clan>(*this, id, resp.clan.name, resp.clan.title, members);
            }

            default:
                throw std::runtime_error(std::format("cannot get clan (error : {})", resp.error));
            }
        });
}

void server::update_clan(clan& clan, fb::protocol::internal::Clan& resp1, const std::vector<fb::protocol::internal::ClanMember>& resp2) const
{
    auto members = std::vector<clan_member>{};
    for (auto& member : resp2)
    {
        auto cm = clan_member{member.name, static_cast<CLAN_ROLE>(member.role)};
        members.push_back(std::move(cm));
    }

    clan.update(resp1.name, resp1.title, members);
}

async::task<void> server::create_clan(character& me, std::string name)
{
    if (me.clan_id().has_value())
        throw std::runtime_error(_TEXT(MESSAGE_ALREADY_JOINED_CLAN));

    auto   weak = me.weak_from_this_as<character>();
    auto&& resp = co_await this->http.post("internal", "/clan/create", CreateClan{me.id(), name});
    co_await this->threads.switching(weak);

    this->assert_clan(resp.error);

    auto id     = resp.clan.id;
    std::ignore = this->upsert_clan_then(id, [=, &resp, this](auto& clan) -> async::task<void> {
        auto me = weak.lock();
        if (me == nullptr)
            co_return;

        me->clan_id(id);
        clan->attach_character(weak);

        // Log clan create event
        auto log_data              = Json::Value();
        log_data["character_id"]   = static_cast<Json::Int64>(me->id());
        log_data["character_name"] = UTF8(me->name(), PLATFORM::WINDOWS);
        log_data["clan_id"]        = static_cast<Json::Int64>(id);
        log_data["clan_name"]      = UTF8(resp.clan.name, PLATFORM::WINDOWS);
        this->log.write("clan_create", log_data);

        co_return;
    });
}

async::task<void> server::destroy_clan(character& me)
{
    auto weak    = me.weak_from_this_as<character>();
    auto clan_id = me.clan_id(); // don't use auto& because it will be invalidated after the clan is destroyed
    if (clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    auto destroyed = co_await this->clans.async_write(clan_id.value(), [this, weak](auto& clan) -> async::task<bool> {
        auto me = weak.lock();
        if (me == nullptr)
            co_return false;

        auto members = std::vector<std::shared_ptr<fb::game::character>>();
        for (auto& [_, weak_ptr] : clan->characters())
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                continue;

            members.push_back(shared_ptr);
        }

        auto&& resp = co_await this->http.post("internal", "/clan/destroy", DestroyClan{me->id()});
        co_await this->threads.switching(weak);

        this->assert_clan(resp.error);
        auto message = std::format("{} 문파가 해체되었습니다.", clan->name());
        co_await this->characters.async_write([this, message, members](auto& characters) -> async::task<void> {
            co_await characters.foreach (
                [this, message](auto& ch) {
                    ch->message(message, MESSAGE_TYPE::NOTIFY);
                    ch->clan_reset();
                },
                members);
        });
        co_return true;
    });

    if (destroyed)
        this->clans.erase(clan_id.value());
}

async::task<void> server::set_clan_title(uint32_t changer_uid, std::string title)
{
    auto&& resp = co_await this->http.post("internal", "/clan/title", SetClanTitle{config<uint32_t>("host"), changer_uid, title});

    co_await this->on_clan_title_changed(resp);
}

async::task<void> server::join_clan_member(character& inviter, character& invitee)
{
    auto&& resp = co_await this->http.post("internal", "/clan/join", JoinClan{config<uint32_t>("host"), inviter.id(), invitee.id()});

    co_await this->on_clan_join_member(resp);
}

async::task<void> server::leave_clan_member(const clan& clan, const std::string& name)
{
    auto&& resp = co_await this->http.post("internal", "/clan/leave", LeaveClan{config<uint32_t>("host"), clan.id(), name});

    co_await this->on_clan_leave_member(resp);
}

async::task<void> server::kick_clan_member(const clan& clan, const std::string& kicker, const std::string& target)
{
    auto&& resp = co_await this->http.post("internal", "/clan/kick", KickClan{config<uint32_t>("host"), clan.id(), kicker, target});

    co_await this->on_clan_kick_member(resp);
}

async::task<void> server::change_clan_member_role(const clan& clan, uint32_t changer_uid, const std::string& target, CLAN_ROLE role)
{
    auto&& resp = co_await this->http.post("internal", "/clan/change-role", ChangeClanRole{config<uint32_t>("host"), changer_uid, target, clan.id(), static_cast<uint32_t>(role)});

    co_await this->on_clan_change_role(resp);
}

async::task<void> server::broadcast(const clan& clan, const std::string& message, MESSAGE_TYPE type)
{
    auto&& resp = co_await this->http.post("internal", "/clan/broadcast", BroadcastClan{config<uint32_t>("host"), clan.id(), message, static_cast<uint8_t>(type)});

    co_await this->on_clan_broadcast(resp);
}

async::task<void> server::on_clan_title_changed(const internal_resp::SetClanTitle& resp)
{
    this->assert_clan(resp.error);

    co_await this->upsert_clan_then(resp.clan, [this, &resp](auto& clan) -> async::task<void> {
        auto old_title = clan->title();
        clan->title(resp.title);

        // Log clan title change event
        auto log_data         = Json::Value();
        log_data["clan_id"]   = static_cast<Json::Int64>(clan->id());
        log_data["clan_name"] = UTF8(clan->name(), PLATFORM::WINDOWS);
        log_data["old_title"] = old_title.has_value() ? Json::Value(UTF8(old_title.value(), PLATFORM::WINDOWS)) : Json::Value::null;
        log_data["new_title"] = resp.title.has_value() ? Json::Value(UTF8(resp.title.value(), PLATFORM::WINDOWS)) : Json::Value::null;
        // log_data["changer_uid"] = static_cast<Json::Int64>(resp.changer_uid);
        this->log.write("clan_title_change", log_data);

        co_return;
    });
}

async::task<void> server::on_clan_join_member(const internal_resp::JoinClan& resp)
{
    this->assert_clan(resp.error);

    co_await this->upsert_clan_then(resp.clan, [this, resp](auto& clan) -> async::task<void> {
        co_await this->characters.async_write([this, &resp, clan](auto& characters) -> async::task<void> {
            auto ch = characters.find(resp.member.name);
            if (ch == nullptr)
                co_return;

            if (ch->clan_id().has_value())
                co_return;

            auto weak    = ch->weak_from_this_as<character>();
            auto members = std::vector<std::shared_ptr<fb::game::character>>();
            for (auto& [_, weak_ptr] : clan->characters())
            {
                auto shared_ptr = weak_ptr.lock();
                if (shared_ptr == nullptr)
                    continue;

                members.push_back(shared_ptr);
            }

            co_await characters.foreach (
                [this, resp](auto& member) {
                    member->message(std::format("{}님이 문파에 가입했습니다.", resp.member.name), MESSAGE_TYPE::NOTIFY);
                },
                members);

            auto cm = clan_member{resp.member.name, static_cast<CLAN_ROLE>(resp.member.role)};
            clan->join(cm);

            if (weak.expired() == false)
            {
                clan->attach_character(weak);
                ch->clan_id(clan->id());
                ch->update_external(false);
                ch->message(std::format("{} 문파에 가입되었습니다.", clan->name()), MESSAGE_TYPE::NOTIFY);

                // Log clan join event
                auto log_data              = Json::Value();
                log_data["character_id"]   = static_cast<Json::Int64>(ch->id());
                log_data["character_name"] = UTF8(resp.member.name, PLATFORM::WINDOWS);
                log_data["clan_id"]        = static_cast<Json::Int64>(clan->id());
                log_data["clan_name"]      = UTF8(clan->name(), PLATFORM::WINDOWS);
                this->log.write("clan_join", log_data);
            }
        });
    });
}

async::task<void> server::on_clan_leave_member(const internal_resp::LeaveClan& resp)
{
    this->assert_clan(resp.error);
    co_await this->upsert_clan_then(resp.clan, [this, resp](auto& clan) -> async::task<void> {
        co_await this->characters.async_write([this, &resp, clan](auto& characters) -> async::task<void> {
            auto ch = characters.find(resp.uname);
            if (ch != nullptr)
            {
                auto weak = ch->weak_from_this_as<character>();
                clan->detach(weak);
                ch->clan_reset();
                ch->update_external(false);
                ch->message("문파에서 탈퇴했습니다.", MESSAGE_TYPE::NOTIFY);

                // Log clan leave event
                auto log_data              = Json::Value();
                log_data["character_id"]   = static_cast<Json::Int64>(ch->id());
                log_data["character_name"] = UTF8(resp.uname, PLATFORM::WINDOWS);
                log_data["clan_id"]        = static_cast<Json::Int64>(clan->id());
                log_data["clan_name"]      = UTF8(clan->name(), PLATFORM::WINDOWS);
                this->log.write("clan_leave", log_data);
            }

            clan->leave(resp.uname);
            auto members = std::vector<std::shared_ptr<fb::game::character>>();
            for (auto& [_, weak_ptr] : clan->characters())
            {
                auto shared_ptr = weak_ptr.lock();
                if (shared_ptr == nullptr)
                    continue;

                members.push_back(shared_ptr);
            }

            auto message = std::format("{}님이 문파에서 탈퇴하였습니다.", resp.uname);
            co_await characters.foreach (
                [this, message](auto& member) {
                    member->message(message, MESSAGE_TYPE::NOTIFY);
                },
                members);

            clan->leave(resp.uname);
        });
    });
}

async::task<void> server::on_clan_kick_member(const internal_resp::KickClan& resp)
{
    this->assert_clan(resp.error);
    co_await this->upsert_clan_then(resp.clan, [this, resp](auto& clan) -> async::task<void> {
        co_await this->characters.async_write([this, &resp, clan](auto& characters) -> async::task<void> {
            auto ch = characters.find(resp.uname);
            if (ch != nullptr)
            {
                auto weak = ch->weak_from_this_as<character>();
                clan->detach(weak);
                ch->clan_reset();
                ch->update_external(false);
                ch->message("문파에서 추방당했습니다.", MESSAGE_TYPE::NOTIFY);

                // Log clan kick event
                auto log_data              = Json::Value();
                log_data["character_id"]   = static_cast<Json::Int64>(ch->id());
                log_data["character_name"] = UTF8(resp.uname, PLATFORM::WINDOWS);
                log_data["clan_id"]        = static_cast<Json::Int64>(clan->id());
                log_data["clan_name"]      = UTF8(clan->name(), PLATFORM::WINDOWS);
                this->log.write("clan_kick", log_data);
            }

            clan->leave(resp.uname);
            auto members = std::vector<std::shared_ptr<fb::game::character>>();
            for (auto& [_, weak_ptr] : clan->characters())
            {
                auto shared_ptr = weak_ptr.lock();
                if (shared_ptr == nullptr)
                    continue;

                members.push_back(shared_ptr);
            }

            auto message = std::format("{}님이 문파에서 추방당했습니다.", resp.uname);
            co_await characters.foreach (
                [this, message](auto& member) {
                    member->message(message, MESSAGE_TYPE::NOTIFY);
                },
                members);
            clan->leave(resp.uname);
        });
    });
}

async::task<void> server::on_clan_change_role(const internal_resp::ChangeClanRole& resp)
{
    this->assert_clan(resp.error);
    co_await this->upsert_clan_then(resp.clan, [this, resp](auto& clan) -> async::task<void> {
        this->characters.write([this, &resp, clan](auto& characters) {
            auto member = clan->member(resp.target_name);
            if (member != nullptr)
                member->role = static_cast<CLAN_ROLE>(resp.new_role);

            auto target = characters.find(resp.target_uid);
            if (target != nullptr)
            {
                target->message(std::format("문파 직책이 변경되었습니다. ({} -> {})", resp.old_role, resp.new_role), MESSAGE_TYPE::NOTIFY);
            }

            for (auto& [uid, weak] : clan->characters())
            {
                if (uid == resp.target_uid)
                    continue;

                auto shared = weak.lock();
                if (shared == nullptr)
                    continue;

                shared->message(std::format("{}님의 문파 직책이 {}에서 {}로 변경되었습니다.", resp.target_name, resp.old_role, resp.new_role), MESSAGE_TYPE::NOTIFY);
            }
        });
        co_return;
    });
}

async::task<void> server::on_clan_broadcast(const internal_resp::BroadcastClan& resp)
{
    this->assert_clan(resp.error);

    co_await this->upsert_clan_then(resp.clan, [this, resp](auto& clan) -> async::task<void> {
        auto members = std::vector<std::shared_ptr<fb::game::character>>();
        for (auto& [_, weak_ptr] : clan->characters())
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                continue;

            members.push_back(shared_ptr);
        }

        co_await this->characters.async_write([this, &resp, &members](auto& characters) -> async::task<void> {
            co_await characters.foreach (
                [this, resp](auto& member) {
                    member->message(resp.message, static_cast<MESSAGE_TYPE>(resp.type));
                },
                members);
        });
    });
}