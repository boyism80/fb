#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/thread.h>
#include <fb/game/map.h>
#include <fb/game/mob.h>
#include <fb/game/ai.h>

using namespace fb::game;
using table = fb::model::table;

rezen::rezen(server& server, const fb::model::mob_spawn& model) :
    _server(server),
    model(model)
{
    this->_respawn_time = this->_server.now();
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
    if (this->_server.maps.contains(this->model.parent) == false)
        co_return;

    auto map = this->_server.maps[this->model.parent];
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
            auto map      = this->_server.maps[this->model.parent];
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

    this->hidden(!params.alive);
    if (params.alive)
    {
        this->stat.heal(this->stat.base_hp());
        this->stat.mp_up(this->stat.base_mp());
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
    if (model.script.empty())
        co_return true;

    if (model.on_attack.empty())
        co_return true;

    if (this->_attack_thread != nullptr)
        co_return false;

    this->_attack_thread = fb::lua::new_context();
    if (this->_attack_thread == nullptr)
        co_return true;

#if defined DEBUG || defined _DEBUG
    this->_attack_thread->load(model.script);
#endif
    this->_attack_thread->func(model.on_attack);
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

    return this->alive();
}

uint32_t mob::normal_attack_damage(MOB_SIZE size) const
{
    this->assert_thread();

    auto& model      = this->based<fb::model::mob>();
    auto  difference = model.damage.max - model.damage.min;
    return model.damage.min + (std::rand() % difference);
}

void mob::on_die(std::shared_ptr<object> from, DESTROY_TYPE destroy_type)
{
    this->assert_thread();
    this->listener.on_dead(*this, from);

    // Drop items when mob dies
    std::ignore = this->drop_items();

    // Handle spawned mob ownership
    auto owner = this->owner.lock();
    if (owner != nullptr)
    {
        owner->detach_spawned_mob(*this);
        return;
    }

    // Handle experience distribution
    if (from != nullptr && from->is(OBJECT_TYPE::MOB))
        from = std::static_pointer_cast<fb::game::mob>(from)->owner.lock();

    if (from == nullptr)
        return;

    if (owner == nullptr && from->is(OBJECT_TYPE::CHARACTER))
    {
        auto& ch       = static_cast<character&>(*from);
        auto& group_id = ch.group_id();
        auto  map      = ch.map();
        auto  exp      = this->based<fb::model::mob>().exp;

        if (group_id.has_value() && map != nullptr)
        {
            // Group experience distribution
            auto server = &ch.server;
            {
                auto  guard      = server->groups.enter_read(group_id.value());
                auto& group      = guard.value();
                auto  nears      = group->nears(*map, ch.position());
                auto  size       = nears.size();
                auto  divide_exp = exp / size;
                for (auto& member : nears)
                {
                    auto shared_ptr = member.lock();
                    if (shared_ptr == nullptr)
                        continue;

                    shared_ptr->add_exp(divide_exp, true, true);
                }
            }
        }
        else
        {
            // Solo experience
            ch.add_exp(exp, true, true);
        }
    }
    std::ignore = this->destroy(destroy_type);
}

void mob::kill(std::shared_ptr<object> from, DESTROY_TYPE destroy_type)
{
    this->assert_thread();
    life::kill(from, destroy_type);

    auto& model = this->based<fb::model::mob>();
    if (model.script.empty() == false && model.on_die.empty() == false)
    {
        auto lua = fb::lua::new_context();
        if (lua != nullptr)
        {
#if defined DEBUG || defined _DEBUG
            lua->load(model.script);
#endif
            lua->func(model.on_die);
            lua->pushobject(*this);
            if (from != nullptr)
                lua->pushobject(from);
            else
                lua->pushnil();

            this->invincible(true);
            async::awaitable_then(lua->call(2), [this, from, destroy_type](async::awaitable_result<bool> result) {
                try
                {
                    result();
                    this->on_die(from, destroy_type);
                }
                catch (std::exception& e)
                {
                    fb::logger::fatal("error in mob on_die: {}", e.what());
                }
                catch (...)
                {
                    fb::logger::fatal("unknown error in mob on_die");
                }
            });
        }
    }
    else
    {
        this->on_die(from, destroy_type);
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
    if (owner == nullptr && !model.drop.empty())
    {
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

    if (!oids.empty())
        this->map()->bulk_update(oids);
}

void mob::assert_thread() const
{
    if (this->_map != nullptr)
        object::assert_thread();
    else
        return;
}

bool mob::move(DIRECTION direction)
{
    this->assert_thread();
    auto map = this->map();
    if (map == nullptr)
        return false;

    auto position = this->side_position(direction);
    for (auto& obj : map->nears(position, OBJECT_TYPE::LIFE))
    {
        if (obj.get() == this)
            continue;

        auto life = std::static_pointer_cast<fb::game::life>(obj);
        if (life->cover() == false)
            continue;

        const auto& life_position = life->position();
        if (life_position.x > 0 && life_position.x - 1 == position.x && life_position.y == position.y)
            return false;

        if (life_position.x < map->width() - 1 && life_position.x + 1 == position.x && life_position.y == position.y)
            return false;

        if (life_position.y > 0 && life_position.y - 1 == position.y && life_position.x == position.x)
            return false;

        if (life_position.y < map->height() - 1 && life_position.y + 1 == position.y && life_position.x == position.x)
            return false;
    }

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
    this->_hidden = enabled;
}

std::shared_ptr<fb::game::appearance> mob::appearance() const
{
    return this->based<fb::model::mob>().create_appearance();
}