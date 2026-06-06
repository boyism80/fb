#include <fb/game/clan/container.h>
#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/game/thread_params.h>
#include <fb/encoding.h>
#include <json/json.h>
#include <macro.h>

using namespace fb::game;
namespace internal_resp = fb::protocol::internal::response;
namespace internal_reqs = fb::protocol::internal::request;

async::task<void> clan_container::apply_updated(clan_container& container, const internal_resp::UpdatedClan& resp)
{
    co_await container.on_error(resp.error);

    switch (resp.action)
    {
    case internal::ClanActionType::Join:
    {
        auto role = resp.new_role.has_value() ? static_cast<CLAN_ROLE>(resp.new_role.value()) : CLAN_ROLE::MATE;
        co_await container.on_join(resp.clan_id,
                                   resp.new_member.has_value() ? std::optional(resp.new_member.value().name)
                                                               : std::nullopt,
                                   role);
        break;
    }

    case internal::ClanActionType::Leave:
        co_await container.on_leave(resp.clan_id,
                                    resp.deleted_member.has_value() ? std::optional(resp.deleted_member.value().name)
                                                                    : std::nullopt);
        break;

    case internal::ClanActionType::Kick:
        co_await container.on_kick(resp.clan_id,
                                   resp.deleted_member.has_value() ? std::optional(resp.deleted_member.value().name)
                                                                   : std::nullopt);
        break;

    case internal::ClanActionType::ChangeRole:
        co_await container.on_change_role(
            resp.clan_id,
            resp.target.has_value() ? std::optional(resp.target.value().uid) : std::nullopt,
            resp.target.has_value() ? std::optional(resp.target.value().name) : std::nullopt,
            resp.old_role,
            resp.new_role);
        break;

    case internal::ClanActionType::SetTitle:
        co_await container.on_set_title(resp.clan_id, resp.new_title);
        break;

    default:
        break;
    }
}

clan_container::clan_container(fb::game::server& server) :
    fb::sharded_container<clan_ptr, 16>([](const clan_ptr& clan) -> uint32_t {
        return clan->id();
    }),
    server(server)
{ }

void clan_container::detach(std::weak_ptr<character> weak, uint32_t clan_id)
{
    this->write(clan_id, [weak](auto& clan) {
        clan->detach(weak);
    });
}

async::task<void> clan_container::on_error(uint32_t error)
{
    switch (static_cast<ERROR_CODE>(error))
    {
    case ERROR_CODE::NONE:
        co_return;

    case ERROR_CODE::CLAN_NAME_ALREADY_EXISTS:
        throw std::runtime_error(_TEXT(MESSAGE_CLAN_NAME_ALREADY_EXISTS));

    default:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), error));
    }
}

async::task<void> clan_container::ensure(uint32_t id, ensure_fn fn)
{
    auto thread = this->server.threads.current();
    if (thread == nullptr)
        throw std::runtime_error(std::format("No thread available for ensure (clan_id: {})", id));

    auto builder        = thread->new_builder<>();
    builder.retry_count = 10;
    builder.func        = [=, this](auto& thread) -> async::task<void> {
        if (co_await this->try_async_write(
                id,
                [this, fn, &thread](auto& clan) -> async::task<void> {
                    co_await fn(clan);
                    co_await thread.switching();
                },
                [=, this]() -> async::task<clan_ptr> {
                    auto   world = fb::config<uint32_t>("world");
                    auto&& resp  = co_await this->server.http.get<internal_resp::ClanDetails>(
                        "internal",
                        std::format("/clan/{}/{}", world, id));
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
                        co_return this->server.make<clan>(id, resp.clan.name, resp.clan.title, members);
                    }

                    default:
                        throw std::runtime_error(std::format("cannot get clan (error : {})", resp.error));
                    }
                }) == false)
        {
            throw fb::thread::retry_exception{};
        }
    };
    co_await builder.dispatch();
}

async::task<void> clan_container::on_create(uint32_t                                     clan_id,
                                            std::string                                  name,
                                            std::string                                  title,
                                            std::unordered_map<std::string, clan_member> members)
{
    auto clan_title = title.empty() ? std::optional<std::string>{} : std::optional<std::string>{title};

    auto master_name = std::optional<std::string>{};
    for (const auto& [member_name, member] : members)
    {
        std::ignore = member_name;
        if (member.role == CLAN_ROLE::MASTER)
        {
            master_name = member.name;
            break;
        }
    }

    co_await this->async_write(
        clan_id,
        [=, this](auto& clan) -> async::task<void> {
            clan->update(name, clan_title, members);

            if (master_name.has_value())
            {
                auto  guard      = co_await this->server.characters.enter_read_async();
                auto& characters = guard.value();
                auto  ch         = characters.find(master_name.value());
                if (ch != nullptr)
                {
                    auto weak = ch->template weak_from_this_as<character>();
                    clan->attach(weak);

                    auto before = this->server.threads.current();
                    co_await this->server.threads.switching(weak);
                    auto ptr = weak.lock();
                    if (ptr != nullptr)
                        ptr->clan_id(clan_id);
                    if (before != nullptr)
                        co_await before->switching();

                    auto log_data              = Json::Value();
                    log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
                    log_data["character_name"] = UTF8(ch->name(), PLATFORM::WINDOWS);
                    log_data["clan_id"]        = static_cast<Json::Int64>(clan_id);
                    log_data["clan_name"]      = UTF8(clan->name(), PLATFORM::WINDOWS);
                    this->server.log.write("clan_create", log_data);
                }
            }
            co_return;
        },
        [=, this]() -> async::task<clan_ptr> {
            co_return this->server.make<clan>(clan_id, name, clan_title, members);
        });
}

async::task<void> clan_container::create(character& me, std::string_view name)
{
    if (me.clan_id().has_value())
        throw std::runtime_error(_TEXT(MESSAGE_ALREADY_JOINED_CLAN));

    auto   weak     = me.weak_from_this_as<character>();
    auto   name_str = std::string(name);
    auto   world    = fb::config<uint32_t>("world");
    auto&& resp =
        co_await this->server.http.post("internal",
                                        "/clan/create",
                                        internal_reqs::CreateClan{world, fb::config<uint32_t>("id"), me.id, name_str});
    co_await this->server.threads.switching(weak);

    co_await this->on_error(resp.error);

    auto members = std::unordered_map<std::string, clan_member>{};
    for (const auto& member : resp.members)
    {
        members.insert({
            member.name,
            clan_member{member.name, static_cast<CLAN_ROLE>(member.role)}
        });
    }

    auto title = resp.clan.title.has_value() ? resp.clan.title.value() : std::string{};
    co_await this->on_create(resp.clan.id, resp.clan.name, title, std::move(members));
}

async::task<void> clan_container::destroy(character& me)
{
    auto clan_id = me.clan_id();
    if (clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    co_await this->ensure(clan_id.value(), [this, &me](auto& clan) -> async::task<void> {
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

    auto   weak  = me.weak_from_this_as<character>();
    auto   world = fb::config<uint32_t>("world");
    auto&& resp  = co_await this->server.http.post("internal",
                                                  "/clan/destroy",
                                                  internal_reqs::DestroyClan{world, fb::config<uint32_t>("id"), me.id});
    co_await this->server.threads.switching(weak);

    co_await this->on_error(resp.error);
    co_await this->on_destroyed(resp.clan_id, resp.clan_name);
}

async::task<void> clan_container::join_member(character& inviter, std::string_view target_name)
{
    auto inviter_clan_id = inviter.clan_id();
    if (inviter_clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    auto weak            = inviter.weak_from_this_as<character>();
    auto target_name_str = std::string(target_name);

    auto current_thread = this->server.threads.current();
    if (current_thread != nullptr)
    {
        auto params = current_thread->template data<thread_params>();
        auto target = params->characters.find(target_name_str);
        if (target != nullptr)
        {
            if (target->clan_id().has_value())
                throw std::runtime_error(_TEXT(MESSAGE_ALREADY_JOINED_CLAN));
        }
    }

    auto   world = fb::config<uint32_t>("world");
    auto&& resp  = co_await this->server.http.post(
        "internal",
        "/clan/join",
        internal_reqs::JoinClan{world, fb::config<uint32_t>("host"), inviter.id, target_name_str});
    co_await this->server.threads.switching(weak);
    co_await clan_container::apply_updated(*this, resp);
}

async::task<void> clan_container::leave_member(character& leaver)
{
    auto clan_id = leaver.clan_id();
    if (clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    co_await this->ensure(clan_id.value(), [this, &leaver](auto& clan) -> async::task<void> {
        auto member = clan->member(leaver.name());
        if (member != nullptr)
        {
            if (member->role == CLAN_ROLE::MASTER)
                throw std::runtime_error(_TEXT(MESSAGE_CLAN_CANNOT_LEAVE_MASTER));
        }
        co_return;
    });

    auto   weak  = leaver.weak_from_this_as<character>();
    auto   world = fb::config<uint32_t>("world");
    auto&& resp  = co_await this->server.http.post(
        "internal",
        "/clan/leave",
        internal_reqs::LeaveClan{world, fb::config<uint32_t>("host"), clan_id.value(), leaver.name()});
    co_await this->server.threads.switching(weak);
    co_await clan_container::apply_updated(*this, resp);
}

async::task<void> clan_container::kick_member(character& kicker, std::string_view target_name)
{
    auto kicker_clan_id = kicker.clan_id();
    if (kicker_clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    auto weak            = kicker.weak_from_this_as<character>();
    auto target_name_str = std::string(target_name);

    auto current_thread = this->server.threads.current();
    if (current_thread != nullptr)
    {
        auto params = current_thread->template data<thread_params>();
        auto target = params->characters.find(target_name_str);
        if (target != nullptr)
        {
            auto target_clan_id = target->clan_id();
            if (target_clan_id.has_value() == false || target_clan_id.value() != kicker_clan_id.value())
                throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));
        }
    }

    auto   world = fb::config<uint32_t>("world");
    auto&& resp  = co_await this->server.http.post("internal",
                                                  "/clan/kick",
                                                  internal_reqs::KickClan{world,
                                                                          fb::config<uint32_t>("host"),
                                                                          kicker_clan_id.value(),
                                                                          kicker.name(),
                                                                          target_name_str});
    co_await this->server.threads.switching(weak);
    co_await clan_container::apply_updated(*this, resp);
}

async::task<void> clan_container::change_role(character& changer, std::string_view target_name, CLAN_ROLE role)
{
    auto changer_clan_id = changer.clan_id();
    if (changer_clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    auto weak            = changer.weak_from_this_as<character>();
    auto target_name_str = std::string(target_name);

    auto current_thread = this->server.threads.current();
    if (current_thread != nullptr)
    {
        auto params = current_thread->template data<thread_params>();
        auto target = params->characters.find(target_name_str);
        if (target != nullptr)
        {
            auto target_clan_id = target->clan_id();
            if (target_clan_id.has_value() == false || target_clan_id.value() != changer_clan_id.value())
                throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));
        }
    }

    auto   world = fb::config<uint32_t>("world");
    auto&& resp  = co_await this->server.http.post("internal",
                                                  "/clan/change-role",
                                                  internal_reqs::ChangeClanRole{world,
                                                                                fb::config<uint32_t>("host"),
                                                                                changer.id,
                                                                                target_name_str,
                                                                                changer_clan_id.value(),
                                                                                static_cast<uint32_t>(role)});
    co_await this->server.threads.switching(weak);
    co_await clan_container::apply_updated(*this, resp);
}

async::task<void> clan_container::set_title(character& changer, std::string_view title)
{
    auto changer_clan_id = changer.clan_id();
    if (changer_clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    auto weak      = changer.weak_from_this_as<character>();
    auto title_str = std::string(title);

    co_await this->ensure(changer_clan_id.value(), [this, &changer, &title_str](auto& clan) -> async::task<void> {
        auto member = clan->member(changer.name());
        if (member != nullptr)
        {
            if (static_cast<uint32_t>(member->role) <
                static_cast<uint32_t>(fb::model::const_value::clan::MINIMUM_CHANGE_TITLE_PRIVILEGE))
                throw std::runtime_error(_TEXT(MESSAGE_CLAN_NO_PRIVILEGE));
        }

        auto current_title = clan->title();
        if (current_title.has_value() && current_title.value() == title_str)
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_TITLE_NOT_CHANGED));

        if (title_str.empty() == false && title_str.length() < 2)
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_TITLE_TOO_SHORT));
        co_return;
    });

    auto   world = fb::config<uint32_t>("world");
    auto&& resp  = co_await this->server.http.post(
        "internal",
        "/clan/title",
        internal_reqs::SetClanTitle{world, fb::config<uint32_t>("host"), changer.id, title_str});
    co_await this->server.threads.switching(weak);
    co_await clan_container::apply_updated(*this, resp);
}

async::task<void> clan_container::broadcast(uint32_t clan_id, std::string_view message, MESSAGE_TYPE type)
{
    auto   message_str = std::string(message);
    auto   world       = fb::config<uint32_t>("world");
    auto&& resp        = co_await this->server.http.post("internal",
                                                  "/clan/broadcast",
                                                  internal_reqs::BroadcastClan{world,
                                                                               fb::config<uint32_t>("host"),
                                                                               clan_id,
                                                                               message_str,
                                                                               static_cast<uint8_t>(type)});
    co_await this->on_error(resp.error);
    co_await this->on_broadcast(resp.clan, std::move(resp.message), resp.type);
}

async::task<void> clan_container::on_destroyed(uint32_t clan_id, std::string clan_name)
{
    this->erase(clan_id, [this, clan_name](const auto& clan) {
        auto members = std::vector<std::shared_ptr<character>>{};
        for (auto& [uid, weak_ptr] : clan->characters())
        {
            std::ignore     = uid;
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                continue;
            members.push_back(shared_ptr);
        }

        auto message = std::format(_TEXT(MESSAGE_CLAN_DISBANDED), clan_name);
        auto guard   = this->server.characters.enter_write();
        guard.value().foreach_enqueue(
            [message](auto& ch) -> async::task<void> {
                ch->message(message, MESSAGE_TYPE::NOTIFY);
                ch->clan_reset();
                co_return;
            },
            members);
    });
    co_return;
}

async::task<void> clan_container::on_broadcast(uint32_t clan_id, std::string message, uint8_t type)
{
    co_await this->ensure(clan_id, [this, message = std::move(message), type](auto& clan) -> async::task<void> {
        auto members = std::vector<std::shared_ptr<character>>{};
        for (auto& [uid, weak_ptr] : clan->characters())
        {
            std::ignore     = uid;
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                continue;
            members.push_back(shared_ptr);
        }

        auto guard = this->server.characters.enter_write();
        guard.value().foreach_enqueue(
            [message, type](auto& member) -> async::task<void> {
                member->message(message, static_cast<MESSAGE_TYPE>(type));
                co_return;
            },
            members);
        co_return;
    });
}

async::task<void> clan_container::on_set_title(uint32_t clan_id, std::optional<std::string> new_title)
{
    co_await this->ensure(clan_id, [this, new_title](auto& clan) -> async::task<void> {
        auto old_title = clan->title();
        clan->title(new_title);

        auto log_data         = Json::Value();
        log_data["clan_id"]   = static_cast<Json::Int64>(clan->id());
        log_data["clan_name"] = UTF8(clan->name(), PLATFORM::WINDOWS);
        log_data["old_title"] =
            old_title.has_value() ? Json::Value(UTF8(old_title.value(), PLATFORM::WINDOWS)) : Json::Value::null;
        log_data["new_title"] =
            new_title.has_value() ? Json::Value(UTF8(new_title.value(), PLATFORM::WINDOWS)) : Json::Value::null;
        this->server.log.write("clan_title_change", log_data);
        co_return;
    });
}

async::task<void> clan_container::on_join(uint32_t clan_id, std::optional<std::string> new_member, CLAN_ROLE role)
{
    if (new_member.has_value() == false)
        co_return;

    auto joined_member_name = new_member.value();
    co_await this->ensure(clan_id, [this, joined_member_name, role](auto& clan) -> async::task<void> {
        clan->join(clan_member{joined_member_name, role});

        auto  guard      = co_await this->server.characters.enter_write_async();
        auto& characters = guard.value();
        auto  ch         = characters.find(joined_member_name);
        if (ch == nullptr)
            co_return;

        if (ch->clan_id().has_value())
            co_return;

        auto weak    = ch->template weak_from_this_as<character>();
        auto members = std::vector<std::shared_ptr<character>>{};
        for (auto& [uid, weak_ptr] : clan->characters())
        {
            std::ignore     = uid;
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                continue;
            members.push_back(shared_ptr);
        }

        characters.foreach_enqueue(
            [joined_member_name](auto& member) -> async::task<void> {
                member->message(std::format(_TEXT(MESSAGE_CLAN_MEMBER_JOINED), joined_member_name),
                                MESSAGE_TYPE::NOTIFY);
                co_return;
            },
            members);

        if (weak.expired() == false)
        {
            clan->attach(weak);

            auto before = this->server.threads.current();
            co_await this->server.threads.switching(weak);
            auto ptr = weak.lock();
            if (ptr != nullptr)
            {
                ptr->clan_id(clan->id());
                ptr->update_external(false);
                ptr->message(std::format(_TEXT(MESSAGE_CLAN_JOINED_SUCCESS), clan->name()), MESSAGE_TYPE::NOTIFY);
            }
            if (before != nullptr)
                co_await before->switching();

            auto log_data              = Json::Value();
            log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
            log_data["character_name"] = UTF8(joined_member_name, PLATFORM::WINDOWS);
            log_data["clan_id"]        = static_cast<Json::Int64>(clan->id());
            log_data["clan_name"]      = UTF8(clan->name(), PLATFORM::WINDOWS);
            this->server.log.write("clan_join", log_data);
        }
    });
}

async::task<void> clan_container::on_leave(uint32_t clan_id, std::optional<std::string> deleted_member)
{
    if (deleted_member.has_value() == false)
        co_return;

    auto deleted_member_name = deleted_member.value();
    co_await this->ensure(clan_id, [this, deleted_member_name](auto& clan) -> async::task<void> {
        auto  guard      = co_await this->server.characters.enter_write_async();
        auto& characters = guard.value();
        auto  ch         = characters.find(deleted_member_name);
        if (ch != nullptr)
        {
            auto weak = ch->template weak_from_this_as<character>();
            clan->detach(weak);

            auto before = this->server.threads.current();
            co_await this->server.threads.switching(weak);
            auto ptr = weak.lock();
            if (ptr != nullptr)
            {
                ptr->clan_reset();
                ptr->update_external(false);
                ptr->message(_TEXT(MESSAGE_CLAN_LEFT), MESSAGE_TYPE::NOTIFY);
            }
            if (before != nullptr)
                co_await before->switching();

            auto log_data              = Json::Value();
            log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
            log_data["character_name"] = UTF8(deleted_member_name, PLATFORM::WINDOWS);
            log_data["clan_id"]        = static_cast<Json::Int64>(clan->id());
            log_data["clan_name"]      = UTF8(clan->name(), PLATFORM::WINDOWS);
            this->server.log.write("clan_leave", log_data);
        }

        clan->leave(deleted_member_name);
        auto members = std::vector<std::shared_ptr<character>>{};
        for (auto& [uid, weak_ptr] : clan->characters())
        {
            std::ignore     = uid;
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                continue;
            members.push_back(shared_ptr);
        }

        auto message = std::format(_TEXT(MESSAGE_CLAN_MEMBER_LEFT), deleted_member_name);
        characters.foreach_enqueue(
            [message](auto& member) -> async::task<void> {
                member->message(message, MESSAGE_TYPE::NOTIFY);
                co_return;
            },
            members);
    });
}

async::task<void> clan_container::on_kick(uint32_t clan_id, std::optional<std::string> deleted_member)
{
    if (deleted_member.has_value() == false)
        co_return;

    auto deleted_member_name = deleted_member.value();
    co_await this->ensure(clan_id, [this, deleted_member_name](auto& clan) -> async::task<void> {
        auto  guard      = co_await this->server.characters.enter_write_async();
        auto& characters = guard.value();
        auto  ch         = characters.find(deleted_member_name);
        if (ch != nullptr)
        {
            auto weak = ch->template weak_from_this_as<character>();
            clan->detach(weak);

            auto before = this->server.threads.current();
            co_await this->server.threads.switching(weak);
            auto ptr = weak.lock();
            if (ptr != nullptr)
            {
                ptr->clan_reset();
                ptr->update_external(false);
                ptr->message(_TEXT(MESSAGE_CLAN_KICKED), MESSAGE_TYPE::NOTIFY);
            }
            if (before != nullptr)
                co_await before->switching();

            auto log_data              = Json::Value();
            log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
            log_data["character_name"] = UTF8(deleted_member_name, PLATFORM::WINDOWS);
            log_data["clan_id"]        = static_cast<Json::Int64>(clan->id());
            log_data["clan_name"]      = UTF8(clan->name(), PLATFORM::WINDOWS);
            this->server.log.write("clan_kick", log_data);
        }

        clan->leave(deleted_member_name);
        auto members = std::vector<std::shared_ptr<character>>{};
        for (auto& [uid, weak_ptr] : clan->characters())
        {
            std::ignore     = uid;
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                continue;
            members.push_back(shared_ptr);
        }

        auto message = std::format(_TEXT(MESSAGE_CLAN_MEMBER_KICKED), deleted_member_name);
        characters.foreach_enqueue(
            [message](auto& member) -> async::task<void> {
                member->message(message, MESSAGE_TYPE::NOTIFY);
                co_return;
            },
            members);
    });
}

async::task<void> clan_container::on_change_role(uint32_t                   clan_id,
                                                 std::optional<uint32_t>    target_uid,
                                                 std::optional<std::string> target_name,
                                                 std::optional<uint32_t>    old_role,
                                                 std::optional<uint32_t>    new_role)
{
    if (target_name.has_value() == false)
        co_return;

    co_await this->ensure(
        clan_id,
        [this, target_uid, target_name, old_role, new_role](auto& clan) -> async::task<void> {
            auto  guard      = this->server.characters.enter_write();
            auto& characters = guard.value();

            auto member = clan->member(target_name.value());
            if (member != nullptr && new_role.has_value())
                member->role = static_cast<CLAN_ROLE>(new_role.value());

            if (target_uid.has_value() && old_role.has_value() && new_role.has_value())
            {
                auto target = characters.find(target_uid.value());
                if (target != nullptr)
                {
                    auto targets = std::vector<std::shared_ptr<character>>{target};
                    auto message = std::format(_TEXT(MESSAGE_CLAN_ROLE_CHANGED), old_role.value(), new_role.value());
                    characters.foreach_enqueue(
                        [message](auto& target_member) -> async::task<void> {
                            target_member->message(message, MESSAGE_TYPE::NOTIFY);
                            co_return;
                        },
                        targets);
                }
            }

            auto members = std::vector<std::shared_ptr<character>>{};
            for (auto& [uid, weak] : clan->characters())
            {
                if (target_uid.has_value() && uid == target_uid.value())
                    continue;

                auto shared = weak.lock();
                if (shared == nullptr)
                    continue;
                members.push_back(shared);
            }

            if (old_role.has_value() && new_role.has_value())
            {
                auto message = std::format(_TEXT(MESSAGE_CLAN_ROLE_CHANGED_DETAILED),
                                           target_name.value(),
                                           old_role.value(),
                                           new_role.value());
                characters.foreach_enqueue(
                    [message](auto& member_ptr) -> async::task<void> {
                        member_ptr->message(message, MESSAGE_TYPE::NOTIFY);
                        co_return;
                    },
                    members);
            }
            co_return;
        });
}