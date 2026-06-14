#include <fb/game/clan.h>
#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/game/thread_params.h>
#include <fb/encoding.h>
#include <json/json.h>
#include <macro.h>

using namespace fb::game;
namespace internal_resp = fb::protocol::internal::response;

async::task<void> clan::container::apply_updated(const internal_resp::UpdatedClan& resp)
{
    co_await this->on_error(resp.error);

    switch (resp.action)
    {
    case internal::ClanActionType::Join:
    {
        auto role = resp.new_role.has_value() ? static_cast<CLAN_ROLE>(resp.new_role.value()) : CLAN_ROLE::MATE;
        co_await this->on_join(resp.clan_id,
                               resp.new_member.has_value() ? std::optional(resp.new_member.value().name) : std::nullopt,
                               role);
        break;
    }

    case internal::ClanActionType::Leave:
        co_await this->on_leave(resp.clan_id,
                                resp.deleted_member.has_value() ? std::optional(resp.deleted_member.value().name)
                                                                : std::nullopt);
        break;

    case internal::ClanActionType::Kick:
        co_await this->on_kick(resp.clan_id,
                               resp.deleted_member.has_value() ? std::optional(resp.deleted_member.value().name)
                                                               : std::nullopt);
        break;

    case internal::ClanActionType::ChangeRole:
        co_await this->on_change_role(resp.clan_id,
                                      resp.target.has_value() ? std::optional(resp.target.value().uid) : std::nullopt,
                                      resp.target.has_value() ? std::optional(resp.target.value().name) : std::nullopt,
                                      resp.old_role,
                                      resp.new_role);
        break;

    case internal::ClanActionType::SetTitle:
        co_await this->on_set_title(resp.clan_id, resp.new_title);
        break;

    default:
        break;
    }
}

async::task<void> clan::container::on_error(uint32_t error)
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

async::task<void> clan::container::on_create(uint32_t                                     clan_id,
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
                auto  guard      = co_await this->_server.characters.enter_read_async();
                auto& characters = guard.value();
                auto  ch         = characters.find(master_name.value());
                if (ch != nullptr)
                {
                    auto weak = ch->template weak_from_this_as<character>();
                    clan->attach(weak);

                    auto before = this->_server.threads.current();
                    co_await this->_server.threads.switching(weak);
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
                    this->_server.log.write("clan_create", log_data);
                }
            }
            co_return;
        },
        [=, this]() -> async::task<entity_ptr> {
            co_return this->_server.make<fb::game::clan>(clan_id, name, clan_title, members);
        });
}

async::task<void> clan::container::on_destroyed(uint32_t clan_id, std::string clan_name)
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
        auto guard   = this->_server.characters.enter_write();
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

async::task<void> clan::container::on_broadcast(uint32_t clan_id, std::string message, uint8_t type)
{
    auto guard = co_await this->ensure(clan_id);
    if (guard.value() == nullptr)
        co_return;
    auto& clan = guard.value();

    auto members = std::vector<std::shared_ptr<character>>{};
    for (auto& [uid, weak_ptr] : clan->characters())
    {
        std::ignore     = uid;
        auto shared_ptr = weak_ptr.lock();
        if (shared_ptr == nullptr)
            continue;
        members.push_back(shared_ptr);
    }

    auto char_guard = this->_server.characters.enter_write();
    char_guard.value().foreach_enqueue(
        [message, type](auto& member) -> async::task<void> {
            member->message(message, static_cast<MESSAGE_TYPE>(type));
            co_return;
        },
        members);
}

async::task<void> clan::container::on_set_title(uint32_t clan_id, std::optional<std::string> new_title)
{
    auto guard = co_await this->ensure(clan_id);
    if (guard.value() == nullptr)
        co_return;
    auto& clan = guard.value();

    auto old_title = clan->title();
    clan->title(new_title);

    auto log_data         = Json::Value();
    log_data["clan_id"]   = static_cast<Json::Int64>(clan->id());
    log_data["clan_name"] = UTF8(clan->name(), PLATFORM::WINDOWS);
    log_data["old_title"] =
        old_title.has_value() ? Json::Value(UTF8(old_title.value(), PLATFORM::WINDOWS)) : Json::Value::null;
    log_data["new_title"] =
        new_title.has_value() ? Json::Value(UTF8(new_title.value(), PLATFORM::WINDOWS)) : Json::Value::null;
    this->_server.log.write("clan_title_change", log_data);
}

async::task<void> clan::container::on_join(uint32_t clan_id, std::optional<std::string> new_member, CLAN_ROLE role)
{
    if (new_member.has_value() == false)
        co_return;

    auto joined_member_name = new_member.value();
    auto guard              = co_await this->ensure(clan_id);
    if (guard.value() == nullptr)
        co_return;
    auto& clan = guard.value();

    clan->join(clan_member{joined_member_name, role});

    auto  char_guard = co_await this->_server.characters.enter_write_async();
    auto& characters = char_guard.value();
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
            member->message(std::format(_TEXT(MESSAGE_CLAN_MEMBER_JOINED), joined_member_name), MESSAGE_TYPE::NOTIFY);
            co_return;
        },
        members);

    if (weak.expired() == false)
    {
        clan->attach(weak);

        auto before = this->_server.threads.current();
        co_await this->_server.threads.switching(weak);
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
        this->_server.log.write("clan_join", log_data);
    }
}

async::task<void> clan::container::on_leave(uint32_t clan_id, std::optional<std::string> deleted_member)
{
    if (deleted_member.has_value() == false)
        co_return;

    auto deleted_member_name = deleted_member.value();
    auto guard               = co_await this->ensure(clan_id);
    if (guard.value() == nullptr)
        co_return;
    auto& clan = guard.value();

    auto  char_guard = co_await this->_server.characters.enter_write_async();
    auto& characters = char_guard.value();
    auto  ch         = characters.find(deleted_member_name);
    if (ch != nullptr)
    {
        auto weak = ch->template weak_from_this_as<character>();
        clan->detach(weak);

        auto before = this->_server.threads.current();
        co_await this->_server.threads.switching(weak);
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
        this->_server.log.write("clan_leave", log_data);
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
}

async::task<void> clan::container::on_kick(uint32_t clan_id, std::optional<std::string> deleted_member)
{
    if (deleted_member.has_value() == false)
        co_return;

    auto deleted_member_name = deleted_member.value();
    auto guard               = co_await this->ensure(clan_id);
    if (guard.value() == nullptr)
        co_return;
    auto& clan = guard.value();

    auto  char_guard = co_await this->_server.characters.enter_write_async();
    auto& characters = char_guard.value();
    auto  ch         = characters.find(deleted_member_name);
    if (ch != nullptr)
    {
        auto weak = ch->template weak_from_this_as<character>();
        clan->detach(weak);

        auto before = this->_server.threads.current();
        co_await this->_server.threads.switching(weak);
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
        this->_server.log.write("clan_kick", log_data);
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
}

async::task<void> clan::container::on_change_role(uint32_t                   clan_id,
                                                  std::optional<uint32_t>    target_uid,
                                                  std::optional<std::string> target_name,
                                                  std::optional<uint32_t>    old_role,
                                                  std::optional<uint32_t>    new_role)
{
    if (target_name.has_value() == false)
        co_return;

    auto guard = co_await this->ensure(clan_id);
    if (guard.value() == nullptr)
        co_return;
    auto& clan = guard.value();

    auto  char_guard = this->_server.characters.enter_write();
    auto& characters = char_guard.value();

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
}
