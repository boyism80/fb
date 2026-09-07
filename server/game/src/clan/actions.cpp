#include <fb/game/clan.h>
#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/game/thread_params.h>
#include <fb/config.h>
#include <fb/amqp_route.h>
#include <fb/model/model.h>
#include <macro.h>

using namespace fb::game;
namespace internal_reqs = fb::protocol::internal::request;

async::task<void> clan::container::create(character& me, std::string_view name)
{
    if (me.clan_id().has_value())
        throw std::runtime_error(_TEXT(MESSAGE_ALREADY_JOINED_CLAN));

    auto   weak     = me.weak_from_this_as<character>();
    auto   name_str = std::string(name);
    auto   world    = me.world();
    auto&& resp =
        co_await this->_server.http.post("internal",
                                         "/clan/create",
                                         internal_reqs::CreateClan{world, fb::config<uint32_t>("id"), me.id, name_str});
    co_await this->_server.threads.switching(weak);

    co_await this->on_error(resp.error);

    auto members = std::unordered_map<std::string, clan_member>{};
    for (const auto& member : resp.members)
    {
        members.insert({
            member.name,
            clan_member{member.name, static_cast<CLAN_ROLE>(member.role)}
        });
    }

    auto title = resp.clan.title.has_value() ? resp.clan.title.value() : std::string{};
    co_await this->on_create(resp.clan.id, resp.clan.name, title, std::move(members));
}

async::task<void> clan::container::destroy(character& me)
{
    auto clan_id = me.clan_id();
    if (clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    {
        auto guard = this->try_enter_read(clan_id.value());
        if (guard.has_value() == false || guard->value() == nullptr)
            throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

        auto& clan = guard->value();
        if (clan->members().size() != 1)
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_MEMBER_EXISTS));

        auto member = clan->member(me.name());
        if (member == nullptr)
            throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

        if (member->role != CLAN_ROLE::MASTER)
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_NO_PRIVILEGE));

        if (member->name != me.name())
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_NO_PRIVILEGE));
    }

    auto   weak  = me.weak_from_this_as<character>();
    auto   world = me.world();
    auto&& resp =
        co_await this->_server.http.post("internal",
                                         "/clan/destroy",
                                         internal_reqs::DestroyClan{world, fb::config<uint32_t>("id"), me.id});
    co_await this->_server.threads.switching(weak);

    co_await this->on_error(resp.error);
    co_await this->on_destroyed(resp.clan_id, resp.clan_name);
    me.clan_reset();
}

async::task<void> clan::container::join_member(character& inviter, std::string_view target_name)
{
    auto inviter_clan_id = inviter.clan_id();
    if (inviter_clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    auto weak            = inviter.weak_from_this_as<character>();
    auto target_name_str = std::string(target_name);

    auto target = this->_server.characters.find(target_name_str);
    if (target != nullptr && target->world() != inviter.world())
        throw std::runtime_error("다른 월드 플레이어를 문파에 초대할 수 없습니다.");

    auto current_thread = this->_server.threads.current();
    if (current_thread != nullptr)
    {
        auto params = current_thread->template data<thread_params>();
        auto target = params->characters.find(target_name_str);
        if (target != nullptr)
        {
            if (target->clan_id().has_value())
                throw std::runtime_error(_TEXT(MESSAGE_ALREADY_JOINED_CLAN));
        }
    }

    auto   world = inviter.world();
    auto&& resp  = co_await this->_server.http.post(
        "internal",
        "/clan/join",
        internal_reqs::JoinClan{world, fb::config<uint32_t>("host"), inviter.id, target_name_str});
    co_await this->_server.threads.switching(weak);
    co_await this->on_updated(resp);
}

async::task<void> clan::container::leave_member(character& leaver)
{
    auto clan_id = leaver.clan_id();
    if (clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    {
        auto guard = this->try_enter_read(clan_id.value());
        if (guard.has_value() == false || guard->value() == nullptr)
            throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

        auto& clan   = guard->value();
        auto  member = clan->member(leaver.name());
        if (member != nullptr)
        {
            if (member->role == CLAN_ROLE::MASTER)
                throw std::runtime_error(_TEXT(MESSAGE_CLAN_CANNOT_LEAVE_MASTER));
        }
    }

    auto   weak  = leaver.weak_from_this_as<character>();
    auto   world = leaver.world();
    auto&& resp  = co_await this->_server.http.post(
        "internal",
        "/clan/leave",
        internal_reqs::LeaveClan{world, fb::config<uint32_t>("host"), clan_id.value(), leaver.name()});
    co_await this->_server.threads.switching(weak);
    co_await this->on_updated(resp);
}

async::task<void> clan::container::kick_member(character& kicker, std::string_view target_name)
{
    auto kicker_clan_id = kicker.clan_id();
    if (kicker_clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    auto weak            = kicker.weak_from_this_as<character>();
    auto target_name_str = std::string(target_name);

    auto target = this->_server.characters.find(target_name_str);
    if (target != nullptr && target->world() != kicker.world())
        throw std::runtime_error("다른 월드 플레이어를 문파에서 추방할 수 없습니다.");

    auto current_thread = this->_server.threads.current();
    if (current_thread != nullptr)
    {
        auto params = current_thread->template data<thread_params>();
        auto target = params->characters.find(target_name_str);
        if (target != nullptr)
        {
            auto target_clan_id = target->clan_id();
            if (target_clan_id.has_value() == false || target_clan_id.value() != kicker_clan_id.value())
                throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));
        }
    }

    auto   world = kicker.world();
    auto&& resp  = co_await this->_server.http.post("internal",
                                                   "/clan/kick",
                                                   internal_reqs::KickClan{world,
                                                                           fb::config<uint32_t>("host"),
                                                                           kicker_clan_id.value(),
                                                                           kicker.name(),
                                                                           target_name_str});
    co_await this->_server.threads.switching(weak);
    co_await this->on_updated(resp);
}

async::task<void> clan::container::change_role(character& changer, std::string_view target_name, CLAN_ROLE role)
{
    auto changer_clan_id = changer.clan_id();
    if (changer_clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    auto weak            = changer.weak_from_this_as<character>();
    auto target_name_str = std::string(target_name);

    auto target = this->_server.characters.find(target_name_str);
    if (target != nullptr && target->world() != changer.world())
        throw std::runtime_error("다른 월드 플레이어의 문파 직책을 변경할 수 없습니다.");

    auto current_thread = this->_server.threads.current();
    if (current_thread != nullptr)
    {
        auto params = current_thread->template data<thread_params>();
        auto target = params->characters.find(target_name_str);
        if (target != nullptr)
        {
            auto target_clan_id = target->clan_id();
            if (target_clan_id.has_value() == false || target_clan_id.value() != changer_clan_id.value())
                throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));
        }
    }

    auto   world = changer.world();
    auto&& resp  = co_await this->_server.http.post("internal",
                                                   "/clan/change-role",
                                                   internal_reqs::ChangeClanRole{world,
                                                                                 fb::config<uint32_t>("host"),
                                                                                 changer.id,
                                                                                 target_name_str,
                                                                                 changer_clan_id.value(),
                                                                                 static_cast<uint32_t>(role)});
    co_await this->_server.threads.switching(weak);
    co_await this->on_updated(resp);
}

async::task<void> clan::container::set_title(character& changer, std::string_view title)
{
    auto changer_clan_id = changer.clan_id();
    if (changer_clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    auto weak      = changer.weak_from_this_as<character>();
    auto title_str = std::string(title);

    {
        auto guard = this->try_enter_read(changer_clan_id.value());
        if (guard.has_value() == false || guard->value() == nullptr)
            throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

        auto& clan   = guard->value();
        auto  member = clan->member(changer.name());
        if (member != nullptr)
        {
            if (static_cast<uint32_t>(member->role) <
                static_cast<uint32_t>(fb::model::const_value::clan::MINIMUM_CHANGE_TITLE_PRIVILEGE))
                throw std::runtime_error(_TEXT(MESSAGE_CLAN_NO_PRIVILEGE));
        }

        auto current_title = clan->title();
        if (current_title.has_value() && current_title.value() == title_str)
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_TITLE_NOT_CHANGED));

        if (title_str.empty() == false && title_str.length() < 2)
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_TITLE_TOO_SHORT));
    }

    auto   world = changer.world();
    auto&& resp  = co_await this->_server.http.post(
        "internal",
        "/clan/title",
        internal_reqs::SetClanTitle{world, fb::config<uint32_t>("host"), changer.id, title_str});
    co_await this->_server.threads.switching(weak);
    co_await this->on_updated(resp);
}

async::task<void> clan::container::request_ally(character& requester, uint32_t target_clan_id)
{
    auto clan_id = requester.clan_id();
    if (clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    if (clan_id.value() == target_clan_id)
        throw std::runtime_error(_TEXT(MESSAGE_CLAN_CANNOT_ALLY_SELF));

    {
        auto target_guard = this->try_enter_read(target_clan_id);
        if (target_guard.has_value() && target_guard->value() != nullptr)
        {
            for (auto& [name, _] : target_guard->value()->members())
            {
                auto ch = this->_server.characters.find(name);
                if (ch != nullptr && ch->world() != requester.world())
                    throw std::runtime_error("다른 월드 문파와는 동맹할 수 없습니다.");
            }
        }
    }

    {
        auto guard = this->try_enter_read(clan_id.value());
        if (guard.has_value() == false || guard->value() == nullptr)
            throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

        auto& clan   = guard->value();
        auto  member = clan->member(requester.name());
        if (member == nullptr || member->role != CLAN_ROLE::MASTER)
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_NO_PRIVILEGE));

        if (clan->allied_clan_id().has_value())
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_ALREADY_ALLIED));

        if (clan->is_hostile(target_clan_id))
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_CANNOT_ALLY_ENEMY));
    }

    auto   weak  = requester.weak_from_this_as<character>();
    auto   world = requester.world();
    auto&& resp  = co_await this->_server.http.post(
        "internal",
        "/clan/ally",
        internal_reqs::AllyClan{world, fb::config<uint32_t>("host"), requester.id, clan_id.value(), target_clan_id});
    co_await this->_server.threads.switching(weak);
    co_await this->on_updated(resp);
}

async::task<void> clan::container::break_ally(character& requester)
{
    auto clan_id = requester.clan_id();
    if (clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    {
        auto guard = this->try_enter_read(clan_id.value());
        if (guard.has_value() == false || guard->value() == nullptr)
            throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

        auto& clan   = guard->value();
        auto  member = clan->member(requester.name());
        if (member == nullptr || member->role != CLAN_ROLE::MASTER)
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_NO_PRIVILEGE));

        if (clan->allied_clan_id().has_value() == false)
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_NOT_ALLIED));
    }

    auto   weak  = requester.weak_from_this_as<character>();
    auto   world = requester.world();
    auto&& resp  = co_await this->_server.http.post(
        "internal",
        "/clan/unally",
        internal_reqs::UnallyClan{world, fb::config<uint32_t>("host"), requester.id, clan_id.value()});
    co_await this->_server.threads.switching(weak);
    co_await this->on_updated(resp);
}

async::task<void> clan::container::declare_enemy(character& requester, uint32_t target_clan_id)
{
    auto clan_id = requester.clan_id();
    if (clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    if (clan_id.value() == target_clan_id)
        throw std::runtime_error(_TEXT(MESSAGE_CLAN_CANNOT_ENEMY_SELF));

    {
        auto target_guard = this->try_enter_read(target_clan_id);
        if (target_guard.has_value() && target_guard->value() != nullptr)
        {
            for (auto& [name, _] : target_guard->value()->members())
            {
                auto ch = this->_server.characters.find(name);
                if (ch != nullptr && ch->world() != requester.world())
                    throw std::runtime_error("다른 월드 문파와는 적대할 수 없습니다.");
            }
        }
    }

    {
        auto guard = this->try_enter_read(clan_id.value());
        if (guard.has_value() == false || guard->value() == nullptr)
            throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

        auto& clan   = guard->value();
        auto  member = clan->member(requester.name());
        if (member == nullptr || member->role != CLAN_ROLE::MASTER)
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_NO_PRIVILEGE));

        if (clan->is_allied(target_clan_id))
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_CANNOT_ENEMY_ALLY));

        if (clan->is_hostile(target_clan_id))
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_ALREADY_ENEMY));
    }

    auto   weak  = requester.weak_from_this_as<character>();
    auto   world = requester.world();
    auto&& resp  = co_await this->_server.http.post("internal",
                                                   "/clan/enemy",
                                                   internal_reqs::DeclareClanEnemy{world,
                                                                                   fb::config<uint32_t>("host"),
                                                                                   requester.id,
                                                                                   clan_id.value(),
                                                                                   target_clan_id});
    co_await this->_server.threads.switching(weak);
    co_await this->on_updated(resp);
}

async::task<void> clan::container::end_enemy(character& requester, uint32_t target_clan_id)
{
    auto clan_id = requester.clan_id();
    if (clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    {
        auto guard = this->try_enter_read(clan_id.value());
        if (guard.has_value() == false || guard->value() == nullptr)
            throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

        auto& clan   = guard->value();
        auto  member = clan->member(requester.name());
        if (member == nullptr || member->role != CLAN_ROLE::MASTER)
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_NO_PRIVILEGE));

        if (clan->is_hostile(target_clan_id) == false)
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_NOT_ENEMY));
    }

    auto   weak  = requester.weak_from_this_as<character>();
    auto   world = requester.world();
    auto&& resp  = co_await this->_server.http.post("internal",
                                                   "/clan/unenemy",
                                                   internal_reqs::EndClanEnemy{world,
                                                                               fb::config<uint32_t>("host"),
                                                                               requester.id,
                                                                               clan_id.value(),
                                                                               target_clan_id});
    co_await this->_server.threads.switching(weak);
    co_await this->on_updated(resp);
}

async::task<void>
clan::container::broadcast(uint32_t world, uint32_t clan_id, std::string_view message, MESSAGE_TYPE type)
{
    auto   message_str = std::string(message);
    auto&& resp        = co_await this->_server.http.post("internal",
                                                   "/clan/broadcast",
                                                   internal_reqs::BroadcastClan{world,
                                                                                fb::config<uint32_t>("host"),
                                                                                clan_id,
                                                                                message_str,
                                                                                static_cast<uint8_t>(type)});
    co_await this->on_error(resp.error);
    co_await this->on_broadcast(resp.clan, std::move(resp.message), resp.type);
}

async::task<void> clan::container::add_money(uint32_t world, uint32_t clan_id, int64_t delta)
{
    if (delta == 0)
        co_return;

    auto*  before = this->_server.threads.current();
    auto&& resp   = co_await this->_server.http.post(
        "internal",
        "/clan/money",
        internal_reqs::SetClanMoney{world, fb::config<uint32_t>("id"), clan_id, delta});
    if (before != nullptr)
        co_await before->switching();
    co_await this->on_updated(resp);
}
