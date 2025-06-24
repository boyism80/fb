#include <fb/game/character.h>
#include <fb/game/context.h>
#include <fb/game/regex.h>

using namespace fb::game;

character::character(fb::game::context& context, fb::socket<character>& socket) :
    life(context, context.model.life[0], initial_params{{.id = (uint32_t)socket.fd()}}),
    listener(context.listener),
    _socket(socket)
{ }

character::~character()
{
    if (this->dialog != nullptr)
        this->dialog->release();
}

async::task<size_t> character::send(const fb::stream& stream, bool encrypt, bool wrap)
{
    this->assert_thread();

    co_return co_await this->_socket.send(stream, encrypt, wrap);
}

async::task<size_t> character::send(const fb::protocol::header& response, bool encrypt, bool wrap)
{
    this->assert_thread();

    co_return co_await this->_socket.send(response, encrypt, wrap);
}

OBJECT_TYPE character::what() const
{
    this->assert_thread();

    return OBJECT_TYPE::CHARACTER;
}

async::task<bool> character::map(std::shared_ptr<fb::game::map> map,
                                 const fb::model::point16_t&    position,
                                 DESTROY_TYPE                   destroy_type)
{
    if (this->_thread == nullptr)
        co_return true;

    this->assert_thread();

    if (this->_map != map)
    {
        if (this->trade.trading())
            this->trade.cancel();
    }

    auto switch_process = (map != nullptr && map->active == false);
    if (switch_process)
    {
        co_return co_await this->listener.on_transfer(*this, *map, position);
    }
    else
    {
        if (co_await object::map(map, position) == false)
            co_return false;

        co_return true;
    }
}

uint32_t character::limited_exp(uint32_t exp) const
{
#if defined DEBUG | defined _DEBUG
    return exp * 100;
#else
    if (this->max_level())
        return exp;

    auto range = this->context.model.ability[this->_class][this->_level].exp;
    return std::min(uint32_t(range / 100.0f * 3.3f + 1), exp);
#endif
}

uint32_t character::auto_attack_damage(MOB_SIZE size) const
{
    this->assert_thread();

    auto weapon = this->items.weapon();
    auto model  = weapon != nullptr ? &weapon->based<fb::model::weapon>() : nullptr;

    // TODO: 이거 크리터졌을때가 아니라 때리는 몹 타입으로 small, large 써야함

    if (weapon == nullptr) // no weapon
    {
        return 1 + std::rand() % 5;
    }
    else
    {
        auto& range = size == MOB_SIZE::SMALL ? model->damage_small : model->damage_large;
        return std::max(uint32_t(1), range.min) + std::rand() % std::max(uint32_t(1), range.max);
    }
}

uint32_t character::damage(uint32_t value, std::shared_ptr<fb::game::object> from, bool critical)
{
    this->assert_thread();

    if (this->alive() == false)
        return 0;

    auto result = life::damage(value, from, critical);
    if (from == nullptr)
        return result;

    for (auto mob : this->spawned_mobs())
    {
        if (mob->target() != nullptr)
            continue;

        if (from->is(OBJECT_TYPE::LIFE) == false)
            continue;

        mob->target(std::static_pointer_cast<fb::game::life>(from));
    }

    if (this->_hp == 0)
    {
        this->kill(from, DESTROY_TYPE::DEAD);
        return result;
    }

    for (auto& [parts, equipment] : this->items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto durability = equipment->durability();
        if (durability.has_value() == false)
            continue;

        auto& model = equipment->based<fb::model::equipment>();
        if (equipment->durability_down(1))
        {
            auto equipment = this->items.equipment_off(parts);
            this->message(std::format("{} 깨졌습니다.", equipment->name()));
            equipment.reset();
        }
    }
    return result;
}

character::operator fb::socket<character>& ()
{
    return this->_socket;
}

bool character::inited() const
{
    if (this->_thread == nullptr)
        return false;

    return true;
}

uint32_t character::id() const
{
    return this->_id;
}

void character::id(uint32_t id)
{
    this->assert_thread();

    this->_id = id;
}

uint32_t character::fd()
{
    return this->_socket.fd();
}

ROLE character::role() const
{
    return this->_role;
}

void character::role(ROLE value)
{
    this->assert_thread();

    this->_role = value;
}

void character::attack(DURATION duration)
{
    this->assert_thread();
    try
    {
        this->assert_state({STATE::RIDING, STATE::GHOST});
        life::attack(duration);
    }
    catch (const std::exception& e)
    {
        this->message(e.what());
    }
}

void character::action(ACTION action, DURATION duration, uint8_t sound)
{
    this->assert_thread();

    try
    {
        this->assert_state({STATE::GHOST, STATE::RIDING});
        life::action(action, duration, sound);
    }
    catch (std::exception& e)
    {
        this->message(e.what());
    }
}

const std::string& character::name() const
{
    this->assert_thread();

    return this->_name;
}

void character::name(const std::string& value)
{
    this->assert_thread();

    this->_name = value;
}

void character::pw(const std::string& value)
{
    this->assert_thread();

    this->_pw = value;
}

const std::optional<uint32_t>& fb::game::character::birthday() const
{
    return this->_birthday;
}

void fb::game::character::birthday(const std::optional<uint32_t>& value)
{
    this->_birthday = value;
}

const fb::model::datetime& character::updated_date() const
{
    this->assert_thread();

    return this->_updated_date;
}

void character::updated_date(const fb::model::datetime& value)
{
    this->assert_thread();

    this->_updated_date = value;
}

uint16_t character::look() const
{
    this->assert_thread();

    return this->_look;
}

void character::look(uint16_t value)
{
    this->assert_thread();

    this->_look = value;
    this->update_external(false);
}

uint8_t character::color() const
{
    this->assert_thread();

    return this->_color;
}

void character::color(uint8_t value)
{
    this->assert_thread();

    this->_color = value;
    this->update_external(false);
}

std::optional<uint8_t> character::armor_color() const
{
    this->assert_thread();

    return this->_armor_color;
}

void character::armor_color(std::optional<uint8_t> value)
{
    this->assert_thread();

    this->_armor_color = value;
    this->update_external(false);
}

uint8_t character::current_armor_color() const
{
    this->assert_thread();

    auto armor = this->items.armor();
    return this->_armor_color.value_or(armor != nullptr ? armor->color() : 0x00);
}

std::optional<uint16_t> character::disguise() const
{
    this->assert_thread();

    return this->_disguise;
}

void character::disguise(uint16_t value)
{
    this->assert_thread();

    this->_disguise = value;
    this->state(STATE::DISGUISE);
}

void character::undisguise()
{
    this->assert_thread();

    this->_disguise = std::nullopt;
    if (this->state() == STATE::DISGUISE)
        this->state(STATE::NORMAL);

    this->update(STATE_LEVEL::LEVEL_MAX);
}

NATION character::nation() const
{
    this->assert_thread();

    return this->_nation;
}

bool character::nation(NATION value)
{
    this->assert_thread();

    if (value != NATION::GOGURYEO && value != NATION::BUYEO)
        return false;

    this->_nation = value;
    return true;
}

CREATURE character::creature() const
{
    this->assert_thread();

    return this->_creature;
}

bool character::creature(CREATURE value)
{
    this->assert_thread();

    if (value != CREATURE::DRAGON && value != CREATURE::PHOENIX && value != CREATURE::TIGER &&
        value != CREATURE::TURTLE)
        return false;

    this->_creature = value;
    return true;
}

uint8_t character::level() const
{
    this->assert_thread();

    return this->_level;
}

void character::level(uint8_t value)
{
    this->assert_thread();

    this->_level = value;
    this->update(STATE_LEVEL::LEVEL_MAX);
}

bool character::level_up()
{
    this->assert_thread();

    if (this->max_level())
        return false;

    auto& ability = this->context.model.ability[this->_class][this->_level];
    this->base_str(this->base_str() + ability.strength);
    this->base_int(this->base_int() + ability.intelligence);
    this->base_dex(this->base_dex() + ability.dexterity);
    this->base_hp(this->base_hp() + ability.hp + std::rand() % 10);
    this->base_mp(this->base_mp() + ability.mp + std::rand() % 10);

    this->hp(this->base_hp());
    this->mp(this->base_mp());

    this->level(this->_level + 1);
    this->message(_TEXT(MESSAGE_LEVEL_UP));

    this->listener.on_level_up(*this);
    return true;
}

bool character::max_level() const
{
    this->assert_thread();

    return this->context.model.ability[this->_class].contains(this->_level + 1) == false;
}

SEX character::sex() const
{
    this->assert_thread();

    return this->_sex;
}

void character::sex(SEX value)
{
    this->assert_thread();

    this->_sex = value;
    this->update_external(false);
}

STATE character::state() const
{
    this->assert_thread();

    return this->_state;
}

STATE character::state_to(const fb::game::object& to) const
{
    this->assert_thread();

    if (this == &to)
        return this->_state;

    if (to.is(OBJECT_TYPE::CHARACTER) == false)
        return this->_state;

    const auto& ch = static_cast<const fb::game::character&>(to);

    if (this->_state == STATE::HALF_CLOACK)
    {
        if (this->role() < ch.role())
            return STATE::HALF_CLOACK;

        if (ch.detect())
            return STATE::HALF_CLOACK;

        auto& g1 = this->_group;
        auto& g2 = ch._group;
        if (g1 != nullptr && g2 != nullptr && g1.get() == g2.get())
            return STATE::HALF_CLOACK;

        return STATE::CLOACK;
    }

    if (this->_state == STATE::TRANSLUCENCY)
    {
        return STATE::CLOACK;
    }

    return this->_state;
}

void character::state(STATE value)
{
    this->assert_thread();

    this->_state = value;
    this->update_external(false);
}

CLASS character::cls() const
{
    this->assert_thread();

    return this->_class;
}

void character::cls(CLASS value)
{
    this->assert_thread();

    this->_class = value;
    this->update_id();
    this->update(STATE_LEVEL::LEVEL_MAX);
}

uint8_t character::promotion() const
{
    this->assert_thread();
    return this->_promotion;
}

void character::promotion(uint8_t value)
{
    this->assert_thread();

    this->_promotion = value;
    this->update_id();
    this->update(STATE_LEVEL::LEVEL_MAX);
}

uint32_t character::exp() const
{
    this->assert_thread();

    return this->_experience;
}

void character::exp(uint32_t value)
{
    this->assert_thread();

    if (this->_experience == value)
        return;

    this->_experience = value;
    this->update(STATE_LEVEL::EXP_MONEY);
}

uint32_t character::add_exp(uint32_t value, bool limit, bool notify)
{
    this->assert_thread();

    if (limit)
        value = this->limited_exp(value);

    auto capacity = 0xFFFFFFFF - this->_experience;
    auto lack     = 0;

    try
    {
        // 직업이 없는 경우 정확히 5레벨을 찍을 경험치만 얻도록 제한
        if (this->_class == CLASS::NONE)
        {
            auto require = this->context.model.ability[CLASS::NONE][5].stacked_exp;
            if (this->_experience > require)
                value = 0;

            if (this->_experience + value > require)
                value = require - this->_experience;
        }

        if (value > 0)
        {
            if (value > capacity)
            {
                lack = value - capacity;
                this->exp(this->_experience + capacity);
            }
            else
            {
                this->exp(this->_experience + value);
            }

            if (notify)
                this->message(std::format("경험치가 {}({}%) 올랐습니다.", value, int(this->experience_percent())));
        }

        if (this->context.model.ability.contains(this->_class) == false)
            throw std::runtime_error("what?");

        while (true)
        {
            if (this->max_level())
                break;

            auto& next = this->context.model.ability[this->_class][this->_level];
            if (next.exp == 0)
                break;

            if (this->_experience < next.stacked_exp)
                break;

            if (this->level_up() == false)
                break;
        }

        if (this->_class == CLASS::NONE && this->max_level())
            throw std::runtime_error(_TEXT(MESSAGE_EXCEPTION_REQUIRE_CLASS));
    }
    catch (std::exception& e)
    {
        this->message(e.what());
    }

    return lack;
}

uint32_t character::reduce_exp(uint32_t value)
{
    this->assert_thread();

    if (this->_experience < value)
    {
        uint32_t lack     = value - this->_experience;
        this->_experience = 0;
        this->update(STATE_LEVEL::EXP_MONEY);
        return lack;
    }
    else
    {
        this->_experience -= value;
        this->update(STATE_LEVEL::EXP_MONEY);
        return 0;
    }
}

uint32_t character::experience_remained() const
{
    this->assert_thread();

    if (this->max_level())
        return 0;

    if (this->context.model.ability.contains(this->_class) == false)
        return 0;

    if (this->context.model.ability[this->_class].contains(this->_level) == false)
        return 0;

    return this->context.model.ability[this->_class][this->_level].stacked_exp - this->exp();
}

float character::experience_percent() const
{
    this->assert_thread();

    if (this->max_level())
    {
        return std::min(100.0f, (this->_experience / float(0xFFFFFFFF)) * 100.0f);
    }
    else
    {
        auto level    = this->level();
        auto required = this->context.model.ability[this->_class][level].exp;

        auto prev_stack_exp = uint32_t{0};
        if (this->context.model.ability[this->_class].contains(level - 1))
            prev_stack_exp = this->context.model.ability[this->_class][level - 1].stacked_exp;
        else if (this->context.model.ability[CLASS::NONE].contains(level - 1))
            prev_stack_exp = this->context.model.ability[CLASS::NONE][level - 1].stacked_exp;

        return std::min(100.0f, ((this->_experience - prev_stack_exp) / float(required)) * 100.0f);
    }
}

uint32_t character::money() const
{
    this->assert_thread();

    return this->_money;
}

void character::money(uint32_t value)
{
    this->assert_thread();

    this->_money = value;
    this->update(STATE_LEVEL::EXP_MONEY);
}

uint32_t character::money_add(uint32_t value) // 먹고 남은 값 리턴
{
    this->assert_thread();

    uint32_t capacity = 0xFFFFFFFF - this->_money;
    uint32_t lack     = 0;
    if (value > capacity)
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

void character::money_reduce(uint32_t value)
{
    this->assert_thread();
    value = std::min(this->_money, value);
    this->money(this->_money - value);
}

fb::game::cash* character::money_drop(uint32_t value)
{
    this->assert_thread();

    try
    {
        this->assert_state({STATE::RIDING, STATE::GHOST});

        if (value == 0)
            return 0;

        value = std::min(this->_money, value);
        this->money_reduce(value);

        // TODO: Phase 3 - Convert to smart pointer return type
        // For now, use make_shared but return raw pointer for compatibility
        auto cash_shared = this->context.make<fb::game::cash>(value);
        auto cash        = cash_shared.get();
        cash->map(this->_map, this->_position);
        this->action(ACTION::PICKUP, DURATION::PICKUP);
        this->message(_TEXT(MESSAGE_MONEY_DROP));
        return cash;
    }
    catch (std::exception& e)
    {
        this->message(e.what());
    }

    return 0;
}

uint32_t character::regenerative() const
{
    this->assert_thread();

    return this->_regenerative;
}

void character::regenerative(uint8_t value)
{
    this->assert_thread();

    this->_regenerative = value;
}

bool character::option(OPTION key) const
{
    this->assert_thread();

    auto opt = static_cast<uint8_t>(key);
    if (opt == 0 || opt > static_cast<uint8_t>(OPTION::EFFECT_SOUND))
        throw std::runtime_error(std::format("invalid setting key : {:#x}", opt));

    return this->_options[opt];
}

void character::option(OPTION key, bool value, bool notify)
{
    this->assert_thread();

    auto opt = static_cast<uint8_t>(key);
    if (opt == 0 || opt > static_cast<uint8_t>(OPTION::EFFECT_SOUND))
        return;

    if (this->_options[opt] == value)
        return;

    this->update(STATE_LEVEL::LEVEL_MIN);
    this->_options[opt] = value;

    if (notify)
        this->listener.on_option_changed(*this, key, value);
    this->update_option();
}

bool character::option_toggle(OPTION key, bool notify)
{
    this->assert_thread();

    auto opt = static_cast<uint8_t>(key);
    if (opt == 0 || opt > static_cast<uint8_t>(OPTION::EFFECT_SOUND))
        throw std::runtime_error(std::format("invalid setting key : {:#x}", opt));

    this->option(key, !this->_options[opt], notify);
    return this->_options[opt];
}

void character::update_option()
{
    this->listener.on_update_option(*this);
}

void character::update_map(const fb::game::map& map)
{
    this->listener.on_update_map(*this, map);
}

void character::update_map()
{
    if (this->_map != nullptr)
        this->update_map(*this->_map);
}

void character::update_map(const fb::game::map& map, const fb::model::point16_t& begin, const fb::model::size8_t& size)
{
    this->listener.on_update_map(*this, map, begin, size);
}

void character::update_bgm(uint16_t bgm, uint8_t volume)
{
    this->listener.on_update_bgm(*this, bgm, volume);
}

void character::update_buff()
{
    this->listener.on_update_buff(*this, this->buffs);
}

void character::update_internal()
{
    this->listener.on_update_internal(*this);
}

void character::update_time(uint16_t hours)
{
    this->listener.on_update_time(*this, hours);
}

void character::init()
{
    this->listener.on_character_init(*this);
}

void character::update_position()
{
    this->listener.on_update_position(*this);
}

const std::string& character::title() const
{
    this->assert_thread();

    return this->_title;
}

void character::title(const std::string& value)
{
    this->assert_thread();

    this->_title = value;
}

const shared_group_lock& character::group() const
{
    this->assert_thread();

    return this->_group;
}

shared_group_lock& character::group()
{
    this->assert_thread();

    return this->_group;
}

void character::group(shared_group_lock& value)
{
    this->assert_thread();

    this->_group = value;
}

const std::optional<uint32_t>& character::clan_id() const
{
    return this->_clan_id;
}

void character::clan_id(std::optional<uint32_t> value)
{
    this->assert_thread();

    this->_clan_id = value;
}

void character::clan_reset()
{
    this->assert_thread();

    this->_clan_id.reset();
    this->update_external(true);
}

void character::assert_state(STATE value) const
{
    this->assert_thread();

    static const auto error = std::map<STATE, const std::string>{
        {STATE::GHOST,    _TEXT(MESSAGE_EXCEPTION_GHOST)   },
        {STATE::RIDING,   _TEXT(MESSAGE_EXCEPTION_RIDDING) },
        {STATE::DISGUISE, _TEXT(MESSAGE_EXCEPTION_DISGUISE)}
    };

    if (this->_state == value)
        throw std::runtime_error(error.at(value));
}

void character::assert_state(const std::vector<STATE>& values) const
{
    this->assert_thread();

    for (auto value : values)
        this->assert_state(value);
}

bool character::move(const fb::model::point16_t& before)
{
    this->assert_thread();

    return this->move(this->_direction, before);
}

bool character::move(DIRECTION direction, const fb::model::point16_t& before)
{
    this->assert_thread();

    if (this->_position != before)
    {
        this->update_position();
        return false;
    }
    else if (object::move(direction) == false)
    {
        this->update_position();
        return false;
    }
    else
    {
        return true;
    }
}

void character::ride(mob& horse)
{
    this->assert_thread();

    try
    {
        this->assert_state({STATE::GHOST, STATE::DISGUISE});

        if (this->state() == STATE::RIDING)
            throw std::runtime_error(_TEXT(MESSAGE_RIDE_ALREADY_RIDE));

        if (horse.based<fb::model::mob>() != this->context.model.mob[fb::model::const_value::mob::horse])
            throw std::runtime_error(_TEXT(MESSAGE_EXCEPTION_NO_CONVEYANCE));

        if (horse.map() != this->_map)
            throw std::runtime_error(_TEXT(MESSAGE_ERROR_UNKNOWN));

        horse.map(nullptr);
        this->state(STATE::RIDING);
        horse.kill();
        this->message(_TEXT(MESSAGE_RIDE_ON));
    }
    catch (std::exception& e)
    {
        this->message(e.what());
    }
}

void character::ride()
{
    this->assert_thread();

    try
    {
        this->assert_state({STATE::GHOST, STATE::DISGUISE});

        auto front = this->forward(OBJECT_TYPE::MOB);
        if (front == nullptr)
            throw std::runtime_error(_TEXT(MESSAGE_EXCEPTION_NO_CONVEYANCE));

        this->ride(static_cast<mob&>(*front));
    }
    catch (std::exception& e)
    {
        this->message(e.what());
    }
}

void character::unride()
{
    this->assert_thread();

    try
    {
        this->assert_state({STATE::GHOST, STATE::DISGUISE});
        if (this->state() != STATE::RIDING)
            throw std::runtime_error(_TEXT(MESSAGE_RIDE_UNRIDE));

        auto& model = this->context.model.mob[fb::model::const_value::mob::horse];
        // Use smart pointer for horse creation
        auto horse_shared = this->context.make<mob>(model, mob::initial_params{.alive = true});
        auto horse        = horse_shared.get(); // For compatibility with existing code
        horse->map(this->_map, this->front_position());

        this->state(STATE::NORMAL);
        this->message(_TEXT(MESSAGE_RIDE_OFF));
    }
    catch (std::exception& e)
    {
        this->message(e.what());
    }
}

bool character::alive() const
{
    this->assert_thread();

    return this->_state != STATE::GHOST;
}

bool character::condition(const std::vector<fb::model::dsl>& conditions) const
{
    this->assert_thread();

    for (auto& dsl : conditions)
    {
        switch (dsl.header)
        {
        case DSL::level:
        {
            auto params = fb::model::dsl::level(dsl.params);
            if (params.min.has_value() && *params.min > this->_level)
                return false;

            if (params.max.has_value() && *params.max < this->_level)
                return false;
        }
        break;

        case DSL::sex:
        {
            auto params = fb::model::dsl::sex(dsl.params);
            if (ENUM_IN(params.value, this->_sex) == false)
                return false;
        }
        break;

        case DSL::strength:
        {
            auto params = fb::model::dsl::strength(dsl.params);
            if (params.value > this->str())
                return false;
        }
        break;

        case DSL::intelligence:
        {
            auto params = fb::model::dsl::intelligence(dsl.params);
            if (params.value > this->intelligence())
                return false;
        }
        break;

        case DSL::dexterity:
        {
            auto params = fb::model::dsl::dexterity(dsl.params);
            if (params.value > this->dex())
                return false;
        }
        break;

        case DSL::promotion:
        {
            auto params = fb::model::dsl::promotion(dsl.params);
            if (params.value > this->_promotion)
                return false;
        }
        break;

        case DSL::class_t:
        {
            auto params = fb::model::dsl::class_t(dsl.params);
            if (ENUM_IN(params.value, this->_class) == false)
                return false;
        }

        case DSL::role:
        {
            auto params = fb::model::dsl::role(dsl.params);
            if (params.value > this->_role)
                return false;
        }
        break;
        }
    }

    return true;
}

void character::message(const std::string& message, MESSAGE_TYPE type)
{
    this->assert_thread();
    this->listener.on_message(*this, message, type);
}

fb::thread* character::thread() const
{
    if (this->_thread != nullptr)
        return this->_thread;
    else
        return this->context.threads.modular(this->_socket.fd());
}

void fb::game::character::thread(fb::thread* value)
{
    this->_thread = value;
}

void character::assert_thread() const
{
    object::assert_thread();
}

void character::update(STATE_LEVEL value)
{
    this->assert_thread();
    this->listener.on_update(*this, value);
}

fb::protocol::internal::Character character::to_protocol() const
{
    this->assert_thread();

    auto dto             = fb::protocol::internal::Character();
    dto.id               = this->_id;
    dto.name             = this->_name;
    dto.pw               = this->_pw;
    dto.birth            = this->_birthday;
    dto.updated_date     = fb::model::datetime().to_string();
    dto.role             = static_cast<uint8_t>(this->_role);
    dto.look             = this->_look;
    dto.color            = this->_color;
    dto.sex              = (uint16_t)this->_sex;
    dto.nation           = (uint16_t)this->_nation;
    dto.creature         = (uint16_t)this->_creature;
    dto.map              = this->_map != nullptr ? this->_map->model.id : 0;
    dto.position         = fb::protocol::internal::Position{this->_position.x, this->_position.y};
    dto.direction        = (uint8_t)this->_direction;
    dto.state            = (uint8_t)this->_state;
    dto.class_type       = (uint8_t)this->_class;
    dto.promotion        = this->_promotion;
    dto.level            = this->_level;
    dto.exp              = this->_experience;
    dto.money            = this->_money;
    dto.deposited_money  = this->items.deposited();
    dto.disguise         = this->_disguise;
    dto.hp               = this->_hp;
    dto.base_hp          = this->base_hp();
    dto.additional_hp    = 0;
    dto.mp               = this->_mp;
    dto.base_mp          = this->base_mp();
    dto.additional_mp    = 0;
    dto.weapon_color     = std::nullopt;
    dto.helmet_color     = std::nullopt;
    dto.armor_color      = this->_armor_color;
    dto.shield_color     = std::nullopt;
    dto.ring_left_color  = std::nullopt;
    dto.ring_right_color = std::nullopt;
    dto.aux_top_color    = std::nullopt;
    dto.aux_bot_color    = std::nullopt;
    dto.title            = this->_title;

    for (auto& [_, buff] : this->buffs)
    {
        auto time = (uint32_t)(buff->time().count() / 1000);
        dto.buffs.push_back({buff->model.id, time});
    }
    return dto;
}

uint16_t character::unread_mail() const
{
    this->assert_thread();

    return this->_unread_mail;
}

void character::unread_mail(uint16_t value)
{
    this->assert_thread();

    if (this->_unread_mail != value)
    {
        this->_unread_mail = value;
        this->update(STATE_LEVEL::LEVEL_MIN);
    }
}

void character::browse_ch(const character& ch)
{
    this->listener.on_browse_character(*this, ch);
}

void character::item_tooltip(const item& item, uint16_t position)
{
    this->listener.on_item_tooltip(*this, item, position);
}

void character::show_user_list()
{
    this->listener.on_show_user_list(*this);
}

void character::show_bulletin()
{
    this->listener.on_show_bulletin(*this);
}

void character::show_bulletin(const fb::model::bulletin&                    section,
                              const std::list<fb::game::bulletin::article>& articles,
                              BULLETIN_BUTTON_ENABLE                        flag)
{
    this->listener.on_show_bulletin(*this, section, articles, flag);
}

void character::show_bulletin(const fb::game::bulletin::article& article, BULLETIN_BUTTON_ENABLE flag)
{
    this->listener.on_show_bulletin(*this, article, flag);
}

void character::show_mail_box(const std::vector<MailSummary>& mails, MAIL_BUTTON_ENABLE flag)
{
    this->listener.on_show_mail_box(*this, mails, flag);
}

void character::show_mail_box(const Mail& mail, MAIL_BUTTON_ENABLE flag)
{
    this->listener.on_show_mail_box(*this, mail, flag);
}

void character::show_bulletin_message(const std::string& message, bool success, bool mail)
{
    this->listener.on_show_bulletin_message(*this, message, success, mail);
}

void character::show_world_map(uint32_t id, uint16_t index)
{
    this->listener.on_show_world_map(*this, id, index);
}

void character::timer(uint32_t time, TIMER_TYPE type)
{
    this->listener.on_timer(*this, time, type);
}
void character::weather(WEATHER_TYPE weather)
{
    this->listener.on_weather(*this, weather);
}

void character::update_id()
{
    this->listener.on_update_id(*this);
}

void character::weapon_damage(uint16_t value)
{
    this->assert_thread();

    this->_weapon_damage = value;
}

uint16_t character::weapon_damage() const
{
    return this->_weapon_damage;
}

void character::detect(bool value)
{
    this->assert_thread();
    this->_detect = value;

    for (auto& obj : this->nears(OBJECT_TYPE::CHARACTER))
    {
        auto ch = std::static_pointer_cast<fb::game::character>(obj);
        if (ch->state() != STATE::HALF_CLOACK)
            continue;

        ch->update_external(*this, true);
    }
}

bool character::detect() const
{
    return this->_detect;
}

std::shared_ptr<fb::game::mob> character::spawn_mob(const fb::model::mob&       model,
                                                    const fb::model::point16_t& position,
                                                    bool                        owned)
{
    auto map = this->_map;
    if (map == nullptr)
        return nullptr;

    auto  params    = fb::game::mob::initial_params{.alive = true, .owner = owned ? this : nullptr};
    auto& mob_model = static_cast<const fb::model::mob&>(model);
    auto  mob       = std::make_shared<fb::game::mob>(this->context, mob_model, params);
    mob->map(map, position);

    if (owned)
        this->_spawned_mobs.push_back(mob);

    return mob;
}

const std::vector<std::shared_ptr<fb::game::mob>>& character::spawned_mobs() const
{
    return this->_spawned_mobs;
}

bool character::detach_spawned_mob(fb::game::mob& mob)
{
    auto owner = mob.owner.lock();
    if (owner == nullptr)
        return false;

    if (owner.get() != this)
        return false;

    if (this->_spawned_mobs.size() == 0)
        return false;

    for (auto it = this->_spawned_mobs.begin(); it != this->_spawned_mobs.end(); ++it)
    {
        if (it->get() == &mob)
        {
            this->_spawned_mobs.erase(it);
            return true;
        }
    }
    return false;
}

void character::bright(uint8_t value)
{
    this->listener.on_bright(*this, value);
}

uint32_t character::base_hp() const
{
    return this->_max_hp.base;
}

uint32_t character::buff_hp() const
{
    return this->_max_hp.buff;
}

uint32_t character::maxhp() const
{
    auto hp         = fb::game::life::maxhp();
    auto additional = (uint32_t)0;
    for (auto& [_, equipment] : this->items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += model.base_hp;
    }

    auto limit = std::numeric_limits<uint32_t>::max();
    if (limit - hp < additional)
        return limit;

    return hp + additional;
}

void character::base_hp(uint32_t value)
{
    this->assert_thread();
    this->_max_hp.base = value;

    auto flag   = STATE_LEVEL::BASED;
    auto before = this->_hp;
    this->_hp   = std::min(this->_hp, this->maxhp());
    if (before != this->_hp)
        flag |= STATE_LEVEL::HP_MP;

    this->update(flag);
}
void character::buff_hp(uint32_t value)
{
    this->assert_thread();
    this->_max_hp.buff = value;

    auto flag   = STATE_LEVEL::BASED;
    auto before = this->_hp;
    this->_hp   = std::min(this->_hp, this->maxhp());
    if (before != this->_hp)
        flag |= STATE_LEVEL::HP_MP;

    this->update(flag);
}

uint32_t character::base_mp() const
{
    return this->_max_mp.base;
}

uint32_t character::buff_mp() const
{
    return this->_max_mp.buff;
}

uint32_t character::maxmp() const
{
    auto mp         = fb::game::life::maxmp();
    auto additional = (uint32_t)0;
    for (auto& [_, equipment] : this->items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += model.base_mp;
    }

    auto limit = std::numeric_limits<uint32_t>::max();
    if (limit - mp < additional)
        return limit;

    return mp + additional;
}

void character::base_mp(uint32_t value)
{
    this->assert_thread();
    this->_max_mp.base = value;

    auto flag   = STATE_LEVEL::BASED;
    auto before = this->_mp;
    this->_mp   = std::min(this->_mp, this->maxmp());
    if (before != this->_mp)
        flag |= STATE_LEVEL::HP_MP;

    this->update(flag);
}
void character::buff_mp(uint32_t value)
{
    this->assert_thread();
    this->_max_mp.buff = value;

    auto flag   = STATE_LEVEL::BASED;
    auto before = this->_mp;
    this->_mp   = std::min(this->_mp, this->maxmp());
    if (before != this->_mp)
        flag |= STATE_LEVEL::HP_MP;

    this->update(flag);
}

uint8_t character::base_str() const
{
    return this->_str.base;
}

uint8_t character::buff_str() const
{
    return this->_str.buff;
}

uint8_t character::str() const
{
    auto str        = fb::game::life::str();
    auto additional = (uint32_t)0;
    for (auto& [_, equipment] : this->items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += model.strength;
    }

    auto limit = std::numeric_limits<uint32_t>::max();
    if (limit - str < additional)
        return limit;

    return str + additional;
}

void character::base_str(uint8_t value)
{
    this->assert_thread();
    this->_str.base = value;
    this->update(STATE_LEVEL::BASED);
}
void character::buff_str(uint8_t value)
{
    this->assert_thread();
    this->_str.buff = value;
    this->update(STATE_LEVEL::BASED);
}

uint8_t character::base_dex() const
{
    return this->_dex.base;
}

uint8_t character::buff_dex() const
{
    return this->_dex.buff;
}

uint8_t character::dex() const
{
    auto dex        = fb::game::life::dex();
    auto additional = (uint32_t)0;
    for (auto& [_, equipment] : this->items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += model.dexterity;
    }

    auto limit = std::numeric_limits<uint32_t>::max();
    if (limit - dex < additional)
        return limit;

    return dex + additional;
}

void character::base_dex(uint8_t value)
{
    this->assert_thread();
    this->_dex.base = value;
    this->update(STATE_LEVEL::BASED);
}
void character::buff_dex(uint8_t value)
{
    this->assert_thread();
    this->_dex.buff = value;
    this->update(STATE_LEVEL::BASED);
}

uint8_t character::base_int() const
{
    return this->_int.base;
}

uint8_t character::buff_int() const
{
    return this->_int.buff;
}

uint8_t character::intelligence() const
{
    auto intelligence = fb::game::life::intelligence();
    auto additional   = (uint32_t)0;
    for (auto& [_, equipment] : this->items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += model.intelligence;
    }

    auto limit = std::numeric_limits<uint32_t>::max();
    if (limit - intelligence < additional)
        return limit;

    return intelligence + additional;
}

void character::base_int(uint8_t value)
{
    this->assert_thread();
    this->_int.base = value;
    this->update(STATE_LEVEL::BASED);
}
void character::buff_int(uint8_t value)
{
    this->assert_thread();
    this->_int.buff = value;
    this->update(STATE_LEVEL::BASED);
}

int8_t character::base_phydef() const
{
    return this->_phydef.base;
}

int8_t character::buff_phydef() const
{
    return this->_phydef.buff;
}

int8_t character::phydef() const
{
    auto phydef     = fb::game::life::phydef();
    auto additional = (uint32_t)0;
    for (auto& [_, equipment] : this->items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += model.defensive_physical;
    }

    auto sum = (int16_t)phydef + (int16_t)additional;
    return (int8_t)std::max<int16_t>(-127, std::min<int16_t>(128, sum));
}

void character::base_phydef(int8_t value)
{
    this->assert_thread();
    this->_phydef.base = value;
}
void character::buff_phydef(int8_t value)
{
    this->assert_thread();
    this->_phydef.buff = value;
}

int8_t character::base_magdef() const
{
    return this->_magdef.base;
}

int8_t character::buff_magdef() const
{
    return this->_magdef.buff;
}

int8_t character::magdef() const
{
    auto magdef     = fb::game::life::magdef();
    auto additional = (uint32_t)0;
    for (auto& [_, equipment] : this->items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += model.defensive_magical;
    }

    auto sum = (int16_t)magdef + (int16_t)additional;
    return (int8_t)std::max<int16_t>(-127, std::min<int16_t>(128, sum));
}

void character::base_magdef(int8_t value)
{
    this->assert_thread();
    this->_magdef.base = value;
}
void character::buff_magdef(int8_t value)
{
    this->assert_thread();
    this->_magdef.buff = value;
}

uint8_t character::base_dam() const
{
    return this->_dam.base;
}

uint8_t character::buff_dam() const
{
    return this->_dam.buff;
}

uint8_t character::dam() const
{
    auto dam        = fb::game::life::dam();
    auto additional = (uint32_t)0;
    for (auto& [_, equipment] : this->items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += model.damage;
    }

    auto limit = std::numeric_limits<uint32_t>::max();
    if (limit - dam < additional)
        return limit;

    return dam + additional;
}

void character::base_dam(uint8_t value)
{
    this->assert_thread();
    this->_dam.base = value;
}
void character::buff_dam(uint8_t value)
{
    this->assert_thread();
    this->_dam.buff = value;
}

uint8_t character::base_hit() const
{
    return this->_hit.base;
}

uint8_t character::buff_hit() const
{
    return this->_hit.buff;
}

uint8_t character::hit() const
{
    auto hit        = fb::game::life::hit();
    auto additional = (uint32_t)0;
    for (auto& [_, equipment] : this->items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += model.hit;
    }

    auto limit = std::numeric_limits<uint32_t>::max();
    if (limit - hit < additional)
        return limit;

    return hit + additional;
}

void character::base_hit(uint8_t value)
{
    this->assert_thread();
    this->_hit.base = value;
}
void character::buff_hit(uint8_t value)
{
    this->assert_thread();
    this->_hit.buff = value;
}

bool character::super_hide() const
{
    return this->_super_hide;
}

void character::super_hide(bool enabled)
{
    this->_super_hide = enabled;
    if (enabled)
    {
        for (auto& obj : this->nears(OBJECT_TYPE::CHARACTER))
        {
            if (this->hidden(*obj))
                this->hide(*obj);
            else
                this->update_external(*obj, false);
        }
    }
    else
    {
        for (auto& obj : this->nears(OBJECT_TYPE::CHARACTER))
        {
            this->update_external(*obj, false);
        }
    }
}

bool character::hidden(const fb::game::object& target) const
{
    if (this->super_hide() == false)
        return false;

    if (target.is(OBJECT_TYPE::CHARACTER) == false)
        return true;

    auto& ch = static_cast<const fb::game::character&>(target);
    return this->role() > ch.role();
}

async::task<void> character::death_penalty()
{
    auto buff_keys = std::vector<uint32_t>{};
    for (auto& [k, v] : this->buffs)
    {
        buff_keys.push_back(k);
    }
    for (auto k : buff_keys)
    {
        this->buffs.remove(k);
    }

    auto money = this->money();
    if (money > 0)
    {
        this->money_reduce(money);
        // TODO: Phase 3 - Convert to smart pointer return type
        auto cash_shared = this->context.make<fb::game::cash>(money);
        auto cash        = cash_shared.get();
        cash->death_cid(this->id());
        std::ignore = co_await cash->map(this->map(), this->position());
    }

    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = this->items[i];
        if (item == nullptr)
            continue;

        auto& model = item->based<fb::model::item>();
        if (model.attr(ITEM_ATTRIBUTE::EQUIPMENT))
        {
            auto  equipment       = std::static_pointer_cast<fb::game::equipment>(item);
            auto& equipment_model = equipment->based<fb::model::equipment>();
            auto  penalty         = equipment_model.durability * fb::model::const_value::death_penalty::durability;
            if (equipment->durability_down(penalty))
            {
                this->items.remove(i, 1, ITEM_DELETE_TYPE::DESTROY);
                continue;
            }
        }

        if (ENUM_IN(model.death_penalty, DEATH_PENALTY::DROP))
        {
            this->items.drop(i, item->count(), false, ITEM_DELETE_TYPE::NONE);
            item->death_cid(this->id());
            std::ignore = co_await item->map(this->map(), this->position());
        }
    }

    for (auto& [parts, equipment] : this->items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model   = equipment->based<fb::model::equipment>();
        auto  penalty = model.durability * fb::model::const_value::death_penalty::durability;
        if (equipment->durability_down(penalty))
        {
            this->items.equipment_off(parts);
            this->message(std::format("{} 깨졌습니다.", equipment->name()));
            equipment.reset();
            continue;
        }

        if (ENUM_IN(model.death_penalty, DEATH_PENALTY::DROP))
        {
            this->items.equipment_off(parts);
            equipment->death_cid(this->id());
            co_await equipment->map(this->map(), this->position());
        }
        else if (this->items.free())
        {
            this->items.equipment_off(parts);
            this->items.add(equipment);
        }
    }

    auto cls   = this->cls();
    auto level = this->level();
    if (this->context.model.ability.contains(cls) && this->context.model.ability[cls].contains(level) &&
        this->context.model.ability[cls].contains(level - 1))
    {
        auto penalty =
            uint32_t(this->context.model.ability[cls][level].exp * fb::model::const_value::death_penalty::exp);
        auto gained = this->exp() - this->context.model.ability[cls][level - 1].stacked_exp;

        penalty = std::min(gained, penalty);
        if (penalty > 0)
        {
            this->exp(this->exp() - penalty);
            this->message(std::format("경험치를 {} 잃었습니다.", penalty));
        }
    }
}