#include <fb/game/character.h>
#include <fb/game/context.h>
#include <fb/thread.h>
#include <fb/game/map.h>
#include <fb/game/mob.h>
#include <fb/game/ai.h>

using namespace fb::game;

rezen::rezen(context& context, const fb::model::mob_spawn& model) :
    _context(context),
    model(model)
{
    this->_respawn_time = fb::model::datetime();
}

void rezen::decrease()
{
    auto now = fb::model::datetime();

    if (!this->_respawn_time.has_value())
        this->_respawn_time = now + this->model.rezen;
    this->_count = std::max(0, this->_count - 1);
}

async::task<void> rezen::spawn(std::thread::id thread_id)
{
    if (this->_context.maps.contains(this->model.parent) == false)
        co_return;

    auto map = this->_context.maps[this->model.parent];
    if (map->active == false)
        co_return;

    if (map->is_active() == false)
        co_return;

    auto thread = this->_context.threads.at(thread_id);
    if (thread == nullptr || thread->id() != thread_id)
        co_return;

    auto now = fb::model::datetime();
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
        auto mob = this->_context.make<fb::game::mob>(this->_context.model.mob[this->model.mob],
                                                      mob::initial_params{.alive = true, .rezen = this});

        mob->direction(DIRECTION(std::rand() % 4));
        mob->heal(mob->base_hp());

        while (true)
        {
            auto width    = this->model.end.x - this->model.begin.x;
            auto height   = this->model.end.y - this->model.begin.y;
            auto map      = this->_context.maps[this->model.parent];
            auto position = fb::model::point16_t(this->model.begin.x + (width > 0 ? std::rand() % width : 0),
                                                 this->model.begin.y + (height > 0 ? std::rand() % height : 0));

            if (position.x > map->width() - 1 || position.y > map->height() - 1)
                continue;

            if (map->blocked(position.x, position.y))
                continue;

            mob->position(position, true);
            std::ignore = co_await mob->map(map, position, DESTROY_TYPE::DEFAULT, false);
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
    this->_respawn_time = fb::model::datetime();
}

mob::mob(fb::game::context& context, const fb::model::mob& model, const initial_params& params) :
    life(context, model, params),
    listener(context.listener),
    _rezen(params.rezen),
    owner(params.owner != nullptr ? params.owner->weak_from_this_as<character>() : std::weak_ptr<character>())
{
    // Initialize AI strategy based on mob's attack type
    this->_ai_strategy = ai::create(model.attack_type);

    this->hidden(!params.alive);
    if (params.alive)
    {
        this->heal(this->base_hp());
        this->mp_up(this->base_mp());
    }
}

mob::mob(const mob& right) :
    life(right),
    listener(right.listener),
    _action_time(right._action_time),
    _target(right._target)
{ }

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

#if defined DEBUG | defined _DEBUG
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

    auto& ctx   = this->context;
    auto  weak  = this->weak_from_this();
    std::ignore = co_await this->_attack_thread->call(2);

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
    for (auto x : this->sight_in(OBJECT_TYPE::CHARACTER))
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

    if (ENUM_IN(this->crowd_control(), CROWD_CONTROL::SIGHT))
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

uint32_t mob::damage(uint32_t value, std::shared_ptr<object> from, bool critical)
{
    this->assert_thread();

    auto result = life::damage(value, from, critical);
    if (!this->alive())
    {
        this->kill(from, DESTROY_TYPE::DEAD);
        return result;
    }

    // Handle damage in AI strategy
    if (this->_ai_strategy && from && from->is(OBJECT_TYPE::LIFE))
    {
        this->_ai_strategy->on_damage(*this, std::static_pointer_cast<life>(from), fb::model::datetime());
    }

    return result;
}

uint32_t mob::auto_attack_damage(MOB_SIZE size) const
{
    this->assert_thread();

    auto& model      = this->based<fb::model::mob>();
    auto  difference = model.damage.max - model.damage.min;
    return model.damage.min + (std::rand() % difference);
}

void mob::kill(std::shared_ptr<object> from, DESTROY_TYPE destroy_type)
{
    life::kill(from, destroy_type);
    std::ignore = this->destroy(destroy_type);
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
        co_await item->map(map, position, DESTROY_TYPE::DEFAULT, false);
        oids.push_back(item->oid());
    }
    this->_items.clear();

    auto owner = this->owner.lock();
    if (owner == nullptr && !model.drop.empty())
    {
        auto& drop = this->context.model.drop[model.drop];
        for (auto& dsl : drop.dsl)
        {
            switch (dsl.header)
            {
            case DSL::item:
            {
                auto params = fb::model::dsl::item(dsl.params);
                auto random = std::rand() % 100;
                if (random > (int)params.percent)
                    continue;

                // Use smart pointer for item creation
                auto item_shared = this->context.model.item[params.id].make(this->context);
                auto item        = item_shared.get(); // For compatibility with existing map system
                co_await item->map(map, position, DESTROY_TYPE::DEFAULT, false);
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
    return this->_items;
}

bool mob::push_item(std::shared_ptr<fb::game::item> item)
{
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
    this->_hidden = enabled;
}

uint32_t mob::base_hp() const
{
    auto& model = this->based<fb::model::mob>();
    return model.hp;
}

uint32_t mob::buff_hp() const
{
    return this->_buff_hp;
}

void mob::buff_hp(uint32_t value)
{
    this->_buff_hp = value;
}

uint32_t mob::base_mp() const
{
    auto& model = this->based<fb::model::mob>();
    return model.mp;
}

uint32_t mob::buff_mp() const
{
    return this->_buff_mp;
}

void mob::buff_mp(uint32_t value)
{
    this->_buff_mp = value;
}

uint8_t mob::base_str() const
{
    // auto& model = this->based<fb::model::mob>();
    // return model.str;
    return 0;
}

uint8_t mob::buff_str() const
{
    return this->_buff_str;
}

void mob::buff_str(uint8_t value)
{
    this->_buff_str = value;
}

uint8_t mob::base_dex() const
{
    // auto& model = this->based<fb::model::mob>();
    // return model.dex;
    return 0;
}

uint8_t mob::buff_dex() const
{
    return this->_buff_dex;
}

void mob::buff_dex(uint8_t value)
{
    this->_buff_dex = value;
}

uint8_t mob::base_int() const
{
    // auto& model = this->based<fb::model::mob>();
    // return model.int;
    return 0;
}

uint8_t mob::buff_int() const
{
    return this->_buff_int;
}

void mob::buff_int(uint8_t value)
{
    this->_buff_int = value;
}

int8_t mob::base_phydef() const
{
    auto& model = this->based<fb::model::mob>();
    return model.defensive_physical;
}

int8_t mob::buff_phydef() const
{
    return this->_buff_phydef;
}

void mob::buff_phydef(int8_t value)
{
    this->_buff_phydef = value;
}

int8_t mob::base_magdef() const
{
    auto& model = this->based<fb::model::mob>();
    return model.defensive_magical;
}

int8_t mob::buff_magdef() const
{
    return this->_buff_magdef;
}

void mob::buff_magdef(int8_t value)
{
    this->_buff_magdef = value;
}

uint8_t mob::base_dam() const
{
    // auto& model = this->based<fb::model::mob>();
    // return model.dam;
    return 0;
}

uint8_t mob::buff_dam() const
{
    return this->_buff_dam;
}

void mob::buff_dam(uint8_t value)
{
    this->_buff_dam = value;
}

uint8_t mob::base_hit() const
{
    // auto& model = this->based<fb::model::mob>();
    // return model.hit;
    return 0;
}

uint8_t mob::buff_hit() const
{
    return this->_buff_hit;
}

void mob::buff_hit(uint8_t value)
{
    this->_buff_hit = value;
}
