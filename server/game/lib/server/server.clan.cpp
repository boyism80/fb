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

async::task<void> server::ensure_clan(uint32_t id, std::function<async::task<void>(std::shared_ptr<fb::game::clan>&)> fn)
{
    co_await this->clans.async_write(
        id,
        [this, id, fn](auto& clan) -> async::task<void> {
            co_await fn(clan);
        },
        [=, this]() -> async::task<std::shared_ptr<fb::game::clan>> {
            auto&& resp = co_await this->http.get<internal_resp::ClanDetails>("internal", std::format("/clan/{}", id));
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
    auto members = std::unordered_map<std::string, clan_member>{};
    for (auto& member : resp2)
    {
        members.insert({
            member.name,
            clan_member{member.name, static_cast<CLAN_ROLE>(member.role)}
        });
    }

    clan.update(resp1.name, resp1.title, members);
}

async::task<void> server::on_create_clan(const internal_resp::ClanDetails& resp)
{
    this->assert_clan(resp.error);

    // Create clan directly from ClanDetails without calling GET API
    // ClanDetails response contains all necessary data (same as GET /clan/{id} response)
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

    // Store master member info for character attachment
    auto master_uid = static_cast<uint32_t>(0);
    for (auto& member : resp.members)
    {
        if (static_cast<CLAN_ROLE>(member.role) == CLAN_ROLE::MASTER)
        {
            master_uid = member.uid;
            break;
        }
    }

    // Use async_write with factory to create or update clan
    // Factory creates clan from ClanDetails data (no API call needed)
    co_await this->clans.async_write(
        id,
        [=, this](auto& clan) -> async::task<void> {
            // Update existing clan or use newly created one
            clan->update(clan_name, clan_title, members);

            // Try to attach master character if it exists in this server
            // Character may have moved to another server after API request, so we check at message receive time
            if (master_uid != 0)
            {
                co_await this->characters.async_read([id, master_uid, &clan, this](auto& characters) -> async::task<void> {
                    auto ch = characters.find(master_uid);
                    if (ch != nullptr)
                    {
                        ch->clan_id(id);
                        clan->attach_character(ch->weak_from_this_as<character>());

                        // Log clan create event only if master is in this server
                        auto log_data              = Json::Value();
                        log_data["character_id"]   = static_cast<Json::Int64>(ch->id());
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
            // Factory: Create new clan from ClanDetails data (no API call)
            co_return std::make_shared<fb::game::clan>(*this, id, clan_name, clan_title, members);
        });
}

async::task<void> server::create_clan(character& me, std::string name)
{
    auto weak = me.weak_from_this_as<character>();
    try
    {
        // Check if already in a clan
        if (me.clan_id().has_value())
            throw std::runtime_error(_TEXT(MESSAGE_ALREADY_JOINED_CLAN));

        // Call API (clan name uniqueness check is done on server side)
        auto&& resp = co_await this->http.post("internal", "/clan/create", CreateClan{fb::config<uint32_t>("id"), me.id(), name});
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
        // Check if in a clan
        auto clan_id = me.clan_id(); // don't use auto& because it will be invalidated after the clan is destroyed
        if (clan_id.has_value() == false)
            throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

        // Verify privilege before API call (ensure_clan is thread-safe)
        co_await this->ensure_clan(clan_id.value(), [this, &me](auto& clan) -> async::task<void> {
            // Check if clan has only one member (master only)
            if (clan->members().size() != 1)
                throw std::runtime_error(_TEXT(MESSAGE_CLAN_MEMBER_EXISTS));

            // Check if the member is the master
            auto member = clan->member(me.name());
            if (member == nullptr)
                throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

            // Check if master role
            if (member->role != CLAN_ROLE::MASTER)
                throw std::runtime_error(_TEXT(MESSAGE_CLAN_NO_PRIVILEGE));

            // Verify that this is the master (name match)
            if (member->name != me.name())
                throw std::runtime_error(_TEXT(MESSAGE_CLAN_NO_PRIVILEGE));
            co_return;
        });

        // Call API (checks passed)
        auto&& resp = co_await this->http.post("internal", "/clan/destroy", DestroyClan{fb::config<uint32_t>("id"), me.id()});
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
        // Check inviter's clan status
        auto inviter_clan_id = inviter.clan_id();
        if (inviter_clan_id.has_value() == false)
            throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

        // Try to find target in the same thread's thread_params
        auto current_thread = this->threads.current();
        if (current_thread != nullptr)
        {
            auto params = current_thread->template data<thread_params>();
            auto target = params->characters.find(target_name);
            if (target != nullptr)
            {
                // Found target in same thread - check state without thread switching
                if (target->clan_id().has_value())
                    throw std::runtime_error(_TEXT(MESSAGE_ALREADY_JOINED_CLAN));
            }
        }

        // Call API (either target not found in same thread, or checks passed)
        auto&& resp = co_await this->http.post("internal", "/clan/join", request::JoinClan{fb::config<uint32_t>("host"), inviter.id(), target_name});
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
    try
    {
        // Check if in a clan
        auto clan_id = leaver.clan_id();
        if (clan_id.has_value() == false)
            throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

        // Verify that leaver is not master before API call (ensure_clan is thread-safe)
        co_await this->ensure_clan(clan_id.value(), [this, &leaver](auto& clan) -> async::task<void> {
            auto member = clan->member(leaver.name());
            if (member != nullptr)
            {
                // Master cannot leave clan (must use destroy_clan instead)
                if (member->role == CLAN_ROLE::MASTER)
                    throw std::runtime_error(_TEXT(MESSAGE_CLAN_CANNOT_LEAVE_MASTER));
            }
            co_return;
        });

        // Call API (checks passed)
        auto&& resp = co_await this->http.post("internal", "/clan/leave", request::LeaveClan{fb::config<uint32_t>("host"), clan_id.value(), leaver.name()});
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

async::task<void> server::kick_clan_member(character& kicker, const std::string& target_name)
{
    auto weak = kicker.weak_from_this_as<character>();
    try
    {
        // Check kicker's clan status
        auto kicker_clan_id = kicker.clan_id();
        if (kicker_clan_id.has_value() == false)
            throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

        // Try to find target in the same thread's thread_params
        auto current_thread = this->threads.current();
        if (current_thread != nullptr)
        {
            auto params = current_thread->template data<thread_params>();
            auto target = params->characters.find(target_name);

            if (target != nullptr)
            {
                // Found target in same thread - check state without thread switching
                auto target_clan_id = target->clan_id();
                if (target_clan_id.has_value() == false || target_clan_id.value() != kicker_clan_id.value())
                    throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));
            }
        }

        // Call API (either target not found in same thread, or checks passed)
        auto&& resp = co_await this->http.post("internal", "/clan/kick", request::KickClan{fb::config<uint32_t>("host"), kicker_clan_id.value(), kicker.name(), target_name});
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

async::task<void> server::change_clan_role(character& changer, const std::string& target_name, CLAN_ROLE role)
{
    auto weak = changer.weak_from_this_as<character>();
    try
    {
        // Check changer's clan status
        auto changer_clan_id = changer.clan_id();
        if (changer_clan_id.has_value() == false)
            throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

        // Try to find target in the same thread's thread_params
        auto current_thread = this->threads.current();
        if (current_thread != nullptr)
        {
            auto params = current_thread->template data<thread_params>();
            auto target = params->characters.find(target_name);

            if (target != nullptr)
            {
                // Found target in same thread - check state without thread switching
                auto target_clan_id = target->clan_id();
                if (target_clan_id.has_value() == false || target_clan_id.value() != changer_clan_id.value())
                    throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));
            }
        }

        // Call API (either target not found in same thread, or checks passed)
        auto&& resp =
            co_await this->http.post("internal",
                                     "/clan/change-role",
                                     request::ChangeClanRole{fb::config<uint32_t>("host"), changer.id(), target_name, changer_clan_id.value(), static_cast<uint32_t>(role)});
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

async::task<void> server::set_clan_title(character& changer, const std::string& title)
{
    auto weak = changer.weak_from_this_as<character>();
    try
    {
        // Check changer's clan status
        auto changer_clan_id = changer.clan_id();
        if (changer_clan_id.has_value() == false)
            throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

        // Verify privilege and title before API call (ensure_clan is thread-safe)
        co_await this->ensure_clan(changer_clan_id.value(), [this, &changer, &title](auto& clan) -> async::task<void> {
            auto member = clan->member(changer.name());
            if (member != nullptr)
            {
                // Check if changer has sufficient privileges (Master role or higher)
                if (static_cast<uint32_t>(member->role) < static_cast<uint32_t>(fb::model::const_value::clan::MINIMUM_CHANGE_TITLE_PRIVILEGE))
                    throw std::runtime_error(_TEXT(MESSAGE_CLAN_NO_PRIVILEGE));
            }

            // Check if title is the same (no change needed)
            auto current_title = clan->title();
            if (current_title.has_value() && current_title.value() == title)
                throw std::runtime_error(_TEXT(MESSAGE_CLAN_TITLE_NOT_CHANGED));

            // Check if title is too short (minimum 2 characters)
            if (!title.empty() && title.length() < 2)
                throw std::runtime_error(_TEXT(MESSAGE_CLAN_TITLE_TOO_SHORT));
            co_return;
        });

        // Call API (checks passed)
        auto&& resp = co_await this->http.post("internal", "/clan/title", request::SetClanTitle{fb::config<uint32_t>("host"), changer.id(), title});
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

async::task<void> server::broadcast_clan(uint32_t clan_id, const std::string& message, MESSAGE_TYPE type)
{
    auto&& resp = co_await this->http.post("internal", "/clan/broadcast", request::BroadcastClan{fb::config<uint32_t>("host"), clan_id, message, static_cast<uint8_t>(type)});
    co_await this->on_clan_broadcast(resp);
}

async::task<void> server::on_destroyed_clan(const internal_resp::DestroyClan& resp)
{
    this->assert_clan(resp.error);

    // Atomic operation: read clan data, send message, and erase in one atomic block
    // async_erase callback receives the clan before erasure, and callback is only called if clan exists
    co_await this->clans.async_erase(resp.clan_id, [this, &resp](const auto& clan) -> async::task<void> {
        auto members = std::vector<std::shared_ptr<fb::game::character>>();
        for (auto& [_, weak_ptr] : clan->characters())
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                continue;
            members.push_back(shared_ptr);
        }

        auto message = std::format(_TEXT(MESSAGE_CLAN_DISBANDED), resp.clan_name);
        co_await this->characters.async_write([this, message, members](auto& characters) -> async::task<void> {
            co_await characters.foreach (
                [this, message](auto& ch) {
                    ch->message(message, MESSAGE_TYPE::NOTIFY);
                    ch->clan_reset();
                },
                members);
        });
    });
    // If clan doesn't exist, callback is not called (no exception thrown)
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

            // Log clan title change event
            auto log_data         = Json::Value();
            log_data["clan_id"]   = static_cast<Json::Int64>(clan->id());
            log_data["clan_name"] = UTF8(clan->name(), PLATFORM::WINDOWS);
            log_data["old_title"] = old_title.has_value() ? Json::Value(UTF8(old_title.value(), PLATFORM::WINDOWS)) : Json::Value::null;
            log_data["new_title"] = resp.new_title.has_value() ? Json::Value(UTF8(resp.new_title.value(), PLATFORM::WINDOWS)) : Json::Value::null;
            this->log.write("clan_title_change", log_data);
            break;
        }

        case internal::ClanActionType::Join:
        {
            if (resp.new_member.has_value() == false || resp.target.has_value() == false)
                break;

            co_await this->characters.async_write([this, &resp, clan](auto& characters) -> async::task<void> {
                auto ch = characters.find(resp.new_member.value().name);
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
                    [this, &resp](auto& member) {
                        member->message(std::format(_TEXT(MESSAGE_CLAN_MEMBER_JOINED), resp.new_member.value().name), MESSAGE_TYPE::NOTIFY);
                    },
                    members);

                if (weak.expired() == false)
                {
                    clan->attach_character(weak);
                    ch->clan_id(clan->id());
                    ch->update_external(false);
                    ch->message(std::format(_TEXT(MESSAGE_CLAN_JOINED_SUCCESS), clan->name()), MESSAGE_TYPE::NOTIFY);

                    // Log clan join event
                    auto log_data              = Json::Value();
                    log_data["character_id"]   = static_cast<Json::Int64>(ch->id());
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

            co_await this->characters.async_write([this, &resp, clan](auto& characters) -> async::task<void> {
                auto ch = characters.find(resp.deleted_member.value().name);
                if (ch != nullptr)
                {
                    auto weak = ch->weak_from_this_as<character>();
                    clan->detach(weak);
                    ch->clan_reset();
                    ch->update_external(false);
                    ch->message(resp.action == internal::ClanActionType::Kick ? _TEXT(MESSAGE_CLAN_KICKED) : _TEXT(MESSAGE_CLAN_LEFT), MESSAGE_TYPE::NOTIFY);

                    // Log clan leave/kick event
                    auto log_data              = Json::Value();
                    log_data["character_id"]   = static_cast<Json::Int64>(ch->id());
                    log_data["character_name"] = UTF8(resp.deleted_member.value().name, PLATFORM::WINDOWS);
                    log_data["clan_id"]        = static_cast<Json::Int64>(clan->id());
                    log_data["clan_name"]      = UTF8(clan->name(), PLATFORM::WINDOWS);
                    this->log.write(resp.action == internal::ClanActionType::Kick ? "clan_kick" : "clan_leave", log_data);
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

                auto message = resp.action == internal::ClanActionType::Kick ? std::format(_TEXT(MESSAGE_CLAN_MEMBER_KICKED), resp.deleted_member.value().name)
                                                                             : std::format(_TEXT(MESSAGE_CLAN_MEMBER_LEFT), resp.deleted_member.value().name);
                co_await characters.foreach (
                    [this, message](auto& member) {
                        member->message(message, MESSAGE_TYPE::NOTIFY);
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
                    target->message(std::format(_TEXT(MESSAGE_CLAN_ROLE_CHANGED), resp.old_role.value(), resp.new_role.value()), MESSAGE_TYPE::NOTIFY);
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
                        shared->message(std::format(_TEXT(MESSAGE_CLAN_ROLE_CHANGED_DETAILED), resp.target.value().name, resp.old_role.value(), resp.new_role.value()),
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

        co_await this->characters.async_write([this, &resp, &members](auto& characters) -> async::task<void> {
            co_await characters.foreach (
                [this, resp](auto& member) {
                    member->message(resp.message, static_cast<MESSAGE_TYPE>(resp.type));
                },
                members);
        });
    });
}