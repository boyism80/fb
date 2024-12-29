#include <character.h>
#include <context.h>
#include <regex.h>

using namespace fb::game;

character::character(fb::game::context& context, fb::socket<character>& socket) :
    life(context, context.model.life[0], initial_params{{.id = (uint32_t)socket.fd()}}),
    _socket(socket)
{ }

character::~character()
{ }

async::task<size_t> character::send(const fb::stream& stream, bool encrypt, bool wrap)
{
    this->assert_thread();

    co_return co_await this->_socket.send(stream, encrypt, wrap);
}

async::task<size_t> character::send(const fb::protocol::base::header& response, bool encrypt, bool wrap)
{
    this->assert_thread();

    co_return co_await this->_socket.send(response, encrypt, wrap);
}

OBJECT_TYPE character::what() const
{
    this->assert_thread();

    return OBJECT_TYPE::CHARACTER;
}

async::task<bool> character::map(fb::game::map* map, const point16_t& position, DESTROY_TYPE destroy_type)
{
    this->assert_thread();

    if (this->_map_lock)
        co_return false;

    auto switch_process = (map != nullptr && map->active == false);
    if (switch_process)
    {
        auto listener = this->get_listener<character>();
        if (listener != nullptr)
            co_return co_await listener->on_transfer(*this, *map, position);
        else
            co_return false;
    }
    else
    {
        co_return co_await object::map(map, position, destroy_type);
    }
}

async::task<bool> character::map(fb::game::map* map, DESTROY_TYPE destroy_type)
{
    this->assert_thread();

    return this->map(map, point16_t(0, 0), destroy_type);
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

void character::on_hold()
{
    this->assert_thread();

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_hold(*this);
}

void character::on_update()
{
    this->assert_thread();

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_updated(*this, STATE_LEVEL::LEVEL_MIDDLE);
}

uint32_t character::on_calculate_damage(bool critical) const
{
    this->assert_thread();

    auto weapon = this->items.weapon();
    auto model  = weapon != nullptr ? &weapon->based<fb::model::weapon>() : nullptr;

    // TODO: 이거 크리터졌을때가 아니라 때리는 몹 타입으로 small, large 써야함

    if (weapon == nullptr) // no weapon
    {
        return 1 + std::rand() % 5;
    }
    else if (critical)
    {
        auto& range = model->damage_small;
        return std::max(uint32_t(1), range.min) + std::rand() % std::max(uint32_t(1), range.max);
    }
    else // normal
    {
        auto& range = model->damage_large;
        return std::max(uint32_t(1), range.min) + std::rand() % std::max(uint32_t(1), range.max);
    }
}

void character::on_attack(object* you)
{
    this->assert_thread();

    life::on_attack(you);

    auto thread = lua::get();
    if (thread == nullptr)
        return;

    thread->from("scripts/common/attack.lua").func("on_attack").pushobject(*this);
    if (you != nullptr)
        thread->pushobject(*you);
    else
        thread->pushnil();
    thread->resume(2);
}

void character::on_kill(life& you)
{
    this->assert_thread();

    life::on_kill(you);

    auto exp = you.on_exp();
    if (exp == 0)
        return;

    if (this->_group != nullptr && this->_map != nullptr)
    {
        this->_group->lock([this, exp](auto& group) {
            auto nears      = group.nears(*this->_map, this->_position);
            auto size       = nears.size();
            auto divide_exp = exp / size;
            for (auto ch : nears)
            {
                ch->add_exp(ch->limited_exp(divide_exp), true);
            }
        });
    }
    else
    {
        this->add_exp(this->limited_exp(exp), true);
    }
}

void character::on_die(object* from)
{
    this->assert_thread();

    life::on_die(from);
    this->state(STATE::GHOST);
}

character::operator fb::socket<character>& ()
{
    return this->_socket;
}

bool character::inited() const
{
    this->assert_thread();

    return this->_init;
}

void character::init(bool value)
{
    this->assert_thread();

    this->_init = value;
}

uint32_t character::id() const
{
    this->assert_thread();

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

bool character::admin() const
{
    this->assert_thread();

    return this->_admin;
}

void character::admin(bool value)
{
    this->assert_thread();

    this->_admin = value;
}

void character::attack()
{
    this->assert_thread();

    auto listener = this->get_listener<character>();
    auto error    = std::string();
    try
    {
        this->assert_state({STATE::RIDING, STATE::GHOST});
        life::attack();
    }
    catch (std::exception& e)
    {
        this->message(e.what());
    }
}

void character::action(ACTION action, DURATION duration, uint8_t sound)
{
    this->assert_thread();

    auto listener = this->get_listener<character>();
    auto error    = std::string();
    try
    {
        this->assert_state({STATE::GHOST, STATE::RIDING});
        if (listener != nullptr)
            listener->on_action(*this, action, duration, sound);
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

const datetime& character::updated_date() const
{
    this->assert_thread();

    return this->_updated_date;
}

void character::updated_date(const datetime& value)
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

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_show(*this, false);
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

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_show(*this, false);
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

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_show(*this, false);
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

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_updated(*this, STATE_LEVEL::LEVEL_MAX);
}

void character::undisguise()
{
    this->assert_thread();

    this->_disguise = std::nullopt;
    if (this->state() == STATE::DISGUISE)
        this->state(STATE::NORMAL);

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_updated(*this, STATE_LEVEL::LEVEL_MAX);
}

uint32_t character::defensive_physical() const
{
    this->assert_thread();

    return this->_defensive.physical;
}

void character::defensive_physical(uint8_t value)
{
    this->assert_thread();

    this->_defensive.physical = value;
}

uint32_t character::defensive_magical() const
{
    this->assert_thread();

    return this->_defensive.magical;
}

void character::defensive_magical(uint8_t value)
{
    this->assert_thread();

    this->_defensive.magical = value;
}

void character::base_hp_up(uint32_t value)
{
    this->assert_thread();

    this->_base_hp += value;
}

void character::base_mp_up(uint32_t value)
{
    this->assert_thread();

    this->_base_mp += value;
}

void character::base_hp(uint32_t value)
{
    this->assert_thread();

    this->_base_hp = value;
    this->_hp      = std::min(this->_hp, this->_base_hp);
}

void character::base_mp(uint32_t value)
{
    this->assert_thread();

    this->_base_mp = value;
    this->_mp      = std::min(this->_mp, this->_base_mp);
}

uint32_t character::base_hp() const
{
    this->assert_thread();

    return this->_base_hp;
}

uint32_t character::base_mp() const
{
    this->assert_thread();

    return this->_base_mp;
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

    auto listener = this->get_listener<character>();

    this->_level = value;
    if (listener != nullptr)
        listener->on_updated(*this, STATE_LEVEL::LEVEL_MAX);
}

bool character::level_up()
{
    this->assert_thread();

    if (this->max_level())
        return false;

    auto& ability = this->context.model.ability[this->_class][this->_level];
    this->strength_up(ability.strength);
    this->intelligence_up(ability.intelligence);
    this->dexteritry_up(ability.dexteritry);
    this->base_hp_up(ability.hp + std::rand() % 10);
    this->base_mp_up(ability.mp + std::rand() % 10);

    this->hp(this->base_hp());
    this->mp(this->base_mp());

    this->level(this->_level + 1);
    this->message(message::level::UP);

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_level_up(*this);

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
}

STATE character::state() const
{
    this->assert_thread();

    return this->_state;
}

void character::state(STATE value)
{
    this->assert_thread();

    if (this->_state == value)
        return;

    this->_state = value;

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_show(*this, false);
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
}

uint8_t character::promotion() const
{
    this->assert_thread();

    return uint8_t();
}

void character::promotion(uint8_t value)
{
    this->assert_thread();

    this->_promotion = value;
}

uint8_t character::strength() const
{
    this->assert_thread();

    return this->_strength;
}

void character::strength(uint8_t value)
{
    this->assert_thread();

    this->_strength = value;
}

void character::strength_up(uint8_t value)
{
    this->assert_thread();

    this->_strength += value;
}

uint8_t character::intelligence() const
{
    this->assert_thread();

    return this->_intelligence;
}

void character::intelligence(uint8_t value)
{
    this->assert_thread();

    this->_intelligence = value;
}

void character::intelligence_up(uint8_t value)
{
    this->assert_thread();

    this->_intelligence += value;
}

uint8_t character::dexteritry() const
{
    this->assert_thread();

    return this->_dexteritry;
}

void character::dexteritry(uint8_t value)
{
    this->assert_thread();

    this->_dexteritry = value;
}

void character::dexteritry_up(uint8_t value)
{
    this->assert_thread();

    this->_dexteritry += value;
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

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_updated(*this, STATE_LEVEL::LEVEL_MIN);
}

uint32_t character::add_exp(uint32_t value, bool notify)
{
    this->assert_thread();

    auto capacity = 0xFFFFFFFF - this->_experience;
    auto lack     = 0;
    auto error    = std::string();

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
            throw require_class_exception();
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
        return lack;
    }
    else
    {
        this->_experience -= value;
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

    auto level    = this->level();
    auto required = 0;
    if (this->max_level())
        required = 0xFFFFFFFF;
    else
        required = this->context.model.ability[this->_class][level].exp;

    auto prev_stack_exp = 0;
    if (this->context.model.ability[this->_class].contains(level - 1))
        prev_stack_exp = this->context.model.ability[this->_class][level - 1].stacked_exp;

    return std::min(100.0f, ((this->_experience - prev_stack_exp) / float(required)) * 100.0f);
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

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
    {
        listener->on_updated(*this, STATE_LEVEL::LEVEL_MIN);
        listener->on_money_changed(*this, value);
    }
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

uint32_t character::money_reduce(uint32_t value)
{
    this->assert_thread();

    uint32_t lack = 0;
    if (this->_money < value)
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

uint32_t character::money_drop(uint32_t value)
{
    this->assert_thread();

    try
    {
        if (value == 0)
            return 0;

        this->assert_state({STATE::RIDING, STATE::GHOST});

        auto lack = this->money_reduce(value);

        auto cash = this->context.make<fb::game::cash>(value);
        cash->map(this->_map, this->_position);

        this->action(ACTION::PICKUP, DURATION::PICKUP);
        this->message(message::money::DROP);

        return lack;
    }
    catch (std::exception& e)
    {
        this->message(e.what());
    }

    return 0;
}

uint32_t character::deposited_money() const
{
    this->assert_thread();

    return this->_deposited_money;
}

void character::deposited_money(uint32_t value)
{
    this->assert_thread();

    this->_deposited_money = value;
}

uint32_t character::deposit_money(uint32_t value)
{
    this->assert_thread();

    uint32_t capacity = 0xFFFFFFFF - this->_deposited_money;
    uint32_t lack     = 0;
    if (value > capacity)
    {
        this->deposited_money(this->_deposited_money + capacity);
        lack = value - capacity;
    }
    else
    {
        this->deposited_money(this->_deposited_money + value);
    }

    return lack;
}

uint32_t character::withdraw_money(uint32_t value)
{
    this->assert_thread();

    uint32_t lack = 0;
    if (this->_deposited_money < value)
    {
        lack = value - this->_deposited_money;
        this->deposited_money(0);
    }
    else
    {
        this->deposited_money(this->_deposited_money - value);
    }

    return lack;
}

bool character::deposit_item(item& item)
{
    this->assert_thread();

    item.owner(this);
    if (item.based<fb::model::item>().attr(ITEM_ATTRIBUTE::BUNDLE))
    {
        auto found =
            std::find_if(this->_deposited_items.begin(), this->_deposited_items.end(), [&item](auto* deposited_item) {
                auto& model = deposited_item->template based<fb::model::item>();
                return item.based<fb::model::item>() == model;
            });

        if (found == this->_deposited_items.end())
        {
            this->_deposited_items.push_back(&item);
        }
        else
        {
            auto deposited_item = *found;
            auto capacity       = 0xFFFF - deposited_item->count();
            if (item.count() > capacity)
                return false;

            deposited_item->count(deposited_item->count() + item.count());
        }
    }
    else
    {
        this->_deposited_items.push_back(&item);
    }

    return true;
}

bool character::deposit_item(uint8_t index, uint16_t count)
{
    this->assert_thread();

    auto item = this->items.at(index);
    if (item == nullptr)
        return false;

    if (item->count() < count)
        return false;

    auto deleted = this->items.remove(*item, count);
    auto result  = this->deposit_item(*deleted);
    if (result == false)
        this->items.add(deleted);

    return result;
}

bool character::deposit_item(const std::string& name, uint16_t count)
{
    this->assert_thread();

    auto item = this->items.find(name);
    if (item == nullptr)
        return false;

    auto index = this->items.index(*item);
    if (index == 0xFF)
        return false;

    return this->deposit_item(index, count);
}

fb::game::item* character::deposited_item(const fb::model::item& item) const
{
    this->assert_thread();

    auto found =
        std::find_if(this->_deposited_items.cbegin(), this->_deposited_items.cend(), [&item](auto* deposited_item) {
            return deposited_item->template based<fb::model::item>() == item;
        });

    if (found == this->_deposited_items.cend())
        return nullptr;

    return *found;
}

const std::vector<fb::game::item*>& character::deposited_items() const
{
    this->assert_thread();

    return this->_deposited_items;
}

fb::game::item* character::withdraw_item(uint8_t index, uint16_t count)
{
    this->assert_thread();

    if (index > this->_deposited_items.size() - 1)
        return nullptr;

    if (this->items.free() == false)
        return nullptr;

    auto deposited_item  = this->_deposited_items.at(index);
    auto deposited_count = deposited_item->count();
    if (deposited_count < count)
        return nullptr;

    auto& model  = deposited_item->based<fb::model::item>();
    auto  exists = model.attr(ITEM_ATTRIBUTE::BUNDLE) ? this->items.find(model) : nullptr;
    if (exists != nullptr)
    {
        if (exists->count() + count > model.capacity)
            return nullptr;

        deposited_item->count(deposited_count - count);
        auto added_slot = this->items.add(deposited_item->based<fb::model::item>().make(this->context, count));
        if (deposited_item->empty())
        {
            auto i = this->_deposited_items.begin() + index;
            this->_deposited_items.erase(i);
        }

        return this->items.at(added_slot);
    }
    else
    {
        if (this->items.free() == false)
            return nullptr;

        deposited_item->count(deposited_count - count);
        if (deposited_item->empty())
        {
            auto i = this->_deposited_items.begin() + index;
            this->_deposited_items.erase(i);
        }

        auto added_slot = this->items.add(deposited_item->based<fb::model::item>().make(this->context, count));

        return this->items.at(added_slot);
    }
}

fb::game::item* character::withdraw_item(const std::string& name, uint16_t count)
{
    this->assert_thread();

    auto found =
        std::find_if(this->_deposited_items.begin(), this->_deposited_items.end(), [&name](auto* deposited_item) {
            auto& model = deposited_item->template based<fb::model::item>();
            return model.name == name;
        });

    if (found == this->_deposited_items.end())
        return nullptr;

    auto index = std::distance(this->_deposited_items.begin(), found);
    return this->withdraw_item((uint8_t)index, count);
}

fb::game::item* character::withdraw_item(const fb::model::item& item, uint16_t count)
{
    this->assert_thread();

    auto found =
        std::find_if(this->_deposited_items.begin(), this->_deposited_items.end(), [&item](auto* deposited_item) {
            auto& model = deposited_item->template based<fb::model::item>();
            return model == item;
        });

    if (found == this->_deposited_items.end())
        return nullptr;

    auto index = std::distance(this->_deposited_items.begin(), found);
    return this->withdraw_item((uint8_t)index, count);
}

uint32_t character::damage() const
{
    this->assert_thread();

    return this->_damage;
}

void character::damage(uint8_t value)
{
    this->assert_thread();

    this->_damage = value;
}

uint32_t character::hit() const
{
    this->assert_thread();

    return this->_hit;
}

void character::hit(uint8_t value)
{
    this->assert_thread();

    this->_hit = value;
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

bool character::option(SETTING key) const
{
    this->assert_thread();

    auto opt = static_cast<uint8_t>(key);
    if (opt == 0 || opt > static_cast<uint8_t>(SETTING::PK_PROTECT))
        throw std::runtime_error(std::format("invalid setting key : {:#x}", opt));

    return this->_options[opt];
}

void character::option(SETTING key, bool value, bool notify)
{
    this->assert_thread();

    auto opt = static_cast<uint8_t>(key);
    if (opt == 0 || opt > static_cast<uint8_t>(SETTING::PK_PROTECT))
        return;

    if (this->_options[opt] == value)
        return;

    auto listener = this->get_listener<character>();
    if (listener != nullptr && notify)
    {
        listener->on_updated(*this, STATE_LEVEL::LEVEL_MIN);
        listener->on_option(*this, key, value);
    }

    this->_options[opt] = value;
}

bool character::option_toggle(SETTING key, bool notify)
{
    this->assert_thread();

    auto opt = static_cast<uint8_t>(key);
    if (opt == 0 || opt > static_cast<uint8_t>(SETTING::PK_PROTECT))
        throw std::runtime_error(std::format("invalid setting key : {:#x}", opt));

    this->option(key, !this->_options[opt], notify);
    return this->_options[opt];
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

shared_clan_lock& character::clan()
{
    this->assert_thread();

    return this->_clan;
}

void character::clan(shared_clan_lock& value)
{
    this->assert_thread();

    this->_clan = value;
}

void character::assert_state(STATE value) const
{
    this->assert_thread();

    static const auto pairs = std::map<STATE, const std::runtime_error>{
        {STATE::GHOST,    ghost_exception()   },
        {STATE::RIDING,   ridding_exception() },
        {STATE::DISGUISE, disguise_exception()}
    };

    if (this->_state == value)
        throw pairs.at(value);
}

void character::assert_state(const std::vector<STATE>& values) const
{
    this->assert_thread();

    for (auto value : values)
        this->assert_state(value);
}

bool character::move(const point16_t& before)
{
    this->assert_thread();

    return this->move(this->_direction, before);
}

bool character::move(DIRECTION direction, const point16_t& before)
{
    this->assert_thread();

    auto listener = this->get_listener<character>();

    if (this->_position != before)
    {
        if (listener != nullptr)
            listener->on_hold(*this);
        return false;
    }
    else if (object::move(direction) == false)
    {
        if (listener != nullptr)
            listener->on_hold(*this);
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
            throw std::runtime_error(message::ride::ALREADY_RIDE);

        if (horse.based<fb::model::mob>() != this->context.model.mob[fb::model::const_value::mob::horse])
            throw character::no_conveyance_exception();

        if (horse.map() != this->_map)
            throw std::runtime_error(message::error::UNKNOWN);

        horse.map(nullptr);
        this->state(STATE::RIDING);
        horse.kill();
        this->message(message::ride::ON);
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
            throw character::no_conveyance_exception();

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
            throw std::runtime_error(message::ride::UNRIDE);

        auto& model = this->context.model.mob[const_value::mob::horse];
        auto  horse = this->context.make<mob>(model, mob::initial_params{.alive = true});
        horse->map(this->_map, this->position_forward());

        this->state(STATE::NORMAL);
        this->message(message::ride::OFF);
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

void character::refresh_map()
{
    this->assert_thread();

    if (this->_map == nullptr)
        return;

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_map_changed(*this, this->_map, this->_map);
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
            auto params = dsl::level(dsl.params);
            if (params.min.has_value() && *params.min > this->_level)
                return false;

            if (params.max.has_value() && *params.max < this->_level)
                return false;
        }
        break;

        case DSL::sex:
        {
            auto params = dsl::sex(dsl.params);
            if (enum_in(params.value, this->_sex) == false)
                return false;
        }
        break;

        case DSL::strength:
        {
            auto params = dsl::strength(dsl.params);
            if (params.value > this->_strength)
                return false;
        }
        break;

        case DSL::intelligence:
        {
            auto params = dsl::intelligence(dsl.params);
            if (params.value > this->_intelligence)
                return false;
        }
        break;

        case DSL::dexteritry:
        {
            auto params = dsl::dexteritry(dsl.params);
            if (params.value > this->_dexteritry)
                return false;
        }
        break;

        case DSL::promotion:
        {
            auto params = dsl::promotion(dsl.params);
            if (params.value > this->_promotion)
                return false;
        }
        break;

        case DSL::class_t:
        {
            auto params = dsl::class_t(dsl.params);
            if (enum_in(params.value, this->_class) == false)
                return false;
        }

        case DSL::admin:
        {
            auto params = dsl::admin(dsl.params);
            if (params.value != this->_admin)
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

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_message(*this, message, type);
}

fb::thread* character::thread() const
{
    if (this->_map == nullptr)
        return this->context.threads.modular(this->_socket.fd());
    else
        return this->context.threads.modular(this->_map->model.id);
}

void character::assert_thread() const
{
    fb::game::object::assert_thread();
}

fb::protocol::internal::Character character::to_protocol() const
{
    this->assert_thread();

    auto dto             = fb::protocol::internal::Character();
    dto.id               = this->_id;
    dto.name             = this->_name;
    dto.pw               = this->_pw;
    dto.updated_date     = datetime().to_string();
    dto.admin            = this->_admin;
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
    dto.deposited_money  = this->_deposited_money;
    dto.disguise         = this->_disguise;
    dto.hp               = this->_hp;
    dto.base_hp          = this->_base_hp;
    dto.additional_hp    = 0;
    dto.mp               = this->_mp;
    dto.base_mp          = this->_base_mp;
    dto.additional_mp    = 0;
    dto.weapon_color     = std::nullopt;
    dto.helmet_color     = std::nullopt;
    dto.armor_color      = this->_armor_color;
    dto.shield_color     = std::nullopt;
    dto.ring_left_color  = std::nullopt;
    dto.ring_right_color = std::nullopt;
    dto.aux_top_color    = std::nullopt;
    dto.aux_bot_color    = std::nullopt;
    return dto;
}

character::container::container()
{ }

character::container::container(const std::vector<character*>& right)
{
    this->insert(this->begin(), right.begin(), right.end());
}

character::container::~container()
{ }

character::container& character::container::push(character& ch)
{
    this->push_back(&ch);
    return *this;
}

character::container& character::container::erase(character& ch)
{
    std::vector<character*>::erase(std::find(this->begin(), this->end(), &ch));
    return *this;
}

character* character::container::find(const std::string& name)
{
    auto i = std::find_if(this->begin(), this->end(), [&name](auto x) {
        return x->name() == name;
    });
    return i != this->end() ? *i : nullptr;
}

bool character::container::contains(const character& ch) const
{
    return std::find(this->cbegin(), this->cend(), &ch) != this->end();
}

character* character::container::operator[] (const std::string& name)
{
    return this->find(name);
}