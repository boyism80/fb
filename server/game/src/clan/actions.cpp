#include <fb/amqp_route.h>
#include <fb/config.h>
#include <fb/encoding.h>
#include <fb/game/character.h>
#include <fb/game/clan.h>
#include <fb/game/server.h>
#include <fb/game/thread_params.h>
#include <fb/logger.h>
#include <fb/model/model.h>
#include <macro.h>

#include <cstdint>
#include <format>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

using namespace fb::game;
namespace internal_reqs = fb::protocol::internal::request;
namespace internal_resp = fb::protocol::internal::response;

namespace {

constexpr size_t CLAN_NAME_MIN  = 2;
constexpr size_t CLAN_NAME_MAX  = 12;
constexpr size_t CLAN_TITLE_MAX = 45;

} // namespace

async::task<void> clan::container::create(character& me, std::string_view name)
{
    if (me.clan.id().has_value())
        throw std::runtime_error(_TEXT(MESSAGE_ALREADY_JOINED_CLAN));

    auto name_str = std::string(name);
    auto cp949    = fb::CP949(name_str);
    auto valid    = cp949.length() >= CLAN_NAME_MIN && cp949.length() <= CLAN_NAME_MAX && fb::assert_korean(cp949) &&
                 fb::model::table::blocked_name->contains_substring(name_str) == false &&
                 fb::model::table::blocked_word->contains_substring(name_str) == false;
    if (valid == false)
        throw std::runtime_error(_TEXT(MESSAGE_CLAN_NAME_INVALID));

    auto   weak  = me.weak_from_this_as<character>();
    auto   world = me.world();
    auto&& resp =
        co_await this->_server.http.post(weak,
                                         "internal",
                                         "/clan/create",
                                         internal_reqs::CreateClan{world, fb::config<uint32_t>("id"), me.id, name_str});

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
    co_await this->on_create(resp.clan.id, resp.clan.name, title, std::move(members), resp.revision);
}

async::task<void> clan::container::destroy(character& me)
{
    auto clan_id = me.clan.id();
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
        co_await this->_server.http.post(weak,
                                         "internal",
                                         "/clan/destroy",
                                         internal_reqs::DestroyClan{world, fb::config<uint32_t>("id"), me.id});

    co_await this->on_error(resp.error);
    co_await this->on_destroyed(resp.clan_id, resp.clan_name, resp.revision);
    me.clan.reset();
}

async::task<void> clan::container::join_member(character& inviter, std::string_view target_name)
{
    auto inviter_clan_id = inviter.clan.id();
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
            if (target->clan.id().has_value())
                throw std::runtime_error(_TEXT(MESSAGE_ALREADY_JOINED_CLAN));
        }
    }

    auto   world = inviter.world();
    auto&& resp  = co_await this->_server.http.post(
        weak,
        "internal",
        "/clan/join",
        internal_reqs::JoinClan{world, fb::config<uint32_t>("id"), inviter.id, target_name_str});
    co_await this->on_updated(resp);
}

async::task<void> clan::container::leave_member(character& leaver)
{
    auto clan_id = leaver.clan.id();
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
        weak,
        "internal",
        "/clan/leave",
        internal_reqs::LeaveClan{world, fb::config<uint32_t>("id"), clan_id.value(), leaver.name()});
    co_await this->on_updated(resp);
}

async::task<void> clan::container::kick_member(character& kicker, std::string_view target_name)
{
    auto kicker_clan_id = kicker.clan.id();
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
            auto target_clan_id = target->clan.id();
            if (target_clan_id.has_value() == false || target_clan_id.value() != kicker_clan_id.value())
                throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));
        }
    }

    auto   world = kicker.world();
    auto&& resp  = co_await this->_server.http.post(weak,
                                                   "internal",
                                                   "/clan/kick",
                                                   internal_reqs::KickClan{world,
                                                                           fb::config<uint32_t>("id"),
                                                                           kicker_clan_id.value(),
                                                                           kicker.name(),
                                                                           target_name_str});
    co_await this->on_updated(resp);
}

async::task<void> clan::container::change_role(character& changer, std::string_view target_name, CLAN_ROLE role)
{
    auto changer_clan_id = changer.clan.id();
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
            auto target_clan_id = target->clan.id();
            if (target_clan_id.has_value() == false || target_clan_id.value() != changer_clan_id.value())
                throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));
        }
    }

    auto   world = changer.world();
    auto&& resp  = co_await this->_server.http.post(weak,
                                                   "internal",
                                                   "/clan/change-role",
                                                   internal_reqs::ChangeClanRole{world,
                                                                                 fb::config<uint32_t>("id"),
                                                                                 changer.id,
                                                                                 target_name_str,
                                                                                 changer_clan_id.value(),
                                                                                 static_cast<uint32_t>(role)});
    co_await this->on_updated(resp);
}

async::task<void> clan::container::set_title(character& changer, std::string_view title)
{
    auto changer_clan_id = changer.clan.id();
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

        if (fb::CP949(title_str).length() > CLAN_TITLE_MAX ||
            fb::model::table::blocked_word->contains_substring(title_str))
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_TITLE_INVALID));
    }

    auto   world = changer.world();
    auto&& resp  = co_await this->_server.http.post(
        weak,
        "internal",
        "/clan/title",
        internal_reqs::SetClanTitle{world, fb::config<uint32_t>("id"), changer.id, title_str});
    co_await this->on_updated(resp);
}

async::task<void> clan::container::request_ally(character& requester, uint32_t target_clan_id)
{
    auto clan_id = requester.clan.id();
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

            if (target_guard->value()->allied_clan_id().has_value())
                throw std::runtime_error(_TEXT(MESSAGE_CLAN_TARGET_ALREADY_ALLIED));
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
        weak,
        "internal",
        "/clan/ally",
        internal_reqs::AllyClan{world, fb::config<uint32_t>("id"), requester.id, clan_id.value(), target_clan_id});
    co_await this->on_updated(resp);
}

async::task<void> clan::container::break_ally(character& requester, uint32_t target_clan_id)
{
    auto clan_id = requester.clan.id();
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

        if (clan->allied_clan_id() != target_clan_id)
            throw std::runtime_error(_TEXT(MESSAGE_CLAN_NOT_ALLIED));
    }

    auto   weak  = requester.weak_from_this_as<character>();
    auto   world = requester.world();
    auto&& resp  = co_await this->_server.http.post(
        weak,
        "internal",
        "/clan/unally",
        internal_reqs::UnallyClan{world, fb::config<uint32_t>("id"), requester.id, clan_id.value(), target_clan_id});
    co_await this->on_updated(resp);
}

async::task<void> clan::container::declare_enemy(character& requester, uint32_t target_clan_id)
{
    auto clan_id = requester.clan.id();
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
    auto&& resp  = co_await this->_server.http.post(weak,
                                                   "internal",
                                                   "/clan/enemy",
                                                   internal_reqs::DeclareClanEnemy{world,
                                                                                   fb::config<uint32_t>("id"),
                                                                                   requester.id,
                                                                                   clan_id.value(),
                                                                                   target_clan_id});
    co_await this->on_updated(resp);
}

async::task<void> clan::container::end_enemy(character& requester, uint32_t target_clan_id)
{
    auto clan_id = requester.clan.id();
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
    auto&& resp  = co_await this->_server.http.post(
        weak,
        "internal",
        "/clan/unenemy",
        internal_reqs::EndClanEnemy{world, fb::config<uint32_t>("id"), requester.id, clan_id.value(), target_clan_id});
    co_await this->on_updated(resp);
}

async::task<void>
clan::container::broadcast(uint32_t world, uint32_t clan_id, std::string_view message, MESSAGE_TYPE type)
{
    auto   message_str = std::string(message);
    auto&& resp        = co_await this->_server.http.post("internal",
                                                   "/clan/broadcast",
                                                   internal_reqs::BroadcastClan{world,
                                                                                fb::config<uint32_t>("id"),
                                                                                clan_id,
                                                                                message_str,
                                                                                static_cast<uint8_t>(type)});
    co_await this->on_error(resp.error);
    co_await this->on_broadcast(resp.clan, std::move(resp.message), resp.type);
}

async::task<std::optional<bool>>
clan::container::add_money(uint32_t world, uint32_t clan_id, int64_t delta, uint64_t request_id)
{
    auto sent = co_await this->send_money(world, clan_id, delta, request_id);
    if (sent.has_value() && sent.value())
        co_return true;
    else
        co_return co_await this->abort_money(world, clan_id, request_id);
}

async::task<std::optional<bool>>
clan::container::send_money(uint32_t world, uint32_t clan_id, int64_t delta, uint64_t request_id)
{
    if (delta == 0)
        co_return true;

    auto* before = this->_server.threads.current();
    auto  host   = fb::config<uint32_t>("id");

    auto updated = std::optional<internal_resp::UpdatedClan>{};
    auto missing = false;
    try
    {
        auto&& resp =
            co_await this->_server.http.post("internal",
                                             "/clan/money",
                                             internal_reqs::SetClanMoney{world, host, clan_id, delta, request_id});
        auto error = static_cast<ERROR_CODE>(resp.error);
        if (error == ERROR_CODE::NONE)
            updated = std::move(resp);
        else if (error == ERROR_CODE::NOT_FOUND_CLAN)
            missing = true;
    }
    catch (std::exception&)
    { }

    if (before != nullptr)
        co_await before->switching();

    if (updated.has_value())
    {
        co_await this->on_updated(updated.value());
        co_return true;
    }
    else if (missing)
    {
        co_return false;
    }
    else
    {
        co_return std::nullopt;
    }
}

async::task<std::optional<bool>> clan::container::abort_money(uint32_t world, uint32_t clan_id, uint64_t request_id)
{
    auto* before = this->_server.threads.current();

    auto aborted = std::optional<internal_resp::AbortClanMoney>{};
    try
    {
        auto&& resp = co_await this->_server.http.post("internal",
                                                       "/clan/money/abort",
                                                       internal_reqs::AbortClanMoney{world, clan_id, request_id});
        if (static_cast<ERROR_CODE>(resp.error) == ERROR_CODE::NONE)
            aborted = std::move(resp);
    }
    catch (std::exception&)
    { }

    if (before != nullptr)
        co_await before->switching();

    if (aborted.has_value() == false)
    {
        co_return std::nullopt;
    }
    else if (aborted.value().applied)
    {
        co_await this->on_set_money(clan_id, aborted.value().money, aborted.value().revision);
        co_return true;
    }
    else
    {
        co_return false;
    }
}
