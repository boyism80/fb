#include <async/awaitable_then.h>
#include <fb/encoding.h>
#include <fb/game/character.h>
#include <fb/game/client_amount.h>
#include <fb/game/group.h>
#include <fb/game/protocol/group_portrait.h>
#include <fb/game/server.h>
#include <fb/game/thread_params.h>
#include <fb/logger.h>
#include <macro.h>

#include <json/json.h>

#include <algorithm>
#include <cstdint>
#include <format>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

using namespace fb::game;

namespace game_resp = fb::protocol::game::response;

async::task<void> group::container::on_error(uint32_t error, std::string_view actor)
{
    switch (static_cast<ERROR_CODE>(error))
    {
    case ERROR_CODE::NONE:
        co_return;

    case ERROR_CODE::CANNOT_GROUP_SELF:
        throw std::runtime_error(_TEXT(MESSAGE_CANNOT_GROUP_SELF));

    case ERROR_CODE::GROUP_ALREADY_JOINED:
        throw std::runtime_error(_TEXT(MESSAGE_ALREADY_JOINED_GROUP));

    case ERROR_CODE::OFFLINE:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_USER_NOT_LOGIN), actor));

    case ERROR_CODE::GROUP_TARGET_ALREADY_JOINED:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_GROUP_ALREADY_JOINED), actor));

    case ERROR_CODE::DISABLED_GROUP:
        throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_MINE));

    case ERROR_CODE::DISABLED_GROUP_TARGET:
        throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_TARGET));

    case ERROR_CODE::NOT_GROUP_MASTER:
        throw std::runtime_error(_TEXT(MESSAGE_GROUP_NOT_OWNER));

    case ERROR_CODE::GROUP_FULL_MEMBER:
        throw std::runtime_error(_TEXT(MESSAGE_GROUP_FULL_MEMBER));

    case ERROR_CODE::GROUP_NOT_JOINED:
    case ERROR_CODE::GROUP_NOT_FOUND:
        throw std::runtime_error(_TEXT(MESSAGE_GROUP_NOT_JOINED));

    default:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), error));
    }
}

async::task<void> group::container::on_create(std::string                     target,
                                              uint64_t                        group_id,
                                              std::string                     master,
                                              std::map<uint32_t, std::string> members,
                                              uint64_t                        revision)
{
    auto member_names = std::vector<std::string>{};
    auto master_uid   = uint32_t{0};
    member_names.reserve(members.size());
    for (const auto& [uid, name] : members)
    {
        member_names.push_back(name);
        if (name == master)
            master_uid = uid;
    }

    auto online = std::vector<std::shared_ptr<character>>();
    auto drop   = false;
    {
        auto guard = co_await this->enter_write(group_id);
        if (guard.holds_lock() == false)
            co_return;

        if (guard.value() == nullptr)
        {
            drop = true;
        }
        else
        {
            // A later message may have loaded a newer group first. Its state stays, but the members still
            // in it have to be entered, which no later message does.
            if (guard.value()->revision() < revision)
            {
                guard.value() = this->_server.make<fb::game::group>(group_id, master, member_names, revision);
            }

            if (master_uid == 0)
                co_return;

            auto& group   = guard.value();
            auto  current = group->members();
            member_names.erase(std::remove_if(member_names.begin(),
                                              member_names.end(),
                                              [&current](const std::string& name) {
                                                  return std::find(current.begin(), current.end(), name) ==
                                                         current.end();
                                              }),
                               member_names.end());

            auto master_ch = std::find(member_names.begin(), member_names.end(), master) != member_names.end()
                                 ? this->_server.characters.find(master_uid)
                                 : nullptr;
            if (master_ch != nullptr)
            {
                online.push_back(master_ch);

                auto log_data              = Json::Value();
                log_data["character_id"]   = static_cast<Json::Int64>(master_ch->id);
                log_data["character_name"] = UTF8(master_ch->name(), PLATFORM::WINDOWS);
                log_data["group_id"]       = static_cast<Json::Int64>(group_id);
                this->_server.log.write("group_create", log_data);
            }

            for (const auto& name : member_names)
            {
                if (name == master)
                    continue;

                auto ch = this->_server.characters.find(name);
                if (ch != nullptr)
                    online.push_back(ch);
            }
        }
    }

    if (drop)
    {
        this->erase(group_id);
        co_return;
    }

    // Entering on the character's thread orders it with disconnect cleanup, which detaches by group_id.
    co_await this->_server.characters.foreach_async(
        [this, group_id, member_names](auto& ch) -> async::task<void> {
            if (this->_server.characters.find(ch->id) != ch)
                co_return;

            // A leave or kick handled since the create already removed the character, so it is not entered again.
            auto entered = false;
            try
            {
                entered =
                    this->write(group_id,
                                [weak = ch->template weak_from_this_as<character>(), name = ch->name()](auto& group) {
                                    auto members = group != nullptr ? group->members() : std::vector<std::string>{};
                                    if (std::find(members.begin(), members.end(), name) == members.end())
                                        return false;

                                    group->enter(weak);
                                    return true;
                                });
            }
            catch (const std::runtime_error&)
            { }

            if (entered == false)
                co_return;

            ch->group.id(group_id);
            for (const auto& name : member_names)
            {
                if (name != ch->name())
                    ch->message(std::format(_TEXT(MESSAGE_GROUP_JOINED), name), MESSAGE_TYPE::STATE);
            }
            ch->message(_TEXT(MESSAGE_GROUP_JOINED_SUCCESS), MESSAGE_TYPE::STATE);

            // A member on another host was not checked before joining, so its solo ticket ends here.
            if (ch->matchmaker.queued())
            {
                co_await ch->matchmaker.dequeue(matchmaker::initiator::SERVER);
                ch->message("그룹에 들어가 매치메이킹 대기가 취소되었습니다.", MESSAGE_TYPE::STATE);
            }
            co_return;
        },
        online);

    this->update_portraits(group_id);
}

async::task<void> group::container::on_enter(std::string                target,
                                             uint64_t                   group_id,
                                             std::optional<std::string> new_member,
                                             uint64_t                   revision)
{
    if (new_member.has_value() == false)
        co_return;

    auto name    = new_member.value();
    auto weak    = std::weak_ptr<character>();
    auto ch_id   = uint32_t{0};
    auto members = std::vector<std::shared_ptr<character>>();
    auto reload  = false;
    {
        auto  guard = co_await this->enter_write(group_id);
        auto& group = guard.value();
        if (guard.holds_lock() == false || group == nullptr)
            co_return;
        if (group->revision() > revision)
            co_return;

        if (group->revision() + 1 == revision)
        {
            group->add_member(name);
            group->revision(revision);
        }
        else if (group->revision() < revision)
        {
            reload = true;
        }

        auto ch = this->_server.characters.find(name);
        if (ch != nullptr)
        {
            weak    = ch->weak_from_this_as<character>();
            ch_id   = ch->id;
            members = group->characters();
        }
    }

    if (reload)
        co_await this->reload(group_id);

    if (weak.expired() == false)
    {
        this->_server.characters.foreach_enqueue(
            [name](auto& member) -> async::task<void> {
                member->message(std::format(_TEXT(MESSAGE_GROUP_JOINED), name), MESSAGE_TYPE::STATE);
                co_return;
            },
            members);

        auto log_data              = Json::Value();
        log_data["character_id"]   = static_cast<Json::Int64>(ch_id);
        log_data["character_name"] = UTF8(name, PLATFORM::WINDOWS);
        log_data["group_id"]       = static_cast<Json::Int64>(group_id);
        this->_server.log.write("group_enter", log_data);

        auto before = this->_server.threads.current();
        co_await this->_server.threads.switching(weak);

        auto ptr = weak.lock();
        if (ptr != nullptr)
            std::ignore = co_await this->enter_member(*ptr, group_id);

        if (before != nullptr)
            co_await before->switching();
    }

    this->update_portraits(group_id);
}

async::task<void> group::container::on_leave(std::string                target,
                                             uint64_t                   group_id,
                                             std::optional<std::string> deleted_member,
                                             uint64_t                   revision)
{
    if (deleted_member.has_value() == false)
        co_return;

    auto name = deleted_member.value();
    co_await this->remove_member(group_id,
                                 name,
                                 _TEXT(MESSAGE_GROUP_LEFT_SUCCESS),
                                 std::format(_TEXT(MESSAGE_GROUP_MEMBER_LEFT), name),
                                 "group_leave",
                                 revision);
}

async::task<void> group::container::on_kick(std::string                target,
                                            uint64_t                   group_id,
                                            std::optional<std::string> deleted_member,
                                            uint64_t                   revision)
{
    if (deleted_member.has_value() == false)
        co_return;

    auto name = deleted_member.value();
    co_await this->remove_member(group_id,
                                 name,
                                 _TEXT(MESSAGE_GROUP_KICKED),
                                 std::format(_TEXT(MESSAGE_GROUP_MEMBER_KICKED), name),
                                 "group_kick",
                                 revision);
}

async::task<void> group::container::remove_member(uint64_t    group_id,
                                                  std::string name,
                                                  std::string message,
                                                  std::string others_message,
                                                  std::string log_name,
                                                  uint64_t    revision)
{
    auto weak    = std::weak_ptr<character>();
    auto ch_id   = uint32_t{0};
    auto members = std::vector<std::shared_ptr<character>>();
    auto reload  = false;
    auto stale   = false;
    {
        auto  guard = co_await this->enter_write(group_id);
        auto& group = guard.value();
        if (guard.holds_lock() == false || group == nullptr)
            co_return;

        if (group->revision() > revision)
        {
            auto current = group->members();
            if (std::find(current.begin(), current.end(), name) != current.end())
                co_return;

            stale = true;
        }
        else if (group->revision() + 1 == revision)
        {
            group->remove_member(name);
            group->revision(revision);
        }
        else if (group->revision() < revision)
        {
            reload = true;
        }

        auto ch = this->_server.characters.find(name);
        if (ch != nullptr)
        {
            weak  = ch->weak_from_this_as<character>();
            ch_id = ch->id;
            group->detach(weak);
        }
        members = group->characters();
    }

    if (reload)
        co_await this->reload(group_id);

    if (weak.expired() == false)
    {
        auto before = this->_server.threads.current();
        co_await this->_server.threads.switching(weak);

        auto ptr   = weak.lock();
        auto reset = ptr != nullptr && (stale == false || ptr->group.id() == group_id);
        if (reset)
            co_await this->reset_member(*ptr, message);

        if (stale == false || reset)
        {
            auto log_data              = Json::Value();
            log_data["character_id"]   = static_cast<Json::Int64>(ch_id);
            log_data["character_name"] = UTF8(name, PLATFORM::WINDOWS);
            log_data["group_id"]       = static_cast<Json::Int64>(group_id);
            this->_server.log.write(log_name, log_data);
        }

        if (before != nullptr)
            co_await before->switching();
    }

    if (stale == false)
    {
        this->_server.characters.foreach_enqueue(
            [others_message](auto& member) -> async::task<void> {
                member->message(others_message, MESSAGE_TYPE::STATE);
                co_return;
            },
            members);
    }
    this->update_portraits(group_id);
}

async::task<void> group::container::reset_member(character& ch, std::string message)
{
    ch.assert_thread();

    co_await ch.matchmaker.dequeue(matchmaker::initiator::SERVER);
    ch.group.reset();
    this->clear_portraits(ch);
    ch.message(message, MESSAGE_TYPE::STATE);

    auto map = ch.map();
    if (map != nullptr && map->is_instance() &&
        map->model().instance_rule == fb::model::enum_value::INSTANCE_RULE_TYPE::GROUP)
    {
        std::ignore = co_await map->kick(ch);
    }
}

async::task<bool> group::container::enter_member(character& ch, uint64_t group_id)
{
    ch.assert_thread();

    if (this->_server.characters.find(ch.id).get() != &ch || ch.group.id() == group_id)
        co_return false;

    // Entering on the character's thread orders it with disconnect cleanup, which detaches by group_id.
    // A leave or kick handled since the enter already removed the character, so it is not entered again.
    auto entered = false;
    try
    {
        entered = this->write(group_id, [weak = ch.weak_from_this_as<character>(), name = ch.name()](auto& group) {
            auto members = group != nullptr ? group->members() : std::vector<std::string>{};
            if (std::find(members.begin(), members.end(), name) == members.end())
                return false;

            group->enter(weak);
            return true;
        });
    }
    catch (const std::runtime_error&)
    { }

    if (entered == false)
        co_return false;

    ch.group.id(group_id);
    ch.message(_TEXT(MESSAGE_GROUP_JOINED_SUCCESS), MESSAGE_TYPE::STATE);

    // A member on another host was not checked before joining, so its solo ticket ends here.
    if (ch.matchmaker.queued())
    {
        co_await ch.matchmaker.dequeue(matchmaker::initiator::SERVER);
        ch.message("그룹에 들어가 매치메이킹 대기가 취소되었습니다.", MESSAGE_TYPE::STATE);
    }
    co_return true;
}

async::task<void> group::container::drop_stale_membership(character& ch, uint64_t group_id)
{
    auto weak = ch.weak_from_this_as<character>();
    {
        auto guard = co_await this->enter_write(group_id);
        if (guard.holds_lock() && guard.value() != nullptr)
            guard.value()->detach(weak);
    }

    co_await this->_server.threads.follow(weak);
    auto ptr = weak.lock();
    if (ptr != nullptr && ptr->group.id() == group_id)
        co_await this->reset_member(*ptr, _TEXT(MESSAGE_GROUP_NOT_JOINED));

    co_await this->reload(group_id);
}

async::task<void> group::container::on_destroyed(std::string actor, uint64_t group_id, uint64_t revision)
{
    std::ignore = actor;

    auto members = std::vector<std::shared_ptr<character>>{};
    auto drop    = false;
    {
        auto guard = co_await this->enter_write(group_id);
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
            members = guard.value()->characters();
            drop    = true;
        }
    }

    if (drop)
        this->erase(group_id);

    this->_server.characters.foreach_enqueue(
        [this](auto& ch) -> async::task<void> {
            co_await this->reset_member(*ch, _TEXT(MESSAGE_GROUP_DISBANDED));
        },
        members);
    co_return;
}

async::task<void> group::container::on_broadcast(uint64_t group_id, std::string message, uint8_t type)
{
    auto  guard = co_await this->enter_write(group_id);
    auto& group = guard.value();
    if (guard.holds_lock() == false || group == nullptr)
        co_return;

    this->_server.characters.foreach_enqueue(
        [message, type](auto& member) -> async::task<void> {
            member->message(message, static_cast<MESSAGE_TYPE>(type));
            co_return;
        },
        group->characters());
}

game_resp::group_portrait_entry group::container::portrait_entry(const character& ch, bool leader)
{
    ch.assert_thread();

    game_resp::group_portrait_entry e;
    e.name           = ch.name();
    e.leader         = leader ? 1 : 0;
    e.hair           = static_cast<uint16_t>(200 + ch.face());
    e.color          = static_cast<uint8_t>(ch.hair());
    e.face_hair_tint = ch.color();

    auto helmet = ch.items.helmet();
    if (helmet != nullptr && ch.option(OPTION::VISIBLE_HELMET))
    {
        e.hair_to_hat  = 1;
        e.helmet       = static_cast<uint8_t>(helmet->model().dress);
        e.helmet_color = helmet->color();
    }

    e.accessory_pack = 0xFFFF;
    auto [hp, maxhp] = encode_client_pool(ch.stat.hp(), ch.stat.maxhp());
    e.cur_hp         = hp;
    e.max_hp         = maxhp;
    return e;
}

async::task<void> group::container::send_portraits(std::vector<std::string>                names,
                                                   std::string                             master,
                                                   std::vector<std::shared_ptr<character>> online,
                                                   std::vector<std::shared_ptr<character>> recipients)
{
    auto mutex   = std::make_shared<std::mutex>();
    auto entries = std::make_shared<std::map<std::string, game_resp::group_portrait_entry>>();
    co_await this->_server.characters.foreach_async(
        [mutex, entries, master](auto& ch) -> async::task<void> {
            auto entry = portrait_entry(*ch, ch->name() == master);
            auto _     = std::lock_guard(*mutex);
            entries->emplace(entry.name, std::move(entry));
            co_return;
        },
        online);

    auto ordered = std::vector<game_resp::group_portrait_entry>{};
    ordered.reserve(names.size());
    for (const auto& name : names)
    {
        auto found = entries->find(name);
        if (found != entries->end())
        {
            ordered.push_back(std::move(found->second));
        }
        else
        {
            auto offline           = game_resp::group_portrait_entry{};
            offline.name           = name;
            offline.leader         = name == master ? 1 : 0;
            offline.accessory_pack = 0xFFFF;
            ordered.push_back(std::move(offline));
        }
    }

    this->_server.characters.foreach_enqueue(
        [ordered](auto& ch) -> async::task<void> {
            ch->listener.on_group_portrait(*ch, ordered);
            co_return;
        },
        recipients);
}

void group::container::update_portraits(const group& g, std::vector<std::shared_ptr<character>> recipients)
{
    auto names  = g.members();
    auto master = g.master();
    if (master.empty() == false && std::find(names.begin(), names.end(), master) == names.end())
        names.insert(names.begin(), master);

    auto online = g.characters();
    if (recipients.empty())
        recipients = online;

    async::awaitable_then(
        this->send_portraits(std::move(names), std::move(master), std::move(online), std::move(recipients)),
        [](async::awaitable_result<void> result) {
            try
            {
                result();
            }
            catch (const std::exception& e)
            {
                fb::logger::warn("group portrait update failed: {}", e.what());
            }
        });
}

void group::container::update_portraits(uint64_t group_id)
{
    auto guard = this->try_enter_read(group_id);
    if (guard.has_value() == false)
        return;

    auto& g = guard->value();
    if (g == nullptr)
        return;

    this->update_portraits(*g);
}

void group::container::update_portraits(character& ch)
{
    ch.assert_thread();

    auto& gid = ch.group.id();
    if (gid.has_value() == false)
    {
        this->clear_portraits(ch);
        return;
    }

    auto guard = this->try_enter_read(gid.value());
    if (guard.has_value() == false || guard->value() == nullptr)
    {
        this->clear_portraits(ch);
        return;
    }

    this->update_portraits(*guard->value(), {ch.shared_from_this_as<character>()});
}

void group::container::clear_portraits(character& ch)
{
    ch.assert_thread();
    ch.listener.on_group_portrait(ch, {});
}

void group::container::update_hp(character& source, bool include_max)
{
    source.assert_thread();

    auto& gid = source.group.id();
    if (gid.has_value() == false)
        return;

    if (include_max)
    {
        this->update_portraits(gid.value());
    }
    else
    {
        auto guard = this->try_enter_read(gid.value());
        if (guard.has_value() == false || guard->value() == nullptr)
            return;

        auto name      = source.name();
        auto [hp, max] = encode_client_pool(source.stat.hp(), source.stat.maxhp());
        std::ignore    = max;
        this->_server.characters.foreach_enqueue(
            [name, hp](auto& ch) -> async::task<void> {
                ch->listener.on_group_portrait_hp(*ch, name, hp);
                co_return;
            },
            guard->value()->characters());
    }
}
