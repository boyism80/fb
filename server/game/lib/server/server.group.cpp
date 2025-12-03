#include <fb/game/server.h>
#include <fb/game/group.h>
#include <fb/game/character.h>
#include <fb/encoding.h>
#include <json/json.h>

using namespace fb::game;

void server::assert_group(uint32_t error, const std::string& actor) const
{
    // Convert error codes to localized error messages
    switch (static_cast<ERROR_CODE>(error))
    {
    case ERROR_CODE::NONE:
        return;

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

async::task<void> server::ensure_group(uint32_t id, std::function<async::task<void>(std::shared_ptr<fb::game::group>&)> fn)
{
    co_await this->groups.async_write(
        id,
        [this, id, fn](auto& group) -> async::task<void> {
            co_await fn(group);
        },
        [=, this]() -> async::task<std::shared_ptr<fb::game::group>> {
            auto&& resp = co_await this->http.get<internal_resp::GroupDetails>("internal", std::format("/group/{}", id));
            switch (static_cast<ERROR_CODE>(resp.error))
            {
            case ERROR_CODE::NONE:
            {
                auto members = std::vector<std::string>{};
                for (auto& member : resp.members)
                {
                    members.push_back(member.name);
                }
                co_return this->make<fb::game::group>(id, resp.group.master, members);
            }

            default:
                throw std::runtime_error(std::format("cannot get group (error : {})", resp.error));
            }
        });
}

async::task<bool> server::create_group(character& me, const std::string& target)
{
    auto weak = me.weak_from_this_as<fb::game::character>();
    try
    {
        if (me.option(OPTION::GROUP) == false)
            throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_MINE));

        // Check current group state and call appropriate API
        co_await this->characters.async_read([this, &me, &target, weak](auto& characters) -> async::task<void> {
            auto invitee = characters.find(target);
            if (invitee == nullptr)
                throw std::runtime_error(_TEXT(MESSAGE_USER_NOT_LOGIN));

            auto my_group_id      = me.group_id();
            auto invitee_group_id = invitee->group_id();

            co_await this->threads.switching(weak);

            if (my_group_id.has_value() == false)
            {
                // I don't have a group - create new group
                if (invitee_group_id.has_value())
                    throw std::runtime_error(_TEXT(MESSAGE_GROUP_ALREADY_JOINED));

                auto&& resp = co_await this->http.post("internal", "/group/create", request::CreateGroup{fb::config<uint32_t>("host"), me.id(), target});
                co_await this->on_create_group(resp);
            }
            else
            {
                // I have a group - check if I'm the master
                co_await this->ensure_group(my_group_id.value(), [this, &me, invitee, weak](auto& group) -> async::task<void> {
                    if (group->master() != me.name())
                        throw std::runtime_error(_TEXT(MESSAGE_GROUP_NOT_OWNER));

                    auto invitee_group_id = invitee->group_id();
                    if (invitee_group_id.has_value())
                    {
                        // Invitee is already in a group - check if same group
                        if (invitee_group_id.value() == group->id())
                        {
                            // Same group - kick member
                            co_await group->kick_member(me, invitee->name());
                        }
                        else
                        {
                            throw std::runtime_error(_TEXT(MESSAGE_GROUP_ALREADY_JOINED));
                        }
                    }
                    else
                    {
                        // Invitee is not in a group - enter group
                        co_await group->enter_member(me, invitee->name());
                    }
                });
            }
        });

        co_return true;
    }
    catch (std::exception& e)
    {
        me.message(e.what(), MESSAGE_TYPE::STATE);
        co_return false;
    }
}

async::task<void> server::destroy_group(character& me)
{
    auto group_id = me.group_id();
    if (group_id.has_value() == false)
        co_return;

    auto   weak = me.weak_from_this_as<fb::game::character>();
    auto&& resp = co_await this->http.post("internal", "/group/destroy", request::DestroyGroup{fb::config<uint32_t>("host"), me.name()});
    co_await this->threads.switching(weak);
    co_await this->on_destroyed_group(resp);
}

async::task<bool> server::handle_group_action(character& actor, const std::string& target_name)
{
    auto weak = actor.weak_from_this_as<fb::game::character>();
    try
    {
        if (actor.option(OPTION::GROUP) == false)
            throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_MINE));

        // Case 1: Actor clicks their own group icon (actor == target)
        if (actor.name() == target_name)
        {
            auto group_id = actor.group_id();
            if (group_id.has_value() == false)
            {
                // No group - do nothing
                co_return false;
            }

            // Has group - check if master or member
            co_await this->ensure_group(group_id.value(), [this, &actor, weak](auto& group) -> async::task<void> {
                if (group->master() == actor.name())
                {
                    // Master - destroy group
                    co_await this->destroy_group(actor);
                }
                else
                {
                    // Member - leave group
                    co_await group->leave_member(actor);
                }
            });

            co_return true;
        }

        // Case 2: Actor clicks target's group icon (actor != target)
        // Check if target is on the same server
        co_await this->characters.async_read([this, &actor, &target_name, weak](auto& characters) -> async::task<void> {
            auto target = characters.find(target_name);

            if (target != nullptr)
            {
                // Target is on the same server - can check state before API call
                auto actor_group_id  = actor.group_id();
                auto target_group_id = target->group_id();

                co_await this->threads.switching(weak);

                if (actor_group_id.has_value() == false)
                {
                    // Actor has no group
                    if (target_group_id.has_value())
                    {
                        // Target has a group - cannot create
                        throw std::runtime_error(_TEXT(MESSAGE_GROUP_ALREADY_JOINED));
                    }

                    // Both have no group - create
                    auto&& resp = co_await this->http.post("internal", "/group/create", request::CreateGroup{fb::config<uint32_t>("host"), actor.id(), target_name});
                    co_await this->on_create_group(resp);
                }
                else
                {
                    // Actor has a group - check if master
                    co_await this->ensure_group(actor_group_id.value(), [this, &actor, target, target_group_id, weak](auto& group) -> async::task<void> {
                        if (group->master() != actor.name())
                        {
                            // Actor is a member, not master - do nothing
                            co_return;
                        }

                        // Actor is master
                        if (target_group_id.has_value())
                        {
                            // Target has a group
                            if (target_group_id.value() == group->id())
                            {
                                // Same group - kick member
                                co_await group->kick_member(actor, target->name());
                            }
                            else
                            {
                                // Different group - cannot invite
                                throw std::runtime_error(_TEXT(MESSAGE_GROUP_ALREADY_JOINED));
                            }
                        }
                        else
                        {
                            // Target has no group - enter group
                            co_await group->enter_member(actor, target->name());
                        }
                    });
                }
            }
            else
            {
                // Target is on a different server - must call API and handle errors
                auto actor_group_id = actor.group_id();

                co_await this->threads.switching(weak);

                if (actor_group_id.has_value() == false)
                {
                    // Actor has no group - try to create (API will handle errors)
                    auto&& resp = co_await this->http.post("internal", "/group/create", request::CreateGroup{fb::config<uint32_t>("host"), actor.id(), target_name});
                    co_await this->on_create_group(resp);
                }
                else
                {
                    // Actor has a group - check if master
                    co_await this->ensure_group(actor_group_id.value(), [this, &actor, &target_name, weak](auto& group) -> async::task<void> {
                        if (group->master() != actor.name())
                        {
                            // Actor is a member, not master - do nothing
                            co_return;
                        }

                        // Actor is master - try to enter (API will handle errors)
                        auto&& resp = co_await this->http.post("internal", "/group/enter", request::EnterGroup{fb::config<uint32_t>("host"), actor.id(), target_name});
                        co_await this->on_updated_group(resp);
                    });
                }
            }
        });

        co_return true;
    }
    catch (std::exception& e)
    {
        actor.message(e.what(), MESSAGE_TYPE::STATE);
        co_return false;
    }
}

async::task<void> server::on_create_group(const internal_resp::GroupDetails& resp)
{
    this->assert_group(resp.error, "");

    // Create group directly from GroupDetails without calling GET API
    auto id      = resp.group.id;
    auto master  = resp.group.master;
    auto members = std::vector<std::string>{};
    for (auto& member : resp.members)
    {
        members.push_back(member.name);
    }

    // Store master UID for character attachment
    auto master_uid = static_cast<uint32_t>(0);
    for (auto& member : resp.members)
    {
        if (member.name == master)
        {
            master_uid = member.uid;
            break;
        }
    }

    // Use async_write with factory to create or update group
    co_await this->groups.async_write(
        id,
        [=, this](auto& group) -> async::task<void> {
            group->update(master, members);

            if (master_uid != 0)
            {
                co_await this->characters.async_read([id, master_uid, &group, this](auto& characters) -> async::task<void> {
                    auto ch = characters.find(master_uid);
                    if (ch != nullptr)
                    {
                        ch->group_id(id);
                        group->enter(ch->weak_from_this_as<character>());

                        auto log_data              = Json::Value();
                        log_data["character_id"]   = static_cast<Json::Int64>(ch->id());
                        log_data["character_name"] = UTF8(ch->name(), PLATFORM::WINDOWS);
                        log_data["group_id"]       = static_cast<Json::Int64>(id);
                        this->log.write("group_create", log_data);
                    }
                    co_return;
                });
            }
            co_return;
        },
        [=, this]() -> async::task<std::shared_ptr<fb::game::group>> {
            co_return this->make<fb::game::group>(id, master, members);
        });
}

async::task<void> server::on_updated_group(const internal_resp::UpdatedGroup& resp)
{
    this->assert_group(resp.error, resp.actor.name);

    co_await this->ensure_group(resp.group_id, [this, &resp](auto& group) -> async::task<void> {
        switch (static_cast<internal::GroupActionType>(resp.action))
        {
        case internal::GroupActionType::Enter:
        {
            if (resp.new_member.has_value() == false)
                break;

            co_await this->characters.async_write([this, &resp, group](auto& characters) -> async::task<void> {
                auto ch = characters.find(resp.new_member.value().name);
                if (ch == nullptr)
                    co_return;

                if (ch->group_id().has_value())
                    co_return;

                auto weak    = ch->weak_from_this_as<character>();
                auto members = std::vector<std::shared_ptr<fb::game::character>>();
                for (auto& member_ptr : group->characters())
                {
                    members.push_back(member_ptr);
                }

                co_await characters.foreach (
                    [this, &resp](auto& member) {
                        member->message(std::format("{}님 그룹 참여", resp.new_member.value().name), MESSAGE_TYPE::STATE);
                    },
                    members);

                if (weak.expired() == false)
                {
                    group->enter(weak);
                    ch->group_id(group->id());
                    ch->message("그룹에 참여했습니다.", MESSAGE_TYPE::STATE);

                    // Log group enter event
                    auto log_data              = Json::Value();
                    log_data["character_id"]   = static_cast<Json::Int64>(ch->id());
                    log_data["character_name"] = UTF8(resp.new_member.value().name, PLATFORM::WINDOWS);
                    log_data["group_id"]       = static_cast<Json::Int64>(group->id());
                    this->log.write("group_enter", log_data);
                }
            });
            break;
        }

        case internal::GroupActionType::Leave:
        case internal::GroupActionType::Kick:
        {
            if (resp.deleted_member.has_value() == false)
                break;

            co_await this->characters.async_write([this, &resp, group](auto& characters) -> async::task<void> {
                auto ch = characters.find(resp.deleted_member.value().name);
                if (ch != nullptr)
                {
                    auto weak = ch->weak_from_this_as<character>();
                    group->detach(weak);
                    ch->group_reset();
                    ch->message(resp.action == internal::GroupActionType::Kick ? "그룹에서 추방당했습니다." : "그룹 탈퇴", MESSAGE_TYPE::STATE);

                    // Log group leave/kick event
                    auto log_data              = Json::Value();
                    log_data["character_id"]   = static_cast<Json::Int64>(ch->id());
                    log_data["character_name"] = UTF8(resp.deleted_member.value().name, PLATFORM::WINDOWS);
                    log_data["group_id"]       = static_cast<Json::Int64>(group->id());
                    this->log.write(resp.action == internal::GroupActionType::Kick ? "group_kick" : "group_leave", log_data);
                }

                auto members = std::vector<std::shared_ptr<fb::game::character>>();
                for (auto& member_ptr : group->characters())
                {
                    members.push_back(member_ptr);
                }

                auto message = std::format("{}님이 그룹에서 {}했습니다.", resp.deleted_member.value().name, resp.action == internal::GroupActionType::Kick ? "추방당" : "탈퇴");
                co_await characters.foreach (
                    [this, message](auto& member) {
                        member->message(message, MESSAGE_TYPE::STATE);
                    },
                    members);
            });
            break;
        }

        default:
            break;
        }
    });
}

async::task<void> server::on_destroyed_group(const internal_resp::DestroyGroup& resp)
{
    this->assert_group(resp.error, resp.actor.name);

    auto gid = resp.group_id;
    co_await this->groups.async_erase(gid, [this, gid, &resp](const auto& group) -> async::task<void> {
        auto members = std::vector<std::string>{group->members()};
        members.push_back(group->master());

        co_await this->characters.async_write([this, &resp, &members](auto& characters) -> async::task<void> {
            co_await characters.foreach (members, [](auto& ch) {
                ch->group_reset();
                ch->message("그룹 해체", MESSAGE_TYPE::STATE);
            });
        });
    });
}

async::task<void> server::on_group_broadcast(const internal_resp::BroadcastGroup& resp)
{
    this->assert_group(resp.error, "");

    co_await this->ensure_group(resp.group, [this, message = resp.message, type = resp.type](auto& group) -> async::task<void> {
        auto members = std::vector<std::shared_ptr<fb::game::character>>();
        for (auto& member_ptr : group->characters())
        {
            members.push_back(member_ptr);
        }

        co_await this->characters.async_write([this, &message, &type, &members](auto& characters) -> async::task<void> {
            co_await characters.foreach (
                [this, message, type](auto& member) {
                    member->message(message, static_cast<MESSAGE_TYPE>(type));
                },
                members);
        });
    });
}
