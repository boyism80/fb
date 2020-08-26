#include <model/session/session.h>
#include <model/map/map.h>
#include <model/item/item.h>
#include <model/master/master.h>
#include <model/group/group.h>
#include <model/clan/clan.h>
#include <model/acceptor/acceptor.game.h>
#include <model/listener/listener.h>

using namespace fb::game;

session::session(fb::socket* socket, listener* listener) : 
    life((life::master*)nullptr, listener, (uint32_t)socket->native_handle(), 0, 0, 0),
    _listener(listener),
    _socket(socket),
    _look(0), _color(0), _dress_color(0),
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
    trade(*this, listener),
    items(*this, listener),
    dialog(*this, listener)
{
    memset(this->_options, 0, sizeof(this->_options));
}

session::~session()
{
    delete this->_master;
}

void fb::game::session::send(const fb::ostream& stream, bool encrypt, bool wrap, bool async)
{
    this->_socket->send(stream, encrypt, wrap, async);
}

void fb::game::session::send(const fb::protocol::base::response& response, bool encrypt, bool wrap, bool async)
{
    this->_socket->send(response, encrypt, wrap, async);
}

object::types fb::game::session::type() const
{
    return object::types::SESSION;
}

fb::cryptor& fb::game::session::crt()
{
    return this->_socket->crt();
}

void fb::game::session::crt(const fb::cryptor& crt)
{
    this->_socket->crt(crt);
}

void fb::game::session::crt(uint8_t enctype, const uint8_t* enckey)
{
    this->_socket->crt(enctype, enckey);
}

fb::istream& fb::game::session::in_stream()
{
    return this->_socket->in_stream();
}

fb::game::session::operator fb::socket& ()
{
    return *this->_socket;
}

void fb::game::session::attack()
{
    try
    {
        this->state_assert(state::RIDING | state::GHOST);

        if(this->_map == nullptr)
            return;

        auto front = this->forward_object(object::types::UNKNOWN);
        auto damage = 0;
        auto critical = false;

        if(front != nullptr && front->is(object::types::LIFE) && static_cast<life*>(front)->alive())
        {
            critical = true;
            damage = this->random_damage(*static_cast<fb::game::life*>(front), critical);
        }

        if(this->_listener != nullptr)
            this->_listener->on_attack(*this, front, damage, critical);
    }
    catch(std::exception& e)
    {
        this->_listener->on_notify(*this, e.what());
    }
}

void fb::game::session::action(fb::game::action action, fb::game::duration duration, uint8_t sound)
{
    try
    {
        this->state_assert(state::GHOST | state::RIDING);

        if(this->_listener != nullptr)
            this->_listener->on_action(*this, action, duration, sound);
    }
    catch(std::exception& e)
    {
        if(this->_listener != nullptr)
            this->_listener->on_notify(*this, e.what());
    }
}

uint32_t fb::game::session::hp_down(uint32_t value, fb::game::object* from, bool critical)
{
    value = fb::game::life::hp_down(value, from, critical);
    if(this->_listener != nullptr)
        this->_listener->on_damage(*this, from, value, critical);

    if(this->_hp == 0 && this->_listener != nullptr)
        this->_listener->on_die(*this);
    return value;
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
    this->_look = value;
}

uint8_t fb::game::session::color() const
{
    return this->_color;
}

void fb::game::session::color(uint8_t value)
{
    this->_color = value;
}

uint8_t fb::game::session::dress_color() const
{
    return this->_dress_color;
}

void fb::game::session::dress_color(uint8_t value)
{
    this->_dress_color = value;
}

std::optional<uint16_t> fb::game::session::disguise() const
{
    return this->_disguise;
}

void fb::game::session::disguise(uint16_t value)
{
    this->_disguise = value;
    this->state(state::DISGUISE);
    if(this->_listener != nullptr)
        this->_listener->on_updated(*this, state_level::LEVEL_MAX);
}

void fb::game::session::undisguise()
{
    this->_disguise.reset();
    this->state(state::NORMAL);
    if(this->_listener != nullptr)
        this->_listener->on_updated(*this, state_level::LEVEL_MAX);
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
    this->_level = value;
    this->_listener->on_updated(*this, state_level::LEVEL_MAX);
}

bool fb::game::session::level_up()
{
    if(this->max_level())
        return false;

    if(this->_class == 0 && this->_level >= 5)
        return false;

    this->level(this->_level + 1);
    
    this->strength_up(game::master::get().classes[this->_class]->level_abilities[this->_level]->strength);
    this->intelligence_up(game::master::get().classes[this->_class]->level_abilities[this->_level]->intelligence);
    this->dexteritry_up(game::master::get().classes[this->_class]->level_abilities[this->_level]->dexteritry);

    this->base_hp_up(game::master::get().classes[this->_class]->level_abilities[this->_level]->base_hp + std::rand() % 10);
    this->base_mp_up(game::master::get().classes[this->_class]->level_abilities[this->_level]->base_mp + std::rand() % 10);

    this->hp(this->base_hp());
    this->mp(this->base_mp());

    if(this->_listener != nullptr)
    {
        this->_listener->on_level_up(*this);
        this->_listener->on_notify(*this, message::level::UP);
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
    if(this->_listener != nullptr)
        this->_listener->on_show(*this, false);
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
    if(this->_listener != nullptr)
        this->_listener->on_updated(*this, state_level::LEVEL_MIN);
}

uint32_t fb::game::session::experience_add(uint32_t value, bool notify)
{
    uint32_t capacity = 0xFFFFFFFF - this->_experience;
    uint32_t lack = 0;

    try
    {
        // 직업이 없는 경우 정확히 5레벨을 찍을 경험치만 얻도록 제한
        if(this->_class == 0)
        {
            auto require = game::master::get().required_exp(0, 5);
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

                if(this->_listener != nullptr)
                    this->_listener->on_notify(*this, sstream.str());
            }
        }

        while(true)
        {
            if(this->max_level())
                break;

            auto require = game::master::get().required_exp(this->_class, this->_level + 1);
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
        if(this->_listener != nullptr && notify)
            this->_listener->on_notify(*this, e.what());
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

    return game::master::get().required_exp(this->_class, this->_level+1) - this->experience();
}

float fb::game::session::experience_percent() const
{
    auto                    next_exp = this->max_level() ? 0xFFFFFFFF : game::master::get().required_exp(this->cls(), this->level()+1);
    auto                    prev_exp = this->max_level() ? 0x00000000 : game::master::get().required_exp(this->cls(), this->level());
    auto                    exp_range = next_exp - prev_exp;

    return std::min(100.0f, ((this->_experience - prev_exp) / float(exp_range)) * 100.0f);
}

uint32_t fb::game::session::money() const
{
    return this->_money;
}

void fb::game::session::money(uint32_t value)
{
    this->_money = value;
    if(this->_listener != nullptr)
        this->_listener->on_updated(*this, state_level::LEVEL_MIN);
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
    try
    {
        if(value == 0)
            return 0;

        this->state_assert(state::RIDING | state::GHOST);


        auto lack = this->money_reduce(value);

        auto cash = new fb::game::cash(value, this->_listener);
        this->_map->objects.add(*cash, this->_position);

        this->action(action::PICKUP, duration::DURATION_PICKUP);

        if(this->_listener != nullptr)
            this->_listener->on_notify(*this, message::money::DROP);

        return lack;
    }
    catch(std::exception& e)
    {
        if(this->_listener != nullptr)
            this->_listener->on_notify(*this, e.what());

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

uint32_t fb::game::session::random_damage(fb::game::life& life, bool& critical) const
{
    uint32_t                damage = 0;
    auto                    weapon = this->items.weapon();

    if(weapon == nullptr) // no weapon
    {
        damage = 1 + std::rand() % 5;
        critical = false;
    }
    else if(std::rand() % 100 < 20) // critical
    {
        auto                range = weapon->damage_large();
        damage = std::max(uint32_t(1), range.min) + std::rand() % std::max(uint32_t(1), range.max);
        critical = true;
    }
    else // normal
    {
        auto                range = weapon->damage_small();
        damage = std::max(uint32_t(1), range.min) + std::rand() % std::max(uint32_t(1), range.max);
        critical = false;
    }

    return __super::random_damage(damage, life);
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

    if(this->_listener != nullptr)
    {
        this->_listener->on_updated(*this, state_level::LEVEL_MIN);
        this->_listener->on_option(*this, key, value);
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

void fb::game::session::state_assert(fb::game::state flags) const
{
    if((flags & state::GHOST) == state::GHOST && this->_state == state::GHOST)
        throw ghost_exception();

    if((flags & state::RIDING) == state::RIDING && this->_state == state::RIDING)
        throw ridding_exception();

    if((flags & state::DISGUISE) == state::DISGUISE && this->_state == state::DISGUISE)
        throw disguise_exception();
}

void fb::game::session::state_assert(uint8_t flags) const
{
    return this->state_assert(fb::game::state(flags));
}

bool fb::game::session::move(const point16_t& before)
{
    return this->move(this->_direction, before);
}

bool fb::game::session::move(fb::game::direction direction, const point16_t& before)
{
    if(this->_position != before)
    {
        if(this->_listener != nullptr)
            this->_listener->on_hold(*this);
        
        return false;
    }
    else if(object::move(direction) == false)
    {
        if(this->_listener != nullptr)
            this->_listener->on_hold(*this);
        
        return false;
    }
    else
    {
        return true;
    }
}

void fb::game::session::ride(fb::game::mob& horse)
{
    try
    {
        this->state_assert(state::GHOST | state::DISGUISE);

        if(this->state() == fb::game::state::RIDING)
            throw std::exception("이미 타고 있습니다.");

        if(horse.based<fb::game::mob::master>() != game::master::get().name2mob("말"))
            throw session::no_conveyance_exception();

        if(horse.map() != this->_map)
            throw std::runtime_error("올바르지 않은 명령입니다.");

        this->_map->objects.remove(horse);
        this->state(state::RIDING);
        horse.kill();
        horse.dead_time(fb::timer::now());

        if(this->_listener != nullptr)
            this->_listener->on_notify(*this, message::ride::ON);
    }
    catch(std::exception& e)
    {
        if(this->_listener != nullptr)
            this->_listener->on_notify(*this, e.what());
    }
}

void fb::game::session::ride()
{
    try
    {
        this->state_assert(state::GHOST | state::DISGUISE);

        auto front = this->forward_object(object::types::MOB);
        if(front == nullptr)
            throw session::no_conveyance_exception();

        this->ride(static_cast<fb::game::mob&>(*front));
    }
    catch(std::exception& e)
    {
        if(this->_listener != nullptr)
            this->_listener->on_notify(*this, e.what());
    }
}

void fb::game::session::unride(fb::game::listener* listener)
{
    try
    {
        this->state_assert(state::GHOST | state::DISGUISE);
        if(this->state() != fb::game::state::RIDING)
            throw std::runtime_error("말에 타고 있지 않습니다.");

        auto master = game::master::get().name2mob("말");
        auto horse = static_cast<fb::game::mob*>(master->make(listener));
        horse->hp_up(horse->base_hp());
        this->_map->objects.add(*horse, this->position_forward());
        
        this->state(state::NORMAL);
        if(this->_listener != nullptr)
            this->_listener->on_notify(*this, message::ride::OFF);
    }
    catch(std::exception& e)
    {
        if(this->_listener != nullptr)
            this->_listener->on_notify(*this, e.what());
    }
}

bool fb::game::session::alive() const
{
    return this->_state != state::GHOST;
}

fb::ostream fb::game::session::make_show_stream(bool light) const
{
    return this->make_visual_stream(light);
}

fb::ostream fb::game::session::make_id_stream() const
{
    fb::ostream             ostream;
    ostream.write_u8(0x05)
        .write_u32(this->id())
        .write_u32(this->direction()) // side
        .write_u8(this->_class) // class
        .write_u16(0x00)
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::session::make_state_stream(fb::game::state_level level) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x08) // cmd
        .write_u8(level);

    if(level & state_level::BASED)
    {
        ostream.write_u8(this->nation()) // nation
            .write_u8(this->creature()) // creature
            .write_u8(0x00) // Unknown (clan?)
            .write_u8(this->level()) // level
            .write_u32(this->base_hp()) // base hp
            .write_u32(this->base_mp()) // base mp
            .write_u8(this->strength())
            .write_u8(this->intelligence())
            .write_u8(0x03)
            .write_u8(0x03)
            .write_u8(this->dexteritry())
            .write_u8(0x03)
            .write_u32(0x00)
            .write_u8(0x00);
    }

    if(level & state_level::HP_MP)
    {
        ostream.write_u32(this->hp())  // current hp
            .write_u32(this->mp()); // current mp
    }

    if(level & state_level::EXP_MONEY)
    {
        ostream.write_u32(this->experience())  // exp
            .write_u32(this->money()); // money
    }

    if(level & state_level::CONDITION)
    {
        ostream.write_u8(this->condition_contains(fb::game::condition::MOVE))  // condition::move
            .write_u8(this->condition_contains(fb::game::condition::SIGHT))  // condition::sight
            .write_u8(this->condition_contains(fb::game::condition::HEAR))  // condition::hear?
            .write_u8(this->condition_contains(fb::game::condition::ORAL))  // condition:oral
            .write_u8(this->condition_contains(fb::game::condition::MAP)); // condition:map?
    }

    ostream.write_u8(0x00)  // mail count
        .write_u8(true)  // fast move
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::session::make_appears_stream() const
{
    fb::ostream             ostream;
    auto                    map = this->map();
    if(map == nullptr)
        return fb::ostream();

    ostream.write(object::make_show_stream(this->showings(object::types::OBJECT)));

    for(auto session : this->showings(object::types::SESSION))
        ostream.write(session->make_show_stream());

    return ostream;
}

fb::ostream fb::game::session::make_position_stream() const
{
    fb::ostream             ostream;
    ostream.write_u8(0x04)
        .write_u16(this->_position.x)  // 실제 x 좌표
        .write_u16(this->_position.y); // 실제 y 좌표

                                 // 스크린에서의 x 좌표
    auto                    map = this->map();
    if(map->width() < map::MAX_SCREEN_WIDTH)
        ostream.write_u16(this->_position.x + map::HALF_SCREEN_WIDTH - (map->width() / 2));
    else if(this->_position.x < map::HALF_SCREEN_WIDTH)
        ostream.write_u16(this->_position.x);
    else if(this->_position.x >= map->width() - map::HALF_SCREEN_WIDTH)
        ostream.write_u16(this->_position.x + map::MAX_SCREEN_WIDTH - map->width());
    else
        ostream.write_u16(map::HALF_SCREEN_WIDTH);

    // 스크린에서의 y 좌표
    if(map->height() < map::MAX_SCREEN_HEIGHT)
        ostream.write_u16(this->_position.y + map::HALF_SCREEN_HEIGHT - (map->height() / 2));
    else if(this->_position.y < map::HALF_SCREEN_HEIGHT)
        ostream.write_u16(this->_position.y);
    else if(this->_position.y >= (map->height() - map::HALF_SCREEN_HEIGHT))
        ostream.write_u16(this->_position.y + map::MAX_SCREEN_HEIGHT - map->height());
    else
        ostream.write_u16(map::HALF_SCREEN_HEIGHT);

    ostream.write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::session::make_visual_stream(bool light) const
{
    fb::ostream             ostream;

    ostream.write_u8(light ? 0x1D : 0x33); // cmd
    if(light == false)
    {
        ostream.write_u16(this->_position.x) // x
            .write_u16(this->_position.y) // y
            .write_u8(this->_direction); // side
    }

    ostream.write_u32(this->id())
        .write_u8(this->_state == fb::game::state::DISGUISE) // 변신유무
        .write_u8(this->_sex) // sex
        .write_u8(this->_state); // state

    if(this->_state == fb::game::state::DISGUISE)
    {
        ostream.write_u16(this->_disguise.value())
            .write_u8(this->_dress_color);
    }
    else
    {
        ostream.write_u16(this->look()) // face
            .write_u8(this->color()); // hair color

        auto armor = this->items.armor();
        if(armor != nullptr)
        {
            ostream.write_u8((uint8_t)armor->dress())
                .write_u8(this->_dress_color == 0 ? armor->color() : this->_dress_color);
        }
        else
        {
            ostream.write_u8(this->_sex) // sex
                .write_u8(0x00);
        }

        auto weapon = this->items.weapon();
        if(weapon != nullptr)
        {
            ostream.write_u16(weapon->dress())
                .write_u8(weapon->color());
        }
        else
        {
            ostream.write_u16(0xFFFF)
                .write_u8(0x00);
        }

        auto shield = this->items.shield();
        if(shield != nullptr)
        {
            ostream.write_u8((uint8_t)shield->dress())
                .write_u8(shield->color());
        }
        else
        {
            ostream.write_u8(0xFF) // about shield
                .write_u8(0x00);
        }
    }

    ostream.write_u8(0x04) // head mark
        .write(this->name(), false); // name

    return ostream;
}

fb::ostream fb::game::session::make_internal_info_stream() const
{
    fb::ostream             ostream;

    ostream.write_u8(0x39)
        .write_u8((uint8_t)this->_defensive.physical)
        .write_u8(this->damage())
        .write_u8(this->hit())
        .write(this->_clan != nullptr ? this->_clan->name() : "")
        .write(this->_clan != nullptr ? this->_clan->title() : "")
        .write(this->_title);

    if(this->_group != nullptr)
    {
        std::stringstream   sstream;
        sstream << "그룹원" 
            << std::endl 
            << "  * "
            << this->_group->leader().name()
            << std::endl;

        for(auto member : this->_group->members())
        {
            if(this->_group->leader() == *member)
                continue;

            sstream << "    "
                << member->name()
                << std::endl;
        }
        ostream.write(sstream.str());
    }
    else
    {
        ostream.write("그룹 없음.");
    }
    ostream.write_u8(this->option(options::GROUP));


    uint32_t                remained_exp = this->experience_remained();
    ostream.write_u32(remained_exp);

    auto                    class_name = game::master::get().class2name(this->_class, this->_promotion);
    if(class_name == nullptr)
        return fb::ostream();
    ostream.write(*class_name);

    fb::game::equipment*    equipments[] = {this->items.helmet(), this->items.ring(equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT), this->items.ring(equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT), this->items.auxiliary(equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT), this->items.auxiliary(equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT)};
    for(int i = 0, size = sizeof(equipments) / sizeof(fb::game::equipment*); i < size; i++)
    {
        if(equipments[i] == nullptr)
        {
            ostream.write_u16(0xFFFF)
                .write_u8(0x00);
        }
        else
        {
            ostream.write_u16(equipments[i]->look())
                .write_u8(equipments[i]->color());
        }
    }

    ostream.write_u8(0x00) // fixed;
        .write_u8(this->_options[options::TRADE])
        .write_u8(this->_options[options::PK]);

    ostream.write_u8((uint8_t)this->legends.size());
    for(auto legend : this->legends)
    {
        ostream.write_u8(legend.look)
            .write_u8(legend.color)
            .write(legend.content);
    }
    ostream.write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::session::make_external_info_stream() const
{
    //
    // 다른 캐릭터를 클릭했을 때 표현될 정보
    //

    fb::ostream             ostream;

    ostream.write_u8(0x34)
        .write(this->_title)
        .write("클랜 이름")
        .write("클랜 타이틀");

    // 클래스 이름
    const auto              class_name = game::master::get().class2name(this->_class, this->_promotion);
    if(class_name == nullptr)
        return fb::ostream();

    ostream.write(*class_name)  // 직업
        .write(this->_name);    // 이름

    auto                    disguised = (this->_state == state::DISGUISE);
    ostream.write_u8(disguised)
        .write_u8(this->_sex)
        .write_u8(this->_state);

    auto                    armor = this->items.armor(); // 갑옷
    auto                    weapon = this->items.weapon(); // 무기
    auto                    shield = this->items.shield(); // 방패
    if(disguised)
    {
        ostream.write_u16(this->_disguise.value())
            .write_u8(this->_dress_color);
    }
    else
    {
        ostream.write_u16(this->look())
            .write_u8(this->color());

        ostream.write_u8(armor != nullptr ? armor->dress() : 0xFF)
            .write_u8(armor != nullptr ? armor->color() : 0x00);

        ostream.write_u16(weapon != nullptr ? weapon->dress() : 0xFFFF)
            .write_u8(weapon != nullptr ? weapon->color() : 0x00);

        ostream.write_u8(shield != nullptr ? shield->dress() : 0xFF)
            .write_u8(shield != nullptr ? shield->color() : 0x00);
    }

    // 장비정보
    std::stringstream       sstream;
    auto                    helmet = this->items.helmet(); // 투구
    ostream.write_u16(helmet != nullptr ? helmet->look() : 0xFFFF)
           .write_u8(helmet != nullptr ? helmet->color() : 0x00);

    auto                    ring_l = this->items.ring(equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT); // 왼손
    ostream.write_u16(ring_l != nullptr ? ring_l->look() : 0xFFFF)
           .write_u8(ring_l != nullptr ? ring_l->color() : 0x00);

    auto                    ring_r = this->items.ring(equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT); // 오른손
    ostream.write_u16(ring_r != nullptr ? ring_r->look() : 0xFFFF)
           .write_u8(ring_r != nullptr ? ring_r->color() : 0x00);

    auto                    aux_l = this->items.auxiliary(equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT); // 보조1
    ostream.write_u16(aux_l != nullptr ? aux_l->look() : 0xFFFF)
           .write_u8(aux_l != nullptr ? aux_l->color() : 0x00);

    auto                    aux_r = this->items.auxiliary(equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT); // 보조2
    ostream.write_u16(aux_r != nullptr ? aux_r->look() : 0xFFFF)
           .write_u8(aux_r != nullptr ? aux_r->color() : 0x00);


    // 장비정보 텍스트
    sstream << " w:무기  :" << (weapon != nullptr ? weapon->name() : "없음") << std::endl;
    sstream << " a:갑옷  :" << (armor  != nullptr ? armor->name()  : "없음") << std::endl;
    sstream << " s:방패  :" << (shield != nullptr ? shield->name() : "없음") << std::endl;
    sstream << " h:머리  :" << (helmet != nullptr ? helmet->name() : "없음") << std::endl;
    sstream << " l:왼손  :" << (ring_l != nullptr ? ring_l->name() : "없음") << std::endl;
    sstream << " r:오른손:" << (ring_r != nullptr ? ring_r->name() : "없음") << std::endl;
    sstream << " [:보조1 :" << (aux_l  != nullptr ? aux_l->name()  : "없음") << std::endl;
    sstream << " ]:보조2 :" << (aux_r  != nullptr ? aux_r->name()  : "없음") << std::endl;
    ostream.write(sstream.str());


    ostream.write_u32(this->id())
        .write_u8(this->_options[fb::game::options::GROUP])
        .write_u8(this->_options[fb::game::options::TRADE])
        .write_u32(0x00000000); // unknown

                                // 업적
    ostream.write_u8((uint8_t)this->legends.size());
    for(auto legend : this->legends)
    {
        ostream.write_u8(legend.look)
            .write_u8(legend.color)
            .write(legend.content);
    }
    ostream.write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::session::make_option_stream() const
{
    fb::ostream             ostream;
    ostream.write_u8(0x23)
        .write_u8(this->_options[options::WEATHER_EFFECT]) // weather
        .write_u8(this->_options[options::MAGIC_EFFECT]) // magic effect
        .write_u8(this->_options[options::ROAR_WORLDS]) // listen news
        .write_u8(this->_options[options::FAST_MOVE]) // fast move
        .write_u8(this->_options[options::EFFECT_SOUND]) // effect sound
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::session::make_throw_item_stream(const item& item, const point16_t& to) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x16)
        .write_u32(this->id())
        .write_u16(item.look())
        .write_u8(item.color())
        .write_u32(item.id())
        .write_u16(this->_position.x)
        .write_u16(this->_position.y)
        .write_u16(to.x)
        .write_u16(to.y)
        .write_u32(0x00000000)
        .write_u8(0x02)
        .write_u8(0x00);

    return ostream;
}

int fb::game::session::builtin_look(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto session = *(fb::game::session**)lua_touserdata(lua, 1);

    if(argc == 1)
    {
        lua_pushinteger(lua, session->look());
        return 1;
    }
    else
    {
        auto value = (uint16_t)lua_tointeger(lua, 2);
        session->look(value);
        return 0;
    }
}

int fb::game::session::builtin_color(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto session = *(fb::game::session**)lua_touserdata(lua, 1);

    if(argc == 1)
    {
        lua_pushinteger(lua, session->color());
        return 1;
    }
    else
    {
        auto value = (uint8_t)lua_tointeger(lua, 2);
        session->color(value);
        return 0;
    }
}

int fb::game::session::builtin_money(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto session = *(fb::game::session**)lua_touserdata(lua, 1);

    if(argc == 1)
    {
        lua_pushinteger(lua, session->money());
        return 1;
    }
    else
    {
        auto value = (uint32_t)lua_tointeger(lua, 2);
        session->money(value);
        return 0;
    }
}

int fb::game::session::builtin_exp(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto session = *(fb::game::session**)lua_touserdata(lua, 1);

    if(argc == 1)
    {
        lua_pushinteger(lua, session->experience());
        return 1;
    }
    else
    {
        auto value = (uint32_t)lua_tointeger(lua, 2);
        session->experience(value);
        return 0;
    }
}

int fb::game::session::builtin_base_hp(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto session = *(fb::game::session**)lua_touserdata(lua, 1);

    if(argc == 1)
    {
        lua_pushinteger(lua, session->base_hp());
        return 1;
    }
    else
    {
        auto value = (uint32_t)lua_tointeger(lua, 2);
        session->base_hp(value);
        return 0;
    }
}

int fb::game::session::builtin_base_mp(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto session = *(fb::game::session**)lua_touserdata(lua, 1);

    if(argc == 1)
    {
        lua_pushinteger(lua, session->base_mp());
        return 1;
    }
    else
    {
        auto value = (uint32_t)lua_tointeger(lua, 2);
        session->base_mp(value);
        return 0;
    }
}

int fb::game::session::builtin_strength(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto session = *(fb::game::session**)lua_touserdata(lua, 1);

    if(argc == 1)
    {
        lua_pushinteger(lua, session->strength());
        return 1;
    }
    else
    {
        auto value = (uint8_t)lua_tointeger(lua, 2);
        session->strength(value);
        return 0;
    }
}

int fb::game::session::builtin_dexterity(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto session = *(fb::game::session**)lua_touserdata(lua, 1);

    if(argc == 1)
    {
        lua_pushinteger(lua, session->dexteritry());
        return 1;
    }
    else
    {
        auto value = (uint8_t)lua_tointeger(lua, 2);
        session->dexteritry(value);
        return 0;
    }
}

int fb::game::session::builtin_intelligence(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto session = *(fb::game::session**)lua_touserdata(lua, 1);

    if(argc == 1)
    {
        lua_pushinteger(lua, session->intelligence());
        return 1;
    }
    else
    {
        auto value = (uint8_t)lua_tointeger(lua, 2);
        session->intelligence(value);
        return 0;
    }
}

int fb::game::session::builtin_item(lua_State* lua)
{
    auto session = *(fb::game::session**)lua_touserdata(lua, 1);
    auto item = (fb::game::item*)nullptr;
    if(lua_isnumber(lua, 2))
    {
        auto index = (uint8_t)lua_tointeger(lua, 2);
        item = session->items[index];
    }
    else if(lua_isstring(lua, 2))
    {
        auto name = lua_tostring(lua, 2);
        item = session->items.find(name);
    }

    if(item == nullptr)
        lua_pushnil(lua);
    else
        item->to_lua(lua);

    return 1;
}

int fb::game::session::builtin_items(lua_State* lua)
{
    auto session = *(fb::game::session**)lua_touserdata(lua, 1);

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
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto session = *(fb::game::session**)lua_touserdata(lua, 1);
    auto index = (uint8_t)lua_tointeger(lua, 2);
    auto drop_all = lua_toboolean(lua, 3);

    auto dropped = session->items.drop(index - 1, drop_all ? 1 : -1);
    if(dropped != nullptr)
        dropped->to_lua(lua);
    else
        lua_pushnil(lua);

    return 1;
}

int fb::game::session::builtin_mkitem(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto session = *(fb::game::session**)lua_touserdata(lua, 1);
    auto name = lua_tostring(lua, 2);
    auto store = argc < 3 ? true : lua_toboolean(lua, 3);

    if(store == false)
        return object::builtin_mkitem(lua);

    auto master = game::master::get().name2item(name);
    if(master == nullptr)
    {
        lua_pushnil(lua);
    }
    else
    {
        auto acceptor = lua::env<fb::game::acceptor>("acceptor");
        auto item = master->make<fb::game::item>(acceptor);
        auto slot = session->items.add(item);
        item->to_lua(lua);

        acceptor->send_stream(*session, session->items.make_update_stream(slot), acceptor::scope::SELF);
    }

    return 1;
}

int fb::game::session::builtin_rmitem(lua_State* lua)
{
    try
    {
        auto argc = lua_gettop(lua);
        auto session = *(fb::game::session**)lua_touserdata(lua, 1);
        auto index = uint8_t(0);
        auto count = argc < 3 ? 1 : (int)lua_tointeger(lua, 3);

        if(lua_isuserdata(lua, 2))
        {
            auto item = *(fb::game::item**)lua_touserdata(lua, 2);
            if(item == nullptr)
                throw std::exception();

            index = session->items.to_index(item->based<fb::game::item::master>());
        }
        else if(lua_isnumber(lua, 2))
        {
            index = (uint8_t)lua_tointeger(lua, 2) - 1;
        }
        else if(lua_isstring(lua, 2))
        {
            auto name = lua_tostring(lua, 2);
            if(name == nullptr)
                throw std::exception();

            auto master = game::master::get().name2item(name);
            if(master == nullptr)
                throw std::exception();

            index = session->items.to_index(master);
        }
        else
        {
            throw std::exception();
        }


        auto acceptor = lua::env<fb::game::acceptor>("acceptor");
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
    auto argc = lua_gettop(lua);
    auto session = *(fb::game::session**)lua_touserdata(lua, 1);
    if(argc == 1)
    {
        lua_pushinteger(lua, session->state());
        return 1;
    }
    else
    {
        auto value = fb::game::state(lua_tointeger(lua, 2));
        session->state(value);
        return 0;
    }
}

int fb::game::session::builtin_disguise(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto session = *(fb::game::session**)lua_touserdata(lua, 1);

    if(argc == 1)
    {
        lua_pushinteger(lua, session->disguise().value());
        return 1;
    }
    else if(luaL_checkinteger(lua, 2))
    {
        session->disguise((uint16_t)lua_tointeger(lua, 2));
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
    auto argc = lua_gettop(lua);
    auto session = *(fb::game::session**)lua_touserdata(lua, 1);

    if(argc == 1)
    {
        auto cls = session->_class;
        auto promotion = session->_promotion;

        auto cls_name = game::master::get().class2name(cls, promotion);
        if(cls_name == nullptr)
        {
            lua_pushnil(lua);
        }
        else
        {
            lua_pushstring(lua, cls_name->c_str());
        }
    }
    else
    {
        auto cls_name = lua_tostring(lua, 2);
        uint8_t cls, promotion;
        if(game::master::get().name2class(cls_name, &cls, &promotion) == false)
        {
            lua_pushboolean(lua, false);
        }
        else
        {
            auto acceptor = lua::env<fb::game::acceptor>("acceptor");
            acceptor->send_stream(*session, session->make_id_stream(), acceptor::scope::SELF);
            acceptor->send_stream(*session, session->make_state_stream(state_level::LEVEL_MAX), acceptor::scope::SELF);
            lua_pushboolean(lua, true);
        }
    }

    return 1;
}

int fb::game::session::builtin_level(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto session = *(fb::game::session**)lua_touserdata(lua, 1);

    if(argc == 1)
    {
        lua_pushinteger(lua, session->level());
        return 1;
    }
    else
    {
        auto level = std::max(0, std::min((int)lua_tointeger(lua, 2), 255));
        session->level(level);

        auto acceptor = lua::env<fb::game::acceptor>("acceptor");
        acceptor->send_stream(*session, session->make_state_stream(state_level::LEVEL_MAX), acceptor::scope::SELF);
        return 0;
    }
}

int fb::game::session::builtin_group(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto session = *(fb::game::session**)lua_touserdata(lua, 1);

    auto group = session->group();
    if(group == nullptr)
        lua_pushnil(lua);
    else
        group->to_lua(lua);

    return 1;
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

fb::game::lua::dialog::dialog(fb::game::session& owner, listener* listener) : 
    _owner(owner),
    _thread(nullptr),
    _listener(listener)
{
}

fb::game::lua::dialog::~dialog()
{
    if(this->_thread != nullptr)
        delete this->_thread;
}

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

fb::game::lua::dialog& fb::game::lua::dialog::pushobject(const lua::luable* object)
{
    if(this->_thread != nullptr)
        this->_thread->pushobject(object);

    return *this;
}

fb::game::lua::dialog& fb::game::lua::dialog::pushobject(const lua::luable& object)
{
    if(this->_thread != nullptr)
        this->_thread->pushobject(object);
    
    return *this;
}

fb::game::lua::dialog& fb::game::lua::dialog::from(const char* format, ...)
{
    if(this->_thread != nullptr)
        delete this->_thread;

    this->_thread = new lua::thread();

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
    {
        delete this->_thread;
        this->_thread = nullptr;
    }
    return state;
}

void fb::game::lua::dialog::show(const object::master& object, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction)
{
    if(this->_listener != nullptr)
        this->_listener->on_dialog(this->_owner, object, message, button_prev, button_next, interaction);
}

void fb::game::lua::dialog::show(const object& object, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction)
{
    return this->show(*object.based<object::master>(), message, button_prev, button_next, interaction);
}

void fb::game::lua::dialog::show(const npc::master& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction)
{
    if(this->_listener != nullptr)
        this->_listener->on_dialog(this->_owner, npc, message, menus, interaction);
}

void fb::game::lua::dialog::show(const npc& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction)
{
    this->show(*npc.based<npc::master>(), message, menus, interaction);
}

void fb::game::lua::dialog::show(const npc::master& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction)
{
    if(this->_listener != nullptr)
        this->_listener->on_dialog(this->_owner, npc, message, item_slots, interaction);
}

void fb::game::lua::dialog::show(const npc& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction)
{
    return this->show(*npc.based<npc::master>(), message, item_slots, interaction);
}

void fb::game::lua::dialog::show(const npc::master& npc, const std::string& message, const std::vector<item::master*>& cores, fb::game::dialog::interaction interaction)
{
    if(this->_listener != nullptr)
        this->_listener->on_dialog(this->_owner, npc, message, cores, interaction);
}

void fb::game::lua::dialog::show(const npc& npc, const std::string& message, const std::vector<item::master*>& cores, fb::game::dialog::interaction interaction)
{
    this->show(*npc.based<npc::master>(), message, cores, interaction);
}

void fb::game::lua::dialog::input(const npc::master& npc, const std::string& message, fb::game::dialog::interaction interaction)
{
    if(this->_listener != nullptr)
        this->_listener->on_dialog(this->_owner, npc, message, interaction);
}

void fb::game::lua::dialog::input(const npc& npc, const std::string& message, fb::game::dialog::interaction interaction)
{
    this->input(*npc.based<npc::master>(), message, interaction);
}

void fb::game::lua::dialog::input(const npc::master& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen, bool prev, fb::game::dialog::interaction interaction)
{
    if(this->_listener != nullptr)
        this->_listener->on_dialog(this->_owner, npc, message, top, bottom, maxlen, prev, interaction);
}

void fb::game::lua::dialog::input(const npc& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen, bool prev, fb::game::dialog::interaction interaction)
{
    this->input(*npc.based<npc::master>(), message, top, bottom, maxlen, prev, interaction);
}
