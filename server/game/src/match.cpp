#include <fb/game/match.h>
#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/game/map.h>
#include <fb/game/object.h>
#include <fb/model/model.h>
#include <fb/model/datetime.h>
#include <fb/logger.h>
#include <random.h>
#include <algorithm>
#include <chrono>
#include <format>

using namespace fb::game;
using namespace std::chrono_literals;
using table = fb::model::table;

match::match(server& server, std::string id, uint32_t type, uint32_t slot, uint32_t expected) :
    _server(server),
    _id(std::move(id)),
    _type(type),
    _slot(slot),
    _expected(expected)
{ }

const std::string& match::id() const
{
    return this->_id;
}

uint32_t match::type() const
{
    return this->_type;
}

uint32_t match::slot() const
{
    return this->_slot;
}

uint32_t match::expected() const
{
    return this->_expected;
}

MATCH_STATE match::state() const
{
    auto lock = std::lock_guard(this->_mutex);
    return this->_state;
}

uint32_t match::member_count()
{
    auto lock = std::lock_guard(this->_mutex);
    return this->live_count_locked();
}

std::vector<std::shared_ptr<character>> match::members()
{
    auto lock = std::lock_guard(this->_mutex);
    return this->snapshot();
}

std::vector<std::shared_ptr<match::team>> match::teams()
{
    auto lock   = std::lock_guard(this->_mutex);
    auto result = std::vector<std::shared_ptr<team>>{};
    result.reserve(this->_teams.size());
    for (auto& [id, team] : this->_teams)
    {
        if (team != nullptr)
            result.push_back(team);
    }
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        return a->id() < b->id();
    });
    return result;
}

std::shared_ptr<match::team> match::find(const character& ch)
{
    auto lock = std::lock_guard(this->_mutex);
    for (auto& [_, team] : this->_teams)
    {
        if (team == nullptr)
            continue;

        if (team->contains(ch))
            return team;
    }
    return nullptr;
}

std::shared_ptr<fb::game::map> match::map(uint32_t model_id)
{
    if (this->_server.maps.contains(model_id) == false)
        return nullptr;

    auto source = this->_server.maps[model_id];
    if (source == nullptr)
        return nullptr;

    return this->_server.maps.ensure_instance(source, this->_slot);
}

std::vector<std::shared_ptr<character>> match::snapshot()
{
    auto result = std::vector<std::shared_ptr<character>>{};
    auto live   = std::vector<std::weak_ptr<character>>{};
    for (auto& weak : this->_members)
    {
        auto ch = weak.lock();
        if (ch == nullptr)
            continue;

        live.push_back(weak);
        result.push_back(ch);
    }
    this->_members = std::move(live);
    return result;
}

uint32_t match::live_count_locked()
{
    return static_cast<uint32_t>(this->snapshot().size());
}

void match::cancel_timer()
{
    if (this->_timer == nullptr)
        return;

    this->_timer->cancel();
    this->_timer.reset();
}

async::task<void> match::invoke(std::string_view func, character& ch, std::shared_ptr<object> extra, bool wait)
{
    auto path = std::format("scripts/match/{}.lua", this->_type);
    auto lua  = this->_server.lua.open(path, func);
    if (!lua)
        co_return;

    lua->pushobject(*this);
    lua->pushobject(ch);
    if (extra != nullptr)
        lua->pushobject(*extra);

    auto argc = extra != nullptr ? 3 : 2;
    if (wait)
        std::ignore = co_await lua->call(argc);
    else
        std::ignore = lua->call(argc);
    co_return;
}

async::task<void> match::join(character& ch, uint32_t team)
{
    auto already = false;
    auto full    = false;
    {
        auto lock = std::lock_guard(this->_mutex);
        if (this->_state == MATCH_STATE::ending || this->_state == MATCH_STATE::closed)
            co_return;

        for (auto& weak : this->_members)
        {
            auto existing = weak.lock();
            if (existing != nullptr && existing.get() == &ch)
            {
                already = true;
                break;
            }
        }
        if (already == false)
            this->_members.push_back(ch.weak_from_this_as<character>());
        if (already == false && team != 0)
        {
            auto& slot = this->_teams[team];
            if (slot == nullptr)
                slot = std::make_shared<match::team>(*this, team);
            slot->enter(ch);
        }
    }
    if (already)
        co_return;

    ch.match(this->shared_from_this_as<match>());
    co_await this->invoke("on_join", ch);

    {
        auto lock = std::lock_guard(this->_mutex);
        full      = this->_state == MATCH_STATE::waiting && this->live_count_locked() >= this->_expected;
    }
    if (full)
        this->enqueue_play();
    co_return;
}

void match::leave(character& ch)
{
    auto empty  = false;
    auto closed = false;
    {
        auto lock = std::lock_guard(this->_mutex);
        closed    = this->_state == MATCH_STATE::closed;
        auto it   = this->_members.begin();
        while (it != this->_members.end())
        {
            auto existing = it->lock();
            if (existing == nullptr || existing.get() == &ch)
                it = this->_members.erase(it);
            else
                ++it;
        }
        for (auto& [_, team] : this->_teams)
        {
            if (team != nullptr)
                team->leave(ch);
        }
        empty = this->_members.empty();
    }
    ch.match(nullptr);
    if (empty && closed == false)
        this->enqueue_close();
}

void match::wait(uint32_t seconds)
{
    auto thread = this->_server.threads.current();
    if (thread == nullptr)
    {
        thread = this->_server.threads.at(0);
        if (thread == nullptr)
            return;

        auto self    = this->shared_from_this_as<match>();
        auto builder = thread->new_builder<void>();
        builder.func = [self, seconds](auto&) -> async::task<void> {
            self->wait(seconds);
            co_return;
        };
        builder.enqueue();
        return;
    }

    auto lock = std::lock_guard(this->_mutex);
    if (this->_state != MATCH_STATE::waiting)
        return;

    this->cancel_timer();
    auto weak    = this->weak_from_this_as<match>();
    this->_timer = thread->settimer(
        [weak](const fb::model::datetime&, std::thread::id) -> async::task<void> {
            auto self = weak.lock();
            if (self == nullptr)
                co_return;

            auto waiting = false;
            {
                auto inner = std::lock_guard(self->_mutex);
                waiting    = self->_state == MATCH_STATE::waiting;
            }
            if (waiting)
                self->enqueue_play();
            co_return;
        },
        fb::model::timespan(std::chrono::seconds(seconds)),
        fb::timer::repeat_type::once);
}

void match::duration(uint32_t seconds, uint32_t grace_seconds)
{
    auto thread = this->_server.threads.current();
    if (thread == nullptr)
    {
        thread = this->_server.threads.at(0);
        if (thread == nullptr)
            return;

        auto self    = this->shared_from_this_as<match>();
        auto builder = thread->new_builder<void>();
        builder.func = [self, seconds, grace_seconds](auto&) -> async::task<void> {
            self->duration(seconds, grace_seconds);
            co_return;
        };
        builder.enqueue();
        return;
    }

    auto lock = std::lock_guard(this->_mutex);
    if (this->_state != MATCH_STATE::playing)
        return;

    if (seconds == 0)
    {
        this->cancel_timer();
        return;
    }

    if (this->_timer != nullptr)
        return;

    auto weak    = this->weak_from_this_as<match>();
    this->_timer = thread->settimer(
        [weak, grace_seconds](const fb::model::datetime&, std::thread::id) -> async::task<void> {
            auto self = weak.lock();
            if (self == nullptr)
                co_return;

            auto playing = false;
            {
                auto inner = std::lock_guard(self->_mutex);
                playing    = self->_state == MATCH_STATE::playing;
            }
            if (playing)
                self->finish(grace_seconds);
            co_return;
        },
        fb::model::timespan(std::chrono::seconds(seconds)),
        fb::timer::repeat_type::once);
}

void match::finish(uint32_t grace_seconds)
{
    auto thread = this->_server.threads.current();
    if (thread == nullptr)
    {
        thread = this->_server.threads.at(0);
        if (thread == nullptr)
            return;

        auto self    = this->shared_from_this_as<match>();
        auto builder = thread->new_builder<void>();
        builder.func = [self, grace_seconds](auto&) -> async::task<void> {
            self->finish(grace_seconds);
            co_return;
        };
        builder.enqueue();
        return;
    }

    auto members = std::vector<std::shared_ptr<character>>{};
    {
        auto lock = std::lock_guard(this->_mutex);
        if (this->_state == MATCH_STATE::ending || this->_state == MATCH_STATE::closed)
            return;

        this->_state = MATCH_STATE::ending;
        this->cancel_timer();
        members = this->snapshot();
    }

    auto self    = this->shared_from_this_as<match>();
    auto builder = thread->new_builder<void>();
    builder.func = [self, members, grace_seconds](auto&) -> async::task<void> {
        for (auto& ch : members)
        {
            if (ch == nullptr)
                continue;

            try
            {
                co_await self->_server.threads.switching(ch->weak_from_this_as<character>());
                co_await self->invoke("on_end", *ch);
                ch->timer(0, fb::model::enum_value::TIMER_TYPE::OFF);
            }
            catch (std::exception& e)
            {
                fb::logger::warn("match end hook failed: {}", e.what());
            }
        }

        if (grace_seconds == 0)
        {
            self->enqueue_close();
            co_return;
        }

        auto notify_thread = self->_server.threads.current();
        if (notify_thread == nullptr)
            notify_thread = self->_server.threads.at(0);
        if (notify_thread == nullptr)
            co_return;

        auto weak = self->weak_from_this_as<match>();
        auto lock = std::lock_guard(self->_mutex);
        if (self->_state != MATCH_STATE::ending)
            co_return;

        self->_timer = notify_thread->settimer(
            [weak](const fb::model::datetime&, std::thread::id) -> async::task<void> {
                auto session = weak.lock();
                if (session == nullptr)
                    co_return;

                session->enqueue_close();
                co_return;
            },
            fb::model::timespan(std::chrono::seconds(grace_seconds)),
            fb::timer::repeat_type::once);
        co_return;
    };
    builder.enqueue();
}

void match::on_death(character& ch, std::shared_ptr<object> killer)
{
    auto playing = false;
    {
        auto lock = std::lock_guard(this->_mutex);
        playing   = this->_state == MATCH_STATE::playing;
    }
    if (playing == false)
        return;

    auto path = std::format("scripts/match/{}.lua", this->_type);
    auto lua  = this->_server.lua.open(path, "on_death");
    if (!lua)
        return;

    lua->pushobject(*this);
    lua->pushobject(ch);
    if (killer != nullptr)
        lua->pushobject(*killer);
    else
        lua->pushnil();
    std::ignore = lua->call(3);
}

void match::enqueue_play()
{
    auto thread = this->_server.threads.at(0);
    if (thread == nullptr)
        return;

    auto self    = this->shared_from_this_as<match>();
    auto builder = thread->new_builder<void>();
    builder.func = [self](auto&) -> async::task<void> {
        co_await self->play();
        co_return;
    };
    builder.enqueue();
}

void match::enqueue_close()
{
    auto thread = this->_server.threads.at(0);
    if (thread == nullptr)
        return;

    auto self    = this->shared_from_this_as<match>();
    auto builder = thread->new_builder<void>();
    builder.func = [self](auto&) -> async::task<void> {
        co_await self->close();
        co_return;
    };
    builder.enqueue();
}

async::task<void> match::play()
{
    auto members = std::vector<std::shared_ptr<character>>{};
    {
        auto lock = std::lock_guard(this->_mutex);
        if (this->_state != MATCH_STATE::waiting)
            co_return;

        this->_state = MATCH_STATE::playing;
        this->cancel_timer();
        members = this->snapshot();
        for (auto& [_, team] : this->_teams)
        {
            if (team != nullptr)
                team->pick_leader();
        }
    }

    for (auto& ch : members)
    {
        if (ch == nullptr)
            continue;

        try
        {
            co_await this->_server.threads.switching(ch->weak_from_this_as<character>());
            co_await this->invoke("on_playing", *ch, nullptr, false);
        }
        catch (std::exception& e)
        {
            fb::logger::warn("match play hook failed: {}", e.what());
        }
    }
    co_return;
}

async::task<void> match::close()
{
    auto members = std::vector<std::shared_ptr<character>>{};
    auto slot    = this->_slot;
    {
        auto lock = std::lock_guard(this->_mutex);
        if (this->_state == MATCH_STATE::closed)
            co_return;

        this->_state = MATCH_STATE::closed;
        this->cancel_timer();
        members = this->snapshot();
        this->_members.clear();
        this->_teams.clear();
    }

    for (auto& ch : members)
    {
        if (ch == nullptr)
            continue;

        try
        {
            co_await this->_server.threads.switching(ch->weak_from_this_as<character>());
            ch->timer(0, fb::model::enum_value::TIMER_TYPE::OFF);
            ch->match(nullptr);
            if (ch->map() != nullptr)
                std::ignore = co_await ch->transfer_home();
        }
        catch (std::exception& e)
        {
            fb::logger::warn("match close failed: {}", e.what());
        }
    }

    auto snapshot = this->_server.maps.snapshot();
    if (snapshot != nullptr)
    {
        for (auto& map : *snapshot)
        {
            if (map == nullptr || map->is_instance() == false)
                continue;
            if (map->slot() != slot)
                continue;

            try
            {
                co_await this->_server.maps.destroy(map);
            }
            catch (std::exception& e)
            {
                fb::logger::warn("match instance destroy failed: {}", e.what());
            }
        }
    }

    this->_server.matches.remove(this->_id);
    co_return;
}

match::team::team(match& match, uint32_t id) :
    _match(match),
    _id(id)
{ }

uint32_t match::team::id() const
{
    return this->_id;
}

bool match::team::contains(const character& ch) const
{
    for (auto& weak : this->_members)
    {
        auto member = weak.lock();
        if (member != nullptr && member.get() == &ch)
            return true;
    }
    return false;
}

std::shared_ptr<character> match::team::leader() const
{
    auto lock = std::lock_guard(this->_match._mutex);
    return this->_leader.lock();
}

std::vector<std::shared_ptr<character>> match::team::members()
{
    auto lock   = std::lock_guard(this->_match._mutex);
    auto result = std::vector<std::shared_ptr<character>>{};
    auto live   = std::vector<std::weak_ptr<character>>{};
    for (auto& weak : this->_members)
    {
        auto ch = weak.lock();
        if (ch == nullptr)
            continue;

        live.push_back(weak);
        result.push_back(ch);
    }
    this->_members = std::move(live);
    return result;
}

void match::team::enter(character& ch)
{
    for (auto& weak : this->_members)
    {
        auto existing = weak.lock();
        if (existing != nullptr && existing.get() == &ch)
            return;
    }
    this->_members.push_back(ch.weak_from_this_as<character>());
}

void match::team::leave(character& ch)
{
    auto it = this->_members.begin();
    while (it != this->_members.end())
    {
        auto existing = it->lock();
        if (existing == nullptr || existing.get() == &ch)
            it = this->_members.erase(it);
        else
            ++it;
    }

    auto leader = this->_leader.lock();
    if (leader != nullptr && leader.get() == &ch)
        this->_leader.reset();
}

void match::team::pick_leader()
{
    auto live = std::vector<std::shared_ptr<character>>{};
    for (auto& weak : this->_members)
    {
        auto ch = weak.lock();
        if (ch != nullptr)
            live.push_back(ch);
    }
    if (live.empty())
    {
        this->_leader.reset();
        return;
    }

    auto i        = random<uint32_t>(0, static_cast<uint32_t>(live.size() - 1));
    this->_leader = live[i];
}

match::container::container(server& server) :
    _server(server)
{ }

std::shared_ptr<match> match::container::ensure(std::string_view match_id, uint32_t match_type)
{
    auto key  = std::string(match_id);
    auto lock = std::lock_guard(this->_mutex);
    auto it   = this->_sessions.find(key);
    if (it != this->_sessions.end())
        return it->second;

    auto expected  = uint32_t{1};
    auto type_enum = static_cast<fb::model::enum_value::MATCH_TYPE>(match_type);
    if (table::matchmaking->contains(type_enum))
    {
        auto& row = table::matchmaking[type_enum];
        expected  = row.member_count * row.team_count;
        if (expected == 0)
            expected = 1;
    }

    auto slot = this->_next_slot++;
    if (slot == 0)
        slot = this->_next_slot++;

    auto session = std::make_shared<match>(this->_server, key, match_type, slot, expected);
    this->_sessions.emplace(key, session);
    return session;
}

async::task<void> match::container::join(character& ch, std::string_view match_id, uint32_t match_type, uint32_t team)
{
    if (match_id.empty())
        co_return;

    auto current = ch.match();
    if (current != nullptr && current->id() == match_id)
        co_return;
    if (current != nullptr)
        current->leave(ch);

    auto session = this->ensure(match_id, match_type);
    co_await session->join(ch, team);
    co_return;
}

void match::container::leave(character& ch)
{
    auto session = ch.match();
    if (session == nullptr)
        return;

    session->leave(ch);
}

void match::container::remove(std::string_view match_id)
{
    auto lock = std::lock_guard(this->_mutex);
    this->_sessions.erase(std::string(match_id));
}
