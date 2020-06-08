#include "session.h"
#include "map.h"
#include "item.h"
#include "db.h"
#include "fb_game.h"

using namespace fb::game;

session::session(fb::socket* socket) : 
    life((life::core*)nullptr, 0, 0, 0, 0),
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
    trade(*this),
    items(*this),
    dialog_thread(nullptr)
{
    memset(this->_options, 0, sizeof(this->_options));
}

session::~session()
{
    delete this->_core;

    if(dialog_thread != nullptr)
        delete dialog_thread;
}

void fb::game::session::send(const fb::ostream& stream, bool encrypt, bool wrap)
{
    this->_socket->send(stream, encrypt, wrap);
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

void fb::game::session::color(uint16_t value)
{
    this->_color = value;
}

uint8_t fb::game::session::dress_color() const
{
    return this->_dress_color;
}

void fb::game::session::dress_color(uint16_t value)
{
    this->_dress_color = value;
}

uint16_t fb::game::session::disguise() const
{
    return this->_disguise;
}

void fb::game::session::disguise(uint16_t value)
{
    this->_disguise = value;
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

uint32_t fb::game::session::id() const
{
    return life::id();
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
}

bool fb::game::session::level_up()
{
    if(this->max_level())
        return false;

    this->_level++;
    
    auto&           classes = db::classes();
    this->strength_up(classes[this->_class]->level_abilities[this->_level]->strength);
    this->intelligence_up(classes[this->_class]->level_abilities[this->_level]->intelligence);
    this->dexteritry_up(classes[this->_class]->level_abilities[this->_level]->dexteritry);

    this->base_hp_up(classes[this->_class]->level_abilities[this->_level]->base_hp + std::rand() % 10);
    this->base_mp_up(classes[this->_class]->level_abilities[this->_level]->base_mp + std::rand() % 10);

    this->hp(this->base_hp());
    this->mp(this->base_mp());

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
    this->_state = value;
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
    this->_strength + value;
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
    this->_experience = value;
}

uint32_t fb::game::session::experience_add(uint32_t value)
{
    uint32_t capacity = 0xFFFFFFFF - this->_experience;
    
    if(value > capacity)
    {
        this->_experience += capacity;
        return value - capacity;
    }
    else
    {
        this->_experience += value;
        return 0;
    }
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

    return db::required_exp(this->_class, this->_level+1) - this->experience();
}

uint32_t fb::game::session::money() const
{
    return this->_money;
}

void fb::game::session::money(uint32_t value)
{
    this->_money = value;
}

uint32_t fb::game::session::money_add(uint32_t value) // 먹고 남은 값 리턴
{
    uint32_t capacity = 0xFFFFFFFF - this->_money;
    if(value > capacity)
    {
        this->_money += capacity;
        return value - capacity;
    }
    else
    {
        this->_money += value;
        return 0;
    }
}

uint32_t fb::game::session::money_reduce(uint32_t value)
{
    if(this->_money < value)
    {
        uint32_t lack = value - this->_money;
        this->_money = 0;
        return lack;
    }
    else
    {
        this->_money -= value;
        return 0;
    }
}

uint32_t fb::game::session::random_damage() const
{
    return this->_damage;
}

void fb::game::session::random_damage(uint8_t value)
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

    this->_options[key] = value;
}

bool fb::game::session::option_toggle(options key)
{
    if(key > 0x1B)
        return false;

    this->_options[key] = !this->_options[key];
    return this->_options[key];
}

const std::vector<legend>& fb::game::session::legends() const
{
    return this->_legends;
}

void fb::game::session::legends_add(uint8_t look, uint8_t color, const std::string& content)
{
    this->_legends.push_back(legend(look, color, content));
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

fb::ostream fb::game::session::make_show_stream() const
{
    return this->make_visual_stream(false);
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
        ostream.write_u16(this->_disguise)
            .write_u8(this->_dress_color);
    }
    else
    {
        ostream.write_u16(this->look()) // face
            .write_u8(this->color()); // hair color

        auto armor = this->items.armor();
        if(armor != nullptr)
        {
            ostream.write_u8(armor->dress())
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
            ostream.write_u8(shield->dress())
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
        .write_u8(this->_defensive.physical)
        .write_u8(this->random_damage())
        .write_u8(this->hit())
        .write("클랜 정보")
        .write("클랜 타이틀")
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

    auto                    class_name = db::class2name(this->_class, this->_promotion);
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

    ostream.write_u8(this->_legends.size());
    for(auto legend : this->_legends)
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
    const auto              class_name = db::class2name(this->_class, this->_promotion);
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
        ostream.write_u16(this->_disguise)
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
    ostream.write_u8(this->_legends.size());
    for(auto legend : this->_legends)
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

fb::ostream fb::game::session::make_throw_item_stream(const item& item) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x16)
        .write_u32(this->id())
        .write_u16(item.look())
        .write_u8(item.color())
        .write_u32(item.id())
        .write_u16(this->_position.x)
        .write_u16(this->_position.y)
        .write_u16(item.x())
        .write_u16(item.y())
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
        auto value = lua_tointeger(lua, 2);
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
        auto value = lua_tointeger(lua, 2);
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
        auto value = lua_tointeger(lua, 2);
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
        auto value = lua_tointeger(lua, 2);
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
        auto value = lua_tointeger(lua, 2);
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
        auto value = lua_tointeger(lua, 2);
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
        auto value = lua_tointeger(lua, 2);
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
        auto value = lua_tointeger(lua, 2);
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
        auto value = lua_tointeger(lua, 2);
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
        auto index = lua_tointeger(lua, 2);
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
    auto index = lua_tointeger(lua, 2);
    auto drop_all = lua_toboolean(lua, 3);

    auto dropped = acceptor->macro_drop_item(*session, index - 1, drop_all);
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

    auto core = db::name2item(name);
    if(core == nullptr)
    {
        lua_pushnil(lua);
    }
    else
    {
        auto item = core->make<fb::game::item>();
        auto slot = session->items.add(item);

        auto acceptor = lua::env<fb::game::acceptor>("acceptor");
        acceptor->send_stream(*session, session->items.make_update_stream(slot), acceptor::scope::SELF);
    }
}

int fb::game::session::builtin_rmitem(lua_State* lua)
{
    try
    {
        auto argc = lua_gettop(lua);
        auto session = *(fb::game::session**)lua_touserdata(lua, 1);
        auto index = 0;
        auto count = argc < 3 ? 1 : lua_tointeger(lua, 3);

        if(lua_isuserdata(lua, 2))
        {
            auto item = *(fb::game::item**)lua_touserdata(lua, 2);
            if(item == nullptr)
                throw std::exception();

            index = session->items.to_index(item->based<fb::game::item::core>());
        }
        else if(lua_isnumber(lua, 2))
        {
            index = lua_tointeger(lua, 2) - 1;
        }
        else if(lua_isstring(lua, 2))
        {
            auto name = lua_tostring(lua, 2);
            if(name == nullptr)
                throw std::exception();

            auto core = db::name2item(name);
            if(core == nullptr)
                throw std::exception();

            index = session->items.to_index(core);
        }
        else
        {
            throw std::exception();
        }


        auto acceptor = lua::env<fb::game::acceptor>("acceptor");
        auto dropped = acceptor->macro_remove_item(*session, index, count, item::delete_attr::DELETE_REMOVED);
        if(dropped == nullptr)
            return 0;

        auto map = dropped->map();
        if(map != nullptr)
            map->objects.remove(*dropped);

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

        auto acceptor = lua::env<fb::game::acceptor>("acceptor");
        acceptor->send_stream(*session, session->make_visual_stream(true), acceptor::scope::PIVOT);
        return 0;
    }
}

int fb::game::session::builtin_disguise(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto session = *(fb::game::session**)lua_touserdata(lua, 1);

    if(argc == 1)
    {
        lua_pushinteger(lua, session->disguise());
        return 1;
    }
    else if(luaL_checkinteger(lua, 2))
    {
        auto value = lua_tointeger(lua, 2);
        if(value == 0)
        {
            session->state(state::NORMAL);
            session->disguise(0);
        }
        else
        {
            session->state(state::DISGUISE);
            session->disguise(value);
        }

        auto acceptor = lua::env<fb::game::acceptor>("acceptor");
        acceptor->send_stream(*session, session->make_visual_stream(true), acceptor::scope::PIVOT);
        acceptor->send_stream(*session, session->make_state_stream(state_level::LEVEL_MAX), acceptor::scope::SELF);
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

        auto cls_name = db::class2name(cls, promotion);
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
        if(db::name2class(cls_name, &cls, &promotion) == false)
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