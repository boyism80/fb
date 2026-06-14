#include <fb/game/group.h>
#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/game/thread_params.h>
#include <fb/encoding.h>
#include <json/json.h>
#include <macro.h>

using namespace fb::game;

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
                {
                    auto  guard      = co_await this->_server.characters.enter_write_async();
                    auto& characters = guard.value();
                    auto  ch         = characters.find(master_uid);
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

                        auto invited_ch = characters.find(invited_name);
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
                }

                if (!log_data.empty())
                    this->_server.log.write("group_create", log_data);

                {
                    auto guard = this->_server.characters.enter_read();
                    guard.value().foreach_enqueue(
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
                }
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
    auto guard           = co_await this->ensure(group_id);
    if (guard.value() == nullptr)
        co_return;
    auto& group = guard.value();

    std::weak_ptr<character> weak;
    auto                     members = std::vector<std::shared_ptr<character>>();
    uint32_t                 ch_id   = 0;

    {
        auto  char_guard = co_await this->_server.characters.enter_write_async();
        auto& characters = char_guard.value();
        auto  ch         = characters.find(new_member_name);
        if (ch == nullptr)
            co_return;

        weak  = ch->template weak_from_this_as<character>();
        ch_id = ch->id;

        for (auto& member_ptr : group->characters())
            members.push_back(member_ptr);

        group->enter(weak);
        group->add_member(new_member_name);
    }

    {
        auto read_guard = this->_server.characters.enter_read();
        read_guard.value().foreach_enqueue(
            [new_member_name](auto& member) -> async::task<void> {
                member->message(std::format(_TEXT(MESSAGE_GROUP_JOINED), new_member_name), MESSAGE_TYPE::STATE);
                co_return;
            },
            members);
    }

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
}

async::task<void> group::container::on_leave(std::string                target,
                                             uint32_t                   group_id,
                                             std::optional<std::string> deleted_member)
{
    if (deleted_member.has_value() == false)
        co_return;

    auto deleted_member_name = deleted_member.value();
    auto guard               = co_await this->ensure(group_id);
    if (guard.value() == nullptr)
        co_return;
    auto& group = guard.value();

    group->remove_member(deleted_member_name);

    std::weak_ptr<character> weak;
    uint32_t                 ch_id = 0;
    {
        auto  char_guard = co_await this->_server.characters.enter_write_async();
        auto& characters = char_guard.value();
        auto  ch         = characters.find(deleted_member_name);
        if (ch != nullptr)
        {
            weak  = ch->template weak_from_this_as<character>();
            ch_id = ch->id;
            group->detach(weak);
        }
    }

    if (weak.expired() == false)
    {
        auto before = this->_server.threads.current();
        co_await this->_server.threads.switching(weak);

        auto ptr = weak.lock();
        if (ptr != nullptr)
        {
            ptr->group_reset();
            ptr->message(_TEXT(MESSAGE_GROUP_LEFT_SUCCESS), MESSAGE_TYPE::STATE);
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
    {
        auto read_guard = this->_server.characters.enter_read();
        read_guard.value().foreach_enqueue(
            [message](auto& member) -> async::task<void> {
                member->message(message, MESSAGE_TYPE::STATE);
                co_return;
            },
            members);
    }
}

async::task<void> group::container::on_kick(std::string                target,
                                            uint32_t                   group_id,
                                            std::optional<std::string> deleted_member)
{
    if (deleted_member.has_value() == false)
        co_return;

    auto deleted_member_name = deleted_member.value();
    auto guard               = co_await this->ensure(group_id);
    if (guard.value() == nullptr)
        co_return;
    auto& group = guard.value();

    group->remove_member(deleted_member_name);

    std::weak_ptr<character> weak;
    uint32_t                 ch_id = 0;
    {
        auto  char_guard = co_await this->_server.characters.enter_write_async();
        auto& characters = char_guard.value();
        auto  ch         = characters.find(deleted_member_name);
        if (ch != nullptr)
        {
            weak  = ch->template weak_from_this_as<character>();
            ch_id = ch->id;
            group->detach(weak);
        }
    }

    if (weak.expired() == false)
    {
        auto before = this->_server.threads.current();
        co_await this->_server.threads.switching(weak);

        auto ptr = weak.lock();
        if (ptr != nullptr)
        {
            ptr->group_reset();
            ptr->message(_TEXT(MESSAGE_GROUP_KICKED), MESSAGE_TYPE::STATE);
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
    {
        auto read_guard = this->_server.characters.enter_read();
        read_guard.value().foreach_enqueue(
            [message](auto& member) -> async::task<void> {
                member->message(message, MESSAGE_TYPE::STATE);
                co_return;
            },
            members);
    }
}

async::task<void> group::container::on_destroyed(std::string actor, uint32_t group_id)
{
    this->erase(group_id, [this, group_id](const auto& group) {
        auto members = std::vector<std::string>{group->members()};
        members.push_back(group->master());

        auto guard = this->_server.characters.enter_write();
        guard.value().foreach_enqueue(members, [](auto& ch) -> async::task<void> {
            ch->group_reset();
            ch->message(_TEXT(MESSAGE_GROUP_DISBANDED), MESSAGE_TYPE::STATE);
            co_return;
        });
    });
    co_return;
}

async::task<void> group::container::on_broadcast(uint32_t group_id, std::string message, uint8_t type)
{
    auto guard = co_await this->ensure(group_id);
    if (guard.value() == nullptr)
        co_return;
    auto& group = guard.value();

    auto members = std::vector<std::shared_ptr<character>>();
    for (auto& member_ptr : group->characters())
    {
        members.push_back(member_ptr);
    }

    auto char_guard = this->_server.characters.enter_write();
    char_guard.value().foreach_enqueue(
        [message, type](auto& member) -> async::task<void> {
            member->message(message, static_cast<MESSAGE_TYPE>(type));
            co_return;
        },
        members);
}
