#include <fb/game/server.h>
#include <fb/game/group.h>
#include <fb/game/character.h>
#include <fb/encoding.h>
#include <json/json.h>

using namespace fb::game;

void server::assert_group(uint32_t error, std::string_view actor) const
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

async::task<void> server::ensure_group(uint32_t id, ensure_group_fn fn)
{
    auto thread = this->threads.current();
    if (thread == nullptr)
        throw std::runtime_error(std::format("No thread available for ensure_group (group_id: {})", id));

    // Use try_async_write with retry mechanism
    // Note: We need to ensure we return to the original thread after try_async_write completes
    auto success = co_await thread->enqueue_with_retry(
        [=, this](auto& thread) -> async::task<bool> {
            auto result = co_await this->groups.try_async_write(
                id,
                [this, fn, &thread](auto& group) -> async::task<void> {
                    co_await fn(group);
                    // Ensure we return to the original thread after fn completes
                    co_await thread.switching();
                },
                [=, this]() -> async::task<std::shared_ptr<fb::game::group>> {
                    auto   world = fb::config<uint32_t>("world");
                    auto&& resp =
                        co_await this->http.get<internal_resp::GroupDetails>("internal",
                                                                             std::format("/group/{}/{}", world, id));
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
            co_return result;
        },
        10 // max_retries
    );

    if (!success)
    {
        throw std::runtime_error(std::format("Failed to acquire group lock after retries (group_id: {})", id));
    }
}

async::task<void> server::create_group(character& me, std::string_view target_name)
{
    auto weak            = me.weak_from_this_as<fb::game::character>();
    auto target_name_str = std::string(target_name);

    if (me.option(OPTION::GROUP) == false)
        throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_MINE));

    if (me.group_id().has_value())
        throw std::runtime_error(_TEXT(MESSAGE_GROUP_ALREADY_JOINED));
    if (me.name() == target_name_str)
        throw std::runtime_error(_TEXT(MESSAGE_CANNOT_GROUP_SELF));

    // Try to find target in the same thread's thread_params
    auto current_thread = this->threads.current();
    if (current_thread != nullptr)
    {
        auto params = current_thread->template data<thread_params>();
        auto target = params->characters.find(target_name_str);
        if (target != nullptr)
        {
            // Found target in same thread - check state without thread switching
            if (target->group_id().has_value())
                throw std::runtime_error(_TEXT(MESSAGE_GROUP_ALREADY_JOINED));

            if (target->option(OPTION::GROUP) == false)
                throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_TARGET));
        }
    }

    // Call API (either target not found in same thread, or checks passed)
    auto   world = fb::config<uint32_t>("world");
    auto&& resp  = co_await this->http.post(
        "internal",
        "/group/create",
        internal_reqs::CreateGroup{world, fb::config<uint32_t>("host"), me.id, target_name_str});
    co_await this->threads.switching(weak);
    co_await this->on_create_group(resp);
}

async::task<void> server::destroy_group(character& me)
{
    auto group_id = me.group_id();
    if (group_id.has_value() == false)
        co_return;

    auto   weak  = me.weak_from_this_as<fb::game::character>();
    auto   world = fb::config<uint32_t>("world");
    auto&& resp  = co_await this->http.post("internal",
                                           "/group/destroy",
                                           internal_reqs::DestroyGroup{world, fb::config<uint32_t>("host"), me.name()});
    co_await this->threads.switching(weak);
    co_await this->on_destroyed_group(resp);
}

async::task<void> server::toggle_group_member(character& actor, std::string_view target_name)
{
    auto weak            = actor.weak_from_this_as<character>();
    auto target_name_str = std::string(target_name);

    // Try to find target in the same thread's thread_params
    auto current_thread = this->threads.current();
    if (current_thread != nullptr)
    {
        auto params = current_thread->template data<thread_params>();
        auto target = params->characters.find(target_name_str);
        if (target != nullptr)
        {
            // Found target in same thread - check state without thread switching
            if (target->option(OPTION::GROUP) == false)
                throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_TARGET));
        }
    }

    // Call toggle API (either target not found in same thread, or checks passed)
    auto   world = fb::config<uint32_t>("world");
    auto&& resp  = co_await this->http.post(
        "internal",
        "/group/toggle",
        internal_reqs::EnterGroup{world, fb::config<uint32_t>("host"), actor.id, target_name_str});
    co_await this->threads.switching(weak);
    co_await this->on_updated_group(resp);
}

async::task<void> server::leave_group_member(character& leaver)
{
    auto weak = leaver.weak_from_this_as<character>();
    if (leaver.group_id().has_value() == false)
        co_return;

    auto   world = fb::config<uint32_t>("world");
    auto&& resp =
        co_await this->http.post("internal",
                                 "/group/leave",
                                 internal_reqs::LeaveGroup{world, fb::config<uint32_t>("host"), leaver.name()});
    co_await this->threads.switching(weak);
    co_await this->on_updated_group(resp);
}

async::task<void> server::broadcast_group(uint32_t group_id, std::string_view message, MESSAGE_TYPE type)
{
    auto   message_str = std::string(message);
    auto   world       = fb::config<uint32_t>("world");
    auto&& resp        = co_await this->http.post("internal",
                                           "/group/broadcast",
                                           internal_reqs::BroadcastGroup{world,
                                                                         fb::config<uint32_t>("host"),
                                                                         group_id,
                                                                         message_str,
                                                                         static_cast<uint8_t>(type)});
    co_await this->on_group_broadcast(resp);
}

async::task<void> server::handle_group_action(character& actor, std::string_view target_name)
{
    auto weak            = actor.weak_from_this_as<fb::game::character>();
    auto target_name_str = std::string(target_name);
    if (actor.option(OPTION::GROUP) == false)
        throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_MINE));

    // Define action function type
    using action_func = std::function<async::task<void>(character&, const std::string&)>;
    auto action       = action_func();

    // Case 1: Actor clicks their own group icon (actor == target)
    if (actor.name() == target_name_str)
    {
        auto group_id = actor.group_id();
        if (group_id.has_value() == false)
            throw std::runtime_error(_TEXT(MESSAGE_CANNOT_GROUP_SELF));

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
        // Determine action based on actor's state and group state only
        auto actor_group_id = actor.group_id();

        if (actor_group_id.has_value() == false)
        {
            // Actor has no group - try to create (API will handle errors if target has group)
            action = [this, target_name_str](character& actor, const std::string&) -> async::task<void> {
                co_await this->create_group(actor, target_name_str);
            };
        }
        else
        {
            // Actor has a group - check if master
            auto actor_group_id_value = actor_group_id.value();
            auto actor_name           = actor.name();
            co_await this->ensure_group(
                actor_group_id_value,
                [this, &action, weak, target_name_str, actor_name](auto& group) -> async::task<void> {
                    auto is_master = (group->master() == actor_name);

                    action = [this, is_master, target_name_str, weak](character& actor,
                                                                      const std::string&) -> async::task<void> {
                        if (is_master == false)
                        {
                            auto actor = weak.lock();
                            if (actor != nullptr)
                                actor->message(_TEXT(MESSAGE_GROUP_NOT_OWNER));

                            co_return;
                        }

                        // Actor is master - toggle member (enter if not in group, kick if already in group)
                        co_await this->toggle_group_member(actor, target_name_str);
                    };
                    co_return;
                });
        }
    }

    co_await this->threads.switching(weak);
    if (action)
    {
        try
        {
            co_await action(actor, target_name_str);
        }
        catch (std::exception& e)
        {
            if (weak.expired() == false)
                actor.message(e.what(), MESSAGE_TYPE::STATE);
        }
    }
}

async::task<void> server::on_create_group(const internal_resp::GroupDetails& resp)
{
    this->assert_group(resp.error, resp.target);

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
            if (master_uid != 0)
            {
                co_await this->characters.async_write(
                    [id, master_uid, &group, master, &resp, this](auto& characters) -> async::task<void> {
                        // First, add all members to the group and collect valid character pointers
                        auto group_members = std::vector<std::shared_ptr<fb::game::character>>();
                        auto member_names  = std::vector<std::string>{};
                        member_names.reserve(resp.members.size());
                        for (const auto& member : resp.members)
                        {
                            member_names.push_back(member.name);
                        }

                        // Add master to group
                        auto ch = characters.find(master_uid);
                        if (ch != nullptr)
                        {
                            auto weak = ch->template weak_from_this_as<character>();
                            group->enter(weak);

                            // Log group create event (always log regardless of weak state)
                            auto log_data              = Json::Value();
                            log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
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
                                auto invited_weak = invited_ch->template weak_from_this_as<character>();
                                if (invited_weak.expired() == false)
                                {
                                    group->enter(invited_weak);
                                    group_members.push_back(invited_ch);
                                }
                            }
                        }

                        characters.foreach_enqueue(
                            [id, member_names](auto& member_ptr) -> async::task<void> {
                                member_ptr->group_id(id);
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
    auto target = std::string{};
    if (resp.target.has_value())
        target = resp.target.value().name;
    this->assert_group(resp.error, target);

    co_await this->ensure_group(resp.group_id, [this, &resp](auto& group) -> async::task<void> {
        switch (static_cast<internal::GroupActionType>(resp.action))
        {
        case internal::GroupActionType::Enter:
        {
            if (resp.new_member.has_value() == false)
                break;

            auto new_member = std::string{resp.new_member.value().name};
            co_await this->characters.async_write([this, new_member, group](auto& characters) -> async::task<void> {
                auto ch = characters.find(new_member);
                if (ch == nullptr)
                    co_return;

                auto members = std::vector<std::shared_ptr<fb::game::character>>();
                for (auto& member_ptr : group->characters())
                {
                    members.push_back(member_ptr);
                }

                characters.foreach_enqueue(
                    [this, new_member](auto& member) -> async::task<void> {
                        member->message(std::format(_TEXT(MESSAGE_GROUP_JOINED), new_member), MESSAGE_TYPE::STATE);
                        co_return;
                    },
                    members);

                // Log group enter event (always log regardless of weak state)
                auto log_data              = Json::Value();
                log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
                log_data["character_name"] = UTF8(new_member, PLATFORM::WINDOWS);
                log_data["group_id"]       = static_cast<Json::Int64>(group->id());
                this->log.write("group_enter", log_data);

                auto weak = ch->template weak_from_this_as<character>();
                group->enter(weak);
                group->add_member(new_member);

                auto thread = this->threads.current();
                co_await this->threads.switching(weak);
                auto ptr = weak.lock();
                if (ptr != nullptr)
                {
                    ptr->group_id(group->id());
                    ptr->message(_TEXT(MESSAGE_GROUP_JOINED_SUCCESS), MESSAGE_TYPE::STATE);
                }

                if (thread != nullptr)
                    co_await thread->switching();
            });
            break;
        }

        case internal::GroupActionType::Leave:
        case internal::GroupActionType::Kick:
        {
            if (resp.deleted_member.has_value() == false)
                break;

            group->remove_member(resp.deleted_member.value().name);

            co_await this->characters.async_write([this, &resp, group](auto& characters) -> async::task<void> {
                auto ch            = characters.find(resp.deleted_member.value().name);
                auto before_thread = this->threads.current();
                if (ch != nullptr)
                {
                    auto weak = ch->template weak_from_this_as<character>();
                    group->detach(weak);

                    co_await this->threads.switching(weak);
                    auto ptr = weak.lock();
                    if (ptr != nullptr)
                    {
                        ptr->group_reset();
                        ptr->message(resp.action == internal::GroupActionType::Kick ? _TEXT(MESSAGE_GROUP_KICKED)
                                                                                    : _TEXT(MESSAGE_GROUP_LEFT_SUCCESS),
                                     MESSAGE_TYPE::STATE);
                    }

                    // Log group leave/kick event
                    auto log_data              = Json::Value();
                    log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
                    log_data["character_name"] = UTF8(resp.deleted_member.value().name, PLATFORM::WINDOWS);
                    log_data["group_id"]       = static_cast<Json::Int64>(group->id());
                    switch (resp.action)
                    {
                    case internal::GroupActionType::Kick:
                        this->log.write("group_kick", log_data);
                        break;
                    case internal::GroupActionType::Leave:
                        this->log.write("group_leave", log_data);
                        break;
                    default:
                        break;
                    }
                    if (before_thread != nullptr)
                        co_await before_thread->switching();
                }

                auto members = std::vector<std::shared_ptr<fb::game::character>>();
                for (auto& member_ptr : group->characters())
                {
                    members.push_back(member_ptr);
                }

                auto message = resp.action == internal::GroupActionType::Kick
                                   ? std::format(_TEXT(MESSAGE_GROUP_MEMBER_KICKED), resp.deleted_member.value().name)
                                   : std::format(_TEXT(MESSAGE_GROUP_MEMBER_LEFT), resp.deleted_member.value().name);
                characters.foreach_enqueue(
                    [this, message](auto& member) -> async::task<void> {
                        member->message(message, MESSAGE_TYPE::STATE);
                        co_return;
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
    this->groups.erase(gid, [this, gid](const auto& group) {
        auto members = std::vector<std::string>{group->members()};
        members.push_back(group->master());

        this->characters.write([this, &members](auto& characters) {
            characters.foreach_enqueue(members, [](auto& ch) -> async::task<void> {
                ch->group_reset();
                ch->message(_TEXT(MESSAGE_GROUP_DISBANDED), MESSAGE_TYPE::STATE);
                co_return;
            });
        });
    });
    co_return;
}

async::task<void> server::on_group_broadcast(const internal_resp::BroadcastGroup& resp)
{
    this->assert_group(resp.error, "");

    co_await this->ensure_group(resp.group,
                                [this, message = resp.message, type = resp.type](auto& group) -> async::task<void> {
                                    auto members = std::vector<std::shared_ptr<fb::game::character>>();
                                    for (auto& member_ptr : group->characters())
                                    {
                                        members.push_back(member_ptr);
                                    }

                                    this->characters.write([this, &message, &type, &members](auto& characters) {
                                        characters.foreach_enqueue(
                                            [this, message, type](auto& member) -> async::task<void> {
                                                member->message(message, static_cast<MESSAGE_TYPE>(type));
                                                co_return;
                                            },
                                            members);
                                    });
                                    co_return;
                                });
}
