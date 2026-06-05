#include <fb/game/group/container.h>
#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/game/thread_params.h>
#include <fb/encoding.h>
#include <json/json.h>

using namespace fb::game;
namespace internal_resp = fb::protocol::internal::response;
namespace internal_reqs = fb::protocol::internal::request;

group_container::group_container(fb::game::server& server) :
    fb::sharded_container<group_ptr, 16>([](const group_ptr& group) -> uint32_t {
        return group->id();
    }),
    server(server)
{ }

void group_container::detach(std::weak_ptr<character> weak, uint32_t group_id)
{
    this->write(group_id, [weak](auto& group) {
        group->detach(weak);
    });
}

async::task<void> group_container::on_error(uint32_t error, std::string_view actor)
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

async::task<void> group_container::ensure(uint32_t id, ensure_fn fn)
{
    auto thread = this->server.threads.current();
    if (thread == nullptr)
        throw std::runtime_error(std::format("No thread available for ensure (group_id: {})", id));

    auto builder        = thread->new_builder<>();
    builder.retry_count = 10;
    builder.func        = [=, this](auto& thread) -> async::task<void> {
        if (co_await this->try_async_write(
                id,
                [this, fn, &thread](auto& group) -> async::task<void> {
                    co_await fn(group);
                    co_await thread.switching();
                },
                [=, this]() -> async::task<group_ptr> {
                    auto   world = fb::config<uint32_t>("world");
                    auto&& resp  = co_await this->server.http.get<internal_resp::GroupDetails>(
                        "internal",
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
                        co_return this->server.make<group>(id, resp.group.master, members);
                    }

                    default:
                        throw std::runtime_error(std::format("cannot get group (error : {})", resp.error));
                    }
                }) == false)
        {
            throw fb::thread::retry_exception{};
        }
    };
    co_await builder.dispatch();
}

async::task<void> group_container::create(character& me, std::string_view target_name)
{
    auto weak            = me.weak_from_this_as<character>();
    auto target_name_str = std::string(target_name);

    if (me.option(OPTION::GROUP) == false)
        throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_MINE));

    if (me.group_id().has_value())
        throw std::runtime_error(_TEXT(MESSAGE_GROUP_ALREADY_JOINED));
    if (me.name() == target_name_str)
        throw std::runtime_error(_TEXT(MESSAGE_CANNOT_GROUP_SELF));

    auto current_thread = this->server.threads.current();
    if (current_thread != nullptr)
    {
        auto params = current_thread->template data<thread_params>();
        auto target = params->characters.find(target_name_str);
        if (target != nullptr)
        {
            if (target->group_id().has_value())
                throw std::runtime_error(_TEXT(MESSAGE_GROUP_ALREADY_JOINED));

            if (target->option(OPTION::GROUP) == false)
                throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_TARGET));
        }
    }

    auto   world = fb::config<uint32_t>("world");
    auto&& resp  = co_await this->server.http.post(
        "internal",
        "/group/create",
        internal_reqs::CreateGroup{world, fb::config<uint32_t>("host"), me.id, target_name_str});
    co_await this->server.threads.switching(weak);

    auto members = std::map<uint32_t, std::string>{};
    for (const auto& member : resp.members)
    {
        members.emplace(member.uid, member.name);
    }

    co_await this->on_error(resp.error, resp.target);
    co_await this->on_create(resp.target, resp.group.id, resp.group.master, std::move(members));
}

async::task<void> group_container::destroy(character& me)
{
    auto group_id = me.group_id();
    if (group_id.has_value() == false)
        co_return;

    auto   weak  = me.weak_from_this_as<character>();
    auto   world = fb::config<uint32_t>("world");
    auto&& resp =
        co_await this->server.http.post("internal",
                                        "/group/destroy",
                                        internal_reqs::DestroyGroup{world, fb::config<uint32_t>("host"), me.name()});
    co_await this->server.threads.switching(weak);
    co_await this->on_error(resp.error, resp.actor.name);
    co_await this->on_destroyed(resp.actor.name, resp.group_id);
}

async::task<void> group_container::toggle_member(character& actor, std::string_view target_name)
{
    auto weak            = actor.weak_from_this_as<character>();
    auto target_name_str = std::string(target_name);

    auto current_thread = this->server.threads.current();
    if (current_thread != nullptr)
    {
        auto params = current_thread->template data<thread_params>();
        auto target = params->characters.find(target_name_str);
        if (target != nullptr)
        {
            if (target->option(OPTION::GROUP) == false)
                throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_TARGET));
        }
    }

    auto   world = fb::config<uint32_t>("world");
    auto&& resp  = co_await this->server.http.post(
        "internal",
        "/group/toggle",
        internal_reqs::EnterGroup{world, fb::config<uint32_t>("host"), actor.id, target_name_str});
    co_await this->server.threads.switching(weak);

    auto target = std::string{};
    if (resp.target.has_value())
        target = resp.target.value().name;

    std::optional<std::string> new_member = std::nullopt;
    if (resp.new_member.has_value())
        new_member = resp.new_member.value().name;

    std::optional<std::string> deleted_member = std::nullopt;
    if (resp.deleted_member.has_value())
        deleted_member = resp.deleted_member.value().name;

    co_await this->on_error(resp.error, target);
    switch (resp.action)
    {
    case fb::protocol::internal::GroupActionType::Enter:
        co_await this->on_enter(std::move(target), resp.group_id, std::move(new_member));
        break;

    case fb::protocol::internal::GroupActionType::Leave:
        co_await this->on_leave(std::move(target), resp.group_id, std::move(deleted_member));
        break;

    case fb::protocol::internal::GroupActionType::Kick:
        co_await this->on_kick(std::move(target), resp.group_id, std::move(deleted_member));
        break;

    default:
        break;
    }
}

async::task<void> group_container::leave_member(character& leaver)
{
    auto weak = leaver.weak_from_this_as<character>();
    if (leaver.group_id().has_value() == false)
        co_return;

    auto   world = fb::config<uint32_t>("world");
    auto&& resp =
        co_await this->server.http.post("internal",
                                        "/group/leave",
                                        internal_reqs::LeaveGroup{world, fb::config<uint32_t>("host"), leaver.name()});
    co_await this->server.threads.switching(weak);

    auto target = std::string{};
    if (resp.target.has_value())
        target = resp.target.value().name;

    std::optional<std::string> new_member = std::nullopt;
    if (resp.new_member.has_value())
        new_member = resp.new_member.value().name;

    std::optional<std::string> deleted_member = std::nullopt;
    if (resp.deleted_member.has_value())
        deleted_member = resp.deleted_member.value().name;

    co_await this->on_error(resp.error, target);
    switch (resp.action)
    {
    case fb::protocol::internal::GroupActionType::Enter:
        co_await this->on_enter(std::move(target), resp.group_id, std::move(new_member));
        break;

    case fb::protocol::internal::GroupActionType::Leave:
        co_await this->on_leave(std::move(target), resp.group_id, std::move(deleted_member));
        break;

    case fb::protocol::internal::GroupActionType::Kick:
        co_await this->on_kick(std::move(target), resp.group_id, std::move(deleted_member));
        break;

    default:
        break;
    }
}

async::task<void> group_container::broadcast(uint32_t group_id, std::string_view message, MESSAGE_TYPE type)
{
    auto   message_str = std::string(message);
    auto   world       = fb::config<uint32_t>("world");
    auto&& resp        = co_await this->server.http.post("internal",
                                                  "/group/broadcast",
                                                  internal_reqs::BroadcastGroup{world,
                                                                                fb::config<uint32_t>("host"),
                                                                                group_id,
                                                                                message_str,
                                                                                static_cast<uint8_t>(type)});
    co_await this->on_error(resp.error, "");
    co_await this->on_broadcast(resp.group, std::move(resp.message), resp.type);
}

async::task<void> group_container::handle_action(character& actor, std::string_view target_name)
{
    auto weak            = actor.weak_from_this_as<character>();
    auto target_name_str = std::string(target_name);
    if (actor.option(OPTION::GROUP) == false)
        throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_MINE));

    using action_func = std::function<async::task<void>(character&, const std::string&)>;
    auto action       = action_func();

    if (actor.name() == target_name_str)
    {
        auto group_id = actor.group_id();
        if (group_id.has_value() == false)
            throw std::runtime_error(_TEXT(MESSAGE_CANNOT_GROUP_SELF));

        auto actor_name = actor.name();
        co_await this->ensure(group_id.value(), [this, &action, actor_name](auto& group) -> async::task<void> {
            auto is_master = (group->master() == actor_name);
            action         = [this, is_master](character& actor, const std::string&) -> async::task<void> {
                if (is_master)
                    co_await this->destroy(actor);
                else
                    co_await this->leave_member(actor);
            };
            co_return;
        });
    }
    else
    {
        auto actor_group_id = actor.group_id();

        if (actor_group_id.has_value() == false)
        {
            action = [this, target_name_str](character& actor, const std::string&) -> async::task<void> {
                co_await this->create(actor, target_name_str);
            };
        }
        else
        {
            auto actor_group_id_value = actor_group_id.value();
            auto actor_name           = actor.name();
            co_await this->ensure(actor_group_id_value,
                                  [this, &action, weak, target_name_str, actor_name](auto& group) -> async::task<void> {
                                      auto is_master = (group->master() == actor_name);

                                      action = [this, is_master, target_name_str, weak](
                                                   character& actor,
                                                   const std::string&) -> async::task<void> {
                                          if (is_master == false)
                                          {
                                              auto actor_ptr = weak.lock();
                                              if (actor_ptr != nullptr)
                                                  actor_ptr->message(_TEXT(MESSAGE_GROUP_NOT_OWNER));

                                              co_return;
                                          }

                                          co_await this->toggle_member(actor, target_name_str);
                                      };
                                      co_return;
                                  });
        }
    }

    co_await this->server.threads.switching(weak);
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

async::task<void> group_container::on_create(std::string                     target,
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
                auto  guard         = co_await this->server.characters.enter_write_async();
                auto& characters    = guard.value();
                auto  group_members = std::vector<std::shared_ptr<character>>();
                auto  ch            = characters.find(master_uid);
                if (ch != nullptr)
                {
                    auto weak = ch->template weak_from_this_as<character>();
                    group->enter(weak);

                    auto log_data              = Json::Value();
                    log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
                    log_data["character_name"] = UTF8(ch->name(), PLATFORM::WINDOWS);
                    log_data["group_id"]       = static_cast<Json::Int64>(group_id);
                    this->server.log.write("group_create", log_data);

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

                characters.foreach_enqueue(
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
            co_return;
        },
        [=, this]() -> async::task<group_ptr> {
            co_return this->server.make<group>(group_id, master, member_names);
        });
}

async::task<void> group_container::on_enter(std::string                target,
                                            uint32_t                   group_id,
                                            std::optional<std::string> new_member)
{
    if (new_member.has_value() == false)
        co_return;

    auto new_member_name = new_member.value();
    co_await this->ensure(group_id, [this, new_member_name](auto& group) -> async::task<void> {
        auto  guard      = co_await this->server.characters.enter_write_async();
        auto& characters = guard.value();
        auto  ch         = characters.find(new_member_name);
        if (ch == nullptr)
            co_return;

        auto members = std::vector<std::shared_ptr<character>>();
        for (auto& member_ptr : group->characters())
        {
            members.push_back(member_ptr);
        }

        characters.foreach_enqueue(
            [this, new_member_name](auto& member) -> async::task<void> {
                member->message(std::format(_TEXT(MESSAGE_GROUP_JOINED), new_member_name), MESSAGE_TYPE::STATE);
                co_return;
            },
            members);

        auto log_data              = Json::Value();
        log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
        log_data["character_name"] = UTF8(new_member_name, PLATFORM::WINDOWS);
        log_data["group_id"]       = static_cast<Json::Int64>(group->id());
        this->server.log.write("group_enter", log_data);

        auto weak = ch->template weak_from_this_as<character>();
        group->enter(weak);
        group->add_member(new_member_name);

        auto thread = this->server.threads.current();
        co_await this->server.threads.switching(weak);
        auto ptr = weak.lock();
        if (ptr != nullptr)
        {
            ptr->group_id(group->id());
            ptr->message(_TEXT(MESSAGE_GROUP_JOINED_SUCCESS), MESSAGE_TYPE::STATE);
        }

        if (thread != nullptr)
            co_await thread->switching();
    });
}

async::task<void> group_container::on_leave(std::string                target,
                                            uint32_t                   group_id,
                                            std::optional<std::string> deleted_member)
{
    if (deleted_member.has_value() == false)
        co_return;

    auto deleted_member_name = deleted_member.value();
    co_await this->ensure(group_id, [this, deleted_member_name](auto& group) -> async::task<void> {
        group->remove_member(deleted_member_name);

        auto  guard         = co_await this->server.characters.enter_write_async();
        auto& characters    = guard.value();
        auto  ch            = characters.find(deleted_member_name);
        auto  before_thread = this->server.threads.current();
        if (ch != nullptr)
        {
            auto weak = ch->template weak_from_this_as<character>();
            group->detach(weak);

            co_await this->server.threads.switching(weak);
            auto ptr = weak.lock();
            if (ptr != nullptr)
            {
                ptr->group_reset();
                ptr->message(_TEXT(MESSAGE_GROUP_LEFT_SUCCESS), MESSAGE_TYPE::STATE);
            }

            auto log_data              = Json::Value();
            log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
            log_data["character_name"] = UTF8(deleted_member_name, PLATFORM::WINDOWS);
            log_data["group_id"]       = static_cast<Json::Int64>(group->id());
            this->server.log.write("group_leave", log_data);
            if (before_thread != nullptr)
                co_await before_thread->switching();
        }

        auto members = std::vector<std::shared_ptr<character>>();
        for (auto& member_ptr : group->characters())
        {
            members.push_back(member_ptr);
        }

        auto message = std::format(_TEXT(MESSAGE_GROUP_MEMBER_LEFT), deleted_member_name);
        characters.foreach_enqueue(
            [this, message](auto& member) -> async::task<void> {
                member->message(message, MESSAGE_TYPE::STATE);
                co_return;
            },
            members);
    });
}

async::task<void> group_container::on_kick(std::string                target,
                                           uint32_t                   group_id,
                                           std::optional<std::string> deleted_member)
{
    if (deleted_member.has_value() == false)
        co_return;

    auto deleted_member_name = deleted_member.value();
    co_await this->ensure(group_id, [this, deleted_member_name](auto& group) -> async::task<void> {
        group->remove_member(deleted_member_name);

        auto  guard         = co_await this->server.characters.enter_write_async();
        auto& characters    = guard.value();
        auto  ch            = characters.find(deleted_member_name);
        auto  before_thread = this->server.threads.current();
        if (ch != nullptr)
        {
            auto weak = ch->template weak_from_this_as<character>();
            group->detach(weak);

            co_await this->server.threads.switching(weak);
            auto ptr = weak.lock();
            if (ptr != nullptr)
            {
                ptr->group_reset();
                ptr->message(_TEXT(MESSAGE_GROUP_KICKED), MESSAGE_TYPE::STATE);
            }

            auto log_data              = Json::Value();
            log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
            log_data["character_name"] = UTF8(deleted_member_name, PLATFORM::WINDOWS);
            log_data["group_id"]       = static_cast<Json::Int64>(group->id());
            this->server.log.write("group_kick", log_data);
            if (before_thread != nullptr)
                co_await before_thread->switching();
        }

        auto members = std::vector<std::shared_ptr<character>>();
        for (auto& member_ptr : group->characters())
        {
            members.push_back(member_ptr);
        }

        auto message = std::format(_TEXT(MESSAGE_GROUP_MEMBER_KICKED), deleted_member_name);
        characters.foreach_enqueue(
            [this, message](auto& member) -> async::task<void> {
                member->message(message, MESSAGE_TYPE::STATE);
                co_return;
            },
            members);
    });
}

async::task<void> group_container::on_destroyed(std::string actor, uint32_t group_id)
{
    this->erase(group_id, [this, group_id](const auto& group) {
        auto members = std::vector<std::string>{group->members()};
        members.push_back(group->master());

        auto guard = this->server.characters.enter_write();
        guard.value().foreach_enqueue(members, [](auto& ch) -> async::task<void> {
            ch->group_reset();
            ch->message(_TEXT(MESSAGE_GROUP_DISBANDED), MESSAGE_TYPE::STATE);
            co_return;
        });
    });
    co_return;
}

async::task<void> group_container::on_broadcast(uint32_t group_id, std::string message, uint8_t type)
{
    co_await this->ensure(group_id, [this, message = std::move(message), type](auto& group) -> async::task<void> {
        auto members = std::vector<std::shared_ptr<character>>();
        for (auto& member_ptr : group->characters())
        {
            members.push_back(member_ptr);
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
