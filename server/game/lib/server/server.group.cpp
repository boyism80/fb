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

        if (me.group_id().has_value())
            throw std::runtime_error(_TEXT(MESSAGE_GROUP_ALREADY_JOINED));

        auto&& resp = co_await this->http.post("internal", "/group/create", request::CreateGroup{fb::config<uint32_t>("host"), me.id(), target});
        co_await this->threads.switching(weak);
        co_await this->on_create_group(resp);

        co_return true;
    }
    catch (std::exception& e)
    {
        auto ch = weak.lock();
        if (ch != nullptr)
            ch->message(e.what(), MESSAGE_TYPE::STATE);

        co_return false;
    }
}

async::task<bool> server::create_group(character& me, std::weak_ptr<character> target_weak)
{
    auto weak = me.weak_from_this_as<fb::game::character>();
    try
    {
        if (me.option(OPTION::GROUP) == false)
            throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_MINE));

        if (me.group_id().has_value())
            throw std::runtime_error(_TEXT(MESSAGE_GROUP_ALREADY_JOINED));

        auto target = target_weak.lock();
        if (target == nullptr)
            co_return false;

        // TODO: Verify behavior when target disconnects during sleep
        // If target disconnects between sleep and switching, threads.switching(target_weak) may throw an exception.
        // Need to check:
        // 1. What exception is thrown when switching to an expired weak_ptr?
        // 2. How is the exception handled in the catch block?
        // 3. Does the error message reach the actor correctly?
        // 4. Is there any resource leak or undefined behavior?
        // Switch to target's thread to check target's state
        // co_await this->sleep(10s);

        co_await this->threads.switching(target_weak);
        auto target_name          = target->name();
        auto target_has_group     = target->group_id().has_value();
        auto target_group_enabled = target->option(OPTION::GROUP);

        // Switch back to actor's thread
        co_await this->threads.switching(weak);

        // Check constraints before API call
        if (target_has_group)
            throw std::runtime_error(_TEXT(MESSAGE_GROUP_ALREADY_JOINED));

        if (target_group_enabled == false)
            throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_TARGET));

        auto&& resp = co_await this->http.post("internal", "/group/create", request::CreateGroup{fb::config<uint32_t>("host"), me.id(), target_name});
        co_await this->threads.switching(weak);
        co_await this->on_create_group(resp);

        co_return true;
    }
    catch (std::exception& e)
    {
        auto ch = weak.lock();
        if (ch != nullptr)
            ch->message(e.what(), MESSAGE_TYPE::STATE);

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

async::task<void> server::enter_group_member(character& inviter, const std::string& target_name)
{
    auto   weak = inviter.weak_from_this_as<character>();
    auto&& resp = co_await this->http.post("internal", "/group/enter", request::EnterGroup{fb::config<uint32_t>("host"), inviter.id(), target_name});
    co_await this->threads.switching(weak);
    co_await this->on_updated_group(resp);
}

async::task<void> server::enter_group_member(character& inviter, std::weak_ptr<character> target_weak)
{
    auto weak = inviter.weak_from_this_as<character>();
    try
    {
        auto target = target_weak.lock();
        if (target == nullptr)
            throw std::runtime_error(_TEXT(MESSAGE_USER_NOT_LOGIN));

        // Switch to target's thread to check target's state
        co_await this->threads.switching(target_weak);
        auto target_name          = target->name();
        auto target_has_group     = target->group_id().has_value();
        auto target_group_enabled = target->option(OPTION::GROUP);

        // Switch back to inviter's thread
        co_await this->threads.switching(weak);

        // Check constraints before API call
        if (target_has_group)
            throw std::runtime_error(_TEXT(MESSAGE_GROUP_ALREADY_JOINED));

        if (target_group_enabled == false)
            throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_TARGET));

        auto&& resp = co_await this->http.post("internal", "/group/enter", request::EnterGroup{fb::config<uint32_t>("host"), inviter.id(), target_name});
        co_await this->threads.switching(weak);
        co_await this->on_updated_group(resp);
    }
    catch (std::exception& e)
    {
        auto ch = weak.lock();
        if (ch != nullptr)
            ch->message(e.what(), MESSAGE_TYPE::STATE);
    }
}

async::task<void> server::leave_group_member(character& leaver)
{
    auto   weak = leaver.weak_from_this_as<character>();
    auto&& resp = co_await this->http.post("internal", "/group/leave", request::LeaveGroup{fb::config<uint32_t>("host"), leaver.name()});
    co_await this->threads.switching(weak);
    co_await this->on_updated_group(resp);
}

async::task<void> server::kick_group_member(character& kicker, const std::string& target_name)
{
    auto   weak = kicker.weak_from_this_as<character>();
    auto&& resp = co_await this->http.post("internal", "/group/kick", request::KickGroup{fb::config<uint32_t>("host"), kicker.name(), target_name});
    co_await this->threads.switching(weak);
    co_await this->on_updated_group(resp);
}

async::task<void> server::kick_group_member(character& kicker, std::weak_ptr<character> target_weak)
{
    auto weak = kicker.weak_from_this_as<character>();
    try
    {
        auto target = target_weak.lock();
        if (target == nullptr)
            throw std::runtime_error(_TEXT(MESSAGE_USER_NOT_LOGIN));

        // Switch to target's thread to check target's state
        co_await this->threads.switching(target_weak);
        auto target_name     = target->name();
        auto target_group_id = target->group_id();

        // Switch back to kicker's thread
        co_await this->threads.switching(weak);

        // Check constraints before API call
        // TODO: Fix error message - MESSAGE_ALREADY_JOINED_GROUP means "already joined group" which is contradictory
        // When kicker has no group, the error message should indicate they are not in a group, not that they've already joined one
        // Need to define an appropriate error message constant (e.g., MESSAGE_GROUP_NOT_JOINED) and use it here
        auto kicker_group_id = kicker.group_id();
        if (kicker_group_id.has_value() == false)
            throw std::runtime_error(_TEXT(MESSAGE_ALREADY_JOINED_GROUP));

        if (target_group_id.has_value() == false || target_group_id.value() != kicker_group_id.value())
            throw std::runtime_error(_TEXT(MESSAGE_GROUP_ALREADY_JOINED));

        auto&& resp = co_await this->http.post("internal", "/group/kick", request::KickGroup{fb::config<uint32_t>("host"), kicker.name(), target_name});
        co_await this->threads.switching(weak);
        co_await this->on_updated_group(resp);
    }
    catch (std::exception& e)
    {
        auto ch = weak.lock();
        if (ch != nullptr)
            ch->message(e.what(), MESSAGE_TYPE::STATE);
    }
}

async::task<void> server::broadcast_group(uint32_t group_id, const std::string& message, MESSAGE_TYPE type)
{
    auto&& resp = co_await this->http.post("internal", "/group/broadcast", request::BroadcastGroup{fb::config<uint32_t>("host"), group_id, message, static_cast<uint8_t>(type)});
    co_await this->on_group_broadcast(resp);
}

async::task<bool> server::handle_group_action(character& actor, const std::string& target_name)
{
    auto weak = actor.weak_from_this_as<fb::game::character>();
    try
    {
        if (actor.option(OPTION::GROUP) == false)
            throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_MINE));

        // Define action function type
        using action_func = std::function<async::task<void>(character&, const std::string&)>;
        auto action       = action_func();

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
            auto actor_name = actor.name();
            co_await this->ensure_group(group_id.value(), [this, &action, actor_name](auto& group) -> async::task<void> {
                auto is_master = (group->master() == actor_name);
                action         = [this, is_master](character& actor, const std::string&) -> async::task<void> {
                    if (is_master)
                    {
                        // Master - destroy group
                        co_await this->destroy_group(actor);
                    }
                    else
                    {
                        // Member - leave group
                        co_await this->leave_group_member(actor);
                    }
                };
                co_return;
            });
        }
        else
        {
            // Case 2: Actor clicks target's group icon (actor != target)
            // Get target's weak_ptr from characters lock
            auto target_weak = std::weak_ptr<character>();
            co_await this->characters.async_read([&target_name, &target_weak](auto& characters) -> async::task<void> {
                auto target = characters.find(target_name);
                if (target != nullptr)
                {
                    target_weak = target->weak_from_this_as<character>();
                }
                co_return;
            });

            // Define action function types
            using action_func_with_weak = std::function<async::task<void>(character&, std::weak_ptr<character>)>;
            using action_func_with_name = std::function<async::task<void>(character&, const std::string&)>;
            auto action_with_weak       = action_func_with_weak();
            auto action_with_name       = action_func_with_name();

            if (target_weak.lock() != nullptr)
            {
                // Target is on the same server - switch to target's thread to get information
                co_await this->threads.switching(target_weak);
                auto target = target_weak.lock();
                if (target != nullptr)
                {
                    auto target_group_id = target->group_id();
                    auto actor_group_id  = actor.group_id();

                    // Switch back to actor's thread
                    co_await this->threads.switching(weak);

                    if (actor_group_id.has_value() == false)
                    {
                        // Actor has no group
                        if (target_group_id.has_value())
                        {
                            // Target has a group - cannot create
                            action_with_weak = [](character&, std::weak_ptr<character>) -> async::task<void> {
                                throw std::runtime_error(_TEXT(MESSAGE_GROUP_ALREADY_JOINED));
                            };
                        }
                        else
                        {
                            // Both have no group - create (using weak_ptr version)
                            action_with_weak = [this](character& actor, std::weak_ptr<character> target_weak) -> async::task<void> {
                                co_await this->create_group(actor, target_weak);
                            };
                        }
                    }
                    else
                    {
                        // Actor has a group - check if master
                        auto actor_group_id_value = actor_group_id.value();
                        auto actor_name           = actor.name();
                        co_await this->ensure_group(actor_group_id_value, [this, target_group_id, &action_with_weak, target_weak, actor_name](auto& group) -> async::task<void> {
                            auto is_master  = (group->master() == actor_name);
                            auto same_group = target_group_id.has_value() && (target_group_id.value() == group->id());

                            action_with_weak = [this, is_master, target_group_id, same_group, target_weak](character& actor, std::weak_ptr<character>) -> async::task<void> {
                                if (is_master == false)
                                {
                                    // Actor is a member, not master - do nothing
                                    co_return;
                                }

                                // Actor is master
                                if (target_group_id.has_value())
                                {
                                    if (same_group)
                                    {
                                        // Same group - kick member (using weak_ptr version)
                                        co_await this->kick_group_member(actor, target_weak);
                                    }
                                    else
                                    {
                                        // Different group - cannot invite
                                        throw std::runtime_error(_TEXT(MESSAGE_GROUP_ALREADY_JOINED));
                                    }
                                }
                                else
                                {
                                    // Target has no group - enter group (using weak_ptr version)
                                    co_await this->enter_group_member(actor, target_weak);
                                }
                            };
                            co_return;
                        });
                    }
                }
            }
            else
            {
                // Target is on a different server - must call API and handle errors
                auto actor_group_id = actor.group_id();

                if (actor_group_id.has_value() == false)
                {
                    // Actor has no group - try to create (API will handle errors)
                    action_with_name = [this](character& actor, const std::string& target_name) -> async::task<void> {
                        co_await this->create_group(actor, target_name);
                    };
                }
                else
                {
                    // Actor has a group - check if master
                    auto actor_group_id_value = actor_group_id.value();
                    auto actor_name           = actor.name();
                    co_await this->ensure_group(actor_group_id_value, [this, &action_with_name, target_name, actor_name](auto& group) -> async::task<void> {
                        auto is_master = (group->master() == actor_name);

                        action_with_name = [this, is_master, target_name](character& actor, const std::string&) -> async::task<void> {
                            if (is_master)
                            {
                                // Actor is master - try to enter (API will handle errors)
                                co_await this->enter_group_member(actor, target_name);
                            }
                        };
                        co_return;
                    });
                }
            }

            // Store action functions
            if (action_with_weak)
            {
                action = [action_with_weak, target_weak](character& actor, const std::string&) -> async::task<void> {
                    co_await action_with_weak(actor, target_weak);
                };
            }
            else if (action_with_name)
            {
                action = [action_with_name, target_name](character& actor, const std::string&) -> async::task<void> {
                    co_await action_with_name(actor, target_name);
                };
            }
        }

        co_await this->threads.switching(weak);
        if (action)
        {
            co_await action(actor, target_name);
        }

        co_return true;
    }
    catch (std::exception& e)
    {
        auto ch = weak.lock();
        if (ch != nullptr)
            ch->message(e.what(), MESSAGE_TYPE::STATE);

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
                co_await this->characters.async_write([id, master_uid, &group, master, &resp, this](auto& characters) -> async::task<void> {
                    // First, add all members to the group and collect valid character pointers
                    auto group_members = std::vector<std::shared_ptr<fb::game::character>>();

                    // Add master to group
                    auto ch = characters.find(master_uid);
                    if (ch != nullptr)
                    {
                        auto weak = ch->weak_from_this_as<character>();
                        ch->group_id(id);
                        group->enter(weak);

                        // Log group create event (always log regardless of weak state)
                        auto log_data              = Json::Value();
                        log_data["character_id"]   = static_cast<Json::Int64>(ch->id());
                        log_data["character_name"] = UTF8(ch->name(), PLATFORM::WINDOWS);
                        log_data["group_id"]       = static_cast<Json::Int64>(id);
                        this->log.write("group_create", log_data);

                        if (weak.expired() == false)
                        {
                            group_members.push_back(ch);
                        }
                    }

                    // Add invited members to group
                    for (auto& member : resp.members)
                    {
                        if (member.name == master)
                            continue;

                        auto invited_ch = characters.find(member.name);
                        if (invited_ch != nullptr)
                        {
                            auto invited_weak = invited_ch->weak_from_this_as<character>();
                            if (invited_weak.expired() == false)
                            {
                                invited_ch->group_id(id);
                                group->enter(invited_weak);
                                group_members.push_back(invited_ch);
                            }
                        }
                    }

                    // Send messages to all members
                    for (auto& member_ptr : group_members)
                    {
                        if (member_ptr == nullptr)
                            continue;

                        // Send "{}님 그룹 참여" message for other members
                        for (auto& other_member : resp.members)
                        {
                            if (other_member.name != member_ptr->name())
                            {
                                member_ptr->message(std::format("{}님 그룹 참여", other_member.name), MESSAGE_TYPE::STATE);
                            }
                        }

                        // Send "그룹에 참여했습니다." message
                        member_ptr->message("그룹에 참여했습니다.", MESSAGE_TYPE::STATE);
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

                // Log group enter event (always log regardless of weak state)
                auto log_data              = Json::Value();
                log_data["character_id"]   = static_cast<Json::Int64>(ch->id());
                log_data["character_name"] = UTF8(resp.new_member.value().name, PLATFORM::WINDOWS);
                log_data["group_id"]       = static_cast<Json::Int64>(group->id());
                this->log.write("group_enter", log_data);

                if (weak.expired() == false)
                {
                    group->enter(weak);
                    ch->group_id(group->id());
                    ch->message("그룹에 참여했습니다.", MESSAGE_TYPE::STATE);
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
