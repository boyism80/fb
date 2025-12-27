#include <fb/game/server.h>
#include <fb/model/model.h>
#include <macro.h>
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
        throw std::runtime_error(_TEXT(MESSAGE_CLAN_NAME_ALREADY_EXISTS));

    default:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), error));
    }
}

async::task<void> server::ensure_clan(uint32_t id, ensure_clan_fn fn)
{
    auto thread = this->threads.current();
    if (thread == nullptr)
        throw std::runtime_error(std::format("No thread available for ensure_clan (clan_id: {})", id));

    auto success = co_await thread->enqueue_with_retry(
        [=, this](auto& thread) -> async::task<bool> {
            auto result = co_await this->clans.try_async_write(
                id,
                [this, fn, &thread](auto& clan) -> async::task<void> {
                    co_await fn(clan);
                    co_await thread.switching();
                },
                [=, this]() -> async::task<clan_ptr> {
                    auto&& resp =
                        co_await this->http.get<internal_resp::ClanDetails>("internal", std::format("/clan/{}", id));
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
                        co_return this->make<fb::game::clan>(id, resp.clan.name, resp.clan.title, members);
                    }

                    default:
                        throw std::runtime_error(std::format("cannot get clan (error : {})", resp.error));
                    }
                });
            co_return result;
        },
        10);

    if (!success)
        throw std::runtime_error(std::format("Failed to acquire clan lock after retries (clan_id: {})", id));
}

void server::update_clan(clan&                                    clan,
                         internal::Clan&                          clan_dto,
                         const std::vector<internal::ClanMember>& members_dto) const
{
    auto members = std::unordered_map<std::string, clan_member>{};
    for (auto& member : members_dto)
    {
        members.insert({
            member.name,
            clan_member{member.name, static_cast<CLAN_ROLE>(member.role)}
        });
    }

    clan.update(clan_dto.name, clan_dto.title, members);
}

async::task<void> server::on_create_clan(const internal_resp::ClanDetails& resp)
{
    this->assert_clan(resp.error);

    auto id         = resp.clan.id;
    auto clan_name  = resp.clan.name;
    auto clan_title = resp.clan.title;
    auto members    = std::unordered_map<std::string, clan_member>{};
    for (auto& member : resp.members)
    {
        members.insert({
            member.name,
            clan_member{member.name, static_cast<CLAN_ROLE>(member.role)}
        });
    }

    auto master_uid = static_cast<uint32_t>(0);
    for (auto& member : resp.members)
    {
        if (static_cast<CLAN_ROLE>(member.role) == CLAN_ROLE::MASTER)
        {
            master_uid = member.uid;
            break;
        }
    }

    co_await this->clans.async_write(
        id,
        [=, this](auto& clan) -> async::task<void> {
            clan->update(clan_name, clan_title, members);

            if (master_uid != 0)
            {
                co_await this->characters.async_read(
                    [id, master_uid, &clan, this](auto& characters) -> async::task<void> {
                        auto ch = characters.find(master_uid);
                        if (ch != nullptr)
                        {
                            ch->clan_id(id);
                            clan->attach(ch->template weak_from_this_as<character>());

                            auto log_data              = Json::Value();
                            log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
                            log_data["character_name"] = UTF8(ch->name(), PLATFORM::WINDOWS);
                            log_data["clan_id"]        = static_cast<Json::Int64>(id);
                            log_data["clan_name"]      = UTF8(clan->name(), PLATFORM::WINDOWS);
                            this->log.write("clan_create", log_data);
                        }
                        co_return;
                    });
            }
            co_return;
        },
        [=, this]() -> async::task<std::shared_ptr<fb::game::clan>> {
            co_return std::make_shared<fb::game::clan>(*this, id, clan_name, clan_title, members);
        });
}

async::task<void> server::create_clan(character& me, std::string name)
{
    auto weak = me.weak_from_this_as<character>();
    try
    {
        if (me.clan_id().has_value())
            throw std::runtime_error(_TEXT(MESSAGE_ALREADY_JOINED_CLAN));

        auto&& resp = co_await this->http.post("internal",
                                               "/clan/create",
                                               internal_reqs::CreateClan{fb::config<uint32_t>("id"), me.id, name});
        co_await this->threads.switching(weak);
        co_await this->on_create_clan(resp);
    }
    catch (std::exception& e)
    {
        auto ch = weak.lock();
        if (ch != nullptr)
            ch->message(e.what(), MESSAGE_TYPE::STATE);
    }
}

async::task<void> server::destroy_clan(character& me)
{
    auto weak = me.weak_from_this_as<character>();
    try
    {
        auto clan_id = me.clan_id();
        if (clan_id.has_value() == false)
            throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

        co_await this->ensure_clan(clan_id.value(), [this, &me](auto& clan) -> async::task<void> {
            if (clan->members().size() != 1)
                throw std::runtime_error(_TEXT(MESSAGE_CLAN_MEMBER_EXISTS));

            auto member = clan->member(me.name());
            if (member == nullptr)
                throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

            if (member->role != CLAN_ROLE::MASTER)
                throw std::runtime_error(_TEXT(MESSAGE_CLAN_NO_PRIVILEGE));

            if (member->name != me.name())
                throw std::runtime_error(_TEXT(MESSAGE_CLAN_NO_PRIVILEGE));
            co_return;
        });

        auto&& resp = co_await this->http.post("internal",
                                               "/clan/destroy",
                                               internal_reqs::DestroyClan{fb::config<uint32_t>("id"), me.id});
        co_await this->threads.switching(weak);
        co_await this->on_destroyed_clan(resp);
    }
    catch (std::exception& e)
    {
        auto ch = weak.lock();
        if (ch != nullptr)
            ch->message(e.what(), MESSAGE_TYPE::STATE);
    }
}

async::task<void> server::join_clan_member(character& inviter, const std::string& target_name)
{
    auto weak = inviter.weak_from_this_as<character>();
    try
    {
        auto inviter_clan_id = inviter.clan_id();
        if (inviter_clan_id.has_value() == false)
            throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

        auto current_thread = this->threads.current();
        if (current_thread != nullptr)
        {
            auto params = current_thread->template data<thread_params>();
            auto target = params->characters.find(target_name);
            if (target != nullptr)
            {
                if (target->clan_id().has_value())
                    throw std::runtime_error(_TEXT(MESSAGE_ALREADY_JOINED_CLAN));
            }
        }

        auto&& resp =
            co_await this->http.post("internal",
                                     "/clan/join",
                                     internal_reqs::JoinClan{fb::config<uint32_t>("host"), inviter.id, target_name});
        co_await this->threads.switching(weak);
        co_await this->on_updated_clan(resp);
    }
    catch (std::exception& e)
    {
        auto ch = weak.lock();
        if (ch != nullptr)
            ch->message(e.what(), MESSAGE_TYPE::STATE);
    }
}

async::task<void> server::leave_clan_member(character& leaver)
{
    auto weak = leaver.weak_from_this_as<character>();

    auto clan_id = leaver.clan_id();
    if (clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    co_await this->ensure_clan(clan_id.value(), [this, &leaver](auto& clan) -> async::task<void> {
        auto member = clan->member(leaver.name());
        if (member != nullptr)
        {
            if (member->role == CLAN_ROLE::MASTER)
                throw std::runtime_error(_TEXT(MESSAGE_CLAN_CANNOT_LEAVE_MASTER));
        }
        co_return;
    });

    auto&& resp = co_await this->http.post(
        "internal",
        "/clan/leave",
        internal_reqs::LeaveClan{fb::config<uint32_t>("host"), clan_id.value(), leaver.name()});
    co_await this->threads.switching(weak);
    co_await this->on_updated_clan(resp);
}

async::task<void> server::kick_clan_member(character& kicker, const std::string& target_name)
{
    auto weak = kicker.weak_from_this_as<character>();

    auto kicker_clan_id = kicker.clan_id();
    if (kicker_clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    auto current_thread = this->threads.current();
    if (current_thread != nullptr)
    {
        auto params = current_thread->template data<thread_params>();
        auto target = params->characters.find(target_name);

        if (target != nullptr)
        {
            auto target_clan_id = target->clan_id();
            if (target_clan_id.has_value() == false || target_clan_id.value() != kicker_clan_id.value())
                throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));
        }
    }

    auto&& resp = co_await this->http.post(
        "internal",
        "/clan/kick",
        internal_reqs::KickClan{fb::config<uint32_t>("host"), kicker_clan_id.value(), kicker.name(), target_name});
    co_await this->threads.switching(weak);
    co_await this->on_updated_clan(resp);
}

async::task<void> server::change_clan_role(character& changer, const std::string& target_name, CLAN_ROLE role)
{
    auto weak            = changer.weak_from_this_as<character>();
    auto changer_clan_id = changer.clan_id();
    if (changer_clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    auto current_thread = this->threads.current();
    if (current_thread != nullptr)
    {
        auto params = current_thread->template data<thread_params>();
        auto target = params->characters.find(target_name);

        if (target != nullptr)
        {
            auto target_clan_id = target->clan_id();
            if (target_clan_id.has_value() == false || target_clan_id.value() != changer_clan_id.value())
                throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));
        }
    }

    auto&& resp = co_await this->http.post("internal",
                                           "/clan/change-role",
                                           internal_reqs::ChangeClanRole{fb::config<uint32_t>("host"),
                                                                         changer.id,
                                                                         target_name,
                                                                         changer_clan_id.value(),
                                                                         static_cast<uint32_t>(role)});
    co_await this->threads.switching(weak);
    co_await this->on_updated_clan(resp);
}

async::task<void> server::set_clan_title(character& changer, const std::string& title)
{
    auto weak            = changer.weak_from_this_as<character>();
    auto changer_clan_id = changer.clan_id();
    if (changer_clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    co_await this->ensure_clan(changer_clan_id.value(), [this, &changer, &title](auto& clan) -> async::task<void> {
        auto member = clan->member(changer.name());
        if (member != nullptr)
        {
            if (static_cast<uint32_t>(member->role) <
                static_cast<uint32_t>(fb::model::const_value::clan::MINIMUM_CHANGE_TITLE_PRIVILEGE))
                throw std::runtime_error(_TEXT(MESSAGE_CLAN_NO_PRIVILEGE));
        }

        auto current_title = clan->title();
        if (current_title.has_value() && current_title.value() == title)
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_TITLE_NOT_CHANGED));

        if (!title.empty() && title.length() < 2)
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_TITLE_TOO_SHORT));
        co_return;
    });

    auto&& resp =
        co_await this->http.post("internal",
                                 "/clan/title",
                                 internal_reqs::SetClanTitle{fb::config<uint32_t>("host"), changer.id, title});
    co_await this->threads.switching(weak);
    co_await this->on_updated_clan(resp);
}

async::task<void> server::broadcast_clan(uint32_t clan_id, const std::string& message, MESSAGE_TYPE type)
{
    auto&& resp = co_await this->http.post(
        "internal",
        "/clan/broadcast",
        internal_reqs::BroadcastClan{fb::config<uint32_t>("host"), clan_id, message, static_cast<uint8_t>(type)});
    co_await this->on_clan_broadcast(resp);
}

async::task<void> server::on_destroyed_clan(const internal_resp::DestroyClan& resp)
{
    this->assert_clan(resp.error);

    this->clans.erase(resp.clan_id, [this, &resp](const auto& clan) {
        auto members = std::vector<std::shared_ptr<fb::game::character>>();
        for (auto& [_, weak_ptr] : clan->characters())
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                continue;
            members.push_back(shared_ptr);
        }

        auto message = std::format(_TEXT(MESSAGE_CLAN_DISBANDED), resp.clan_name);
        this->characters.write([this, message, members](auto& characters) {
            characters.foreach_enqueue(
                [this, message](auto& ch) -> async::task<void> {
                    ch->message(message, MESSAGE_TYPE::NOTIFY);
                    ch->clan_reset();
                    co_return;
                },
                members);
        });
    });
    co_return;
}

async::task<void> server::on_updated_clan(const internal_resp::UpdatedClan& resp)
{
    this->assert_clan(resp.error);

    co_await this->ensure_clan(resp.clan_id, [this, &resp](auto& clan) -> async::task<void> {
        switch (static_cast<internal::ClanActionType>(resp.action))
        {
        case internal::ClanActionType::SetTitle:
        {
            auto old_title = clan->title();
            if (resp.new_title.has_value())
                clan->title(resp.new_title.value());
            else
                clan->title(std::nullopt);

            auto log_data         = Json::Value();
            log_data["clan_id"]   = static_cast<Json::Int64>(clan->id());
            log_data["clan_name"] = UTF8(clan->name(), PLATFORM::WINDOWS);
            log_data["old_title"] =
                old_title.has_value() ? Json::Value(UTF8(old_title.value(), PLATFORM::WINDOWS)) : Json::Value::null;
            log_data["new_title"] = resp.new_title.has_value()
                                        ? Json::Value(UTF8(resp.new_title.value(), PLATFORM::WINDOWS))
                                        : Json::Value::null;
            this->log.write("clan_title_change", log_data);
            break;
        }

        case internal::ClanActionType::Join:
        {
            if (resp.new_member.has_value() == false || resp.target.has_value() == false)
                break;

            auto role = resp.new_role.has_value() ? static_cast<CLAN_ROLE>(resp.new_role.value()) : CLAN_ROLE::MATE;
            clan->join(clan_member{resp.new_member.value().name, role});

            co_await this->characters.async_write([this, &resp, clan](auto& characters) -> async::task<void> {
                auto ch = characters.find(resp.new_member.value().name);
                if (ch == nullptr)
                    co_return;

                if (ch->clan_id().has_value())
                    co_return;

                auto weak    = ch->template weak_from_this_as<character>();
                auto members = std::vector<std::shared_ptr<fb::game::character>>();
                for (auto& [_, weak_ptr] : clan->characters())
                {
                    auto shared_ptr = weak_ptr.lock();
                    if (shared_ptr == nullptr)
                        continue;
                    members.push_back(shared_ptr);
                }

                characters.foreach_enqueue(
                    [this, &resp](auto& member) -> async::task<void> {
                        member->message(std::format(_TEXT(MESSAGE_CLAN_MEMBER_JOINED), resp.new_member.value().name),
                                        MESSAGE_TYPE::NOTIFY);
                        co_return;
                    },
                    members);

                if (weak.expired() == false)
                {
                    clan->attach(weak);
                    ch->clan_id(clan->id());
                    ch->update_external(false);
                    ch->message(std::format(_TEXT(MESSAGE_CLAN_JOINED_SUCCESS), clan->name()), MESSAGE_TYPE::NOTIFY);

                    auto log_data              = Json::Value();
                    log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
                    log_data["character_name"] = UTF8(resp.new_member.value().name, PLATFORM::WINDOWS);
                    log_data["clan_id"]        = static_cast<Json::Int64>(clan->id());
                    log_data["clan_name"]      = UTF8(clan->name(), PLATFORM::WINDOWS);
                    this->log.write("clan_join", log_data);
                }
            });
            break;
        }

        case internal::ClanActionType::Leave:
        case internal::ClanActionType::Kick:
        {
            if (resp.deleted_member.has_value() == false)
                break;

            this->characters.write([this, &resp, clan](auto& characters) {
                auto ch = characters.find(resp.deleted_member.value().name);
                if (ch != nullptr)
                {
                    auto weak = ch->template weak_from_this_as<character>();
                    clan->detach(weak);
                    ch->clan_reset();
                    ch->update_external(false);
                    ch->message(resp.action == internal::ClanActionType::Kick ? _TEXT(MESSAGE_CLAN_KICKED)
                                                                              : _TEXT(MESSAGE_CLAN_LEFT),
                                MESSAGE_TYPE::NOTIFY);

                    auto log_data              = Json::Value();
                    log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
                    log_data["character_name"] = UTF8(resp.deleted_member.value().name, PLATFORM::WINDOWS);
                    log_data["clan_id"]        = static_cast<Json::Int64>(clan->id());
                    log_data["clan_name"]      = UTF8(clan->name(), PLATFORM::WINDOWS);
                    this->log.write(resp.action == internal::ClanActionType::Kick ? "clan_kick" : "clan_leave",
                                    log_data);
                }

                clan->leave(resp.deleted_member.value().name);
                auto members = std::vector<std::shared_ptr<fb::game::character>>();
                for (auto& [_, weak_ptr] : clan->characters())
                {
                    auto shared_ptr = weak_ptr.lock();
                    if (shared_ptr == nullptr)
                        continue;
                    members.push_back(shared_ptr);
                }

                auto message = resp.action == internal::ClanActionType::Kick
                                   ? std::format(_TEXT(MESSAGE_CLAN_MEMBER_KICKED), resp.deleted_member.value().name)
                                   : std::format(_TEXT(MESSAGE_CLAN_MEMBER_LEFT), resp.deleted_member.value().name);
                characters.foreach_enqueue(
                    [this, message](auto& member) -> async::task<void> {
                        member->message(message, MESSAGE_TYPE::NOTIFY);
                        co_return;
                    },
                    members);
            });
            break;
        }

        case internal::ClanActionType::ChangeRole:
        {
            if (resp.target.has_value() == false)
                break;

            this->characters.write([this, &resp, clan](auto& characters) {
                auto member = clan->member(resp.target.value().name);
                if (member != nullptr && resp.new_role.has_value())
                    member->role = static_cast<CLAN_ROLE>(resp.new_role.value());

                auto target = characters.find(resp.target.value().uid);
                if (target != nullptr && resp.old_role.has_value() && resp.new_role.has_value())
                {
                    target->message(
                        std::format(_TEXT(MESSAGE_CLAN_ROLE_CHANGED), resp.old_role.value(), resp.new_role.value()),
                        MESSAGE_TYPE::NOTIFY);
                }

                for (auto& [uid, weak] : clan->characters())
                {
                    if (resp.target.has_value() && uid == resp.target.value().uid)
                        continue;

                    auto shared = weak.lock();
                    if (shared == nullptr)
                        continue;

                    if (resp.target.has_value() && resp.old_role.has_value() && resp.new_role.has_value())
                    {
                        shared->message(std::format(_TEXT(MESSAGE_CLAN_ROLE_CHANGED_DETAILED),
                                                    resp.target.value().name,
                                                    resp.old_role.value(),
                                                    resp.new_role.value()),
                                        MESSAGE_TYPE::NOTIFY);
                    }
                }
            });
            break;
        }

        default:
            break;
        }
    });
}

async::task<void> server::on_clan_broadcast(const internal_resp::BroadcastClan& resp)
{
    this->assert_clan(resp.error);

    co_await this->ensure_clan(resp.clan, [this, resp](auto& clan) -> async::task<void> {
        auto members = std::vector<std::shared_ptr<fb::game::character>>();
        for (auto& [_, weak_ptr] : clan->characters())
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                continue;

            members.push_back(shared_ptr);
        }

        this->characters.write([this, &resp, &members](auto& characters) {
            characters.foreach_enqueue(
                [this, resp](auto& member) -> async::task<void> {
                    member->message(resp.message, static_cast<MESSAGE_TYPE>(resp.type));
                    co_return;
                },
                members);
        });
        co_return;
    });
}