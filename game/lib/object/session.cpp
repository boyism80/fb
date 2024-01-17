#include <fb/game/session.h>
#include <fb/game/context.h>
#include <fb/game/model.h>

using namespace fb::game;

session::session(fb::socket<fb::game::session>& socket, fb::game::context& context) : 
    life(context, nullptr, fb::game::life::config { { .id = (uint32_t)socket.fd()} }),
    _socket(socket)
{ }

session::~session()
{ }

void fb::game::session::send(const fb::ostream& stream, bool encrypt, bool wrap)
{
    if (this->inited() == false)
        return;

    this->_socket.send(stream, encrypt, wrap);
}

void fb::game::session::send(const fb::protocol::base::header& response, bool encrypt, bool wrap)
{
    if (this->inited() == false)
        return;
    
    this->_socket.send(response, encrypt, wrap);
}

object::types fb::game::session::type() const
{
    return object::types::SESSION;
}

fb::awaitable<bool> fb::game::session::co_map(fb::game::map* map, const point16_t& position)
{
    if (this->_map_lock)
        return fb::awaitable<bool>([this, map, position](auto& awaitable) { return false; });
    
    auto switch_process = (map != nullptr && map->active == false);
    if(switch_process)
    {
        return fb::awaitable<bool>([this, map, position](auto& awaitable)
        {
            auto listener = this->get_listener<fb::game::session>();
            listener->on_transfer(*this, *map, position, &awaitable);
        });
    }
    else
    {
        return fb::game::object::co_map(map, position);
    }
}

fb::awaitable<bool> fb::game::session::co_map(fb::game::map* map)
{
    return this->co_map(map, point16_t(0, 0));
}

bool fb::game::session::map(fb::game::map* map, const point16_t& position)
{
    if(this->_map_lock)
        return false;

    auto switch_process = (map != nullptr && map->active == false);
    if(switch_process)
    {
        auto listener = this->get_listener<fb::game::session>();
        listener->on_transfer(*this, *map, position);
        return true;
    }
    else
    {
        return fb::game::object::map(map, position);
    }
}

bool fb::game::session::map(fb::game::map* map)
{
    return this->map(map, point16_t(0, 0));
}

void fb::game::session::on_hold()
{
    auto listener = this->get_listener<fb::game::session>();
    listener->on_hold(*this);
}

void fb::game::session::on_update()
{
    auto listener = this->get_listener<fb::game::session>();
    listener->on_updated(*this, fb::game::STATE_LEVEL::LEVEL_MIDDLE);
}

uint32_t fb::game::session::on_calculate_damage(bool critical) const
{
    auto                    weapon = this->items.weapon();
    auto                    model = weapon != nullptr ? weapon->based<fb::game::weapon>() : nullptr;

    if(weapon == nullptr) // no weapon
    {
        return 1 + std::rand() % 5;
    }
    else if(critical)
    {
        auto                range = model->damage_range.large;
        return std::max(uint32_t(1), range.min) + std::rand() % std::max(uint32_t(1), range.max);
    }
    else // normal
    {
        auto                range = model->damage_range.small;
        return std::max(uint32_t(1), range.min) + std::rand() % std::max(uint32_t(1), range.max);
    }
}

void fb::game::session::on_attack(fb::game::object* you)
{
    fb::game::life::on_attack(you);

    auto thread = fb::game::lua::get();
    if(thread == nullptr)
        return;

    thread->from("scripts/common/attack.lua")
          .func("on_attack")
          .pushobject(*this);
    if(you != nullptr)
        thread->pushobject(*you);
    else
        thread->pushnil();
    thread->resume(2);

    auto listener = this->get_listener<fb::game::session>();
    listener->on_attack(*this, you);
}

void fb::game::session::on_hit(fb::game::life& you, uint32_t damage, bool critical)
{
    fb::game::life::on_hit(you, damage, critical);

    auto listener = this->get_listener<fb::game::session>();
    listener->on_hit(*this, you, damage, critical);
}

void fb::game::session::on_kill(fb::game::life& you)
{
    fb::game::life::on_kill(you);

    auto exp = you.on_exp();
    if(exp > 0)
    {
        auto                    range = fb::game::model::classes.exp(this->cls(), this->level());
#if defined DEBUG | defined _DEBUG
        exp *= 100;
#else
        if (this->max_level() == false)
            exp = std::min(uint32_t(range / 100.0f * 3.3f + 1), exp);
#endif
        this->experience_add(exp, true);
    }

    auto listener = this->get_listener<fb::game::session>();
    listener->on_kill(*this, you);
}

void fb::game::session::on_damaged(fb::game::object* from, uint32_t damage, bool critical)
{
    fb::game::life::on_damaged(from, damage, critical);

    auto listener = this->get_listener<fb::game::session>();
    listener->on_damaged(*this, from, damage, critical);
}

void fb::game::session::on_die(fb::game::object* from)
{
    fb::game::life::on_die(from);

    this->state(STATE_TYPE::GHOST);

    auto listener = this->get_listener<fb::game::session>();
    listener->on_die(*this, from);
}

fb::game::session::operator fb::socket<fb::game::session>& ()
{
    return this->_socket;
}

bool fb::game::session::inited() const
{
    return this->_init;
}

void fb::game::session::init(bool value)
{
    this->_init = value;
}

uint32_t fb::game::session::id() const
{
    return this->_id;
}

void fb::game::session::id(uint32_t id)
{
    this->_id = id;
}

uint32_t fb::game::session::fd()
{
    return this->_socket.fd();
}

bool fb::game::session::admin() const
{
    return this->_admin;
}

void fb::game::session::admin(bool value)
{
    this->_admin = value;
}

void fb::game::session::attack()
{
    auto listener = this->get_listener<fb::game::session>();

    try
    {
        this->assert_state({STATE_TYPE::RIDING, STATE_TYPE::GHOST});
        fb::game::life::attack();
    }
    catch(std::exception& e)
    {
        listener->on_notify(*this, e.what());
    }
}

void fb::game::session::action(fb::game::ACTION_TYPE action, fb::game::DURATION duration, uint8_t sound)
{
    auto listener = this->get_listener<fb::game::session>();

    try
    {
        this->assert_state({STATE_TYPE::GHOST, STATE_TYPE::RIDING});
        listener->on_action(*this, action, duration, sound);
    }
    catch(std::exception& e)
    {
        listener->on_notify(*this, e.what());
    }
}

const std::string& fb::game::session::name() const
{
    return this->_name;
}

void fb::game::session::name(const std::string& value)
{
    this->_name = value;
}

uint16_t fb::game::session::look() const
{
    return this->_look;
}

void fb::game::session::look(uint16_t value)
{
    auto listener = this->get_listener<fb::game::session>();

    this->_look = value;
    listener->on_show(*this, false);
}

uint8_t fb::game::session::color() const
{
    return this->_color;
}

void fb::game::session::color(uint8_t value)
{
    auto listener = this->get_listener<fb::game::session>();

    this->_color = value;
    listener->on_show(*this, false);
}

std::optional<uint8_t> fb::game::session::armor_color() const
{
    return this->_armor_color;
}

void fb::game::session::armor_color(std::optional<uint8_t> value)
{
    auto listener = this->get_listener<fb::game::session>();

    this->_armor_color = value;
    listener->on_show(*this, false);
}

uint8_t fb::game::session::current_armor_color() const
{
    auto armor = this->items.armor();
    return this->_armor_color.value_or(armor != nullptr ? armor->color() : 0x00);
}

std::optional<uint16_t> fb::game::session::disguise() const
{
    return this->_disguise;
}

void fb::game::session::disguise(uint16_t value)
{
    auto listener = this->get_listener<fb::game::session>();

    this->_disguise = value;
    this->state(STATE_TYPE::DISGUISE);
    listener->on_updated(*this, STATE_LEVEL::LEVEL_MAX);
}

void fb::game::session::undisguise()
{
    auto listener = this->get_listener<fb::game::session>();

    this->_disguise = std::nullopt;
    if(this->state() == fb::game::STATE_TYPE::DISGUISE)
        this->state(STATE_TYPE::NORMAL);

    listener->on_updated(*this, STATE_LEVEL::LEVEL_MAX);
}

uint32_t fb::game::session::defensive_physical() const
{
    return this->_defensive.physical;
}

void fb::game::session::defensive_physical(uint8_t value)
{
    this->_defensive.physical = value;
}

uint32_t fb::game::session::defensive_magical() const
{
    return this->_defensive.magical;
}

void fb::game::session::defensive_magical(uint8_t value)
{
    this->_defensive.magical = value;
}

void fb::game::session::base_hp_up(uint32_t value)
{
    this->_base_hp += value;
}

void fb::game::session::base_mp_up(uint32_t value)
{
    this->_base_mp += value;
}

void fb::game::session::base_hp(uint32_t value)
{
    this->_base_hp = value;
    this->_hp = std::min(this->_hp, this->_base_hp);
}

void fb::game::session::base_mp(uint32_t value)
{
    this->_base_mp = value;
    this->_mp = std::min(this->_mp, this->_base_mp);
}

uint32_t fb::game::session::base_hp() const
{
    return this->_base_hp;
}

uint32_t fb::game::session::base_mp() const
{
    return this->_base_mp;
}

fb::game::NATION_TYPE fb::game::session::nation() const
{
    return this->_nation;
}

bool fb::game::session::nation(fb::game::NATION_TYPE value)
{
    if(value != fb::game::NATION_TYPE::GOGURYEO &&
       value != fb::game::NATION_TYPE::BUYEO)
        return false;

    this->_nation = value;
    return true;
}

fb::game::CREATURE_TYPE fb::game::session::creature() const
{
    return this->_creature;
}

bool fb::game::session::creature(fb::game::CREATURE_TYPE value)
{
    if(value != fb::game::CREATURE_TYPE::DRAGON  &&
       value != fb::game::CREATURE_TYPE::PHOENIX &&
       value != fb::game::CREATURE_TYPE::TIGER   &&
       value != fb::game::CREATURE_TYPE::TURTLE)
        return false;

    this->_creature = value;
    return true;
}

uint8_t fb::game::session::level() const
{
    return this->_level;
}

void fb::game::session::level(uint8_t value)
{
    auto listener = this->get_listener<fb::game::session>();

    this->_level = value;
    listener->on_updated(*this, STATE_LEVEL::LEVEL_MAX);
}

bool fb::game::session::level_up()
{
    if(this->max_level())
        return false;

    if(this->_class == 0 && this->_level >= 5)
        return false;

    this->strength_up(fb::game::model::classes[this->_class]->abilities[this->_level]->strength);
    this->intelligence_up(fb::game::model::classes[this->_class]->abilities[this->_level]->intelligence);
    this->dexteritry_up(fb::game::model::classes[this->_class]->abilities[this->_level]->dexteritry);

    this->base_hp_up(fb::game::model::classes[this->_class]->abilities[this->_level]->base_hp + std::rand() % 10);
    this->base_mp_up(fb::game::model::classes[this->_class]->abilities[this->_level]->base_mp + std::rand() % 10);

    this->hp(this->base_hp());
    this->mp(this->base_mp());

    this->level(this->_level + 1);

    auto listener = this->get_listener<fb::game::session>();
    listener->on_level_up(*this);
    listener->on_notify(*this, message::level::UP);

    return true;
}

bool fb::game::session::max_level() const
{
    return this->_level >= 99;
}

fb::game::SEX_TYPE fb::game::session::sex() const
{
    return this->_sex;
}

void fb::game::session::sex(fb::game::SEX_TYPE value)
{
    this->_sex = value;
}

fb::game::STATE_TYPE fb::game::session::state() const
{
    return this->_state;
}

void fb::game::session::state(fb::game::STATE_TYPE value)
{
    if(this->_state == value)
        return;

    this->_state = value;

    auto listener = this->get_listener<fb::game::session>();
    listener->on_show(*this, false);
}

uint8_t fb::game::session::cls() const
{
    return this->_class;
}

void fb::game::session::cls(uint8_t value)
{
    this->_class = value;
}

uint8_t fb::game::session::promotion() const
{
    return uint8_t();
}

void fb::game::session::promotion(uint8_t value)
{
    this->_promotion = value;
}

uint8_t fb::game::session::strength() const
{
    return this->_strength;
}

void fb::game::session::strength(uint8_t value)
{
    this->_strength = value;
}

void fb::game::session::strength_up(uint8_t value)
{
    this->_strength += value;
}

uint8_t fb::game::session::intelligence() const
{
    return this->_intelligence;
}

void fb::game::session::intelligence(uint8_t value)
{
    this->_intelligence = value;
}

void fb::game::session::intelligence_up(uint8_t value)
{
    this->_intelligence += value;
}

uint8_t fb::game::session::dexteritry() const
{
    return this->_dexteritry;
}

void fb::game::session::dexteritry(uint8_t value)
{
    this->_dexteritry = value;
}

void fb::game::session::dexteritry_up(uint8_t value)
{
    this->_dexteritry += value;
}

uint32_t fb::game::session::experience() const
{
    return this->_experience;
}

void fb::game::session::experience(uint32_t value)
{
    if(this->_experience == value)
        return;

    this->_experience = value;

    auto listener = this->get_listener<fb::game::session>();
    listener->on_updated(*this, STATE_LEVEL::LEVEL_MIN);
}

uint32_t fb::game::session::experience_add(uint32_t value, bool notify)
{
    auto capacity = 0xFFFFFFFF - this->_experience;
    auto lack = 0;

    auto listener = this->get_listener<fb::game::session>();

    try
    {
        // 직업이 없는 경우 정확히 5레벨을 찍을 경험치만 얻도록 제한
        if(this->_class == 0)
        {
            auto require = fb::game::model::classes.exp(0, 5 - 1);
            if(this->_experience > require)
                value = 0;

            if(this->_experience + value > require)
                value = require - this->_experience;
        }

        if(value > 0)
        {
            if(value > capacity)
            {
                lack = value - capacity;
                this->experience(this->_experience + capacity);
            }
            else
            {
                this->experience(this->_experience + value);
            }

            if(notify)
            {
                std::stringstream       sstream;
                sstream << "경험치가 " << value << '(' << int(this->experience_percent()) << "%) 올랐습니다.";
                listener->on_notify(*this, sstream.str());
            }
        }

        while(true)
        {
            if(this->max_level())
                break;

            auto require = fb::game::model::classes.exp(this->_class, this->_level);
            if(require == 0)
                break;

            if(this->_experience < require)
                break;

            if(this->level_up() == false)
                break;
        }

        if(this->_class == 0 && this->_level >= 5)
            throw require_class_exception();
    }
    catch(std::exception& e)
    {
        if(notify)
            listener->on_notify(*this, e.what());
    }

    return lack;
}

uint32_t fb::game::session::experience_reduce(uint32_t value)
{
    if(this->_experience < value)
    {
        uint32_t lack = value - this->_experience;
        this->_experience = 0;
        return lack;
    }
    else
    {
        this->_experience -= value;
        return 0;
    }
}

uint32_t fb::game::session::experience_remained() const
{
    if(this->max_level())
        return 0;

    if(this->_class == 0 && this->_level >= 5)
        return 0;

    return fb::game::model::classes.exp(this->_class, this->_level) - this->experience();
}

float fb::game::session::experience_percent() const
{
    auto                    current_level = this->level();
    auto                    next_exp = this->max_level() ? 0xFFFFFFFF : fb::game::model::classes.exp(this->cls(), current_level);
    auto                    prev_exp = current_level > 1 ? 
                                       (this->max_level() ? 0x00000000 : fb::game::model::classes.exp(this->cls(), current_level - 1)) : 0;
    auto                    exp_range = next_exp - prev_exp;

    return std::min(100.0f, ((this->_experience - prev_exp) / float(exp_range)) * 100.0f);
}

uint32_t fb::game::session::money() const
{
    return this->_money;
}

void fb::game::session::money(uint32_t value)
{
    auto listener = this->get_listener<fb::game::session>();

    this->_money = value;
    listener->on_updated(*this, STATE_LEVEL::LEVEL_MIN);
    listener->on_money_changed(*this, value);
}

uint32_t fb::game::session::money_add(uint32_t value) // 먹고 남은 값 리턴
{
    uint32_t capacity = 0xFFFFFFFF - this->_money;
    uint32_t lack = 0;
    if(value > capacity)
    {
        this->money(this->_money + capacity);
        lack = value - capacity;
    }
    else
    {
        this->money(this->_money + value);
    }

    return lack;
}

uint32_t fb::game::session::money_reduce(uint32_t value)
{
    uint32_t lack = 0;
    if(this->_money < value)
    {
        lack = value - this->_money;
        this->money(0);
    }
    else
    {
        this->money(this->_money - value);
    }

    return lack;
}

uint32_t fb::game::session::money_drop(uint32_t value)
{
    auto listener = this->get_listener<fb::game::session>();

    try
    {
        if(value == 0)
            return 0;

        this->assert_state({STATE_TYPE::RIDING, STATE_TYPE::GHOST});


        auto lack = this->money_reduce(value);

        auto cash = new fb::game::cash(this->context, value);
        cash->map(this->_map, this->_position);

        this->action(ACTION_TYPE::PICKUP, DURATION::PICKUP);
        listener->on_notify(*this, message::money::DROP);

        return lack;
    }
    catch(std::exception& e)
    {
        listener->on_notify(*this, e.what());
        return 0;
    }
}

uint32_t fb::game::session::damage() const
{
    return this->_damage;
}

void fb::game::session::damage(uint8_t value)
{
    this->_damage = value;
}

uint32_t fb::game::session::hit() const
{
    return this->_hit;
}

void fb::game::session::hit(uint8_t value)
{
    this->_hit = value;
}

uint32_t fb::game::session::regenerative() const
{
    return this->_regenerative;
}

void fb::game::session::regenerative(uint8_t value)
{
    this->_regenerative = value;
}

bool fb::game::session::option(OPTION key) const
{
    if(static_cast<int>(key) > 0x1B)
        return false;

    return this->_options[static_cast<int>(key)];
}

void fb::game::session::option(OPTION key, bool value)
{
    if(static_cast<int>(key) > 0x1B)
        return;

    if(this->_options[static_cast<int>(key)] == value)
        return;

    auto listener = this->get_listener<fb::game::session>();
    listener->on_updated(*this, STATE_LEVEL::LEVEL_MIN);
    listener->on_option(*this, key, value);

    this->_options[static_cast<int>(key)] = value;
}

bool fb::game::session::option_toggle(OPTION key)
{
    if(static_cast<int>(key) > 0x1B)
        return false;

    this->option(key, !this->_options[static_cast<int>(key)]);
    return this->_options[static_cast<int>(key)];
}

const std::string& fb::game::session::title() const
{
    return this->_title;
}

void fb::game::session::title(const std::string& value)
{
    this->_title = value;
}

fb::game::group* fb::game::session::group() const
{
    return this->_group;
}

fb::game::clan* fb::game::session::clan() const
{
    return this->_clan;
}

void fb::game::session::assert_state(fb::game::STATE_TYPE value) const
{
    static const auto pairs = std::map<fb::game::STATE_TYPE, const std::runtime_error>
    {
        {STATE_TYPE::GHOST, ghost_exception()},
        {STATE_TYPE::RIDING, ridding_exception()},
        {STATE_TYPE::DISGUISE, disguise_exception()}
    };

    if(this->_state == value)
        throw pairs.at(value);
}

void fb::game::session::assert_state(const std::vector<fb::game::STATE_TYPE>& values) const
{
    for(auto value : values)
        this->assert_state(value);
}

bool fb::game::session::move(const point16_t& before)
{
    return this->move(this->_direction, before);
}

bool fb::game::session::move(fb::game::DIRECTION_TYPE direction, const point16_t& before)
{
    auto listener = this->get_listener<fb::game::session>();

    if(this->_position != before)
    {
        listener->on_hold(*this);
        return false;
    }
    else if(object::move(direction) == false)
    {
        listener->on_hold(*this);
        return false;
    }
    else
    {
        return true;
    }
}

void fb::game::session::ride(fb::game::mob& horse)
{
    auto listener = this->get_listener<fb::game::session>();

    try
    {
        this->assert_state({STATE_TYPE::GHOST, STATE_TYPE::DISGUISE});

        if(this->state() == fb::game::STATE_TYPE::RIDING)
            throw std::runtime_error(message::ride::ALREADY_RIDE);

        if(horse.based<fb::game::mob>() != fb::game::model::mobs.name2mob("말"))
            throw session::no_conveyance_exception();

        if(horse.map() != this->_map)
            throw std::runtime_error(message::error::UNKNOWN);

        horse.map(nullptr);
        this->state(STATE_TYPE::RIDING);
        horse.kill();
        horse.dead_time(std::chrono::duration_cast<std::chrono::milliseconds>(fb::thread::now()));
        listener->on_notify(*this, message::ride::ON);
    }
    catch(std::exception& e)
    {
        listener->on_notify(*this, e.what());
    }
}

void fb::game::session::ride()
{
    auto listener = this->get_listener<fb::game::session>();

    try
    {
        this->assert_state({STATE_TYPE::GHOST, STATE_TYPE::DISGUISE});

        auto front = this->forward(object::types::MOB);
        if(front == nullptr)
            throw session::no_conveyance_exception();

        this->ride(static_cast<fb::game::mob&>(*front));
    }
    catch(std::exception& e)
    {
        listener->on_notify(*this, e.what());
    }
}

void fb::game::session::unride()
{
    auto listener = this->get_listener<fb::game::session>();

    try
    {
        this->assert_state({STATE_TYPE::GHOST, STATE_TYPE::DISGUISE});
        if(this->state() != fb::game::STATE_TYPE::RIDING)
            throw std::runtime_error(message::ride::UNRIDE);

        auto model = fb::game::model::mobs.name2mob("말");
        auto horse = this->context.make<fb::game::mob>(model, fb::game::mob::config { .alive = true });
        horse->map(this->_map, this->position_forward());
        
        this->state(STATE_TYPE::NORMAL);
        listener->on_notify(*this, message::ride::OFF);
    }
    catch(std::exception& e)
    {
        listener->on_notify(*this, e.what());
    }
}

bool fb::game::session::alive() const
{
    return this->_state != STATE_TYPE::GHOST;
}

void fb::game::session::refresh_map()
{
    if(this->_map == nullptr)
        return;

    auto listener = this->get_listener<fb::game::session>();
    listener->on_map_changed(*this, this->_map, this->_map);
}

fb::game::session::container::container()
{ }

fb::game::session::container::container(const std::vector<fb::game::session*>& right)
{
    this->insert(this->begin(), right.begin(), right.end());
}

fb::game::session::container::~container()
{ }

session::container& fb::game::session::container::push(fb::game::session& session)
{
    this->push_back(&session);
    return *this;
}

session::container& fb::game::session::container::erase(fb::game::session& session)
{
    std::vector<fb::game::session*>::erase(std::find(this->begin(), this->end(), &session));
    return *this;
}

fb::game::session* fb::game::session::container::find(const std::string& name)
{
    auto i = std::find_if
    (
        this->begin(), this->end(), 
        [&name] (auto x) 
        { 
            return x->name() == name; 
        }
    );
    return i != this->end() ? *i : nullptr;
}

bool fb::game::session::container::contains(const fb::game::session& session) const
{
    return std::find(this->cbegin(), this->cend(), &session) != this->end();
}

fb::game::session* fb::game::session::container::operator[](const std::string& name)
{
    return this->find(name);
}