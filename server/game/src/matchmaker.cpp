#include <fb/game/matchmaker.h>

#include <fb/config.h>
#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/logger.h>
#include <fb/model/model.h>
#include <macro.h>

#include <cstdint>
#include <exception>
#include <format>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace fb::game;
using table = fb::model::table;

namespace mp      = fb::protocol::matchmaking;
namespace mp_reqs = mp::request;
namespace mp_resp = mp::response;

matchmaker::matchmaker(character& owner) :
    owner(owner)
{
    this->load({});
}

matchmaking_skill::dto_type matchmaking_skill::to_protocol(uint32_t user) const
{
    return dto_type{user, match_type, mu, sigma};
}

void matchmaker::load(const std::vector<matchmaking_skill::dto_type>& skills)
{
    this->_skills.clear();
    for (auto& skill : skills)
    {
        this->_skills.emplace(skill.match_type,
                              matchmaking_skill{
                                  .match_type = skill.match_type,
                                  .mu         = skill.mu,
                                  .sigma      = skill.sigma,
                              });
    }

    for (auto& [type, row] : table::matchmaking)
    {
        std::ignore = row;
        auto id     = static_cast<uint32_t>(type);
        if (this->_skills.contains(id) == false)
        {
            this->upsert(id, DEFAULT_MU, DEFAULT_SIGMA);
        }
    }
}

std::vector<matchmaking_skill::dto_type> matchmaker::to_protocol() const
{
    auto result = std::vector<matchmaking_skill::dto_type>{};
    result.reserve(this->_skills.size());
    for (auto& [match_type, skill] : this->_skills)
    {
        result.emplace_back(skill.to_protocol(this->owner.id));
    }

    return result;
}

std::optional<matchmaking_skill> matchmaker::get(uint32_t match_type) const
{
    auto i = this->_skills.find(match_type);
    if (i == this->_skills.end())
        return std::nullopt;

    return i->second;
}

void matchmaker::upsert(uint32_t match_type, double mu, double sigma)
{
    this->_skills.insert_or_assign(match_type,
                                   matchmaking_skill{
                                       .match_type = match_type,
                                       .mu         = mu,
                                       .sigma      = sigma,
                                   });
}

const std::optional<uint64_t>& matchmaker::pending_match_id() const
{
    return this->_pending_match_id;
}

void matchmaker::set_pending_match_id(uint64_t match_id)
{
    this->_pending_match_id = match_id;
    this->touch();
}

void matchmaker::touch()
{
    this->_last_activity = std::chrono::steady_clock::now();
}

void matchmaker::clear_pending_match_id()
{
    this->_pending_match_id = std::nullopt;
}

bool matchmaker::clear_pending_match_id_if(uint64_t match_id)
{
    if (this->_pending_match_id != match_id)
        return false;

    this->_pending_match_id = std::nullopt;
    return true;
}

bool matchmaker::mark_ready(uint64_t match_id)
{
    if (this->_ready_match_id == match_id)
        return false;

    this->_ready_match_id = match_id;
    return true;
}

bool matchmaker::on_ready(uint64_t match_id)
{
    if (this->mark_ready(match_id) == false)
        return false;

    this->clear_pending_match_id_if(match_id);
    this->clear_ticket();
    return true;
}

bool matchmaker::on_dissolved(uint64_t match_id, bool ticket_closed)
{
    auto had_ticket = this->ticket_id().has_value();
    this->clear_pending_match_id_if(match_id);
    if (ticket_closed)
        this->clear_ticket();
    return had_ticket;
}

bool matchmaker::on_ticket_removed(uint64_t ticket_id)
{
    if (this->ticket_id() != ticket_id)
        return false;

    this->clear_pending_match_id();
    this->clear_ticket();
    return true;
}

bool matchmaker::queued() const
{
    this->owner.assert_thread();

    return this->_ticket.has_value() || this->_enqueuing;
}

bool matchmaker::enqueuing() const
{
    this->owner.assert_thread();

    return this->_enqueuing;
}

std::optional<uint64_t> matchmaker::ticket_id() const
{
    this->owner.assert_thread();

    if (!this->_ticket.has_value())
        return std::nullopt;

    return this->_ticket->ticket_id;
}

void matchmaker::begin_enqueue()
{
    this->owner.assert_thread();

    this->_enqueuing = true;
    this->touch();
}

void matchmaker::set_ticket(uint32_t match_type, uint64_t ticket_id)
{
    this->owner.assert_thread();

    this->_enqueuing = false;
    this->_ticket    = ticket_state{
           .match_type = match_type,
           .ticket_id  = ticket_id,
    };
    this->touch();
}

void matchmaker::clear_ticket()
{
    this->owner.assert_thread();

    this->_enqueuing = false;
    this->_ticket    = std::nullopt;
}

async::task<void> matchmaker::discard_leftover_ticket()
{
    auto& server       = this->owner.server;
    auto  world        = this->owner.world();
    auto  character_id = this->owner.id;

    try
    {
        auto&& status =
            co_await server.http.post("matchmaking", "/matchmaking/status", mp_reqs::Status{world, character_id});

        if (status.error != 0 || status.ticket_id == 0)
            co_return;

        auto&& resp =
            co_await server.http.post("matchmaking",
                                      "/matchmaking/dequeue",
                                      mp_reqs::Dequeue{status.match_type, status.ticket_id, world, character_id});

        fb::logger::warn("matchmaking dropped leftover ticket {} of character {} at login (success: {})",
                         status.ticket_id,
                         character_id,
                         resp.success);
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("matchmaking leftover check failed for character {}: {}", character_id, e.what());
    }
}

bool matchmaker::reconcile_due(std::chrono::steady_clock::time_point now) const
{
    this->owner.assert_thread();

    if (this->_reconciling || this->_enqueuing)
        return false;

    if (this->_ticket.has_value() == false && this->_pending_match_id.has_value() == false)
        return false;

    return now - this->_last_activity >= RECONCILE_AFTER;
}

async::task<void> matchmaker::reconcile()
{
    this->owner.assert_thread();

    if (this->_reconciling || this->_enqueuing)
        co_return;

    auto ticket  = this->_ticket;
    auto pending = this->_pending_match_id;
    if (ticket.has_value() == false && pending.has_value() == false)
        co_return;

    auto& server       = this->owner.server;
    auto  world        = this->owner.world();
    auto  character_id = this->owner.id;
    auto  weak         = this->owner.weak_from_this_as<character>();
    this->_reconciling = true;

    auto known = std::optional<bool>{};
    try
    {
        auto&& status =
            co_await server.http.post(weak, "matchmaking", "/matchmaking/status", mp_reqs::Status{world, character_id});
        if (status.error == 0)
        {
            if (ticket.has_value())
                known = status.ticket_id == ticket->ticket_id;
            else
                known = status.pending_match_id == pending.value();
        }
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("matchmaking reconcile failed for character {}: {}", character_id, e.what());
    }
    catch (...)
    {
        fb::logger::warn("matchmaking reconcile failed for character {}", character_id);
    }

    co_await server.threads.follow(weak);
    auto ptr = weak.lock();
    if (ptr == nullptr)
        co_return;

    this->_reconciling = false;
    this->touch();

    if (known.value_or(true))
        co_return;

    auto ticket_now = this->_ticket.has_value() ? std::optional<uint64_t>(this->_ticket->ticket_id) : std::nullopt;
    auto ticket_was = ticket.has_value() ? std::optional<uint64_t>(ticket->ticket_id) : std::nullopt;
    if (ticket_now != ticket_was || this->_pending_match_id != pending || this->_enqueuing)
        co_return;

    fb::logger::warn("matchmaking dropped stale local state of character {} (ticket: {}, pending match: {})",
                     character_id,
                     ticket_was.value_or(0),
                     pending.value_or(0));

    this->clear_pending_match_id();
    this->clear_ticket();
    if (ticket.has_value() == false)
        co_return;

    auto lua = server.lua.open("scripts/interaction.lua", "on_matchmaking_dequeue");
    if (lua)
    {
        lua->pushobject(ptr);
        lua->pushinteger(ticket->match_type);
        lua->pushinteger(static_cast<lua_Integer>(ticket->ticket_id));
        std::ignore = co_await fb::lua::call(std::move(lua), weak, 3);
    }
}

async::task<void> matchmaker::dequeue(initiator by)
{
    this->owner.assert_thread();

    if (this->_ticket.has_value() == false)
    {
        // An enqueue still waiting for its response sees the cleared flag and cancels the ticket itself.
        this->clear_ticket();
        co_return;
    }

    auto ticket       = this->_ticket.value();
    auto world        = this->owner.world();
    auto character_id = this->owner.id;
    auto weak         = this->owner.weak_from_this_as<character>();
    this->clear_ticket();

    auto error = std::optional<std::string>{};
    try
    {
        auto&& resp = co_await this->owner.server.http.post(
            weak,
            "matchmaking",
            "/matchmaking/dequeue",
            mp_reqs::Dequeue{ticket.match_type, ticket.ticket_id, world, character_id});

        auto code = static_cast<fb::model::enum_value::ERROR_CODE>(resp.error);
        // The service already dropped the ticket (leftover cleanup or a teammate's dequeue), which is the goal.
        if (code != fb::model::enum_value::ERROR_CODE::NONE &&
            code != fb::model::enum_value::ERROR_CODE::MATCHMAKING_REGISTRY_NOT_FOUND)
            error = enum_tostring(code);
    }
    catch (const std::exception& e)
    {
        error = e.what();
    }

    if (weak.lock() == nullptr)
        co_return;

    if (error.has_value())
    {
        if (by == initiator::USER)
        {
            this->set_ticket(ticket.match_type, ticket.ticket_id);
            throw std::runtime_error(error.value());
        }
        else
        {
            fb::logger::warn("matchmaking dequeue failed for character {}: {}", character_id, error.value());
        }
    }

    auto ptr = weak.lock();
    if (ptr == nullptr)
        co_return;

    auto lua = this->owner.server.lua.open("scripts/interaction.lua", "on_matchmaking_dequeue");
    if (lua)
    {
        lua->pushobject(ptr);
        lua->pushinteger(ticket.match_type);
        lua->pushinteger(static_cast<lua_Integer>(ticket.ticket_id));
        std::ignore = co_await fb::lua::call(std::move(lua), weak, 3);
    }
}

async::task<void> matchmaker::confirm(uint64_t match_id)
{
    this->owner.assert_thread();
    this->touch();

    auto world        = this->owner.world();
    auto character_id = this->owner.id;
    auto weak         = this->owner.weak_from_this_as<character>();
    // A MatchReady published before the response clears the ticket, so the type is read first.
    auto match_type = this->_ticket.has_value() ? this->_ticket->match_type : 0;

    auto error = std::optional<std::string>{};
    try
    {
        auto&& resp = co_await this->owner.server.http.post(weak,
                                                            "matchmaking",
                                                            "/matchmaking/confirm",
                                                            mp_reqs::Confirm{match_id, world, character_id});
        if (resp.error != 0)
            error = enum_tostring(static_cast<fb::model::enum_value::ERROR_CODE>(resp.error));
    }
    catch (const std::exception& e)
    {
        error = e.what();
    }

    if (weak.lock() == nullptr)
        co_return;

    if (error.has_value())
        throw std::runtime_error(error.value());

    auto ptr = weak.lock();
    if (ptr == nullptr)
        co_return;

    auto lua = this->owner.server.lua.open("scripts/interaction.lua", "on_matchmaking_confirm");
    if (lua)
    {
        lua->pushobject(ptr);
        lua->pushinteger(static_cast<lua_Integer>(match_id));
        lua->pushinteger(match_type);
        std::ignore = co_await fb::lua::call(std::move(lua), weak, 3);
    }
}

async::task<void> matchmaker::decline(uint64_t match_id, initiator by)
{
    this->owner.assert_thread();

    auto world        = this->owner.world();
    auto character_id = this->owner.id;
    auto weak         = this->owner.weak_from_this_as<character>();

    // A MatchDissolved published before the response must already find the ticket gone, or it notifies again.
    auto ticket     = this->_ticket;
    auto match_type = ticket.has_value() ? ticket->match_type : 0;
    auto pending    = this->clear_pending_match_id_if(match_id);
    this->clear_ticket();

    auto error = std::optional<std::string>{};
    try
    {
        auto&& resp = co_await this->owner.server.http.post(weak,
                                                            "matchmaking",
                                                            "/matchmaking/decline",
                                                            mp_reqs::Decline{match_id, world, character_id});
        if (resp.error != 0)
            error = enum_tostring(static_cast<fb::model::enum_value::ERROR_CODE>(resp.error));
    }
    catch (const std::exception& e)
    {
        error = e.what();
    }

    if (weak.lock() == nullptr)
        co_return;

    if (error.has_value())
    {
        if (by == initiator::USER)
        {
            // The user may retry, so the ticket comes back unless another one arrived meanwhile.
            if (ticket.has_value() && this->_ticket.has_value() == false)
                this->set_ticket(ticket->match_type, ticket->ticket_id);
            if (pending && this->_pending_match_id.has_value() == false)
                this->set_pending_match_id(match_id);
            throw std::runtime_error(error.value());
        }
        else
        {
            fb::logger::warn("matchmaking decline failed for character {}: {}", character_id, error.value());
        }
    }

    if (by == initiator::SERVER)
        co_return;

    auto ptr = weak.lock();
    if (ptr == nullptr)
        co_return;

    auto lua = this->owner.server.lua.open("scripts/interaction.lua", "on_matchmaking_decline");
    if (lua)
    {
        lua->pushobject(ptr);
        lua->pushinteger(static_cast<lua_Integer>(match_id));
        lua->pushinteger(match_type);
        std::ignore = co_await fb::lua::call(std::move(lua), weak, 3);
    }
}

async::task<void> matchmaker::enqueue(uint32_t match_type)
{
    this->owner.assert_thread();

    if (!fb::config<std::optional<uint32_t>>("world"))
        throw std::runtime_error("교차 서버에서는 매치메이킹을 등록할 수 없습니다.");

    if (this->owner.role() < fb::model::enum_value::ROLE::ADMIN &&
        this->owner.server.maintenance.read([](const maintenance_state& x) {
            return x.cross;
        }))
        throw std::runtime_error("교차 서버 점검 중에는 매치메이킹을 등록할 수 없습니다.");

    auto weak = this->owner.weak_from_this_as<character>();
    auto self = weak.lock();
    if (self == nullptr)
        co_return;

    auto match_type_enum = static_cast<fb::model::enum_value::MATCH_TYPE>(match_type);
    if (table::matchmaking->contains(match_type_enum) == false)
        throw std::runtime_error(enum_tostring(fb::model::enum_value::ERROR_CODE::MATCHMAKING_UNKNOWN_QUEUE));

    auto  matchmaking_table  = table::matchmaking;
    auto& matchmaking_config = matchmaking_table[match_type_enum];
    auto& server             = this->owner.server;
    auto  world              = this->owner.world();
    auto  character_id       = this->owner.id;
    auto  members            = std::vector<mp::TicketMember>{};
    auto  participants       = std::vector<std::shared_ptr<character>>{};

    auto group_id = this->owner.group.id();
    if (group_id.has_value() == false)
    {
        participants.push_back(self);
    }
    else
    {
        auto group_guard = server.groups.try_enter_read(group_id.value());
        if (group_guard.has_value() == false || group_guard->value() == nullptr)
            throw std::runtime_error(_TEXT(MESSAGE_GROUP_NOT_JOINED));

        auto& group = *group_guard->value();
        if (group.master() != this->owner.name())
            throw std::runtime_error(_TEXT(MESSAGE_GROUP_NOT_OWNER));

        auto member_names = group.members();
        participants.reserve(member_names.size());

        for (auto& name : member_names)
        {
            auto ch = server.characters.find(name);
            if (ch == nullptr)
                throw std::runtime_error(_TEXT(MESSAGE_GROUP_CANNOT_FIND_TARGET));

            participants.push_back(ch);
        }
    }

    members.reserve(participants.size());
    for (auto& ch : participants)
    {
        auto member_weak = ch->weak_from_this_as<character>();
        auto builder     = server.threads.new_builder<mp::TicketMember>(member_weak);
        builder.func     = [member_weak, world, match_type](auto&) -> async::task<mp::TicketMember> {
            auto ptr = member_weak.lock();
            if (ptr == nullptr)
                throw std::runtime_error(_TEXT(MESSAGE_GROUP_CANNOT_FIND_TARGET));

            auto skill = ptr->matchmaker.get(match_type);
            auto mu    = skill.has_value() ? skill->mu : matchmaker::DEFAULT_MU;
            auto sigma = skill.has_value() ? skill->sigma : matchmaker::DEFAULT_SIGMA;
            co_return mp::TicketMember{world, ptr->id, mu, sigma};
        };
        members.push_back(co_await builder.dispatch());
    }

    if (members.size() > matchmaking_config.member_count)
        throw std::runtime_error(std::format("이 매치는 {}명까지 참가할 수 있습니다. (현재 그룹 {}명)",
                                             matchmaking_config.member_count,
                                             members.size()));

    for (auto& ch : participants)
    {
        auto member_weak = ch->weak_from_this_as<character>();
        auto builder     = server.threads.new_builder(member_weak);
        builder.func     = [member_weak](auto&) -> async::task<void> {
            auto ptr = member_weak.lock();
            if (ptr == nullptr)
                co_return;
            ptr->matchmaker.begin_enqueue();
            co_return;
        };
        co_await builder.dispatch();
    }

    auto clear_enqueuing = [&server, &participants]() -> async::task<void> {
        for (auto& ch : participants)
        {
            auto member_weak = ch->weak_from_this_as<character>();
            auto builder     = server.threads.new_builder(member_weak);
            builder.func     = [member_weak](auto&) -> async::task<void> {
                auto ptr = member_weak.lock();
                if (ptr == nullptr)
                    co_return;
                if (ptr->matchmaker.ticket_id().has_value() == false)
                    ptr->matchmaker.clear_ticket();
                co_return;
            };
            co_await builder.dispatch();
        }
        co_return;
    };

    mp_resp::Enqueue resp;
    {
        auto http_error = std::optional<std::exception_ptr>{};
        try
        {
            resp =
                co_await server.http.post("matchmaking", "/matchmaking/enqueue", mp_reqs::Enqueue{match_type, members});
        }
        catch (...)
        {
            http_error = std::current_exception();
        }

        if (http_error.has_value())
        {
            co_await clear_enqueuing();
            co_await server.threads.switching(weak);
            std::rethrow_exception(http_error.value());
        }
    }

    if (resp.error != 0)
    {
        co_await clear_enqueuing();
        co_await server.threads.switching(weak);
        throw std::runtime_error(enum_tostring(static_cast<fb::model::enum_value::ERROR_CODE>(resp.error)));
    }

    // A participant who disconnected or left the group meanwhile had its enqueuing flag cleared by dequeue.
    // Claim the ticket only for members still waiting on this request; one gap invalidates the whole ticket.
    auto ticket_id = resp.ticket_id;
    auto claimed   = std::vector<std::shared_ptr<character>>{};
    for (auto& ch : participants)
    {
        auto member_weak = ch->weak_from_this_as<character>();
        auto builder     = server.threads.new_builder<bool>(member_weak);
        builder.func     = [member_weak, match_type, ticket_id](auto&) -> async::task<bool> {
            auto ptr = member_weak.lock();
            if (ptr == nullptr || ptr->matchmaker.enqueuing() == false)
                co_return false;

            ptr->matchmaker.set_ticket(match_type, ticket_id);
            co_return true;
        };
        if (co_await builder.dispatch())
            claimed.push_back(ch);
    }

    if (claimed.size() != participants.size())
    {
        try
        {
            std::ignore = co_await server.http.post("matchmaking",
                                                    "/matchmaking/dequeue",
                                                    mp_reqs::Dequeue{match_type, ticket_id, world, character_id});
        }
        catch (const std::exception& e)
        {
            fb::logger::warn("matchmaking dequeue of abandoned ticket {} failed: {}", ticket_id, e.what());
        }

        for (auto& ch : claimed)
        {
            auto member_weak = ch->weak_from_this_as<character>();
            auto builder     = server.threads.new_builder(member_weak);
            builder.func     = [member_weak, ticket_id](auto&) -> async::task<void> {
                auto ptr = member_weak.lock();
                if (ptr == nullptr)
                    co_return;
                if (ptr->matchmaker.ticket_id() == ticket_id)
                    ptr->matchmaker.clear_ticket();
                co_return;
            };
            co_await builder.dispatch();
        }

        co_await server.threads.switching(weak);
        throw std::runtime_error(_TEXT(MESSAGE_GROUP_CANNOT_FIND_TARGET));
    }
    else
    {
        for (auto& ch : participants)
        {
            auto member_weak = ch->weak_from_this_as<character>();
            auto builder     = server.threads.new_builder(member_weak);
            builder.func     = [member_weak, match_type, ticket_id](auto&) -> async::task<void> {
                auto ptr = member_weak.lock();
                if (ptr == nullptr)
                    co_return;

                auto lua = ptr->server.lua.open("scripts/interaction.lua", "on_matchmaking_enqueue");
                if (lua)
                {
                    lua->pushobject(ptr);
                    lua->pushinteger(match_type);
                    lua->pushinteger(static_cast<lua_Integer>(ticket_id));
                    std::ignore = co_await lua->call(3);
                }
                co_return;
            };
            builder.enqueue();
        }
    }

    co_await server.threads.switching(weak);
}

std::shared_ptr<fb::game::match> matchmaker::session() const
{
    return this->_session.lock();
}

void matchmaker::session(std::shared_ptr<fb::game::match> value)
{
    this->_session = value;
}

void matchmaker::save_return_point()
{
    this->owner.assert_thread();

    auto map = this->owner.map();
    if (map == nullptr)
        return;

    this->_return_map      = map->model().id;
    this->_return_position = this->owner.position();
}

void matchmaker::restore_return_point(uint32_t map, const fb::model::point16_t& position)
{
    this->owner.assert_thread();
    this->_return_map      = map;
    this->_return_position = position;
}

bool matchmaker::has_return_point() const
{
    return this->_return_map.has_value() && this->_return_position.has_value();
}

uint32_t matchmaker::return_map() const
{
    return this->_return_map.value_or(0);
}

fb::model::point16_t matchmaker::return_position() const
{
    return this->_return_position.value_or(fb::model::point16_t{0, 0});
}

void matchmaker::disconnect_after_failed_return()
{
    fb::logger::warn("Character {} could not return home from the cross server; disconnecting", this->owner.name());
    if (auto socket_ptr = this->owner.socket_ptr(); socket_ptr != nullptr)
        socket_ptr->close();
}

async::task<bool> matchmaker::transfer_home()
{
    using fb::model::enum_value::ERROR_CODE;
    using fb::model::enum_value::MESSAGE_TYPE;

    this->owner.assert_thread();
    if (this->has_return_point() == false)
        co_return false;

    auto dest = this->owner.server.maps.find(this->_return_map.value());
    if (dest == nullptr)
        co_return false;

    auto position = this->_return_position.value();
    if (fb::config<std::optional<uint32_t>>("world"))
    {
        co_return co_await this->owner.map(dest, position);
    }
    else
    {
        if (this->owner.map() == nullptr)
            co_return false;

        auto self    = this->owner.shared_from_this_as<character>();
        auto old_map = this->owner.map();
        try
        {
            auto   world = this->owner.world();
            auto&& resp  = co_await this->owner.server.http.post(
                self->weak_from_this_as<character>(),
                "internal",
                "/in-game/transfer",
                fb::protocol::internal::request::Transfer{world,
                                                          fb::protocol::internal::Service::Game,
                                                          dest->model().host,
                                                          this->owner.name(),
                                                          false});

            // Another move may have run while waiting; that move stands and this transfer is dropped.
            if (this->owner.map() != old_map)
                co_return false;

            switch (static_cast<ERROR_CODE>(resp.error))
            {
            case ERROR_CODE::NONE:
                break;

            case ERROR_CODE::MAINTENANCE:
            {
                this->owner.message(resp.maintenance_message.value_or(_TEXT(MESSAGE_MAINTENANCE_IN_PROGRESS)),
                                    MESSAGE_TYPE::STATE);
                if (auto socket_ptr = this->owner.socket_ptr(); socket_ptr != nullptr)
                    socket_ptr->close();
                co_return false;
            }

            case ERROR_CODE::SERVER_NOT_READY:
                throw std::runtime_error(_TEXT(MESSAGE_NOT_READY_GAME_SERVER));

            case ERROR_CODE::BANNED:
                throw std::runtime_error(
                    character::container::build_ban_message(resp.ban_reason, resp.ban_expire_date));

            default:
                throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), resp.error));
            }

            std::ignore = co_await this->owner.map(nullptr);
            co_await this->owner.server.save(this->owner);
            co_await this->owner.listener.on_transfer(this->owner, *dest, position, resp.ip, resp.port);
        }
        catch (std::exception& e)
        {
            this->owner.update_map();
            this->owner.show();
            this->owner.listener.on_message(this->owner, e.what(), MESSAGE_TYPE::STATE);
            this->disconnect_after_failed_return();
            co_return false;
        }
        catch (boost::system::error_code& /*e*/)
        {
            this->owner.update_map();
            this->owner.show();
            this->owner.listener.on_message(this->owner, _TEXT(MESSAGE_NOT_READY_GAME_SERVER), MESSAGE_TYPE::STATE);
            this->disconnect_after_failed_return();
            co_return false;
        }

        co_return true;
    }
}
