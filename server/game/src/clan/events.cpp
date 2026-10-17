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

    switch (resp.action)
    {
    case internal::ClanActionType::Join:
    {
        auto role = resp.new_role.has_value() ? static_cast<CLAN_ROLE>(resp.new_role.value()) : CLAN_ROLE::MATE;
        co_await this->on_join(resp.clan_id,
                               resp.new_member.has_value() ? std::optional(resp.new_member.value().name) : std::nullopt,
                               role,
                               resp.revision);
        break;
    }

    case internal::ClanActionType::Leave:
        co_await this->on_leave(resp.clan_id,
                                resp.deleted_member.has_value() ? std::optional(resp.deleted_member.value().name)
                                                                : std::nullopt,
                                resp.revision);
        break;

    case internal::ClanActionType::Kick:
        co_await this->on_kick(resp.clan_id,
                               resp.deleted_member.has_value() ? std::optional(resp.deleted_member.value().name)
                                                               : std::nullopt,
                               resp.revision);
        break;

    case internal::ClanActionType::ChangeRole:
        co_await this->on_change_role(resp.clan_id,
                                      resp.target.has_value() ? std::optional(resp.target.value().uid) : std::nullopt,
                                      resp.target.has_value() ? std::optional(resp.target.value().name) : std::nullopt,
                                      resp.old_role,
                                      resp.new_role,
                                      resp.revision);
        break;

    case internal::ClanActionType::SetTitle:
        co_await this->on_set_title(resp.clan_id, resp.new_title, resp.revision);
        break;

    case internal::ClanActionType::Ally:
        co_await this->on_ally(resp.clan_id, resp.related_clan_id, resp.revision, resp.related_revision);
        break;

    case internal::ClanActionType::Unally:
        co_await this->on_unally(resp.clan_id, resp.related_clan_id, resp.revision, resp.related_revision);
        break;

    case internal::ClanActionType::Enemy:
        co_await this->on_enemy(resp.clan_id, resp.related_clan_id, resp.revision, resp.related_revision);
        break;

    case internal::ClanActionType::Unenemy:
        co_await this->on_unenemy(resp.clan_id, resp.related_clan_id, resp.revision, resp.related_revision);
        break;

    case internal::ClanActionType::SetMoney:
        co_await this->on_set_money(resp.clan_id, resp.money, resp.revision);
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

    case ERROR_CODE::CLAN_FULL_MEMBER:
        throw std::runtime_error(_TEXT(MESSAGE_CLAN_FULL_MEMBER));

    default:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), error));
    }
}

async::task<void> clan::container::on_create(uint32_t                                     clan_id,
                                             std::string                                  name,
                                             std::string                                  title,
                                             std::unordered_map<std::string, clan_member> members,
                                             uint64_t                                     revision)
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

    // The bucket write lock must be released before switching to the master's thread.
    auto weak      = std::weak_ptr<character>();
    auto ch_id     = uint32_t{0};
    auto clan_name = std::string{};
    auto drop      = false;
    {
        auto guard = co_await this->enter_write(clan_id);
        if (guard.holds_lock() == false)
            co_return;

        if (guard.value() == nullptr)
        {
            drop = true;
        }
        else
        {
            // A later message may have loaded a newer clan first. Its state stays, but the master still
            // has to be attached, which no later message does.
            if (guard.value()->revision() < revision)
            {
                guard.value()->update(name, clan_title, members);
                guard.value()->revision(revision);
            }

            clan_name = guard.value()->name();
            if (master_name.has_value() == false || guard.value()->contains(master_name.value()) == false)
                co_return;

            auto ch = this->_server.characters.find(master_name.value());
            if (ch == nullptr)
                co_return;

            weak  = ch->template weak_from_this_as<character>();
            ch_id = ch->id;
        }
    }

    if (drop)
    {
        this->erase(clan_id);
        co_return;
    }

    if (weak.expired())
        co_return;

    auto before = this->_server.threads.current();
    co_await this->_server.threads.switching(weak);

    // Attaching on the character's thread orders it with disconnect cleanup, which detaches by clan_id.
    auto ptr = weak.lock();
    if (ptr != nullptr && this->_server.characters.find(ptr->id) == ptr)
    {
        ptr->clan_id(clan_id);
        try
        {
            this->write(clan_id, [weak](auto& clan) {
                if (clan != nullptr)
                    clan->attach(weak);
            });
        }
        catch (const std::runtime_error&)
        { }
    }

    if (before != nullptr)
        co_await before->switching();

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(ch_id);
    log_data["character_name"] = UTF8(master_name.value(), PLATFORM::WINDOWS);
    log_data["clan_id"]        = static_cast<Json::Int64>(clan_id);
    log_data["clan_name"]      = UTF8(clan_name, PLATFORM::WINDOWS);
    this->_server.log.write("clan_create", log_data);
}

async::task<void> clan::container::on_destroyed(uint32_t clan_id, std::string clan_name, uint64_t revision)
{
    auto members = std::vector<std::shared_ptr<character>>{};
    auto drop    = false;
    {
        auto guard = co_await this->enter_write(clan_id);
        if (guard.holds_lock() == false)
            co_return;
        if (guard.value() == nullptr)
        {
            drop = true;
        }
        else if (guard.value()->revision() > revision)
        {
            co_return;
        }
        else
        {
            members = guard.value()->online_characters();
            drop    = true;
        }
    }

    if (drop)
        this->erase(clan_id);

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
    auto guard = co_await this->enter_write(clan_id);
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
                                                uint64_t                   revision)
{
    auto old_title = std::optional<std::string>{};
    auto clan_name = std::string{};
    auto reload    = false;
    {
        auto guard = co_await this->enter_write(clan_id);
        if (guard.holds_lock() == false || guard.value() == nullptr)
            co_return;
        if (guard.value()->revision() > revision)
            co_return;
        auto& clan = guard.value();

        old_title = clan->title();
        clan_name = clan->name();
        if (clan->revision() + 1 == revision)
        {
            clan->title(new_title);
            clan->revision(revision);
        }
        else if (clan->revision() < revision)
        {
            reload = true;
        }
    }

    if (reload)
        co_await this->reload(clan_id);

    auto log_data         = Json::Value();
    log_data["clan_id"]   = static_cast<Json::Int64>(clan_id);
    log_data["clan_name"] = UTF8(clan_name, PLATFORM::WINDOWS);
    log_data["old_title"] =
        old_title.has_value() ? Json::Value(UTF8(old_title.value(), PLATFORM::WINDOWS)) : Json::Value::null;
    log_data["new_title"] =
        new_title.has_value() ? Json::Value(UTF8(new_title.value(), PLATFORM::WINDOWS)) : Json::Value::null;
    this->_server.log.write("clan_title_change", log_data);
}

async::task<void>
clan::container::on_join(uint32_t clan_id, std::optional<std::string> new_member, CLAN_ROLE role, uint64_t revision)
{
    if (new_member.has_value() == false)
        co_return;

    auto joined_member_name = new_member.value();
    auto weak               = std::weak_ptr<character>();
    auto ch_id              = uint32_t{0};
    auto clan_name          = std::string{};
    auto members            = std::vector<std::shared_ptr<character>>{};
    auto reload             = false;
    {
        auto guard = co_await this->enter_write(clan_id);
        if (guard.holds_lock() == false || guard.value() == nullptr)
            co_return;
        if (guard.value()->revision() > revision)
            co_return;
        auto& clan = guard.value();

        if (clan->revision() + 1 == revision)
        {
            clan->join(clan_member{joined_member_name, role});
            clan->revision(revision);
        }
        else if (clan->revision() < revision)
        {
            reload = true;
        }

        auto ch = this->_server.characters.find(joined_member_name);
        if (ch != nullptr && ch->clan_id().has_value() == false)
        {
            weak      = ch->template weak_from_this_as<character>();
            ch_id     = ch->id;
            clan_name = clan->name();
            members   = clan->online_characters();
        }
    }

    if (reload)
        co_await this->reload(clan_id);

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

    // Attaching on the character's thread orders it with disconnect cleanup, which detaches by clan_id.
    auto ptr = weak.lock();
    if (ptr != nullptr && this->_server.characters.find(ptr->id) == ptr)
    {
        ptr->clan_id(clan_id);
        try
        {
            this->write(clan_id, [weak](auto& clan) {
                if (clan != nullptr)
                    clan->attach(weak);
            });
        }
        catch (const std::runtime_error&)
        { }
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
                                            uint64_t                   revision)
{
    if (deleted_member.has_value() == false)
        co_return;

    auto deleted_member_name = deleted_member.value();
    auto weak                = std::weak_ptr<character>();
    auto ch_id               = uint32_t{0};
    auto clan_name           = std::string{};
    auto members             = std::vector<std::shared_ptr<character>>{};
    auto reload              = false;
    {
        auto guard = co_await this->enter_write(clan_id);
        if (guard.holds_lock() == false || guard.value() == nullptr)
            co_return;
        if (guard.value()->revision() > revision)
            co_return;
        auto& clan = guard.value();

        auto ch = this->_server.characters.find(deleted_member_name);
        if (ch != nullptr)
        {
            weak  = ch->template weak_from_this_as<character>();
            ch_id = ch->id;
            clan->detach(weak);
        }

        if (clan->revision() + 1 == revision)
        {
            clan->leave(deleted_member_name);
            clan->revision(revision);
        }
        else if (clan->revision() < revision)
        {
            reload = true;
        }
        clan_name = clan->name();
        members   = clan->online_characters();
    }

    if (reload)
        co_await this->reload(clan_id);

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
                                           uint64_t                   revision)
{
    if (deleted_member.has_value() == false)
        co_return;

    auto deleted_member_name = deleted_member.value();
    auto weak                = std::weak_ptr<character>();
    auto ch_id               = uint32_t{0};
    auto clan_name           = std::string{};
    auto members             = std::vector<std::shared_ptr<character>>{};
    auto reload              = false;
    {
        auto guard = co_await this->enter_write(clan_id);
        if (guard.holds_lock() == false || guard.value() == nullptr)
            co_return;
        if (guard.value()->revision() > revision)
            co_return;
        auto& clan = guard.value();

        auto ch = this->_server.characters.find(deleted_member_name);
        if (ch != nullptr)
        {
            weak  = ch->template weak_from_this_as<character>();
            ch_id = ch->id;
            clan->detach(weak);
        }

        if (clan->revision() + 1 == revision)
        {
            clan->leave(deleted_member_name);
            clan->revision(revision);
        }
        else if (clan->revision() < revision)
        {
            reload = true;
        }
        clan_name = clan->name();
        members   = clan->online_characters();
    }

    if (reload)
        co_await this->reload(clan_id);

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

async::task<void> clan::container::on_ally(uint32_t                clan_id,
                                           std::optional<uint32_t> related_clan_id,
                                           uint64_t                revision,
                                           uint64_t                related_revision)
{
    if (related_clan_id.has_value() == false)
        co_return;

    auto other_id = related_clan_id.value();

    // Bucket locks are not reentrant; never hold two clan guards at once.
    auto members      = std::vector<std::shared_ptr<character>>{};
    auto reload       = false;
    auto reload_other = false;
    {
        auto guard = co_await this->enter_write(clan_id);
        if (guard.holds_lock() && guard.value() != nullptr && guard.value()->revision() <= revision)
        {
            if (guard.value()->revision() + 1 == revision)
            {
                guard.value()->allied_clan_id(other_id);
                guard.value()->revision(revision);
            }
            else if (guard.value()->revision() < revision)
            {
                reload = true;
            }
            members = guard.value()->online_characters();
        }
    }

    {
        auto other_guard = co_await this->enter_write(other_id);
        if (other_guard.holds_lock() && other_guard.value() != nullptr &&
            other_guard.value()->revision() <= related_revision)
        {
            if (other_guard.value()->revision() + 1 == related_revision)
            {
                other_guard.value()->allied_clan_id(clan_id);
                other_guard.value()->revision(related_revision);
            }
            else if (other_guard.value()->revision() < related_revision)
            {
                reload_other = true;
            }
            auto others = other_guard.value()->online_characters();
            members.insert(members.end(), others.begin(), others.end());
        }
    }

    if (reload)
        co_await this->reload(clan_id);
    if (reload_other)
        co_await this->reload(other_id);

    this->_server.characters.foreach_enqueue(
        [](auto& member) -> async::task<void> {
            member->update_external();
            co_return;
        },
        members);
}

async::task<void> clan::container::on_unally(uint32_t                clan_id,
                                             std::optional<uint32_t> related_clan_id,
                                             uint64_t                revision,
                                             uint64_t                related_revision)
{
    auto members      = std::vector<std::shared_ptr<character>>{};
    auto reload       = false;
    auto reload_other = false;
    {
        auto guard = co_await this->enter_write(clan_id);
        if (guard.holds_lock() && guard.value() != nullptr && guard.value()->revision() <= revision)
        {
            if (guard.value()->revision() + 1 == revision)
            {
                guard.value()->allied_clan_id(std::nullopt);
                guard.value()->revision(revision);
            }
            else if (guard.value()->revision() < revision)
            {
                reload = true;
            }
            members = guard.value()->online_characters();
        }
    }

    if (related_clan_id.has_value())
    {
        auto other_id    = related_clan_id.value();
        auto other_guard = co_await this->enter_write(other_id);
        if (other_guard.holds_lock() && other_guard.value() != nullptr &&
            other_guard.value()->revision() <= related_revision)
        {
            if (other_guard.value()->revision() + 1 == related_revision)
            {
                other_guard.value()->allied_clan_id(std::nullopt);
                other_guard.value()->revision(related_revision);
            }
            else if (other_guard.value()->revision() < related_revision)
            {
                reload_other = true;
            }
            auto others = other_guard.value()->online_characters();
            members.insert(members.end(), others.begin(), others.end());
        }
    }

    if (reload)
        co_await this->reload(clan_id);
    if (reload_other)
        co_await this->reload(related_clan_id.value());

    this->_server.characters.foreach_enqueue(
        [](auto& member) -> async::task<void> {
            member->update_external();
            co_return;
        },
        members);
}

async::task<void> clan::container::on_enemy(uint32_t                clan_id,
                                            std::optional<uint32_t> related_clan_id,
                                            uint64_t                revision,
                                            uint64_t                related_revision)
{
    if (related_clan_id.has_value() == false)
        co_return;

    auto other_id = related_clan_id.value();

    auto members      = std::vector<std::shared_ptr<character>>{};
    auto reload       = false;
    auto reload_other = false;
    {
        auto guard = co_await this->enter_write(clan_id);
        if (guard.holds_lock() && guard.value() != nullptr && guard.value()->revision() <= revision)
        {
            if (guard.value()->revision() + 1 == revision)
            {
                guard.value()->add_enemy_clan(other_id);
                guard.value()->revision(revision);
            }
            else if (guard.value()->revision() < revision)
            {
                reload = true;
            }
            members = guard.value()->online_characters();
        }
    }

    {
        auto other_guard = co_await this->enter_write(other_id);
        if (other_guard.holds_lock() && other_guard.value() != nullptr &&
            other_guard.value()->revision() <= related_revision)
        {
            if (other_guard.value()->revision() + 1 == related_revision)
            {
                other_guard.value()->add_enemy_clan(clan_id);
                other_guard.value()->revision(related_revision);
            }
            else if (other_guard.value()->revision() < related_revision)
            {
                reload_other = true;
            }
            auto others = other_guard.value()->online_characters();
            members.insert(members.end(), others.begin(), others.end());
        }
    }

    if (reload)
        co_await this->reload(clan_id);
    if (reload_other)
        co_await this->reload(other_id);

    this->_server.characters.foreach_enqueue(
        [](auto& member) -> async::task<void> {
            member->update_external();
            co_return;
        },
        members);
}

async::task<void> clan::container::on_unenemy(uint32_t                clan_id,
                                              std::optional<uint32_t> related_clan_id,
                                              uint64_t                revision,
                                              uint64_t                related_revision)
{
    if (related_clan_id.has_value() == false)
        co_return;

    auto other_id = related_clan_id.value();

    auto members      = std::vector<std::shared_ptr<character>>{};
    auto reload       = false;
    auto reload_other = false;
    {
        auto guard = co_await this->enter_write(clan_id);
        if (guard.holds_lock() && guard.value() != nullptr && guard.value()->revision() <= revision)
        {
            if (guard.value()->revision() + 1 == revision)
            {
                guard.value()->remove_enemy_clan(other_id);
                guard.value()->revision(revision);
            }
            else if (guard.value()->revision() < revision)
            {
                reload = true;
            }
            members = guard.value()->online_characters();
        }
    }

    {
        auto other_guard = co_await this->enter_write(other_id);
        if (other_guard.holds_lock() && other_guard.value() != nullptr &&
            other_guard.value()->revision() <= related_revision)
        {
            if (other_guard.value()->revision() + 1 == related_revision)
            {
                other_guard.value()->remove_enemy_clan(clan_id);
                other_guard.value()->revision(related_revision);
            }
            else if (other_guard.value()->revision() < related_revision)
            {
                reload_other = true;
            }
            auto others = other_guard.value()->online_characters();
            members.insert(members.end(), others.begin(), others.end());
        }
    }

    if (reload)
        co_await this->reload(clan_id);
    if (reload_other)
        co_await this->reload(other_id);

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
                                                  uint64_t                   revision)
{
    if (target_name.has_value() == false)
        co_return;

    auto members = std::vector<std::shared_ptr<character>>{};
    auto reload  = false;
    {
        auto guard = co_await this->enter_write(clan_id);
        if (guard.holds_lock() == false || guard.value() == nullptr)
            co_return;
        if (guard.value()->revision() > revision)
            co_return;
        auto& clan = guard.value();

        if (clan->revision() + 1 == revision)
        {
            auto member = clan->member(target_name.value());
            if (member != nullptr && new_role.has_value())
                member->role = static_cast<CLAN_ROLE>(new_role.value());
            clan->revision(revision);
        }
        else if (clan->revision() < revision)
        {
            reload = true;
        }

        for (auto& [uid, weak] : clan->characters())
        {
            if (target_uid.has_value() && uid == target_uid.value())
                continue;

            auto shared = weak.lock();
            if (shared == nullptr)
                continue;
            members.push_back(shared);
        }
    }

    if (reload)
        co_await this->reload(clan_id);

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

async::task<void> clan::container::on_set_money(uint32_t clan_id, uint64_t money, uint64_t revision)
{
    auto reload = false;
    {
        auto guard = co_await this->enter_write(clan_id);
        if (guard.holds_lock() == false || guard.value() == nullptr)
            co_return;
        if (guard.value()->revision() + 1 == revision)
        {
            guard.value()->money(money);
            guard.value()->revision(revision);
        }
        else if (guard.value()->revision() < revision)
        {
            reload = true;
        }
    }

    if (reload)
        co_await this->reload(clan_id);
}
