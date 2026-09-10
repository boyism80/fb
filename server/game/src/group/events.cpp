#include <fb/game/group.h>
#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/game/thread_params.h>
#include <fb/game/protocol/group_portrait.h>
#include <fb/game/client_amount.h>
#include <fb/encoding.h>
#include <json/json.h>
#include <macro.h>
#include <algorithm>
#include <memory>
#include <mutex>

using namespace fb::game;

namespace {

using cv            = fb::protocol::CLIENT_VERSION;
namespace game_resp = fb::protocol::game::response;

std::vector<std::string> roster_names(const group& g)
{
    auto        names  = g.members();
    const auto& master = g.master();
    if (master.empty() == false && std::find(names.begin(), names.end(), master) == names.end())
        names.insert(names.begin(), master);
    return names;
}

game_resp::group_portrait_entry make_portrait_entry(const character& ch, bool leader)
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

void send_portrait_list(character& ch, std::vector<game_resp::group_portrait_entry> entries)
{
    ch.listener.on_group_portrait(ch, std::move(entries));
}

void dispatch_portraits(server&                                 srv,
                        std::vector<std::string>                names,
                        std::string                             master,
                        std::vector<std::shared_ptr<character>> online,
                        std::vector<std::shared_ptr<character>> recipients)
{
    struct state_t
    {
        std::mutex                                   mutex;
        std::vector<game_resp::group_portrait_entry> entries;
        std::vector<std::string>                     names;
        std::string                                  master;
        std::vector<std::shared_ptr<character>>      recipients;
        uint32_t                                     remaining = 0;
        server*                                      srv       = nullptr;
    };

    auto state        = std::make_shared<state_t>();
    state->names      = std::move(names);
    state->master     = std::move(master);
    state->recipients = std::move(recipients);
    state->remaining  = static_cast<uint32_t>(online.size());
    state->srv        = &srv;

    auto finish = [state]() {
        auto ordered = std::vector<game_resp::group_portrait_entry>{};
        ordered.reserve(state->names.size());
        for (const auto& name : state->names)
        {
            auto it = std::find_if(state->entries.begin(), state->entries.end(), [&](const auto& e) {
                return e.name == name;
            });
            if (it != state->entries.end())
            {
                ordered.push_back(*it);
            }
            else
            {
                game_resp::group_portrait_entry e;
                e.name           = name;
                e.leader         = (name == state->master) ? 1 : 0;
                e.accessory_pack = 0xFFFF;
                ordered.push_back(std::move(e));
            }
        }

        state->srv->characters.foreach_enqueue(
            [ordered](auto& ch) -> async::task<void> {
                send_portrait_list(*ch, ordered);
                co_return;
            },
            state->recipients);
    };

    if (state->remaining == 0)
    {
        finish();
        return;
    }

    srv.characters.foreach_enqueue(
        [state, finish](auto& ch) -> async::task<void> {
            auto e    = make_portrait_entry(*ch, ch->name() == state->master);
            auto last = false;
            {
                auto lock = std::lock_guard(state->mutex);
                state->entries.push_back(std::move(e));
                last = (--state->remaining == 0);
            }
            if (last)
                finish();
            co_return;
        },
        online);
}

} // namespace

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

    default:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), error));
    }
}

async::task<void> group::container::on_create(std::string                     target,
                                              uint32_t                        group_id,
                                              std::string                     master,
                                              std::map<uint32_t, std::string> members)
{
    auto member_names = std::vector<std::string>{};
    member_names.reserve(members.size());
    for (const auto& [uid, name] : members)
    {
        std::ignore = uid;
        member_names.push_back(name);
    }

    auto master_uid = static_cast<uint32_t>(0);
    for (const auto& [uid, name] : members)
    {
        if (name == master)
        {
            master_uid = uid;
            break;
        }
    }

    co_await this->async_write(
        group_id,
        [=, this](auto& group) -> async::task<void> {
            if (master_uid != 0)
            {
                auto        group_members = std::vector<std::shared_ptr<character>>();
                Json::Value log_data;

                auto ch = this->_server.characters.find(master_uid);
                if (ch != nullptr)
                {
                    auto weak = ch->template weak_from_this_as<character>();
                    group->enter(weak);

                    log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
                    log_data["character_name"] = UTF8(ch->name(), PLATFORM::WINDOWS);
                    log_data["group_id"]       = static_cast<Json::Int64>(group_id);

                    if (weak.expired() == false)
                        group_members.push_back(ch);
                }

                for (const auto& invited_name : member_names)
                {
                    if (invited_name == master)
                        continue;

                    auto invited_ch = this->_server.characters.find(invited_name);
                    if (invited_ch != nullptr)
                    {
                        auto invited_weak = invited_ch->template weak_from_this_as<character>();
                        if (invited_weak.expired() == false)
                        {
                            group->enter(invited_weak);
                            group_members.push_back(invited_ch);
                        }
                    }
                }

                if (!log_data.empty())
                    this->_server.log.write("group_create", log_data);

                this->_server.characters.foreach_enqueue(
                    [group_id, member_names](auto& member_ptr) -> async::task<void> {
                        member_ptr->group_id(group_id);
                        for (const auto& other_member_name : member_names)
                        {
                            if (other_member_name == member_ptr->name())
                                continue;

                            member_ptr->message(std::format(_TEXT(MESSAGE_GROUP_JOINED), other_member_name),
                                                MESSAGE_TYPE::STATE);
                        }

                        member_ptr->message(_TEXT(MESSAGE_GROUP_JOINED_SUCCESS), MESSAGE_TYPE::STATE);
                        co_return;
                    },
                    group_members);
                this->update_portraits(*group, group_members);
            }
            co_return;
        },
        [=, this]() -> async::task<entity_ptr> {
            co_return this->_server.make<fb::game::group>(group_id, master, member_names);
        });
}

async::task<void> group::container::on_enter(std::string                target,
                                             uint32_t                   group_id,
                                             std::optional<std::string> new_member)
{
    if (new_member.has_value() == false)
        co_return;

    auto new_member_name = new_member.value();
    auto guard           = co_await this->enter_write(group_id);
    if (guard.value() == nullptr)
        co_return;
    auto& group = guard.value();

    auto ch = this->_server.characters.find(new_member_name);
    if (ch == nullptr)
        co_return;

    auto weak    = ch->template weak_from_this_as<character>();
    auto ch_id   = ch->id;
    auto members = std::vector<std::shared_ptr<character>>();

    for (auto& member_ptr : group->characters())
        members.push_back(member_ptr);

    group->enter(weak);
    group->add_member(new_member_name);

    this->_server.characters.foreach_enqueue(
        [new_member_name](auto& member) -> async::task<void> {
            member->message(std::format(_TEXT(MESSAGE_GROUP_JOINED), new_member_name), MESSAGE_TYPE::STATE);
            co_return;
        },
        members);

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(ch_id);
    log_data["character_name"] = UTF8(new_member_name, PLATFORM::WINDOWS);
    log_data["group_id"]       = static_cast<Json::Int64>(group->id());
    this->_server.log.write("group_enter", log_data);

    auto before = this->_server.threads.current();
    co_await this->_server.threads.switching(weak);

    auto ptr = weak.lock();
    if (ptr != nullptr)
    {
        ptr->group_id(group->id());
        ptr->message(_TEXT(MESSAGE_GROUP_JOINED_SUCCESS), MESSAGE_TYPE::STATE);
    }

    if (before != nullptr)
        co_await before->switching();

    this->update_portraits(*group);
}

async::task<void> group::container::on_leave(std::string                target,
                                             uint32_t                   group_id,
                                             std::optional<std::string> deleted_member)
{
    if (deleted_member.has_value() == false)
        co_return;

    auto deleted_member_name = deleted_member.value();
    auto guard               = co_await this->enter_write(group_id);
    if (guard.value() == nullptr)
        co_return;
    auto& group = guard.value();

    group->remove_member(deleted_member_name);

    std::weak_ptr<character> weak;
    uint32_t                 ch_id = 0;
    auto                     ch    = this->_server.characters.find(deleted_member_name);
    if (ch != nullptr)
    {
        weak  = ch->template weak_from_this_as<character>();
        ch_id = ch->id;
        group->detach(weak);
    }

    if (weak.expired() == false)
    {
        auto before = this->_server.threads.current();
        co_await this->_server.threads.switching(weak);

        auto ptr = weak.lock();
        if (ptr != nullptr)
        {
            co_await ptr->matchmaker.unregister_queue(true);
            ptr->group_reset();
            this->clear_portraits(*ptr);
            ptr->message(_TEXT(MESSAGE_GROUP_LEFT_SUCCESS), MESSAGE_TYPE::STATE);

            auto map = ptr->map();
            if (map != nullptr && map->is_instance() &&
                map->model().instance_rule == fb::model::enum_value::INSTANCE_RULE_TYPE::GROUP)
            {
                auto source = map->source();
                if (source != nullptr)
                {
                    map_options opts;
                    opts.skip_instance_rule = true;
                    std::ignore             = co_await ptr->map(source, std::nullopt, opts);
                }
            }
        }

        auto log_data              = Json::Value();
        log_data["character_id"]   = static_cast<Json::Int64>(ch_id);
        log_data["character_name"] = UTF8(deleted_member_name, PLATFORM::WINDOWS);
        log_data["group_id"]       = static_cast<Json::Int64>(group->id());
        this->_server.log.write("group_leave", log_data);

        if (before != nullptr)
            co_await before->switching();
    }

    auto members = std::vector<std::shared_ptr<character>>();
    for (auto& member_ptr : group->characters())
        members.push_back(member_ptr);

    auto message = std::format(_TEXT(MESSAGE_GROUP_MEMBER_LEFT), deleted_member_name);
    this->_server.characters.foreach_enqueue(
        [message](auto& member) -> async::task<void> {
            member->message(message, MESSAGE_TYPE::STATE);
            co_return;
        },
        members);
    this->update_portraits(*group);
}

async::task<void> group::container::on_kick(std::string                target,
                                            uint32_t                   group_id,
                                            std::optional<std::string> deleted_member)
{
    if (deleted_member.has_value() == false)
        co_return;

    auto deleted_member_name = deleted_member.value();
    auto guard               = co_await this->enter_write(group_id);
    if (guard.value() == nullptr)
        co_return;
    auto& group = guard.value();

    group->remove_member(deleted_member_name);

    std::weak_ptr<character> weak;
    uint32_t                 ch_id = 0;
    auto                     ch    = this->_server.characters.find(deleted_member_name);
    if (ch != nullptr)
    {
        weak  = ch->template weak_from_this_as<character>();
        ch_id = ch->id;
        group->detach(weak);
    }

    if (weak.expired() == false)
    {
        auto before = this->_server.threads.current();
        co_await this->_server.threads.switching(weak);

        auto ptr = weak.lock();
        if (ptr != nullptr)
        {
            co_await ptr->matchmaker.unregister_queue(true);
            ptr->group_reset();
            this->clear_portraits(*ptr);
            ptr->message(_TEXT(MESSAGE_GROUP_KICKED), MESSAGE_TYPE::STATE);

            auto map = ptr->map();
            if (map != nullptr && map->is_instance() &&
                map->model().instance_rule == fb::model::enum_value::INSTANCE_RULE_TYPE::GROUP)
            {
                auto source = map->source();
                if (source != nullptr)
                {
                    map_options opts;
                    opts.skip_instance_rule = true;
                    std::ignore             = co_await ptr->map(source, std::nullopt, opts);
                }
            }
        }

        auto log_data              = Json::Value();
        log_data["character_id"]   = static_cast<Json::Int64>(ch_id);
        log_data["character_name"] = UTF8(deleted_member_name, PLATFORM::WINDOWS);
        log_data["group_id"]       = static_cast<Json::Int64>(group->id());
        this->_server.log.write("group_kick", log_data);

        if (before != nullptr)
            co_await before->switching();
    }

    auto members = std::vector<std::shared_ptr<character>>();
    for (auto& member_ptr : group->characters())
        members.push_back(member_ptr);

    auto message = std::format(_TEXT(MESSAGE_GROUP_MEMBER_KICKED), deleted_member_name);
    this->_server.characters.foreach_enqueue(
        [message](auto& member) -> async::task<void> {
            member->message(message, MESSAGE_TYPE::STATE);
            co_return;
        },
        members);
    this->update_portraits(*group);
}

async::task<void> group::container::on_destroyed(std::string actor, uint32_t group_id)
{
    this->erase(group_id, [this, group_id](const auto& group) {
        auto members = roster_names(*group);

        this->_server.characters.foreach_enqueue(members, [this](auto& ch) -> async::task<void> {
            if (ch->matchmaker.enrolled())
                co_await ch->matchmaker.unregister_queue(true);
            ch->group_reset();
            this->clear_portraits(*ch);
            ch->message(_TEXT(MESSAGE_GROUP_DISBANDED), MESSAGE_TYPE::STATE);

            auto map = ch->map();
            if (map != nullptr && map->is_instance() &&
                map->model().instance_rule == fb::model::enum_value::INSTANCE_RULE_TYPE::GROUP)
            {
                auto source = map->source();
                if (source != nullptr)
                {
                    map_options opts;
                    opts.skip_instance_rule = true;
                    std::ignore             = co_await ch->map(source, std::nullopt, opts);
                }
            }
            co_return;
        });
    });
    co_return;
}

async::task<void> group::container::on_broadcast(uint32_t group_id, std::string message, uint8_t type)
{
    auto guard = co_await this->enter_write(group_id);
    if (guard.value() == nullptr)
        co_return;
    auto& group = guard.value();

    auto members = std::vector<std::shared_ptr<character>>();
    for (auto& member_ptr : group->characters())
    {
        members.push_back(member_ptr);
    }

    this->_server.characters.foreach_enqueue(
        [message, type](auto& member) -> async::task<void> {
            member->message(message, static_cast<MESSAGE_TYPE>(type));
            co_return;
        },
        members);
}

void group::container::update_portraits(const group& g, std::vector<std::shared_ptr<character>> recipients)
{
    auto names  = roster_names(g);
    auto master = g.master();
    auto online = std::vector<std::shared_ptr<character>>{};
    for (auto& ch : g.characters())
        online.push_back(ch);

    if (recipients.empty())
        recipients = online;

    dispatch_portraits(this->_server, std::move(names), std::move(master), std::move(online), std::move(recipients));
}

void group::container::update_portraits(uint32_t group_id)
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

    auto& gid = ch.group_id();
    if (gid.has_value() == false)
    {
        this->clear_portraits(ch);
        return;
    }

    auto self  = ch.shared_from_this_as<character>();
    auto guard = this->try_enter_read(gid.value());
    if (guard.has_value() == false)
    {
        this->clear_portraits(ch);
        return;
    }

    auto& g = guard->value();
    if (g == nullptr)
    {
        this->clear_portraits(ch);
        return;
    }

    this->update_portraits(*g, {self});
}

void group::container::clear_portraits(character& ch)
{
    ch.assert_thread();
    ch.listener.on_group_portrait(ch, {});
}

void group::container::update_hp(character& source, bool include_max)
{
    source.assert_thread();

    auto& gid = source.group_id();
    if (gid.has_value() == false)
        return;

    if (include_max)
    {
        this->update_portraits(gid.value());
        return;
    }

    auto name      = source.name();
    auto [hp, max] = encode_client_pool(source.stat.hp(), source.stat.maxhp());
    std::ignore    = max;

    auto guard = this->try_enter_read(gid.value());
    if (guard.has_value() == false)
        return;

    auto& g = guard->value();
    if (g == nullptr)
        return;

    auto recipients = std::vector<std::shared_ptr<character>>{};
    for (auto& member : g->characters())
        recipients.push_back(member);

    this->_server.characters.foreach_enqueue(
        [name, hp](auto& ch) -> async::task<void> {
            ch->listener.on_group_portrait_hp(*ch, name, hp);
            co_return;
        },
        recipients);
}
