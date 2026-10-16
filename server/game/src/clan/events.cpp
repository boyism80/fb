#include <fb/encoding.h>
#include <fb/game/character.h>
#include <fb/game/castle.h>
#include <fb/game/clan.h>
#include <fb/game/server.h>
#include <fb/game/thread_params.h>
#include <macro.h>

#include <json/json.h>

#include <cstdint>
#include <format>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

using namespace fb::game;
namespace internal_resp = fb::protocol::internal::response;

async::task<void> clan::container::on_updated(const internal_resp::UpdatedClan& resp)
{
    co_await this->on_error(resp.error);
    auto at = version_time(resp.updated_date);

    switch (resp.action)
    {
    case internal::ClanActionType::Join:
    {
        auto role = resp.new_role.has_value() ? static_cast<CLAN_ROLE>(resp.new_role.value()) : CLAN_ROLE::MATE;
        co_await this->on_join(resp.clan_id,
                               resp.new_member.has_value() ? std::optional(resp.new_member.value().name) : std::nullopt,
                               role,
                               at);
        break;
    }

    case internal::ClanActionType::Leave:
        co_await this->on_leave(resp.clan_id,
                                resp.deleted_member.has_value() ? std::optional(resp.deleted_member.value().name)
                                                                : std::nullopt,
                                at);
        break;

    case internal::ClanActionType::Kick:
        co_await this->on_kick(resp.clan_id,
                               resp.deleted_member.has_value() ? std::optional(resp.deleted_member.value().name)
                                                               : std::nullopt,
                               at);
        break;

    case internal::ClanActionType::ChangeRole:
        co_await this->on_change_role(resp.clan_id,
                                      resp.target.has_value() ? std::optional(resp.target.value().uid) : std::nullopt,
                                      resp.target.has_value() ? std::optional(resp.target.value().name) : std::nullopt,
                                      resp.old_role,
                                      resp.new_role,
                                      at);
        break;

    case internal::ClanActionType::SetTitle:
        co_await this->on_set_title(resp.clan_id, resp.new_title, at);
        break;

    case internal::ClanActionType::Ally:
        co_await this->on_ally(resp.clan_id, resp.related_clan_id, at);
        break;

    case internal::ClanActionType::Unally:
        co_await this->on_unally(resp.clan_id, resp.related_clan_id, at);
        break;

    case internal::ClanActionType::Enemy:
        co_await this->on_enemy(resp.clan_id, resp.related_clan_id, at);
        break;

    case internal::ClanActionType::Unenemy:
        co_await this->on_unenemy(resp.clan_id, resp.related_clan_id, at);
        break;

    case internal::ClanActionType::SetMoney:
        co_await this->on_set_money(resp.clan_id, resp.money, at);
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
                                             std::unordered_map<std::string, clan_member> members,
                                             const fb::model::datetime&                   updated_date)
{
    if (this->accept(clan_id, updated_date) == false)
        co_return;

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

    // The bucket write lock must be released before switching to the master's thread.
    auto weak      = std::weak_ptr<character>();
    auto ch_id     = uint32_t{0};
    auto clan_name = std::string{};
    {
        auto guard = co_await this->enter_write(clan_id);
        if (guard.holds_lock() == false)
            co_return;

        if (guard.value() == nullptr)
        {
            if (this->accept(clan_id, updated_date) == false)
                co_return;
            guard.value() = this->_server.make<fb::game::clan>(clan_id, name, clan_title, members, updated_date);
            this->note(clan_id, updated_date);
        }
        else if (guard.value()->updated_date() > updated_date)
        {
            co_return;
        }
        else if (guard.value()->updated_date() < updated_date)
        {
            guard.value()->update(name, clan_title, members);
            guard.value()->updated_date(updated_date);
            this->note(clan_id, updated_date);
        }

        clan_name = guard.value()->name();
        if (master_name.has_value() == false)
            co_return;

        auto ch = this->_server.characters.find(master_name.value());
        if (ch == nullptr)
            co_return;

        weak  = ch->template weak_from_this_as<character>();
        ch_id = ch->id;
        guard.value()->attach(weak);
    }

    if (weak.expired())
        co_return;

    auto before = this->_server.threads.current();
    co_await this->_server.threads.switching(weak);

    auto ptr = weak.lock();
    if (ptr != nullptr)
        ptr->clan_id(clan_id);

    if (before != nullptr)
        co_await before->switching();

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(ch_id);
    log_data["character_name"] = UTF8(master_name.value(), PLATFORM::WINDOWS);
    log_data["clan_id"]        = static_cast<Json::Int64>(clan_id);
    log_data["clan_name"]      = UTF8(clan_name, PLATFORM::WINDOWS);
    this->_server.log.write("clan_create", log_data);
}

async::task<void> clan::container::on_destroyed(uint32_t                   clan_id,
                                                std::string                clan_name,
                                                const fb::model::datetime& updated_date)
{
    if (this->accept(clan_id, updated_date) == false)
        co_return;

    auto members = std::vector<std::shared_ptr<character>>{};
    {
        auto guard = co_await this->enter_write(clan_id);
        if (guard.holds_lock() == false)
            co_return;
        if (guard.value() != nullptr && guard.value()->updated_date() > updated_date)
            co_return;
        if (guard.value() == nullptr && this->accept(clan_id, updated_date) == false)
            co_return;
        if (guard.value() != nullptr)
            members = guard.value()->online_characters();
        guard.value() = nullptr;
        this->note(clan_id, updated_date);
    }

    if (members.empty() == false)
    {
        auto message = std::format(_TEXT(MESSAGE_CLAN_DISBANDED), clan_name);
        this->_server.characters.foreach_enqueue(
            [this, message](auto& ch) -> async::task<void> {
                ch->clan_reset();
                ch->message(message, MESSAGE_TYPE::NOTIFY);

                auto lua = this->_server.lua.open("scripts/interaction.lua", "on_clan_left");
                if (lua)
                {
                    lua->pushobject(ch);
                    std::ignore = co_await lua->call(1);
                }
            },
            members);
    }

    for (uint32_t i = 0; i <= 3; i++)
    {
        auto guard = co_await this->_server.castles.enter_write(i);
        if (guard.value() != nullptr && guard.value()->owner_clan_id() == clan_id)
            guard.value()->owner_clan_id(std::nullopt);
    }
    co_return;
}

async::task<void> clan::container::on_broadcast(uint32_t clan_id, std::string message, uint8_t type)
{
    auto guard = co_await this->open(clan_id, fb::model::datetime::utc_now());
    if (guard.holds_lock() == false || guard.value() == nullptr)
        co_return;
    auto members = guard.value()->online_characters();
    this->_server.characters.foreach_enqueue(
        [message, type](auto& member) -> async::task<void> {
            member->message(message, static_cast<MESSAGE_TYPE>(type));
            co_return;
        },
        members);
}

async::task<void> clan::container::on_set_title(uint32_t                   clan_id,
                                                std::optional<std::string> new_title,
                                                const fb::model::datetime& updated_date)
{
    auto guard = co_await this->open(clan_id, updated_date);
    if (guard.holds_lock() == false || guard.value() == nullptr)
        co_return;
    if (guard.value()->updated_date() > updated_date)
        co_return;
    auto& clan = guard.value();

    auto old_title = clan->title();
    if (clan->updated_date() < updated_date)
    {
        clan->title(new_title);
        clan->updated_date(updated_date);
        this->note(clan_id, updated_date);
    }

    auto log_data         = Json::Value();
    log_data["clan_id"]   = static_cast<Json::Int64>(clan->id());
    log_data["clan_name"] = UTF8(clan->name(), PLATFORM::WINDOWS);
    log_data["old_title"] =
        old_title.has_value() ? Json::Value(UTF8(old_title.value(), PLATFORM::WINDOWS)) : Json::Value::null;
    log_data["new_title"] =
        new_title.has_value() ? Json::Value(UTF8(new_title.value(), PLATFORM::WINDOWS)) : Json::Value::null;
    this->_server.log.write("clan_title_change", log_data);
}

async::task<void> clan::container::on_join(uint32_t                   clan_id,
                                           std::optional<std::string> new_member,
                                           CLAN_ROLE                  role,
                                           const fb::model::datetime& updated_date)
{
    if (new_member.has_value() == false)
        co_return;

    auto joined_member_name = new_member.value();
    auto weak               = std::weak_ptr<character>();
    auto ch_id              = uint32_t{0};
    auto clan_name          = std::string{};
    auto members            = std::vector<std::shared_ptr<character>>{};
    {
        auto guard = co_await this->open(clan_id, updated_date);
        if (guard.holds_lock() == false || guard.value() == nullptr)
            co_return;
        if (guard.value()->updated_date() > updated_date)
            co_return;
        auto& clan = guard.value();

        if (clan->updated_date() < updated_date)
        {
            clan->join(clan_member{joined_member_name, role});
            clan->updated_date(updated_date);
            this->note(clan_id, updated_date);
        }

        auto ch = this->_server.characters.find(joined_member_name);
        if (ch == nullptr)
            co_return;

        if (ch->clan_id().has_value())
            co_return;

        weak      = ch->template weak_from_this_as<character>();
        ch_id     = ch->id;
        clan_name = clan->name();
        members   = clan->online_characters();
        if (weak.expired() == false)
            clan->attach(weak);
    }

    this->_server.characters.foreach_enqueue(
        [joined_member_name](auto& member) -> async::task<void> {
            member->message(std::format(_TEXT(MESSAGE_CLAN_MEMBER_JOINED), joined_member_name), MESSAGE_TYPE::NOTIFY);
            co_return;
        },
        members);

    if (weak.expired())
        co_return;

    auto before = this->_server.threads.current();
    co_await this->_server.threads.switching(weak);

    auto ptr = weak.lock();
    if (ptr != nullptr)
    {
        ptr->clan_id(clan_id);
        ptr->update_external();
        ptr->message(std::format(_TEXT(MESSAGE_CLAN_JOINED_SUCCESS), clan_name), MESSAGE_TYPE::NOTIFY);
    }

    if (before != nullptr)
        co_await before->switching();

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(ch_id);
    log_data["character_name"] = UTF8(joined_member_name, PLATFORM::WINDOWS);
    log_data["clan_id"]        = static_cast<Json::Int64>(clan_id);
    log_data["clan_name"]      = UTF8(clan_name, PLATFORM::WINDOWS);
    this->_server.log.write("clan_join", log_data);
}

async::task<void> clan::container::on_leave(uint32_t                   clan_id,
                                            std::optional<std::string> deleted_member,
                                            const fb::model::datetime& updated_date)
{
    if (deleted_member.has_value() == false)
        co_return;

    auto deleted_member_name = deleted_member.value();
    auto weak                = std::weak_ptr<character>();
    auto ch_id               = uint32_t{0};
    auto clan_name           = std::string{};
    auto members             = std::vector<std::shared_ptr<character>>{};
    {
        auto guard = co_await this->open(clan_id, updated_date);
        if (guard.holds_lock() == false || guard.value() == nullptr)
            co_return;
        if (guard.value()->updated_date() > updated_date)
            co_return;
        auto& clan = guard.value();

        auto ch = this->_server.characters.find(deleted_member_name);
        if (ch != nullptr)
        {
            weak  = ch->template weak_from_this_as<character>();
            ch_id = ch->id;
            clan->detach(weak);
        }

        if (clan->updated_date() < updated_date)
        {
            clan->leave(deleted_member_name);
            clan->updated_date(updated_date);
            this->note(clan_id, updated_date);
        }
        clan_name = clan->name();
        members   = clan->online_characters();
    }

    if (weak.expired() == false)
    {
        auto before = this->_server.threads.current();
        co_await this->_server.threads.switching(weak);

        auto ptr = weak.lock();
        if (ptr != nullptr)
        {
            ptr->clan_reset();
            ptr->update_external();
            ptr->message(_TEXT(MESSAGE_CLAN_LEFT), MESSAGE_TYPE::NOTIFY);

            auto lua = this->_server.lua.open("scripts/interaction.lua", "on_clan_left");
            if (lua)
            {
                lua->pushobject(ptr);
                std::ignore = co_await lua->call(1);
            }
        }

        if (before != nullptr)
            co_await before->switching();

        auto log_data              = Json::Value();
        log_data["character_id"]   = static_cast<Json::Int64>(ch_id);
        log_data["character_name"] = UTF8(deleted_member_name, PLATFORM::WINDOWS);
        log_data["clan_id"]        = static_cast<Json::Int64>(clan_id);
        log_data["clan_name"]      = UTF8(clan_name, PLATFORM::WINDOWS);
        this->_server.log.write("clan_leave", log_data);
    }

    auto message = std::format(_TEXT(MESSAGE_CLAN_MEMBER_LEFT), deleted_member_name);
    this->_server.characters.foreach_enqueue(
        [message](auto& member) -> async::task<void> {
            member->message(message, MESSAGE_TYPE::NOTIFY);
            co_return;
        },
        members);
}

async::task<void> clan::container::on_kick(uint32_t                   clan_id,
                                           std::optional<std::string> deleted_member,
                                           const fb::model::datetime& updated_date)
{
    if (deleted_member.has_value() == false)
        co_return;

    auto deleted_member_name = deleted_member.value();
    auto weak                = std::weak_ptr<character>();
    auto ch_id               = uint32_t{0};
    auto clan_name           = std::string{};
    auto members             = std::vector<std::shared_ptr<character>>{};
    {
        auto guard = co_await this->open(clan_id, updated_date);
        if (guard.holds_lock() == false || guard.value() == nullptr)
            co_return;
        if (guard.value()->updated_date() > updated_date)
            co_return;
        auto& clan = guard.value();

        auto ch = this->_server.characters.find(deleted_member_name);
        if (ch != nullptr)
        {
            weak  = ch->template weak_from_this_as<character>();
            ch_id = ch->id;
            clan->detach(weak);
        }

        if (clan->updated_date() < updated_date)
        {
            clan->leave(deleted_member_name);
            clan->updated_date(updated_date);
            this->note(clan_id, updated_date);
        }
        clan_name = clan->name();
        members   = clan->online_characters();
    }

    if (weak.expired() == false)
    {
        auto before = this->_server.threads.current();
        co_await this->_server.threads.switching(weak);

        auto ptr = weak.lock();
        if (ptr != nullptr)
        {
            ptr->clan_reset();
            ptr->update_external();
            ptr->message(_TEXT(MESSAGE_CLAN_KICKED), MESSAGE_TYPE::NOTIFY);

            auto lua = this->_server.lua.open("scripts/interaction.lua", "on_clan_left");
            if (lua)
            {
                lua->pushobject(ptr);
                std::ignore = co_await lua->call(1);
            }
        }

        if (before != nullptr)
            co_await before->switching();

        auto log_data              = Json::Value();
        log_data["character_id"]   = static_cast<Json::Int64>(ch_id);
        log_data["character_name"] = UTF8(deleted_member_name, PLATFORM::WINDOWS);
        log_data["clan_id"]        = static_cast<Json::Int64>(clan_id);
        log_data["clan_name"]      = UTF8(clan_name, PLATFORM::WINDOWS);
        this->_server.log.write("clan_kick", log_data);
    }

    auto message = std::format(_TEXT(MESSAGE_CLAN_MEMBER_KICKED), deleted_member_name);
    this->_server.characters.foreach_enqueue(
        [message](auto& member) -> async::task<void> {
            member->message(message, MESSAGE_TYPE::NOTIFY);
            co_return;
        },
        members);
}

async::task<void> clan::container::on_ally(uint32_t                   clan_id,
                                           std::optional<uint32_t>    related_clan_id,
                                           const fb::model::datetime& updated_date)
{
    if (related_clan_id.has_value() == false)
        co_return;

    auto other_id = related_clan_id.value();

    // Bucket locks are not reentrant; never hold two clan guards at once.
    auto members = std::vector<std::shared_ptr<character>>{};
    {
        auto guard = co_await this->open(clan_id, updated_date);
        if (guard.holds_lock() && guard.value() != nullptr && guard.value()->updated_date() <= updated_date)
        {
            if (guard.value()->updated_date() < updated_date)
            {
                guard.value()->allied_clan_id(other_id);
                guard.value()->updated_date(updated_date);
                this->note(clan_id, updated_date);
            }
            members = guard.value()->online_characters();
        }
    }

    {
        auto other_guard = co_await this->open(other_id, updated_date);
        if (other_guard.holds_lock() && other_guard.value() != nullptr &&
            other_guard.value()->updated_date() <= updated_date)
        {
            if (other_guard.value()->updated_date() < updated_date)
            {
                other_guard.value()->allied_clan_id(clan_id);
                other_guard.value()->updated_date(updated_date);
                this->note(other_id, updated_date);
            }
            auto others = other_guard.value()->online_characters();
            members.insert(members.end(), others.begin(), others.end());
        }
    }

    this->_server.characters.foreach_enqueue(
        [](auto& member) -> async::task<void> {
            member->update_external();
            co_return;
        },
        members);
}

async::task<void> clan::container::on_unally(uint32_t                   clan_id,
                                             std::optional<uint32_t>    related_clan_id,
                                             const fb::model::datetime& updated_date)
{
    auto members = std::vector<std::shared_ptr<character>>{};
    {
        auto guard = co_await this->open(clan_id, updated_date);
        if (guard.holds_lock() && guard.value() != nullptr && guard.value()->updated_date() <= updated_date)
        {
            if (guard.value()->updated_date() < updated_date)
            {
                guard.value()->allied_clan_id(std::nullopt);
                guard.value()->updated_date(updated_date);
                this->note(clan_id, updated_date);
            }
            members = guard.value()->online_characters();
        }
    }

    if (related_clan_id.has_value())
    {
        auto other_id    = related_clan_id.value();
        auto other_guard = co_await this->open(other_id, updated_date);
        if (other_guard.holds_lock() && other_guard.value() != nullptr &&
            other_guard.value()->updated_date() <= updated_date)
        {
            if (other_guard.value()->updated_date() < updated_date)
            {
                other_guard.value()->allied_clan_id(std::nullopt);
                other_guard.value()->updated_date(updated_date);
                this->note(other_id, updated_date);
            }
            auto others = other_guard.value()->online_characters();
            members.insert(members.end(), others.begin(), others.end());
        }
    }

    this->_server.characters.foreach_enqueue(
        [](auto& member) -> async::task<void> {
            member->update_external();
            co_return;
        },
        members);
}

async::task<void> clan::container::on_enemy(uint32_t                   clan_id,
                                            std::optional<uint32_t>    related_clan_id,
                                            const fb::model::datetime& updated_date)
{
    if (related_clan_id.has_value() == false)
        co_return;

    auto other_id = related_clan_id.value();

    auto members = std::vector<std::shared_ptr<character>>{};
    {
        auto guard = co_await this->open(clan_id, updated_date);
        if (guard.holds_lock() && guard.value() != nullptr && guard.value()->updated_date() <= updated_date)
        {
            if (guard.value()->updated_date() < updated_date)
            {
                guard.value()->add_enemy_clan(other_id);
                guard.value()->updated_date(updated_date);
                this->note(clan_id, updated_date);
            }
            members = guard.value()->online_characters();
        }
    }

    {
        auto other_guard = co_await this->open(other_id, updated_date);
        if (other_guard.holds_lock() && other_guard.value() != nullptr &&
            other_guard.value()->updated_date() <= updated_date)
        {
            if (other_guard.value()->updated_date() < updated_date)
            {
                other_guard.value()->add_enemy_clan(clan_id);
                other_guard.value()->updated_date(updated_date);
                this->note(other_id, updated_date);
            }
            auto others = other_guard.value()->online_characters();
            members.insert(members.end(), others.begin(), others.end());
        }
    }

    this->_server.characters.foreach_enqueue(
        [](auto& member) -> async::task<void> {
            member->update_external();
            co_return;
        },
        members);
}

async::task<void> clan::container::on_unenemy(uint32_t                   clan_id,
                                              std::optional<uint32_t>    related_clan_id,
                                              const fb::model::datetime& updated_date)
{
    if (related_clan_id.has_value() == false)
        co_return;

    auto other_id = related_clan_id.value();

    auto members = std::vector<std::shared_ptr<character>>{};
    {
        auto guard = co_await this->open(clan_id, updated_date);
        if (guard.holds_lock() && guard.value() != nullptr && guard.value()->updated_date() <= updated_date)
        {
            if (guard.value()->updated_date() < updated_date)
            {
                guard.value()->remove_enemy_clan(other_id);
                guard.value()->updated_date(updated_date);
                this->note(clan_id, updated_date);
            }
            members = guard.value()->online_characters();
        }
    }

    {
        auto other_guard = co_await this->open(other_id, updated_date);
        if (other_guard.holds_lock() && other_guard.value() != nullptr &&
            other_guard.value()->updated_date() <= updated_date)
        {
            if (other_guard.value()->updated_date() < updated_date)
            {
                other_guard.value()->remove_enemy_clan(clan_id);
                other_guard.value()->updated_date(updated_date);
                this->note(other_id, updated_date);
            }
            auto others = other_guard.value()->online_characters();
            members.insert(members.end(), others.begin(), others.end());
        }
    }

    this->_server.characters.foreach_enqueue(
        [](auto& member) -> async::task<void> {
            member->update_external();
            co_return;
        },
        members);
}

async::task<void> clan::container::on_change_role(uint32_t                   clan_id,
                                                  std::optional<uint32_t>    target_uid,
                                                  std::optional<std::string> target_name,
                                                  std::optional<uint32_t>    old_role,
                                                  std::optional<uint32_t>    new_role,
                                                  const fb::model::datetime& updated_date)
{
    if (target_name.has_value() == false)
        co_return;

    auto guard = co_await this->open(clan_id, updated_date);
    if (guard.holds_lock() == false || guard.value() == nullptr)
        co_return;
    if (guard.value()->updated_date() > updated_date)
        co_return;
    auto& clan = guard.value();

    if (clan->updated_date() < updated_date)
    {
        auto member = clan->member(target_name.value());
        if (member != nullptr && new_role.has_value())
            member->role = static_cast<CLAN_ROLE>(new_role.value());
        clan->updated_date(updated_date);
        this->note(clan_id, updated_date);
    }

    if (target_uid.has_value() && old_role.has_value() && new_role.has_value())
    {
        auto target = this->_server.characters.find(target_uid.value());
        if (target != nullptr)
        {
            auto targets = std::vector<std::shared_ptr<character>>{target};
            auto message = std::format(_TEXT(MESSAGE_CLAN_ROLE_CHANGED), old_role.value(), new_role.value());
            this->_server.characters.foreach_enqueue(
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
        this->_server.characters.foreach_enqueue(
            [message](auto& member_ptr) -> async::task<void> {
                member_ptr->message(message, MESSAGE_TYPE::NOTIFY);
                co_return;
            },
            members);
    }
}

async::task<void> clan::container::on_set_money(uint32_t                   clan_id,
                                                uint64_t                   money,
                                                const fb::model::datetime& updated_date)
{
    auto guard = co_await this->open(clan_id, updated_date);
    if (guard.holds_lock() == false || guard.value() == nullptr)
        co_return;
    if (guard.value()->updated_date() > updated_date)
        co_return;
    if (guard.value()->updated_date() < updated_date)
    {
        guard.value()->money(money);
        guard.value()->updated_date(updated_date);
        this->note(clan_id, updated_date);
    }
}
