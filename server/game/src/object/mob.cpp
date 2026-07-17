#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/thread.h>
#include <fb/game/map.h>
#include <fb/game/mob.h>
#include <fb/game/ai.h>
#include <algorithm>
#include <limits>

using namespace fb::game;
using table = fb::model::table;

rezen::rezen(server& server, const fb::model::mob_spawn& model, const std::shared_ptr<fb::game::map>& map) :
    _server(server),
    _map(map),
    model(model)
{
    this->_respawn_time = this->_server.now();
}

uint32_t rezen::map_id() const
{
    auto map = this->_map.lock();
    return map != nullptr ? map->id : 0;
}

void rezen::decrease()
{
    auto now = this->_server.now();

    if (!this->_respawn_time.has_value())
        this->_respawn_time = now + this->model.rezen;
    this->_count = std::max(0, this->_count - 1);
}

async::task<void> rezen::spawn(std::thread::id thread_id)
{
    auto map = this->_map.lock();
    if (map == nullptr)
        co_return;

    if (map->active == false)
        co_return;

    if (map->is_active() == false)
        co_return;

    auto thread = this->_server.threads.at(thread_id);
    if (thread == nullptr || thread->id() != thread_id)
        co_return;

    auto now = this->_server.now();
    if (!this->_respawn_time.has_value())
        co_return;

    if (now < this->_respawn_time)
        co_return;

    auto spawn_count = this->model.count - this->_count;
    if (spawn_count < 1)
        co_return;

    auto mobs = std::vector<std::shared_ptr<fb::game::mob>>();
    for (int i = 0; i < spawn_count; i++)
    {
        // Use smart pointer for mob creation
        auto mob = this->_server.make<fb::game::mob>(table::mob[this->model.mob],
                                                     mob::initial_params{.alive = true, .rezen = this});

        mob->direction(DIRECTION(std::rand() % 4));
        mob->stat.heal(mob->stat.base_hp());

        while (true)
        {
            auto width    = this->model.end.x - this->model.begin.x;
            auto height   = this->model.end.y - this->model.begin.y;
            auto position = fb::model::point16_t(this->model.begin.x + (width > 0 ? std::rand() % width : 0),
                                                 this->model.begin.y + (height > 0 ? std::rand() % height : 0));

            if (position.x > map->width() - 1 || position.y > map->height() - 1)
                continue;

            if (map->blocked(position.x, position.y))
                continue;

            mob->position(position, true);
            std::ignore = co_await mob->map(map, position, {.notify = false});
            break;
        }

        mobs.push_back(mob);
    }

    for (auto& mob : mobs)
    {
        mob->action_time(now);
        mob->hidden(false);
    }

    auto oids = std::vector<uint32_t>();
    for (auto& mob : mobs)
    {
        oids.push_back(mob->oid());
    }
    map->bulk_update(oids);

    this->_respawn_time.reset();
}

void rezen::force_spawn(std::thread::id thread_id)
{
    this->_respawn_time = this->_server.now();
}

mob::mob(fb::game::server& server, const fb::model::mob& model, const initial_params& params) :
    stat(*this),
    life(server, model, stat, params),
    listener(server.listener),
    _rezen(params.rezen),
    owner(params.owner != nullptr ? params.owner->weak_from_this_as<character>() : std::weak_ptr<character>())
{
    // Initialize AI strategy based on mob's attack type
    this->_ai_strategy = ai::create(model.attack_type);

    this->_hidden = !params.alive;
    if (params.alive)
    {
        // Do not notify during construction: server::send uses weak_from_this
        // which throws bad_weak_ptr before make_shared finishes.
        this->stat.hp(this->stat.base_hp(), false);
        this->stat.mp_up(this->stat.base_mp(), nullptr, false);
    }
}

mob::~mob()
{
    if (this->_rezen != nullptr)
        this->_rezen->decrease();
}

async::task<bool> mob::call_script()
{
    this->assert_thread();
    this->update_target();

    auto& model = this->based<fb::model::mob>();
    auto  path  = std::format("scripts/mob/{}.lua", model.id);
    auto  func  = std::format("ON_MOB_ATTACK_{}", model.id);

    if (this->_attack_thread != nullptr)
        co_return false;

    this->_attack_thread = this->server.lua.new_context();
    if (this->_attack_thread == nullptr)
        co_return true;

    if (this->_attack_thread->load(path) == false)
    {
        this->_attack_thread->release();
        this->_attack_thread = nullptr;
        co_return true;
    }

    if (this->_attack_thread->func(func) == false)
    {
        fb::lua::report_func_missing(path, func);
        this->_attack_thread->release();
        this->_attack_thread = nullptr;
        co_return true;
    }

    this->_attack_thread->pushobject(this);

    if (this->_target.expired() == false)
    {
        auto shared = this->_target.lock();
        if (shared != nullptr)
            this->_attack_thread->pushobject(shared);
    }
    else
        this->_attack_thread->pushnil();

    auto& ctx  = this->server;
    auto  weak = this->weak_from_this();
    try
    {
        std::ignore = co_await this->_attack_thread->call(2);
    }
    catch (std::exception& e)
    {
        fb::logger::warn(e.what());
    }

    auto shared = weak.lock();
    if (shared == nullptr)
        co_return false;

    this->_attack_thread = nullptr;
    co_return true;
}

async::task<void> mob::action(fb::model::datetime now)
{
    if (co_await this->call_script() == false)
        co_return;

    this->AI(now);
}

const fb::model::datetime& mob::action_time() const
{
    this->assert_thread();

    return this->_action_time;
}

void mob::action_time(const fb::model::datetime& dt)
{
    this->assert_thread();

    this->_action_time = dt;
}

std::shared_ptr<life> mob::target() const
{
    this->assert_thread();

    if (this->_target.expired())
        return nullptr;

    auto shared = this->_target.lock();
    if (shared == nullptr)
        return nullptr;

    if (shared->map() != this->map())
        return nullptr;

    if (shared->alive() == false)
        return nullptr;

    if (this->sight(*shared) == false)
        return nullptr;

    if (shared->hidden(*this))
        return nullptr;

    return shared;
}

void mob::target(std::shared_ptr<life> value)
{
    this->assert_thread();

    this->_target = value;
}

std::shared_ptr<life> mob::oblivion() const
{
    this->assert_thread();

    if (this->_oblivion.expired())
        return nullptr;

    auto shared = this->_oblivion.lock();
    if (shared == nullptr)
        return nullptr;

    return shared;
}

void mob::oblivion(std::shared_ptr<life> value)
{
    this->assert_thread();

    this->_oblivion = value;
}

std::shared_ptr<life> mob::update_target()
{
    this->assert_thread();

    auto target = this->target();
    if (target == nullptr)
    {
        this->_target.reset();

        auto& model = this->based<fb::model::mob>();
        if (model.attack_type == MOB_ATTACK_TYPE::CONTAINMENT)
            this->_target = this->find_target();
        else
            this->_target.reset();
    }

    return this->_target.lock();
}

std::weak_ptr<life> mob::find_target()
{
    this->assert_thread();

    auto map = this->_map;
    if (map == nullptr)
        return std::weak_ptr<life>();

    if (this->owner.expired())
        return std::weak_ptr<life>();

    auto min_distance_sqrt = 0xFFFFFFFF;
    for (const auto& x : this->sight_in(OBJECT_TYPE::CHARACTER))
    {
        auto life = std::static_pointer_cast<fb::game::life>(x);
        if (life == this->_oblivion.lock())
            continue;

        if (life->alive() == false)
            continue;

        auto distance_sqrt = (uint32_t)std::abs(x->x() - this->x()) * std::abs(x->y() - this->y());
        if (distance_sqrt > min_distance_sqrt)
            continue;

        this->_target = life->weak_from_this_as<fb::game::life>();
    }

    return this->_target;
}

bool mob::near_target(const std::shared_ptr<fb::game::life>& target, DIRECTION& out) const
{
    this->assert_thread();

    for (int i = 0; i < 4; i++)
    {
        auto direction = DIRECTION(i);
        if (this->side(direction, OBJECT_TYPE::LIFE) != target)
            continue;

        out = direction;
        return true;
    }

    return false;
}

bool mob::move_step(const fb::model::point16_t& position)
{
    this->assert_thread();
    auto x_axis = bool(std::rand() % 2);
    if (x_axis)
    {
        if (this->_position.x > position.x && this->move(DIRECTION::LEFT))
            return true;
        if (this->_position.x < position.x && this->move(DIRECTION::RIGHT))
            return true;
        if (this->_position.y > position.y && this->move(DIRECTION::TOP))
            return true;
        if (this->_position.y < position.y && this->move(DIRECTION::BOTTOM))
            return true;
    }
    else
    {
        if (this->_position.y > position.y && this->move(DIRECTION::TOP))
            return true;
        if (this->_position.y < position.y && this->move(DIRECTION::BOTTOM))
            return true;
        if (this->_position.x > position.x && this->move(DIRECTION::LEFT))
            return true;
        if (this->_position.x < position.x && this->move(DIRECTION::RIGHT))
            return true;
    }

    return false;
}

void mob::AI(const fb::model::datetime& now)
{
    this->assert_thread();

    if (this->_attack_thread != nullptr)
        return;

    if (ENUM_IN(static_cast<CROWD_CONTROL>(this->cc), CROWD_CONTROL::SIGHT))
        return;

    auto& model = this->based<fb::model::mob>();
    if (now < this->_action_time + model.speed)
        return;

    // Execute AI strategy if available
    if (this->_ai_strategy)
    {
        this->_ai_strategy->execute(*this, now);
        this->_action_time = now;
    }

    this->_action_time = now;
}

bool mob::available() const
{
    this->assert_thread();

    return this->alive() && this->_hidden == false && this->_soft_dead == false;
}

uint32_t mob::normal_attack_damage(MOB_SIZE size) const
{
    this->assert_thread();

    auto& model      = this->based<fb::model::mob>();
    auto  difference = model.damage.max - model.damage.min;
    return model.damage.min + (std::rand() % difference);
}

void mob::kill(DESTROY_TYPE destroy_type)
{
    this->assert_thread();
    // Destroy must be awaited by the caller (settle_deaths / settle_kills / ride).
    life::kill(destroy_type);
}

async::task<void> mob::damage_to(const damage_list& targets)
{
    co_await this->damage_to(targets, damage_opts{});
    co_return;
}

async::task<void> mob::damage_to(const damage_list& targets, const damage_opts& opts)
{
    this->assert_thread();

    // Damage is always applied as this mob (not redirected to owner).
    auto dead = this->damage_targets(targets, opts);
    if (dead.empty())
        co_return;

    auto owner = this->owner.lock();
    if (owner != nullptr)
    {
        co_await owner->settle_kills(std::move(dead));
        co_return;
    }

    co_await this->settle_deaths(std::move(dead));
}

async::task<void> mob::drop_model_items(const fb::model::mob&       model,
                                        const fb::model::point16_t& position,
                                        std::vector<uint32_t>&      oids)
{
    auto map = this->map();
    if (map == nullptr || model.drop.empty())
        co_return;

    auto& drop = table::drop[model.drop];
    for (auto& dsl : drop.dsl)
    {
        switch (dsl.header)
        {
        case DSL::item:
        {
            auto params           = fb::model::dsl::item(dsl.params);
            auto multiplier       = this->server.drop_rate_multiplier();
            auto adjusted_percent = std::min(100.0, params.percent * multiplier);
            auto random           = std::rand() % 100;
            if (random > (int)adjusted_percent)
                continue;

            auto item   = table::item[params.id].make(this->server);
            std::ignore = co_await item->map(map, position, {.notify = false});
            oids.push_back(item->oid());
        }
        break;
        }
    }
}

async::task<void> mob::drop_items()
{
    this->assert_thread();

    auto& model    = this->based<fb::model::mob>();
    auto  oids     = std::vector<uint32_t>{};
    auto  map      = this->map();
    auto& position = this->position();
    for (auto& item : this->_items)
    {
        std::ignore = co_await item->map(map, position, {.notify = false});
        oids.push_back(item->oid());
    }
    this->_items.clear();

    auto owner = this->owner.lock();
    if (owner == nullptr)
    {
        co_await this->drop_model_items(model, position, oids);

        // Body death: also roll linked parts' drop tables at body position
        for (auto& part : this->parts())
        {
            if (part == nullptr)
                continue;

            for (auto& item : part->_items)
            {
                std::ignore = co_await item->map(map, position, {.notify = false});
                oids.push_back(item->oid());
            }
            part->_items.clear();
            co_await this->drop_model_items(part->based<fb::model::mob>(), position, oids);
        }
    }

    if (!oids.empty() && this->map() != nullptr)
        this->map()->bulk_update(oids);
}

async::task<void> mob::destroy(DESTROY_TYPE destroy_type)
{
    this->assert_thread();

    if (this->_destroying)
    {
        co_await object::destroy(destroy_type);
        co_return;
    }

    this->_destroying = true;

    if (this->_parts.empty() == false)
    {
        auto linked = this->parts();
        this->_parts.clear();
        for (auto& part : linked)
        {
            if (part == nullptr)
                continue;

            part->_body.reset();
            part->_destroying = true;
            part->kill(destroy_type);
            co_await part->destroy(destroy_type);
        }
    }

    if (auto body = this->_body.lock())
    {
        body->unlink_part(*this);
        this->_body.reset();
    }

    co_await object::destroy(destroy_type);
}

void mob::assert_thread() const
{
    if (this->_map != nullptr)
        object::assert_thread();
    else
        return;
}

bool mob::is_cardinally_adjacent(const fb::model::point16_t& a, const fb::model::point16_t& b)
{
    auto dx = static_cast<int>(a.x) - static_cast<int>(b.x);
    auto dy = static_cast<int>(a.y) - static_cast<int>(b.y);

    if (dx == 0 && (dy == 1 || dy == -1))
        return true;

    if (dy == 0 && (dx == 1 || dx == -1))
        return true;

    return false;
}

bool mob::is_cover_barrier_cell(const fb::model::point16_t& cell, const fb::model::point16_t& cover_center)
{
    if (cell == cover_center)
        return true;

    return mob::is_cardinally_adjacent(cell, cover_center);
}

bool mob::cover_blocks_move(const fb::game::map&        map,
                            const fb::model::point16_t& from,
                            const fb::model::point16_t& to) const
{
    for (const auto* pivot : {&from, &to})
    {
        for (auto& obj : map.nears(*pivot, OBJECT_TYPE::LIFE))
        {
            if (obj.get() == this)
                continue;

            auto life = std::static_pointer_cast<fb::game::life>(obj);
            if (life->cover() == false)
                continue;

            const auto& cover_center = life->position();

            if (mob::is_cardinally_adjacent(from, cover_center))
                return true;

            if (mob::is_cover_barrier_cell(to, cover_center))
                return true;
        }
    }

    return false;
}

bool mob::move(DIRECTION direction)
{
    this->assert_thread();
    auto map = this->map();
    if (map == nullptr)
        return false;

    const auto& from     = this->position();
    const auto  position = this->side_position(direction);

    if (this->cover_blocks_move(*map, from, position))
        return false;

    return fb::game::object::move(direction);
}

const std::vector<std::shared_ptr<fb::game::item>>& mob::items() const
{
    this->assert_thread();
    return this->_items;
}

bool mob::push_item(std::shared_ptr<fb::game::item> item)
{
    this->assert_thread();
    if (this->_items.size() >= CONTAINER_CAPACITY)
        return false;

    this->_items.push_back(item);
    return true;
}

bool mob::hidden(const fb::game::object& target) const
{
    return this->_hidden;
}

void mob::hidden(bool enabled)
{
    this->assert_thread();
    if (this->_hidden == enabled)
        return;

    this->_hidden = enabled;
    if (this->map() == nullptr)
        return;

    if (enabled)
    {
        for (auto& obj : this->nears(OBJECT_TYPE::CHARACTER))
            this->object::hide(*obj);
    }
    else
    {
        for (auto& obj : this->nears(OBJECT_TYPE::CHARACTER))
            this->update_external(*obj, true);
    }
}

std::shared_ptr<fb::game::appearance> mob::appearance() const
{
    return this->based<fb::model::mob>().create_appearance();
}

bool mob::add_part(const std::shared_ptr<mob>& part)
{
    this->assert_thread();
    if (part == nullptr || part.get() == this)
        return false;

    if (part->body() != nullptr)
        return false;

    if (part->has_parts())
        return false;

    if (this->body() != nullptr)
        return false;

    part->_body = this->weak_from_this_as<mob>();
    this->_parts.push_back(part);

    // PARTS mode: body HP tracks the sum of linked parts.
    // BODY mode: body keeps its own model HP; parts are hitboxes only.
    if (this->_parts_mode == MOB_PARTS_MODE::PARTS)
        this->sync_body_hp_from_parts();

    return true;
}

mob::parts_vector_t mob::parts() const
{
    this->assert_thread();
    auto result = parts_vector_t{};
    result.reserve(this->_parts.size());
    for (auto& weak : this->_parts)
    {
        auto part = weak.lock();
        if (part != nullptr)
            result.push_back(part);
    }
    return result;
}

std::shared_ptr<mob> mob::body() const
{
    this->assert_thread();
    return this->_body.lock();
}

bool mob::has_parts() const
{
    this->assert_thread();
    return this->_parts.empty() == false;
}

void mob::parts_mode(MOB_PARTS_MODE mode)
{
    this->assert_thread();
    this->_parts_mode = mode;
    if (mode == MOB_PARTS_MODE::PARTS && this->has_parts())
        this->sync_body_hp_from_parts();
}

MOB_PARTS_MODE mob::parts_mode() const
{
    this->assert_thread();
    return this->_parts_mode;
}

bool mob::soft_dead() const
{
    this->assert_thread();
    return this->_soft_dead;
}

uint32_t mob::total_exp() const
{
    this->assert_thread();
    auto exp = this->based<fb::model::mob>().exp;
    for (auto& part : this->parts())
    {
        if (part != nullptr)
            exp += part->based<fb::model::mob>().exp;
    }
    return exp;
}

void mob::sync_body_hp_from_parts()
{
    this->assert_thread();

    uint64_t sum_cur = 0;
    uint64_t sum_max = 0;
    for (auto& part : this->parts())
    {
        if (part == nullptr)
            continue;
        sum_cur += part->stat.hp();
        sum_max += part->stat.maxhp();
    }

    auto base = static_cast<int64_t>(this->stat.base_hp());
    // buff_hp is int32_t; clamp so maxhp stays within representable range
    auto desired_max = static_cast<int64_t>(std::min<uint64_t>(sum_max, std::numeric_limits<uint32_t>::max()));
    auto buff        = desired_max - base;
    buff             = std::clamp(buff,
                      static_cast<int64_t>(std::numeric_limits<int32_t>::min()),
                      static_cast<int64_t>(std::numeric_limits<int32_t>::max()));
    this->stat.buff_hp(static_cast<int32_t>(buff));

    auto max_hp              = this->stat.maxhp();
    auto hp                  = static_cast<uint32_t>(std::min<uint64_t>(sum_cur, max_hp));
    this->_forwarding_damage = true;
    this->stat.hp(hp, true);
    this->_forwarding_damage = false;
}

void mob::unlink_part(mob& part)
{
    this->assert_thread();
    for (auto it = this->_parts.begin(); it != this->_parts.end();)
    {
        auto locked = it->lock();
        if (locked == nullptr || locked.get() == &part)
            it = this->_parts.erase(it);
        else
            ++it;
    }

    if (this->_parts_mode == MOB_PARTS_MODE::PARTS && this->has_parts())
        this->sync_body_hp_from_parts();
}

void mob::on_part_hp_increased(uint32_t delta)
{
    this->assert_thread();
    if (delta == 0)
        return;

    auto body = this->_body.lock();
    if (body == nullptr || body->parts_mode() != MOB_PARTS_MODE::PARTS)
        return;

    body->_forwarding_damage = true;
    body->stat.heal(delta, nullptr, true);
    body->_forwarding_damage = false;
}

uint32_t mob::damage_as_part(uint32_t                value,
                             std::shared_ptr<object> from,
                             bool                    critical,
                             float                   rate,
                             bool                    physical,
                             bool                    fixed,
                             bool                    notify)
{
    this->assert_thread();

    auto body = this->_body.lock();
    if (body == nullptr)
        return 0;

    switch (body->parts_mode())
    {
    case MOB_PARTS_MODE::PARTS:
    {
        if (this->_soft_dead)
            return 0;

        this->_forwarding_damage = true;
        auto dealt               = this->stat.damage(value, from, critical, rate, physical, fixed, notify);
        this->_forwarding_damage = false;

        if (this->stat.hp() == 0)
            this->_soft_dead = true;

        if (dealt > 0)
        {
            body->_forwarding_damage = true;
            body->stat.damage(dealt, from, critical, rate, physical, true, notify);
            body->_forwarding_damage = false;
        }
        return dealt;
    }
    case MOB_PARTS_MODE::BODY:
    {
        body->_forwarding_damage = true;
        auto dealt               = body->stat.damage(value, from, critical, rate, physical, fixed, notify);
        body->_forwarding_damage = false;
        return dealt;
    }
    default:
        return 0;
    }
}