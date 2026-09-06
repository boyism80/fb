#include <fb/game/group.h>
#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/game/thread_params.h>
#include <fb/config.h>
#include <fb/amqp_route.h>
#include <fb/encoding.h>
#include <macro.h>
#include <functional>

using namespace fb::game;
namespace internal_reqs = fb::protocol::internal::request;

async::task<void> group::container::create(character& me, std::string_view target_name)
{
    auto weak            = me.weak_from_this_as<character>();
    auto target_name_str = std::string(target_name);

    if (me.option(OPTION::GROUP) == false)
        throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_MINE));

    if (me.group_id().has_value())
        throw std::runtime_error(_TEXT(MESSAGE_ALREADY_JOINED_GROUP));
    if (me.name() == target_name_str)
        throw std::runtime_error(_TEXT(MESSAGE_CANNOT_GROUP_SELF));

    auto target = this->_server.characters.find(target_name_str);
    if (target != nullptr && target->world() != me.world())
        throw std::runtime_error("다른 월드 플레이어와는 그룹할 수 없습니다.");

    auto current_thread = this->_server.threads.current();
    if (current_thread != nullptr)
    {
        auto params = current_thread->template data<thread_params>();
        auto target = params->characters.find(target_name_str);
        if (target != nullptr)
        {
            if (target->group_id().has_value())
                throw std::runtime_error(std::format(_TEXT(MESSAGE_GROUP_ALREADY_JOINED), target_name_str));

            if (target->option(OPTION::GROUP) == false)
                throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_TARGET));
        }
    }

    auto   world = me.world();
    auto&& resp  = co_await this->_server.http.post(
        "internal",
        "/group/create",
        internal_reqs::CreateGroup{world, fb::config<uint32_t>("host"), me.id, target_name_str});
    co_await this->_server.threads.switching(weak);

    auto members = std::map<uint32_t, std::string>{};
    for (const auto& member : resp.members)
    {
        members.emplace(member.uid, member.name);
    }

    co_await this->on_error(resp.error, resp.target);
    co_await this->on_create(resp.target, resp.group.id, resp.group.master, std::move(members));
}

async::task<void> group::container::destroy(character& me)
{
    auto group_id = me.group_id();
    if (group_id.has_value() == false)
        co_return;

    auto   weak  = me.weak_from_this_as<character>();
    auto   world = me.world();
    auto&& resp =
        co_await this->_server.http.post("internal",
                                         "/group/destroy",
                                         internal_reqs::DestroyGroup{world, fb::config<uint32_t>("host"), me.name()});
    co_await this->_server.threads.switching(weak);
    co_await this->on_error(resp.error, resp.actor.name);
    co_await this->on_destroyed(resp.actor.name, resp.group_id);
}

async::task<void> group::container::toggle_member(character& actor, std::string_view target_name)
{
    auto weak            = actor.weak_from_this_as<character>();
    auto target_name_str = std::string(target_name);

    auto target = this->_server.characters.find(target_name_str);
    if (target != nullptr && target->world() != actor.world())
        throw std::runtime_error("다른 월드 플레이어와는 그룹할 수 없습니다.");

    auto current_thread = this->_server.threads.current();
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

    auto   world = actor.world();
    auto&& resp  = co_await this->_server.http.post(
        "internal",
        "/group/toggle",
        internal_reqs::EnterGroup{world, fb::config<uint32_t>("host"), actor.id, target_name_str});
    co_await this->_server.threads.switching(weak);

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

async::task<void> group::container::leave_member(character& leaver)
{
    auto weak = leaver.weak_from_this_as<character>();
    if (leaver.group_id().has_value() == false)
        co_return;

    auto   world = leaver.world();
    auto&& resp =
        co_await this->_server.http.post("internal",
                                         "/group/leave",
                                         internal_reqs::LeaveGroup{world, fb::config<uint32_t>("host"), leaver.name()});
    co_await this->_server.threads.switching(weak);

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

async::task<void>
group::container::broadcast(uint32_t world, uint32_t group_id, std::string_view message, MESSAGE_TYPE type)
{
    auto   message_str = std::string(message);
    auto&& resp        = co_await this->_server.http.post("internal",
                                                   "/group/broadcast",
                                                   internal_reqs::BroadcastGroup{world,
                                                                                 fb::config<uint32_t>("host"),
                                                                                 group_id,
                                                                                 message_str,
                                                                                 static_cast<uint8_t>(type)});
    co_await this->on_error(resp.error, "");
    co_await this->on_broadcast(resp.group, std::move(resp.message), resp.type);
}

async::task<void> group::container::handle_action(character& actor, std::string_view target_name)
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
        bool is_master  = false;
        {
            auto guard = this->try_enter_read(group_id.value());
            if (guard.has_value() == false || guard->value() == nullptr)
                throw std::runtime_error(_TEXT(MESSAGE_GROUP_NOT_JOINED));

            is_master = (guard->value()->master() == actor_name);
        }
        action = [this, is_master](character& actor, const std::string&) -> async::task<void> {
            if (is_master)
                co_await this->destroy(actor);
            else
                co_await this->leave_member(actor);
        };
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
            bool is_master            = false;
            {
                auto guard = this->try_enter_read(actor_group_id_value);
                if (guard.has_value() == false || guard->value() == nullptr)
                    throw std::runtime_error(_TEXT(MESSAGE_GROUP_NOT_JOINED));

                is_master = (guard->value()->master() == actor_name);
            }

            action = [this, is_master, target_name_str, weak](character& actor,
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
        }
    }

    co_await this->_server.threads.switching(weak);
    if (action)
    {
        auto error = std::optional<std::string>{};
        try
        {
            co_await action(actor, target_name_str);
        }
        catch (std::exception& e)
        {
            error = e.what();
        }

        if (error.has_value() && weak.expired() == false)
            actor.message(error.value(), MESSAGE_TYPE::STATE);
    }
}
