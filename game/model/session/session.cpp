#include "model/session/session.h"
#include "model/map/map.h"
#include "model/item/item.h"
#include "model/group/group.h"
#include "model/clan/clan.h"
#include "model/acceptor/acceptor.game.h"
#include "model/table/table.game.h"

using namespace fb::game;

session::session(fb::socket<fb::game::session>& socket, listener* listener) : 
    life((life::master*)nullptr, listener, (uint32_t)socket.native_handle(), 0, 0, 0),
    _id(0xFFFFFFFF),
    _socket(socket),
    _look(0), _color(0), _armor_color(0),
    _disguise(0),
    _defensive(0, 0), _base_hp(0), _base_mp(0), _experience(0),
    _nation(nation::GOGURYEO),
    _creature(creature::DRAGON),
    _state(state::NORMAL),
    _level(1),
    _class(0),
    _promotion(0),
    _money(0),
    _group(nullptr),
    _clan(nullptr),
    trade(*this),
    items(*this),
    dialog(*this)
{
    memset(this->_options, 0, sizeof(this->_options));
}

session::~session()
{
    if(this->_master != nullptr)
        delete this->_master;
}

void fb::game::session::send(const fb::ostream& stream, bool encrypt, bool wrap)
{
    this->_socket.send(stream, encrypt, wrap);
}

void fb::game::session::send(const fb::protocol::base::header& response, bool encrypt, bool wrap)
{
    this->_socket.send(response, encrypt, wrap);
}

object::types fb::game::session::type() const
{
    return object::types::SESSION;
}

void fb::game::session::handle_hold()
{
    auto listener = this->get_listener<fb::game::session::listener>();
    if(listener != nullptr)
        listener->on_hold(*this);
}

void fb::game::session::handle_switch_process(fb::game::map& map, const point16_t& position)
{
    auto listener = this->get_listener<fb::game::session::listener>();
    if(listener != nullptr)
        listener->on_transfer(*this, map, position);
}

void fb::game::session::handle_warp()
{
    auto listener = this->get_listener<fb::game::session::listener>();
    if(listener != nullptr)
        listener->on_warp(*this);
}

void fb::game::session::handle_update()
{
    auto listener = this->get_listener<fb::game::session::listener>();
    if(listener != nullptr)
        listener->on_updated(*this, fb::game::state_level::LEVEL_MIDDLE);
}

uint32_t fb::game::session::handle_calculate_damage(bool critical) const
{
    auto                    weapon = this->items.weapon();

    if(weapon == nullptr) // no weapon
    {
        return 1 + std::rand() % 5;
    }
    else if(critical)
    {
        auto                range = weapon->damage_large();
        return std::max(uint32_t(1), range.min) + std::rand() % std::max(uint32_t(1), range.max);
    }
    else // normal
    {
        auto                range = weapon->damage_small();
        return std::max(uint32_t(1), range.min) + std::rand() % std::max(uint32_t(1), range.max);
    }
}

void fb::game::session::handle_attack(fb::game::object* you)
{
    fb::game::life::handle_attack(you);

    auto listener = this->get_listener<fb::game::session::listener>();
    if(listener != nullptr)
        listener->on_attack(*this, you);
}

void fb::game::session::handle_hit(fb::game::life& you, uint32_t damage, bool critical)
{
    fb::game::life::handle_hit(you, damage, critical);

    auto listener = this->get_listener<fb::game::session::listener>();
    if(listener != nullptr)
        listener->on_hit(*this, you, damage, critical);
}

void fb::game::session::handle_kill(fb::game::life& you)
{
    fb::game::life::handle_kill(you);

    auto listener = this->get_listener<fb::game::session::listener>();
    if(listener != nullptr)
        listener->on_kill(*this, you);
}

void fb::game::session::handle_damaged(fb::game::object* from, uint32_t damage, bool critical)
{
    fb::game::life::handle_damaged(from, damage, critical);

    auto listener = this->get_listener<fb::game::session::listener>();
    if(listener != nullptr)
        listener->on_damaged(*this, from, damage, critical);
}

void fb::game::session::handle_die(fb::game::object* from)
{
    fb::game::life::handle_die(from);

    auto listener = this->get_listener<fb::game::session::listener>();
    if(listener != nullptr)
        listener->on_die(*this, from);
}


fb::game::session::operator fb::socket<fb::game::session>& ()
{
    return this->_socket;
}
uint32_t fb::game::session::id() const
{
    return this->_id;
}

void fb::game::session::id(uint32_t id)
{
    this->_id = id;
}

void fb::game::session::attack()
{
    auto listener = this->get_listener<fb::game::session::listener>();

    try
    {
        this->assert_state({state::RIDING, state::GHOST});
        fb::game::life::attack();
    }
    catch(std::exception& e)
    {
        listener->on_notify(*this, e.what());
    }
}

void fb::game::session::action(fb::game::action action, fb::game::duration duration, uint8_t sound)
{
    auto listener = this->get_listener<fb::game::session::listener>();

    try
    {
        this->assert_state({state::GHOST, state::RIDING});

        if(listener != nullptr)
            listener->on_action(*this, action, duration, sound);
    }
    catch(std::exception& e)
    {
        if(listener != nullptr)
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
    auto listener = this->get_listener<fb::game::session::listener>();

    this->_look = value;
    if(listener != nullptr)
        listener->on_show(*this, false);
}

uint8_t fb::game::session::color() const
{
    return this->_color;
}

void fb::game::session::color(uint8_t value)
{
    auto listener = this->get_listener<fb::game::session::listener>();

    this->_color = value;
    if(listener != nullptr)
        listener->on_show(*this, false);
}

std::optional<uint8_t> fb::game::session::armor_color() const
{
    return this->_armor_color;
}

void fb::game::session::armor_color(std::optional<uint8_t> value)
{
    auto listener = this->get_listener<fb::game::session::listener>();

    this->_armor_color = value;
    if(listener != nullptr)
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
    auto listener = this->get_listener<fb::game::session::listener>();

    this->_disguise = value;
    this->state(state::DISGUISE);
    if(listener != nullptr)
        listener->on_updated(*this, state_level::LEVEL_MAX);
}

void fb::game::session::undisguise()
{
    auto listener = this->get_listener<fb::game::session::listener>();

    this->_disguise = std::nullopt;
    if(this->state() == fb::game::state::DISGUISE)
        this->state(state::NORMAL);

    if(listener != nullptr)
        listener->on_updated(*this, state_level::LEVEL_MAX);
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

fb::game::nation fb::game::session::nation() const
{
    return this->_nation;
}

bool fb::game::session::nation(fb::game::nation value)
{
    if(value != fb::game::nation::GOGURYEO &&
       value != fb::game::nation::BUYEO)
        return false;

    this->_nation = value;
    return true;
}

fb::game::creature fb::game::session::creature() const
{
    return this->_creature;
}

bool fb::game::session::creature(fb::game::creature value)
{
    if(value != fb::game::creature::DRAGON  &&
       value != fb::game::creature::PHOENIX &&
       value != fb::game::creature::TIGER   &&
       value != fb::game::creature::TURTLE)
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
    auto listener = this->get_listener<fb::game::session::listener>();

    this->_level = value;
    listener->on_updated(*this, state_level::LEVEL_MAX);
}

bool fb::game::session::level_up()
{
    if(this->max_level())
        return false;

    if(this->_class == 0 && this->_level >= 5)
        return false;

    this->level(this->_level + 1);
    
    this->strength_up(fb::game::table::classes[this->_class]->level_abilities[this->_level]->strength);
    this->intelligence_up(fb::game::table::classes[this->_class]->level_abilities[this->_level]->intelligence);
    this->dexteritry_up(fb::game::table::classes[this->_class]->level_abilities[this->_level]->dexteritry);

    this->base_hp_up(fb::game::table::classes[this->_class]->level_abilities[this->_level]->base_hp + std::rand() % 10);
    this->base_mp_up(fb::game::table::classes[this->_class]->level_abilities[this->_level]->base_mp + std::rand() % 10);

    this->hp(this->base_hp());
    this->mp(this->base_mp());

    auto listener = this->get_listener<fb::game::session::listener>();

    if(listener != nullptr)
    {
        listener->on_level_up(*this);
        listener->on_notify(*this, message::level::UP);
    }

    return true;
}

bool fb::game::session::max_level() const
{
    return this->_level >= 99;
}

fb::game::sex fb::game::session::sex() const
{
    return this->_sex;
}

void fb::game::session::sex(fb::game::sex value)
{
    this->_sex = value;
}

fb::game::state fb::game::session::state() const
{
    return this->_state;
}

void fb::game::session::state(fb::game::state value)
{
    if(this->_state == value)
        return;

    this->_state = value;

    auto listener = this->get_listener<fb::game::session::listener>();
    if(listener != nullptr)
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

    auto listener = this->get_listener<fb::game::session::listener>();
    if(listener != nullptr)
        listener->on_updated(*this, state_level::LEVEL_MIN);
}

uint32_t fb::game::session::experience_add(uint32_t value, bool notify)
{
    uint32_t capacity = 0xFFFFFFFF - this->_experience;
    uint32_t lack = 0;

    auto listener = this->get_listener<fb::game::session::listener>();

    try
    {
        // 직업이 없는 경우 정확히 5레벨을 찍을 경험치만 얻도록 제한
        if(this->_class == 0)
        {
            auto require = fb::game::table::classes.exp(0, 5);
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

                if(listener != nullptr)
                    listener->on_notify(*this, sstream.str());
            }
        }

        while(true)
        {
            if(this->max_level())
                break;

            auto require = fb::game::table::classes.exp(this->_class, this->_level + 1);
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
        if(listener != nullptr && notify)
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

    return fb::game::table::classes.exp(this->_class, this->_level+1) - this->experience();
}

float fb::game::session::experience_percent() const
{
    auto                    next_exp = this->max_level() ? 0xFFFFFFFF : fb::game::table::classes.exp(this->cls(), this->level()+1);
    auto                    prev_exp = this->max_level() ? 0x00000000 : fb::game::table::classes.exp(this->cls(), this->level());
    auto                    exp_range = next_exp - prev_exp;

    return std::min(100.0f, ((this->_experience - prev_exp) / float(exp_range)) * 100.0f);
}

uint32_t fb::game::session::money() const
{
    return this->_money;
}

void fb::game::session::money(uint32_t value)
{
    auto listener = this->get_listener<fb::game::session::listener>();

    this->_money = value;
    if(listener != nullptr)
    {
        listener->on_updated(*this, state_level::LEVEL_MIN);
        listener->on_money_changed(*this, value);
    }
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
    auto listener = this->get_listener<fb::game::session::listener>();

    try
    {
        if(value == 0)
            return 0;

        this->assert_state({state::RIDING, state::GHOST});


        auto lack = this->money_reduce(value);

        auto cash = new fb::game::cash(value, listener);
        cash->map(this->_map, this->_position);

        this->action(action::PICKUP, duration::DURATION_PICKUP);

        if(listener != nullptr)
            listener->on_notify(*this, message::money::DROP);

        return lack;
    }
    catch(std::exception& e)
    {
        if(listener != nullptr)
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

bool fb::game::session::option(options key) const
{
    if(key > 0x1B)
        return false;

    return this->_options[key];
}

void fb::game::session::option(options key, bool value)
{
    if(key > 0x1B)
        return;

    if(this->_options[key] == value)
        return;

    auto listener = this->get_listener<fb::game::session::listener>();

    if(listener != nullptr)
    {
        listener->on_updated(*this, state_level::LEVEL_MIN);
        listener->on_option(*this, key, value);
    }

    this->_options[key] = value;
}

bool fb::game::session::option_toggle(options key)
{
    if(key > 0x1B)
        return false;

    this->option(key, !this->_options[key]);
    return this->_options[key];
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

void fb::game::session::assert_state(fb::game::state value) const
{
    static const auto pairs = std::map<fb::game::state, const std::runtime_error>
    {
        {state::GHOST, ghost_exception()},
        {state::RIDING, ridding_exception()},
        {state::DISGUISE, disguise_exception()}
    };

    if(this->_state == value)
        throw pairs.at(value);
}

void fb::game::session::assert_state(const std::vector<fb::game::state>& values) const
{
    for(auto value : values)
        this->assert_state(value);
}

bool fb::game::session::move(const point16_t& before)
{
    return this->move(this->_direction, before);
}

bool fb::game::session::move(fb::game::direction direction, const point16_t& before)
{
    auto listener = this->get_listener<fb::game::session::listener>();

    if(this->_position != before)
    {
        if(listener != nullptr)
            listener->on_hold(*this);
        
        return false;
    }
    else if(object::move(direction) == false)
    {
        if(listener != nullptr)
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
    auto listener = this->get_listener<fb::game::session::listener>();

    try
    {
        this->assert_state({state::GHOST, state::DISGUISE});

        if(this->state() == fb::game::state::RIDING)
            throw std::runtime_error(message::ride::ALREADY_RIDE);

        if(horse.based<fb::game::mob::master>() != fb::game::table::mobs.name2mob("말"))
            throw session::no_conveyance_exception();

        if(horse.map() != this->_map)
            throw std::runtime_error(message::error::UNKNOWN);

        horse.map(nullptr);
        this->state(state::RIDING);
        horse.kill();
        horse.dead_time(std::chrono::duration_cast<std::chrono::milliseconds>(fb::thread::now()));

        if(listener != nullptr)
            listener->on_notify(*this, message::ride::ON);
    }
    catch(std::exception& e)
    {
        if(listener != nullptr)
            listener->on_notify(*this, e.what());
    }
}

void fb::game::session::ride()
{
    auto listener = this->get_listener<fb::game::session::listener>();

    try
    {
        this->assert_state({state::GHOST, state::DISGUISE});

        auto front = this->forward(object::types::MOB);
        if(front == nullptr)
            throw session::no_conveyance_exception();

        this->ride(static_cast<fb::game::mob&>(*front));
    }
    catch(std::exception& e)
    {
        if(listener != nullptr)
            listener->on_notify(*this, e.what());
    }
}

void fb::game::session::unride(fb::game::mob::listener* listener)
{
    auto _listener = this->get_listener<fb::game::session::listener>();

    try
    {
        this->assert_state({state::GHOST, state::DISGUISE});
        if(this->state() != fb::game::state::RIDING)
            throw std::runtime_error(message::ride::UNRIDE);

        auto master = fb::game::table::mobs.name2mob("말");
        auto horse = new fb::game::mob(master, listener, true);
        horse->map(this->_map, this->position_forward());
        
        this->state(state::NORMAL);
        if(_listener != nullptr)
            _listener->on_notify(*this, message::ride::OFF);
    }
    catch(std::exception& e)
    {
        if(_listener != nullptr)
            _listener->on_notify(*this, e.what());
    }
}

bool fb::game::session::alive() const
{
    return this->_state != state::GHOST;
}

int fb::game::session::builtin_look(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto acceptor = thread->env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || acceptor->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->look());
        return 1;
    }
    else
    {
        auto value = (uint16_t)thread->tointeger(2);
        session->look(value);
        return 0;
    }
}

int fb::game::session::builtin_color(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto acceptor = thread->env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || acceptor->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->color());
        return 1;
    }
    else
    {
        auto value = (uint8_t)thread->tointeger(2);
        session->color(value);
        return 0;
    }
}

int fb::game::session::builtin_money(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto acceptor = thread->env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || acceptor->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->money());
        return 1;
    }
    else
    {
        auto value = (uint32_t)thread->tointeger(2);
        session->money(value);
        return 0;
    }
}

int fb::game::session::builtin_exp(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto acceptor = thread->env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || acceptor->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->experience());
        return 1;
    }
    else
    {
        auto value = (uint32_t)thread->tointeger(2);
        session->experience(value);
        return 0;
    }
}

int fb::game::session::builtin_base_hp(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto acceptor = thread->env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || acceptor->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->base_hp());
        return 1;
    }
    else
    {
        auto value = (uint32_t)thread->tointeger(2);
        session->base_hp(value);
        return 0;
    }
}

int fb::game::session::builtin_base_mp(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto acceptor = thread->env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || acceptor->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->base_mp());
        return 1;
    }
    else
    {
        auto value = (uint32_t)thread->tointeger(2);
        session->base_mp(value);
        return 0;
    }
}

int fb::game::session::builtin_strength(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto acceptor = thread->env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || acceptor->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->strength());
        return 1;
    }
    else
    {
        auto value = (uint8_t)thread->tointeger(2);
        session->strength(value);
        return 0;
    }
}

int fb::game::session::builtin_dexterity(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto acceptor = thread->env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || acceptor->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->dexteritry());
        return 1;
    }
    else
    {
        auto value = (uint8_t)thread->tointeger(2);
        session->dexteritry(value);
        return 0;
    }
}

int fb::game::session::builtin_intelligence(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto acceptor = thread->env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || acceptor->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->intelligence());
        return 1;
    }
    else
    {
        auto value = (uint8_t)thread->tointeger(2);
        session->intelligence(value);
        return 0;
    }
}

int fb::game::session::builtin_item(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto acceptor = thread->env<fb::game::acceptor>("acceptor");
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || acceptor->exists(*session) == false)
        return 0;
    auto item = (fb::game::item*)nullptr;
    if(lua_isnumber(lua, 2))
    {
        auto index = (uint8_t)thread->tointeger(2);
        item = session->items[index];
    }
    else if(lua_isstring(lua, 2))
    {
        auto name = lua_cp949(lua, 2);
        item = session->items.find(name);
    }

    if(item == nullptr)
        thread->pushnil();
    else
        item->to_lua(lua);

    return 1;
}

int fb::game::session::builtin_items(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto acceptor = thread->env<fb::game::acceptor>("acceptor");
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || acceptor->exists(*session) == false)
        return 0;

    lua_newtable(lua);
    for(int i = 0; i < item::MAX_SLOT; i++)
    {
        if(session->items[i] == nullptr)
            continue;

        session->items[i]->to_lua(lua);
        lua_rawseti(lua, -2, i+1);
    }

    return 1;
}

int fb::game::session::builtin_item_drop(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto acceptor = thread->env<fb::game::acceptor>("acceptor");
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || acceptor->exists(*session) == false)
        return 0;
    auto index = (uint8_t)thread->tointeger(2);
    auto drop_all = thread->toboolean(3);

    auto dropped = session->items.drop(index - 1, drop_all ? 1 : -1);
    if(dropped != nullptr)
        dropped->to_lua(lua);
    else
        thread->pushnil();

    return 1;
}

int fb::game::session::builtin_mkitem(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto acceptor = thread->env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || acceptor->exists(*session) == false)
        return 0;
    auto name = lua_cp949(lua, 2);
    auto store = argc < 3 ? true : thread->toboolean(3);

    if(store == false)
        return object::builtin_mkitem(lua);

    auto master = fb::game::table::items.name2item(name);
    if(master == nullptr)
    {
        thread->pushnil();
    }
    else
    {
        auto acceptor = thread->env<fb::game::acceptor>("acceptor");
        auto item = master->make<fb::game::item>(acceptor);
        auto slot = session->items.add(item);
        item->to_lua(lua);

        acceptor->send(*session, fb::protocol::game::response::item::update(*session, slot), acceptor::scope::SELF);
    }

    return 1;
}

int fb::game::session::builtin_rmitem(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto acceptor = thread->env<fb::game::acceptor>("acceptor");
    try
    {
        auto argc = thread->argc();
        auto session = thread->touserdata<fb::game::session>(1);
        if(session == nullptr || acceptor->exists(*session) == false)
            return 0;
        auto index = uint8_t(0);
        auto count = argc < 3 ? 1 : (int)thread->tointeger(3);

        if(lua_isuserdata(lua, 2))
        {
            auto item = thread->touserdata<fb::game::item>(2);
            if(item == nullptr)
                return 0;
            if(item == nullptr)
                throw std::exception();

            index = session->items.to_index(item->based<fb::game::item::master>());
        }
        else if(lua_isnumber(lua, 2))
        {
            index = (uint8_t)thread->tointeger(2) - 1;
        }
        else if(lua_isstring(lua, 2))
        {
            auto name = lua_cp949(lua, 2);
            if(name.empty())
                throw std::exception();

            auto master = fb::game::table::items.name2item(name);
            if(master == nullptr)
                throw std::exception();

            index = session->items.to_index(master);
        }
        else
        {
            throw std::exception();
        }


        auto acceptor = thread->env<fb::game::acceptor>("acceptor");
        auto dropped = session->items.remove(index, item::delete_attr::DELETE_REMOVED);
        if(dropped != nullptr)
            delete dropped;
    }
    catch(...)
    { }

    return 0;
}

int fb::game::session::builtin_state(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto acceptor = thread->env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || acceptor->exists(*session) == false)
        return 0;
    if(argc == 1)
    {
        thread->pushinteger(session->state());
        return 1;
    }
    else
    {
        auto value = fb::game::state(thread->tointeger(2));
        session->state(value);
        return 0;
    }
}

int fb::game::session::builtin_disguise(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto acceptor = thread->env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || acceptor->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->disguise().value());
        return 1;
    }
    else if(luaL_checkinteger(lua, 2))
    {
        session->disguise((uint16_t)thread->tointeger(2));
        return 0;
    }
    else
    {
        session->undisguise();
        return 0;
    }
}

int fb::game::session::builtin_class(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto acceptor = thread->env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || acceptor->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        auto cls = session->_class;
        auto promotion = session->_promotion;

        auto cls_name = fb::game::table::classes.class2name(cls, promotion);
        if(cls_name == nullptr)
        {
            thread->pushnil();
        }
        else
        {
            thread->pushstring(*cls_name);
        }
    }
    else
    {
        auto cls_name = lua_cp949(lua, 2);
        uint8_t cls, promotion;
        if(fb::game::table::classes.name2class(cls_name, &cls, &promotion) == false)
        {
            thread->pushboolean(false);
        }
        else
        {
            auto acceptor = thread->env<fb::game::acceptor>("acceptor");
            acceptor->send(*session, fb::protocol::game::response::session::id(*session), acceptor::scope::SELF);
            acceptor->send(*session, fb::protocol::game::response::session::state(*session, state_level::LEVEL_MAX), acceptor::scope::SELF);
            thread->pushboolean(true);
        }
    }

    return 1;
}

int fb::game::session::builtin_level(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto acceptor = thread->env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || acceptor->exists(*session) == false)
        return 0;

    if(argc == 1)
    {
        thread->pushinteger(session->level());
        return 1;
    }
    else
    {
        auto level = std::max(0, std::min((int)thread->tointeger(2), 255));
        session->level(level);

        auto acceptor = thread->env<fb::game::acceptor>("acceptor");
        acceptor->send(*session, fb::protocol::game::response::session::state(*session, state_level::LEVEL_MAX), acceptor::scope::SELF);
        return 0;
    }
}

int fb::game::session::builtin_group(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto acceptor = thread->env<fb::game::acceptor>("acceptor");
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    if(session == nullptr || acceptor->exists(*session) == false)
        return 0;

    auto group = session->group();
    if(group == nullptr)
        thread->pushnil();
    else
        group->to_lua(lua);

    return 1;
}

int fb::game::session::builtin_assert(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto argc = thread->argc();
    auto session = thread->touserdata<fb::game::session>(1);
    auto size = thread->rawlen(2);
    std::vector<fb::game::state> values;
    for(int i = 0; i < size; i++)
    {
        thread->rawgeti(2, i+1);
        values.push_back((fb::game::state)thread->tointeger(-1));
    }

    try
    {
        session->assert_state(values);
        return 0;
    }
    catch(std::runtime_error& e)
    {
        thread->pushstring(e.what());
        return 1;
    }
}

fb::game::session::container::container()
{
}

fb::game::session::container::container(const std::vector<fb::game::session*>& right)
{
    this->insert(this->begin(), right.begin(), right.end());
}

fb::game::session::container::~container()
{
}

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
        [&name] (fb::game::session* x) 
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

fb::game::lua::dialog::dialog(fb::game::session& owner) : 
    _owner(owner),
    _thread(nullptr)
{
}

fb::game::lua::dialog::~dialog()
{}

fb::game::lua::dialog& fb::game::lua::dialog::pushstring(const std::string& value)
{
    if(this->_thread != nullptr)
        this->_thread->pushstring(value);

    return *this;
}

fb::game::lua::dialog& fb::game::lua::dialog::pushinteger(int value)
{
    if(this->_thread != nullptr)
        this->_thread->pushinteger(value);

    return *this;
}

fb::game::lua::dialog& fb::game::lua::dialog::pushnil()
{
    if(this->_thread != nullptr)
        this->_thread->pushnil();

    return *this;
}

fb::game::lua::dialog& fb::game::lua::dialog::pushboolean(bool value)
{
    if(this->_thread != nullptr)
        this->_thread->pushboolean(value);

    return *this;
}

fb::game::lua::dialog& fb::game::lua::dialog::pushobject(const luable* object)
{
    if(this->_thread != nullptr)
        this->_thread->pushobject(object);

    return *this;
}

fb::game::lua::dialog& fb::game::lua::dialog::pushobject(const luable& object)
{
    if(this->_thread != nullptr)
        this->_thread->pushobject(object);
    
    return *this;
}

fb::game::lua::dialog& fb::game::lua::dialog::from(const char* format, ...)
{
    if(this->_thread != nullptr)
        this->_thread->release();

    this->_thread = &fb::game::lua::get();

    va_list args;
    va_start(args, format);

    char buffer[256];
    vsprintf(buffer, format, args);
    va_end(args);

    luaL_dofile(*this->_thread, buffer);
    return *this;
}

fb::game::lua::dialog& fb::game::lua::dialog::func(const char* format, ...)
{
    if(this->_thread == nullptr)
        return *this;

    va_list args;
    va_start(args, format);

    char buffer[256];
    vsprintf(buffer, format, args);
    va_end(args);

    lua_getglobal(*this->_thread, buffer);
    return *this;
}

bool fb::game::lua::dialog::resume(int argc)
{
    if(this->_thread == nullptr)
        return false;

    auto state = this->_thread->resume(argc);
    if(state != LUA_YIELD)
        this->_thread = nullptr;

    return state;
}

void fb::game::lua::dialog::show(const object::master& object, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction)
{
    auto listener = this->_owner.get_listener<fb::game::session::listener>();
    if(listener != nullptr)
        listener->on_dialog(this->_owner, object, message, button_prev, button_next, interaction);
}

void fb::game::lua::dialog::show(const object& object, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction)
{
    return this->show(*object.based<object::master>(), message, button_prev, button_next, interaction);
}

void fb::game::lua::dialog::show(const npc::master& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction)
{
    auto listener = this->_owner.get_listener<fb::game::session::listener>();
    if(listener != nullptr)
        listener->on_dialog(this->_owner, npc, message, menus, interaction);
}

void fb::game::lua::dialog::show(const npc& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction)
{
    this->show(*npc.based<npc::master>(), message, menus, interaction);
}

void fb::game::lua::dialog::show(const npc::master& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction)
{
    auto listener = this->_owner.get_listener<fb::game::session::listener>();
    if(listener != nullptr)
        listener->on_dialog(this->_owner, npc, message, item_slots, interaction);
}

void fb::game::lua::dialog::show(const npc& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction)
{
    return this->show(*npc.based<npc::master>(), message, item_slots, interaction);
}

void fb::game::lua::dialog::show(const npc::master& npc, const std::string& message, const std::vector<item::master*>& cores, fb::game::dialog::interaction interaction)
{
    auto listener = this->_owner.get_listener<fb::game::session::listener>();
    if(listener != nullptr)
        listener->on_dialog(this->_owner, npc, message, cores, interaction);
}

void fb::game::lua::dialog::show(const npc& npc, const std::string& message, const std::vector<item::master*>& cores, fb::game::dialog::interaction interaction)
{
    this->show(*npc.based<npc::master>(), message, cores, interaction);
}

void fb::game::lua::dialog::input(const npc::master& npc, const std::string& message, fb::game::dialog::interaction interaction)
{
    auto listener = this->_owner.get_listener<fb::game::session::listener>();
    if(listener != nullptr)
        listener->on_dialog(this->_owner, npc, message, interaction);
}

void fb::game::lua::dialog::input(const npc& npc, const std::string& message, fb::game::dialog::interaction interaction)
{
    this->input(*npc.based<npc::master>(), message, interaction);
}

void fb::game::lua::dialog::input(const npc::master& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen, bool prev, fb::game::dialog::interaction interaction)
{
    auto listener = this->_owner.get_listener<fb::game::session::listener>();
    if(listener != nullptr)
        listener->on_dialog(this->_owner, npc, message, top, bottom, maxlen, prev, interaction);
}

void fb::game::lua::dialog::input(const npc& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen, bool prev, fb::game::dialog::interaction interaction)
{
    this->input(*npc.based<npc::master>(), message, top, bottom, maxlen, prev, interaction);
}
