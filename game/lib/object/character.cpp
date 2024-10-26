#include <character.h>
#include <context.h>
#include <regex.h>

using namespace fb::game;

character::character(fb::game::context& context, fb::socket<character>& socket) :
    life(context, context.model.life[0], life::config{{.id = (uint32_t)socket.fd()}}),
    _socket(socket)
{
    inline_interaction_funcs.push_back(
        std::bind(&character::inline_sell, this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(
        std::bind(&character::inline_buy, this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(
        std::bind(&character::inline_repair, this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(
        std::bind(&character::inline_deposit_money, this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(
        std::bind(&character::inline_withdraw_money, this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(
        std::bind(&character::inline_deposit_item, this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(
        std::bind(&character::inline_withdraw_item, this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(
        std::bind(&character::inline_sell_list, this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(
        std::bind(&character::inline_buy_list, this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(
        std::bind(&character::inline_sell_price, this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(
        std::bind(&character::inline_buy_price, this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(
        std::bind(&character::inline_show_deposited_money, this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(
        std::bind(&character::inline_rename_weapon, this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(
        std::bind(&character::inline_hold_item_list, this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(
        std::bind(&character::inline_hold_item_count, this, std::placeholders::_1, std::placeholders::_2));
}

character::~character()
{ }

void character::send(const fb::ostream& stream, bool encrypt, bool wrap)
{
    if (this->inited() == false)
        return;

    this->_socket.send(stream, encrypt, wrap);
}

void character::send(const fb::protocol::base::header& response, bool encrypt, bool wrap)
{
    if (this->inited() == false)
        return;

    this->_socket.send(response, encrypt, wrap);
}

OBJECT_TYPE character::what() const
{
    return OBJECT_TYPE::CHARACTER;
}

async::task<bool> character::map(fb::game::map* map, const point16_t& position, DESTROY_TYPE destroy_type)
{
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
    co_return co_await this->map(map, point16_t(0, 0), destroy_type);
}

void character::on_hold()
{
    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_hold(*this);
}

void character::on_update()
{
    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_updated(*this, STATE_LEVEL::LEVEL_MIDDLE);
}

uint32_t character::on_calculate_damage(bool critical) const
{
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
    life::on_kill(you);

    auto exp = you.on_exp();
    if (exp > 0)
    {
        auto range = this->context.model.ability[this->_class][this->_level].exp;
#if defined DEBUG | defined _DEBUG
        exp *= 100;
#else
        if (this->max_level() == false)
            exp = std::min(uint32_t(range / 100.0f * 3.3f + 1), exp);
#endif
        this->experience_add(exp, true);
    }
}

void character::on_die(object* from)
{
    life::on_die(from);
    this->state(STATE::GHOST);
}

character::operator fb::socket<character>& ()
{
    return this->_socket;
}

bool character::inited() const
{
    return this->_init;
}

void character::init(bool value)
{
    this->_init = value;
}

uint32_t character::id() const
{
    return this->_id;
}

void character::id(uint32_t id)
{
    this->_id = id;
}

uint32_t character::fd()
{
    return this->_socket.fd();
}

bool character::admin() const
{
    return this->_admin;
}

void character::admin(bool value)
{
    this->_admin = value;
}

void character::attack()
{
    auto listener = this->get_listener<character>();

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
    auto listener = this->get_listener<character>();

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
    return this->_name;
}

void character::name(const std::string& value)
{
    this->_name = value;
}

const datetime& character::last_login() const
{
    return this->_last_login;
}

void character::last_login(const datetime& value)
{
    this->_last_login = value;
}

uint16_t character::look() const
{
    return this->_look;
}

void character::look(uint16_t value)
{
    this->_look   = value;

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_show(*this, false);
}

uint8_t character::color() const
{
    return this->_color;
}

void character::color(uint8_t value)
{
    this->_color  = value;

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_show(*this, false);
}

std::optional<uint8_t> character::armor_color() const
{
    return this->_armor_color;
}

void character::armor_color(std::optional<uint8_t> value)
{
    this->_armor_color = value;

    auto listener      = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_show(*this, false);
}

uint8_t character::current_armor_color() const
{
    auto armor = this->items.armor();
    return this->_armor_color.value_or(armor != nullptr ? armor->color() : 0x00);
}

std::optional<uint16_t> character::disguise() const
{
    return this->_disguise;
}

void character::disguise(uint16_t value)
{
    this->_disguise = value;
    this->state(STATE::DISGUISE);

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_updated(*this, STATE_LEVEL::LEVEL_MAX);
}

void character::undisguise()
{
    this->_disguise = std::nullopt;
    if (this->state() == STATE::DISGUISE)
        this->state(STATE::NORMAL);

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_updated(*this, STATE_LEVEL::LEVEL_MAX);
}

uint32_t character::defensive_physical() const
{
    return this->_defensive.physical;
}

void character::defensive_physical(uint8_t value)
{
    this->_defensive.physical = value;
}

uint32_t character::defensive_magical() const
{
    return this->_defensive.magical;
}

void character::defensive_magical(uint8_t value)
{
    this->_defensive.magical = value;
}

void character::base_hp_up(uint32_t value)
{
    this->_base_hp += value;
}

void character::base_mp_up(uint32_t value)
{
    this->_base_mp += value;
}

void character::base_hp(uint32_t value)
{
    this->_base_hp = value;
    this->_hp      = std::min(this->_hp, this->_base_hp);
}

void character::base_mp(uint32_t value)
{
    this->_base_mp = value;
    this->_mp      = std::min(this->_mp, this->_base_mp);
}

uint32_t character::base_hp() const
{
    return this->_base_hp;
}

uint32_t character::base_mp() const
{
    return this->_base_mp;
}

NATION character::nation() const
{
    return this->_nation;
}

bool character::nation(NATION value)
{
    if (value != NATION::GOGURYEO && value != NATION::BUYEO)
        return false;

    this->_nation = value;
    return true;
}

CREATURE character::creature() const
{
    return this->_creature;
}

bool character::creature(CREATURE value)
{
    if (value != CREATURE::DRAGON && value != CREATURE::PHOENIX && value != CREATURE::TIGER &&
        value != CREATURE::TURTLE)
        return false;

    this->_creature = value;
    return true;
}

uint8_t character::level() const
{
    return this->_level;
}

void character::level(uint8_t value)
{
    auto listener = this->get_listener<character>();

    this->_level  = value;
    if (listener != nullptr)
        listener->on_updated(*this, STATE_LEVEL::LEVEL_MAX);
}

bool character::level_up()
{
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
    return this->context.model.ability[this->_class].contains(this->_level + 1) == false;
}

SEX character::sex() const
{
    return this->_sex;
}

void character::sex(SEX value)
{
    this->_sex = value;
}

STATE character::state() const
{
    return this->_state;
}

void character::state(STATE value)
{
    if (this->_state == value)
        return;

    this->_state  = value;

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_show(*this, false);
}

CLASS character::cls() const
{
    return this->_class;
}

void character::cls(CLASS value)
{
    this->_class = value;
}

uint8_t character::promotion() const
{
    return uint8_t();
}

void character::promotion(uint8_t value)
{
    this->_promotion = value;
}

uint8_t character::strength() const
{
    return this->_strength;
}

void character::strength(uint8_t value)
{
    this->_strength = value;
}

void character::strength_up(uint8_t value)
{
    this->_strength += value;
}

uint8_t character::intelligence() const
{
    return this->_intelligence;
}

void character::intelligence(uint8_t value)
{
    this->_intelligence = value;
}

void character::intelligence_up(uint8_t value)
{
    this->_intelligence += value;
}

uint8_t character::dexteritry() const
{
    return this->_dexteritry;
}

void character::dexteritry(uint8_t value)
{
    this->_dexteritry = value;
}

void character::dexteritry_up(uint8_t value)
{
    this->_dexteritry += value;
}

uint32_t character::experience() const
{
    return this->_experience;
}

void character::experience(uint32_t value)
{
    if (this->_experience == value)
        return;

    this->_experience = value;

    auto listener     = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_updated(*this, STATE_LEVEL::LEVEL_MIN);
}

uint32_t character::experience_add(uint32_t value, bool notify)
{
    auto capacity = 0xFFFFFFFF - this->_experience;
    auto lack     = 0;

    try
    {
        // 직업이 없는 경우 정확히 5레벨을 찍을 경험치만 얻도록 제한
        if (this->_class == CLASS::NONE)
        {
            auto require = this->context.model.ability[CLASS::NONE][5].exp;
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
                this->experience(this->_experience + capacity);
            }
            else
            {
                this->experience(this->_experience + value);
            }

            if (notify)
            {
                std::stringstream sstream;
                sstream << "경험치가 " << value << '(' << int(this->experience_percent()) << "%) 올랐습니다.";
                this->message(sstream.str());
            }
        }

        if (this->context.model.ability.contains(this->_class) == false)
            throw std::runtime_error("what?");

        while (true)
        {
            if (this->max_level())
                break;

            auto& next = this->context.model.ability[this->_class][this->_level + 1];
            if (next.exp == 0)
                break;

            if (this->_experience < next.exp)
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

uint32_t character::experience_reduce(uint32_t value)
{
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
    if (this->max_level())
        return 0;

    if (this->_class == CLASS::NONE && this->_level >= 5)
        return 0;

    return this->context.model.ability[this->_class][this->_level].exp - this->experience();
}

float character::experience_percent() const
{
    auto current_level = this->level();
    auto next_exp      = this->max_level() ? 0xFFFFFFFF : this->context.model.ability[this->_class][current_level].exp;
    auto prev_exp =
        current_level > 1
            ? (this->max_level() ? 0x00000000 : this->context.model.ability[this->_class][current_level - 1].exp)
            : 0;
    auto exp_range = next_exp - prev_exp;

    return std::min(100.0f, ((this->_experience - prev_exp) / float(exp_range)) * 100.0f);
}

uint32_t character::money() const
{
    return this->_money;
}

void character::money(uint32_t value)
{
    this->_money  = value;

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
    {
        listener->on_updated(*this, STATE_LEVEL::LEVEL_MIN);
        listener->on_money_changed(*this, value);
    }
}

uint32_t character::money_add(uint32_t value) // 먹고 남은 값 리턴
{
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

async::task<uint32_t> character::money_drop(uint32_t value)
{
    try
    {
        if (value == 0)
            co_return 0;

        this->assert_state({STATE::RIDING, STATE::GHOST});

        auto lack = this->money_reduce(value);

        auto cash = this->context.make<fb::game::cash>(value);
        co_await cash->map(this->_map, this->_position);

        this->action(ACTION::PICKUP, DURATION::PICKUP);
        this->message(message::money::DROP);

        co_return lack;
    }
    catch (std::exception& e)
    {
        this->message(e.what());
        co_return 0;
    }
}

uint32_t character::deposited_money() const
{
    return this->_deposited_money;
}

void character::deposited_money(uint32_t value)
{
    this->_deposited_money = value;
}

uint32_t character::deposit_money(uint32_t value)
{
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
    item.owner(this);
    if (item.based<fb::model::item>().attr(ITEM_ATTRIBUTE::BUNDLE))
    {
        auto found =
            std::find_if(this->_deposited_items.begin(), this->_deposited_items.end(), [&item](auto* deposited_item) {
                auto& model = deposited_item->based<fb::model::item>();
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
    auto found =
        std::find_if(this->_deposited_items.cbegin(), this->_deposited_items.cend(), [&item](auto* deposited_item) {
            return deposited_item->based<fb::model::item>() == item;
        });

    if (found == this->_deposited_items.cend())
        return nullptr;

    return *found;
}

const std::vector<fb::game::item*>& character::deposited_items() const
{
    return this->_deposited_items;
}

async::task<fb::game::item*> character::withdraw_item(uint8_t index, uint16_t count)
{
    if (index > this->_deposited_items.size() - 1)
        co_return nullptr;

    if (this->items.free() == false)
        co_return nullptr;

    auto deposited_item  = this->_deposited_items.at(index);
    auto deposited_count = deposited_item->count();
    if (deposited_count < count)
        co_return nullptr;

    auto& model  = deposited_item->based<fb::model::item>();
    auto  exists = model.attr(ITEM_ATTRIBUTE::BUNDLE) ? this->items.find(model) : nullptr;
    if (exists != nullptr)
    {
        if (exists->count() + count > model.capacity)
            co_return nullptr;

        deposited_item->count(deposited_count - count);
        auto added_slot = co_await this->items.add(deposited_item->based<fb::model::item>().make(this->context, count));
        if (deposited_item->empty())
        {
            auto i = this->_deposited_items.begin() + index;
            this->_deposited_items.erase(i);
        }

        co_return this->items.at(added_slot);
    }
    else
    {
        if (this->items.free() == false)
            co_return nullptr;

        deposited_item->count(deposited_count - count);
        if (deposited_item->empty())
        {
            auto i = this->_deposited_items.begin() + index;
            this->_deposited_items.erase(i);
        }

        auto added_slot = co_await this->items.add(deposited_item->based<fb::model::item>().make(this->context, count));

        co_return this->items.at(added_slot);
    }
}

async::task<fb::game::item*> character::withdraw_item(const std::string& name, uint16_t count)
{
    auto found =
        std::find_if(this->_deposited_items.begin(), this->_deposited_items.end(), [&name](auto* deposited_item) {
            auto& model = deposited_item->based<fb::model::item>();
            return model.name == name;
        });

    if (found == this->_deposited_items.end())
        co_return nullptr;

    auto index = std::distance(this->_deposited_items.begin(), found);
    co_return co_await this->withdraw_item((uint8_t)index, count);
}

async::task<fb::game::item*> character::withdraw_item(const fb::model::item& item, uint16_t count)
{
    auto found =
        std::find_if(this->_deposited_items.begin(), this->_deposited_items.end(), [&item](auto* deposited_item) {
            auto& model = deposited_item->based<fb::model::item>();
            return model == item;
        });

    if (found == this->_deposited_items.end())
        co_return nullptr;

    auto index = std::distance(this->_deposited_items.begin(), found);
    co_return co_await this->withdraw_item((uint8_t)index, count);
}

uint32_t character::damage() const
{
    return this->_damage;
}

void character::damage(uint8_t value)
{
    this->_damage = value;
}

uint32_t character::hit() const
{
    return this->_hit;
}

void character::hit(uint8_t value)
{
    this->_hit = value;
}

uint32_t character::regenerative() const
{
    return this->_regenerative;
}

void character::regenerative(uint8_t value)
{
    this->_regenerative = value;
}

bool character::option(CUSTOM_SETTING key) const
{
    if (static_cast<int>(key) > 0x1B)
        return false;

    return this->_options[static_cast<int>(key)];
}

void character::option(CUSTOM_SETTING key, bool value)
{
    if (static_cast<int>(key) > 0x1B)
        return;

    if (this->_options[static_cast<int>(key)] == value)
        return;

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
    {
        listener->on_updated(*this, STATE_LEVEL::LEVEL_MIN);
        listener->on_option(*this, key, value);
    }

    this->_options[static_cast<int>(key)] = value;
}

bool character::option_toggle(CUSTOM_SETTING key)
{
    if (static_cast<int>(key) > 0x1B)
        return false;

    this->option(key, !this->_options[static_cast<int>(key)]);
    return this->_options[static_cast<int>(key)];
}

const std::string& character::title() const
{
    return this->_title;
}

void character::title(const std::string& value)
{
    this->_title = value;
}

group* character::group() const
{
    return this->_group;
}

clan* character::clan() const
{
    return this->_clan;
}

void character::assert_state(STATE value) const
{
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
    for (auto value : values)
        this->assert_state(value);
}

bool character::move(const point16_t& before)
{
    return this->move(this->_direction, before);
}

bool character::move(DIRECTION direction, const point16_t& before)
{
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

async::task<void> character::ride(mob& horse)
{
    try
    {
        this->assert_state({STATE::GHOST, STATE::DISGUISE});

        if (this->state() == STATE::RIDING)
            throw std::runtime_error(message::ride::ALREADY_RIDE);

        if (horse.based<fb::model::mob>() != this->context.model.mob[fb::model::const_value::mob::horse])
            throw character::no_conveyance_exception();

        if (horse.map() != this->_map)
            throw std::runtime_error(message::error::UNKNOWN);

        co_await horse.map(nullptr);
        this->state(STATE::RIDING);
        horse.kill();
        this->message(message::ride::ON);
    }
    catch (std::exception& e)
    {
        this->message(e.what());
    }
}

async::task<void> character::ride()
{
    try
    {
        this->assert_state({STATE::GHOST, STATE::DISGUISE});

        auto front = this->forward(OBJECT_TYPE::MOB);
        if (front == nullptr)
            throw character::no_conveyance_exception();

        co_await this->ride(static_cast<mob&>(*front));
    }
    catch (std::exception& e)
    {
        this->message(e.what());
    }
}

async::task<void> character::unride()
{
    try
    {
        this->assert_state({STATE::GHOST, STATE::DISGUISE});
        if (this->state() != STATE::RIDING)
            throw std::runtime_error(message::ride::UNRIDE);

        auto& model = this->context.model.mob[const_value::mob::horse];
        auto  horse = this->context.make<mob>(model, mob::config{.alive = true});
        co_await horse->map(this->_map, this->position_forward());

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
    return this->_state != STATE::GHOST;
}

void character::refresh_map()
{
    if (this->_map == nullptr)
        return;

    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_map_changed(*this, this->_map, this->_map);
}

bool character::condition(const std::vector<fb::model::dsl>& conditions) const
{
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
    auto listener = this->get_listener<character>();
    if (listener != nullptr)
        listener->on_notify(*this, message, type);
}

async::task<bool> character::inline_sell(const std::string& message, const std::vector<npc*>& npcs)
{
    auto count = std::optional<uint16_t>();
    auto name  = std::string();
    if (fb::model::const_value::regex::match_sell_message(message, name, count) == false)
        co_return false;

    auto model  = this->context.model.item.name2item(name);
    auto bought = false;
    for (auto npc : npcs)
    {
        if (npc->buy(*this, model, count, bought))
            bought = true;
    }

    co_return bought;
}

async::task<bool> character::inline_buy(const std::string& message, const std::vector<npc*>& npcs)
{
    auto name  = std::string();
    auto count = uint16_t(0);
    if (fb::model::const_value::regex::match_buy_message(message, name, count) == false)
        co_return false;

    auto model = this->context.model.item.name2item(name);
    auto sold  = false;
    for (auto npc : npcs)
    {
        if (npc->sell(*this, model, count, sold))
            sold = true;
    }

    co_return sold;
}

async::task<bool> character::inline_repair(const std::string& message, const std::vector<npc*>& npcs)
{
    auto name = std::string();
    if (fb::model::const_value::regex::match_repair_message(message, name) == false)
        co_return false;

    auto model = this->context.model.item.name2item(name);
    auto done  = false;
    for (auto npc : npcs)
    {
        if (npc->repair(*this, model, done))
            done = true;
    }

    co_return done;
}

async::task<bool> character::inline_deposit_money(const std::string& message, const std::vector<npc*>& npcs)
{
    auto money = std::optional<uint32_t>();
    if (fb::model::const_value::regex::match_deposit_money_message(message, money) == false)
        co_return false;

    for (auto npc : npcs)
    {
        if (npc->hold_money(*this, money))
            co_return true;
    }

    co_return false;
}

async::task<bool> character::inline_withdraw_money(const std::string& message, const std::vector<npc*>& npcs)
{
    auto money = std::optional<uint32_t>();
    if (fb::model::const_value::regex::match_withdraw_money_message(message, money) == false)
        co_return false;

    for (auto npc : npcs)
    {
        if (npc->return_money(*this, money))
            co_return true;
    }

    co_return false;
}

async::task<bool> character::inline_deposit_item(const std::string& message, const std::vector<npc*>& npcs)
{
    auto name  = std::string();
    auto count = std::optional<uint16_t>(0);
    if (fb::model::const_value::regex::match_deposit_item_message(message, name, count) == false)
        co_return false;

    auto model = this->context.model.item.name2item(name);
    for (auto npc : npcs)
    {
        if (npc->hold_item(*this, model, count))
            co_return true;
    }

    co_return false;
}

async::task<bool> character::inline_withdraw_item(const std::string& message, const std::vector<npc*>& npcs)
{
    auto name  = std::string();
    auto count = std::optional<uint16_t>(0);
    if (fb::model::const_value::regex::match_withdraw_item_message(message, name, count) == false)
        co_return false;

    auto model = this->context.model.item.name2item(name);
    for (auto npc : npcs)
    {
        if (co_await npc->return_item(*this, model, count))
            co_return true;
    }

    co_return false;
}

async::task<bool> character::inline_sell_list(const std::string& message, const std::vector<npc*>& npcs)
{
    if (fb::model::const_value::regex::match_sell_list(message) == false)
        co_return false;

    for (auto npc : npcs)
    {
        npc->sell_list();
    }

    co_return true;
}

async::task<bool> character::inline_buy_list(const std::string& message, const std::vector<npc*>& npcs)
{
    if (fb::model::const_value::regex::match_buy_list(message) == false)
        co_return false;

    for (auto npc : npcs)
    {
        npc->buy_list();
    }

    co_return true;
}

async::task<bool> character::inline_sell_price(const std::string& message, const std::vector<npc*>& npcs)
{
    auto name = std::string();
    if (fb::model::const_value::regex::match_sell_price(message, name) == false)
        co_return false;

    auto model = this->context.model.item.name2item(name);
    for (auto npc : npcs)
    {
        npc->sell_price(model);
    }

    co_return true;
}

async::task<bool> character::inline_buy_price(const std::string& message, const std::vector<npc*>& npcs)
{
    auto name = std::string();
    if (fb::model::const_value::regex::match_buy_price(message, name) == false)
        co_return false;

    auto model = this->context.model.item.name2item(name);
    for (auto npc : npcs)
    {
        npc->buy_price(model);
    }

    co_return true;
}

async::task<bool> character::inline_show_deposited_money(const std::string& message, const std::vector<npc*>& npcs)
{
    if (fb::model::const_value::regex::match_deposited_money(message) == false)
        co_return false;

    for (auto npc : npcs)
    {
        if (npc->deposited_money(*this))
            co_return true;
    }

    co_return false;
}

async::task<bool> character::inline_rename_weapon(const std::string& message, const std::vector<npc*>& npcs)
{
    std::string model_name, custom_name;
    if (fb::model::const_value::regex::match_rename_weapon(message, model_name, custom_name) == false)
        co_return false;

    auto model = this->context.model.item.name2item(model_name);
    for (auto npc : npcs)
    {
        if (npc->rename_weapon(*this, model, custom_name))
            co_return true;
    }

    co_return false;
}

async::task<bool> character::inline_hold_item_list(const std::string& message, const std::vector<npc*>& npcs)
{
    if (fb::model::const_value::regex::match_hold_item_list(message) == false)
        co_return false;

    for (auto npc : npcs)
    {
        if (npc->hold_item_list(*this))
            co_return true;
    }

    co_return false;
}

async::task<bool> character::inline_hold_item_count(const std::string& message, const std::vector<npc*>& npcs)
{
    auto name = std::string();
    if (fb::model::const_value::regex::match_hold_item_count(message, name) == false)
        co_return false;

    auto model = this->context.model.item.name2item(name);
    for (auto npc : npcs)
    {
        if (npc->hold_item_count(*this, model))
            co_return true;
    }

    co_return false;
}

async::task<bool> character::inline_interaction(const std::string& message, const std::vector<npc*>& npcs)
{
    if (npcs.size() == 0)
        co_return false;

    for (auto& fn : this->inline_interaction_funcs)
    {
        if (co_await fn(message, npcs))
            co_return true;
    }

    co_return false;
}

fb::protocol::db::Character character::to_protocol() const
{
    auto dto             = fb::protocol::db::Character();
    dto.id               = this->_id;
    dto.name             = UTF8(this->_name, PLATFORM::Windows);
    dto.last_login       = datetime().to_string();
    dto.admin            = this->_admin;
    dto.look             = this->_look;
    dto.color            = this->_color;
    dto.sex              = (uint16_t)this->_sex;
    dto.nation           = (uint16_t)this->_nation;
    dto.creature         = (uint16_t)this->_creature;
    dto.map              = this->_map != nullptr ? this->_map->model.id : 0;
    dto.position         = fb::protocol::db::Position{this->_position.x, this->_position.y};
    dto.direction        = (uint8_t)this->_direction;
    dto.state            = (uint8_t)this->_state;
    dto.class_type       = (uint8_t)this->_class;
    dto.promotion        = this->_promotion;
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
    dto.clan             = std::nullopt;
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

character::container& character::container::push(character& session)
{
    this->push_back(&session);
    return *this;
}

character::container& character::container::erase(character& session)
{
    std::vector<character*>::erase(std::find(this->begin(), this->end(), &session));
    return *this;
}

character* character::container::find(const std::string& name)
{
    auto i = std::find_if(this->begin(), this->end(), [&name](auto x) {
        return x->name() == name;
    });
    return i != this->end() ? *i : nullptr;
}

bool character::container::contains(const character& session) const
{
    return std::find(this->cbegin(), this->cend(), &session) != this->end();
}

character* character::container::operator[] (const std::string& name)
{
    return this->find(name);
}