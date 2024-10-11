#include <fb/game/session.h>
#include <fb/game/context.h>
#include <fb/game/regex.h>

using namespace fb::game;

session::session(fb::game::context& context, fb::socket<fb::game::session>& socket) :
    life(context, context.model.life[0], fb::game::life::config{{.id = (uint32_t)socket.fd()}}),
    _socket(socket)
{
    inline_interaction_funcs.push_back(std::bind(&session::inline_sell,                    this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(std::bind(&session::inline_buy,                     this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(std::bind(&session::inline_repair,                  this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(std::bind(&session::inline_deposit_money,           this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(std::bind(&session::inline_withdraw_money,          this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(std::bind(&session::inline_deposit_item,            this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(std::bind(&session::inline_withdraw_item,           this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(std::bind(&session::inline_sell_list,               this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(std::bind(&session::inline_buy_list,                this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(std::bind(&session::inline_sell_price,              this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(std::bind(&session::inline_buy_price,               this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(std::bind(&session::inline_show_deposited_money,    this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(std::bind(&session::inline_rename_weapon,           this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(std::bind(&session::inline_hold_item_list,          this, std::placeholders::_1, std::placeholders::_2));
    inline_interaction_funcs.push_back(std::bind(&session::inline_hold_item_count,         this, std::placeholders::_1, std::placeholders::_2));
}

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

OBJECT_TYPE fb::game::session::what() const
{
    return OBJECT_TYPE::SESSION;
}

async::task<bool> fb::game::session::map(fb::game::map* map, const point16_t& position, DESTROY_TYPE destroy_type)
{
    if(this->_map_lock)
        co_return false;

    auto switch_process = (map != nullptr && map->active == false);
    if(switch_process)
    {
        auto listener = this->get_listener<fb::game::session>();
        if(listener != nullptr)
            co_return co_await listener->on_transfer(*this, *map, position);
        else
            co_return false;
    }
    else
    {
        co_return co_await fb::game::object::map(map, position, destroy_type);
    }
}

async::task<bool> fb::game::session::map(fb::game::map* map, DESTROY_TYPE destroy_type)
{
    co_return co_await this->map(map, point16_t(0, 0), destroy_type);
}

void fb::game::session::on_hold()
{
    auto listener = this->get_listener<fb::game::session>();
    if(listener != nullptr)
        listener->on_hold(*this);
}

void fb::game::session::on_update()
{
    auto listener = this->get_listener<fb::game::session>();
    if(listener != nullptr)
        listener->on_updated(*this, STATE_LEVEL::LEVEL_MIDDLE);
}

uint32_t fb::game::session::on_calculate_damage(bool critical) const
{
    auto                    weapon = this->items.weapon();
    auto                    model = weapon != nullptr ? &weapon->based<fb::model::weapon>() : nullptr;

    // TODO: 이거 크리터졌을때가 아니라 때리는 몹 타입으로 small, large 써야함

    if(weapon == nullptr) // no weapon
    {
        return 1 + std::rand() % 5;
    }
    else if(critical)
    {
        auto&               range = model->damage_small;
        return std::max(uint32_t(1), range.min) + std::rand() % std::max(uint32_t(1), range.max);
    }
    else // normal
    {
        auto&               range = model->damage_large;
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
}

void fb::game::session::on_kill(fb::game::life& you)
{
    fb::game::life::on_kill(you);

    auto exp = you.on_exp();
    if(exp > 0)
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

void fb::game::session::on_die(fb::game::object* from)
{
    fb::game::life::on_die(from);
    this->state(STATE::GHOST);
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
        this->assert_state({STATE::RIDING, STATE::GHOST});
        fb::game::life::attack();
    }
    catch(std::exception& e)
    {
        this->message(e.what());
    }
}

void fb::game::session::action(ACTION action, DURATION duration, uint8_t sound)
{
    auto listener = this->get_listener<fb::game::session>();

    try
    {
        this->assert_state({STATE::GHOST, STATE::RIDING});
        if(listener != nullptr)
            listener->on_action(*this, action, duration, sound);
    }
    catch(std::exception& e)
    {
        this->message(e.what());
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

const datetime& fb::game::session::last_login() const
{
    return this->_last_login;
}

void fb::game::session::last_login(const datetime& value)
{
    this->_last_login = value;
}

uint16_t fb::game::session::look() const
{
    return this->_look;
}

void fb::game::session::look(uint16_t value)
{
    auto listener = this->get_listener<fb::game::session>();

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
    auto listener = this->get_listener<fb::game::session>();

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
    auto listener = this->get_listener<fb::game::session>();

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
    auto listener = this->get_listener<fb::game::session>();

    this->_disguise = value;
    this->state(STATE::DISGUISE);
    if(listener != nullptr)
        listener->on_updated(*this, STATE_LEVEL::LEVEL_MAX);
}

void fb::game::session::undisguise()
{
    auto listener = this->get_listener<fb::game::session>();

    this->_disguise = std::nullopt;
    if(this->state() == STATE::DISGUISE)
        this->state(STATE::NORMAL);

    if(listener != nullptr)
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

NATION fb::game::session::nation() const
{
    return this->_nation;
}

bool fb::game::session::nation(NATION value)
{
    if(value != NATION::GOGURYEO &&
       value != NATION::BUYEO)
        return false;

    this->_nation = value;
    return true;
}

CREATURE fb::game::session::creature() const
{
    return this->_creature;
}

bool fb::game::session::creature(CREATURE value)
{
    if(value != CREATURE::DRAGON  &&
       value != CREATURE::PHOENIX &&
       value != CREATURE::TIGER   &&
       value != CREATURE::TURTLE)
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
    if(listener != nullptr)
        listener->on_updated(*this, STATE_LEVEL::LEVEL_MAX);
}

bool fb::game::session::level_up()
{
    if(this->max_level())
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

    auto listener = this->get_listener<fb::game::session>();
    if(listener != nullptr)
        listener->on_level_up(*this);

    return true;
}

bool fb::game::session::max_level() const
{
    return this->context.model.ability[this->_class].contains(this->_level + 1) == false;
}

SEX fb::game::session::sex() const
{
    return this->_sex;
}

void fb::game::session::sex(SEX value)
{
    this->_sex = value;
}

STATE fb::game::session::state() const
{
    return this->_state;
}

void fb::game::session::state(STATE value)
{
    if(this->_state == value)
        return;

    this->_state = value;

    auto listener = this->get_listener<fb::game::session>();
    if(listener != nullptr)
        listener->on_show(*this, false);
}

CLASS fb::game::session::cls() const
{
    return this->_class;
}

void fb::game::session::cls(CLASS value)
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
    if(listener != nullptr)
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
        if(this->_class == CLASS::NONE)
        {
            auto require = this->context.model.ability[CLASS::NONE][5].exp;
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
                this->message(sstream.str());
            }
        }

        if (this->context.model.ability.contains(this->_class) == false)
            throw std::runtime_error("what?");

        while(true)
        {
            if(this->max_level())
                break;

            auto& next = this->context.model.ability[this->_class][this->_level + 1];
            if(next.exp == 0)
                break;

            if(this->_experience < next.exp)
                break;

            if(this->level_up() == false)
                break;
        }

        if(this->_class == CLASS::NONE && this->max_level())
            throw require_class_exception();
    }
    catch(std::exception& e)
    {
        this->message(e.what());
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

    if(this->_class == CLASS::NONE && this->_level >= 5)
        return 0;

    return this->context.model.ability[this->_class][this->_level].exp - this->experience();
}

float fb::game::session::experience_percent() const
{
    auto                    current_level = this->level();
    auto                    next_exp = this->max_level() ? 0xFFFFFFFF : this->context.model.ability[this->_class][current_level].exp;
    auto                    prev_exp = current_level > 1 ? 
                                       (this->max_level() ? 0x00000000 : this->context.model.ability[this->_class][current_level - 1].exp) : 0;
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
    if(listener != nullptr)
    {
        listener->on_updated(*this, STATE_LEVEL::LEVEL_MIN);
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

async::task<uint32_t> fb::game::session::money_drop(uint32_t value)
{
    auto listener = this->get_listener<fb::game::session>();

    try
    {
        if(value == 0)
            co_return 0;

        this->assert_state({STATE::RIDING, STATE::GHOST});


        auto lack = this->money_reduce(value);

        auto cash = new fb::game::cash(this->context, value);
        co_await cash->map(this->_map, this->_position);

        this->action(ACTION::PICKUP, DURATION::PICKUP);
        this->message(message::money::DROP);

        co_return lack;
    }
    catch(std::exception& e)
    {
        this->message(e.what());
        co_return 0;
    }
}

uint32_t fb::game::session::deposited_money() const
{
    return this->_deposited_money;
}

void fb::game::session::deposited_money(uint32_t value)
{
    this->_deposited_money = value;
}

uint32_t fb::game::session::deposit_money(uint32_t value)
{
    uint32_t capacity = 0xFFFFFFFF - this->_deposited_money;
    uint32_t lack = 0;
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

uint32_t fb::game::session::withdraw_money(uint32_t value)
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

bool fb::game::session::deposit_item(fb::game::item& item)
{
    if (item.based<fb::model::item>().attr(ITEM_ATTRIBUTE::BUNDLE))
    {
        auto found = std::find_if(this->_deposited_items.begin(), this->_deposited_items.end(), [&item](fb::game::item* deposited_item)
        {
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
            auto capacity = 0xFFFF - deposited_item->count();
            if(item.count() > capacity)
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

bool fb::game::session::deposit_item(uint8_t index, uint16_t count)
{
    auto item = this->items.at(index);
    if(item == nullptr)
        return false;

    if (item->count() < count)
        return false;

    auto deleted = this->items.remove(*item, count);
    auto result = this->deposit_item(*deleted);
    if(result == false)
        this->items.add(deleted);
    
    return result;
}

bool fb::game::session::deposit_item(const std::string& name, uint16_t count)
{
    auto item = this->items.find(name);
    if (item == nullptr)
        return false;

    auto index = this->items.index(*item);
    if (index == 0xFF)
        return false;

    return this->deposit_item(index, count);
}

fb::game::item* fb::game::session::deposited_item(const fb::model::item& item) const
{
    auto found = std::find_if(this->_deposited_items.cbegin(), this->_deposited_items.cend(), [&item](fb::game::item* deposited_item)
    {
        return deposited_item->based<fb::model::item>() == item;
    });

    if(found == this->_deposited_items.cend())
        return nullptr;

    return *found;
}

const std::vector<fb::game::item*>& fb::game::session::deposited_items() const
{
    return this->_deposited_items;
}

async::task<fb::game::item*> fb::game::session::withdraw_item(uint8_t index, uint16_t count)
{
    if (index > this->_deposited_items.size() - 1)
        co_return nullptr;

    if (this->items.free() == false)
        co_return nullptr;

    auto deposited_item = this->_deposited_items.at(index);
    auto deposited_count = deposited_item->count();
    if (deposited_count < count)
        co_return nullptr;

    auto& model = deposited_item->based<fb::model::item>();
    auto exists = model.attr(ITEM_ATTRIBUTE::BUNDLE) ? this->items.find(model) : nullptr;
    if(exists != nullptr)
    {
        if(exists->count() + count > model.capacity)
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

async::task<fb::game::item*> fb::game::session::withdraw_item(const std::string& name, uint16_t count)
{
    auto found = std::find_if(this->_deposited_items.begin(), this->_deposited_items.end(), [&name](fb::game::item* deposited_item)
    {
        auto& model = deposited_item->based<fb::model::item>();
        return model.name == name;
    });

    if (found == this->_deposited_items.end())
        co_return nullptr;

    auto index = std::distance(this->_deposited_items.begin(), found);
    co_return co_await this->withdraw_item((uint8_t)index, count);
}

async::task<fb::game::item*> fb::game::session::withdraw_item(const fb::model::item& item, uint16_t count)
{
    auto found = std::find_if(this->_deposited_items.begin(), this->_deposited_items.end(), [&item](fb::game::item* deposited_item)
    {
        auto& model = deposited_item->based<fb::model::item>();
        return model == item;
    });

    if (found == this->_deposited_items.end())
        co_return nullptr;

    auto index = std::distance(this->_deposited_items.begin(), found);
    co_return co_await this->withdraw_item((uint8_t)index, count);
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

bool fb::game::session::option(CUSTOM_SETTING key) const
{
    if(static_cast<int>(key) > 0x1B)
        return false;

    return this->_options[static_cast<int>(key)];
}

void fb::game::session::option(CUSTOM_SETTING key, bool value)
{
    if(static_cast<int>(key) > 0x1B)
        return;

    if(this->_options[static_cast<int>(key)] == value)
        return;

    auto listener = this->get_listener<fb::game::session>();
    if(listener != nullptr)
    {
        listener->on_updated(*this, STATE_LEVEL::LEVEL_MIN);
        listener->on_option(*this, key, value);
    }

    this->_options[static_cast<int>(key)] = value;
}

bool fb::game::session::option_toggle(CUSTOM_SETTING key)
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

void fb::game::session::assert_state(STATE value) const
{
    static const auto pairs = std::map<STATE, const std::runtime_error>
    {
        {STATE::GHOST, ghost_exception()},
        {STATE::RIDING, ridding_exception()},
        {STATE::DISGUISE, disguise_exception()}
    };

    if(this->_state == value)
        throw pairs.at(value);
}

void fb::game::session::assert_state(const std::vector<STATE>& values) const
{
    for(auto value : values)
        this->assert_state(value);
}

bool fb::game::session::move(const point16_t& before)
{
    return this->move(this->_direction, before);
}

bool fb::game::session::move(DIRECTION direction, const point16_t& before)
{
    auto listener = this->get_listener<fb::game::session>();

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

async::task<void> fb::game::session::ride(fb::game::mob& horse)
{
    auto listener = this->get_listener<fb::game::session>();

    try
    {
        this->assert_state({STATE::GHOST, STATE::DISGUISE});

        if(this->state() == STATE::RIDING)
            throw std::runtime_error(message::ride::ALREADY_RIDE);

        if(horse.based<fb::model::mob>() != this->context.model.mob[fb::model::const_value::mob::horse])
            throw session::no_conveyance_exception();

        if(horse.map() != this->_map)
            throw std::runtime_error(message::error::UNKNOWN);

        co_await horse.map(nullptr);
        this->state(STATE::RIDING);
        horse.kill();
        this->message(message::ride::ON);
    }
    catch(std::exception& e)
    {
        this->message(e.what());
    }
}

async::task<void> fb::game::session::ride()
{
    auto listener = this->get_listener<fb::game::session>();

    try
    {
        this->assert_state({STATE::GHOST, STATE::DISGUISE});

        auto front = this->forward(OBJECT_TYPE::MOB);
        if(front == nullptr)
            throw session::no_conveyance_exception();

        co_await this->ride(static_cast<fb::game::mob&>(*front));
    }
    catch(std::exception& e)
    {
        this->message(e.what());
    }
}

async::task<void> fb::game::session::unride()
{
    auto listener = this->get_listener<fb::game::session>();

    try
    {
        this->assert_state({STATE::GHOST, STATE::DISGUISE});
        if(this->state() != STATE::RIDING)
            throw std::runtime_error(message::ride::UNRIDE);

        auto& model = this->context.model.mob[const_value::mob::horse];
        auto horse = this->context.make<fb::game::mob>(model, fb::game::mob::config { .alive = true });
        co_await horse->map(this->_map, this->position_forward());
        
        this->state(STATE::NORMAL);
        this->message(message::ride::OFF);
    }
    catch(std::exception& e)
    {
        this->message(e.what());
    }
}

bool fb::game::session::alive() const
{
    return this->_state != STATE::GHOST;
}

void fb::game::session::refresh_map()
{
    if(this->_map == nullptr)
        return;

    auto listener = this->get_listener<fb::game::session>();
    if(listener != nullptr)
        listener->on_map_changed(*this, this->_map, this->_map);
}

bool fb::game::session::condition(const std::vector<fb::model::dsl>& conditions) const
{
    for(auto& dsl : conditions)
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

void fb::game::session::message(const std::string& message, MESSAGE_TYPE type)
{
    auto listener = this->get_listener<fb::game::session>();
    if(listener != nullptr)
        listener->on_notify(*this, message, type);
}

async::task<bool> fb::game::session::inline_sell(const std::string& message, const std::vector<fb::game::npc*>& npcs)
{
    auto count = std::optional<uint16_t>();
    auto name = std::string();
    if (fb::model::const_value::regex::match_sell_message(message, name, count) == false)
        co_return false;

    auto model = this->context.model.item.name2item(name);
    auto bought = false;
    for (auto npc : npcs)
    {
        if (npc->buy(*this, model, count, bought))
            bought = true;
    }

    co_return bought;
}

async::task<bool> fb::game::session::inline_buy(const std::string& message, const std::vector<fb::game::npc*>& npcs)
{
    auto name = std::string();
    auto count = uint16_t(0);
    if (fb::model::const_value::regex::match_buy_message(message, name, count) == false)
        co_return false;

    auto model = this->context.model.item.name2item(name);
    auto sold = false;
    for (auto npc : npcs)
    {
        if (npc->sell(*this, model, count, sold))
            sold = true;
    }

    co_return sold;
}

async::task<bool> fb::game::session::inline_repair(const std::string& message, const std::vector<fb::game::npc*>& npcs)
{
    auto name = std::string();
    if (fb::model::const_value::regex::match_repair_message(message, name) == false)
        co_return false;

    auto model = this->context.model.item.name2item(name);
    auto done = false;
    for (auto npc : npcs)
    {
        if (npc->repair(*this, model, done))
            done = true;
    }

    co_return done;
}

async::task<bool> fb::game::session::inline_deposit_money(const std::string& message, const std::vector<fb::game::npc*>& npcs)
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

async::task<bool> fb::game::session::inline_withdraw_money(const std::string& message, const std::vector<fb::game::npc*>& npcs)
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

async::task<bool> fb::game::session::inline_deposit_item(const std::string& message, const std::vector<fb::game::npc*>& npcs)
{
    auto name = std::string();
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

async::task<bool> fb::game::session::inline_withdraw_item(const std::string& message, const std::vector<fb::game::npc*>& npcs)
{
    auto name = std::string();
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

async::task<bool> fb::game::session::inline_sell_list(const std::string& message, const std::vector<fb::game::npc*>& npcs)
{
    if (fb::model::const_value::regex::match_sell_list(message) == false)
        co_return false;

    for (auto npc : npcs)
    {
        npc->sell_list();
    }

    co_return true;
}

async::task<bool> fb::game::session::inline_buy_list(const std::string& message, const std::vector<fb::game::npc*>& npcs)
{
    if (fb::model::const_value::regex::match_buy_list(message) == false)
        co_return false;

    for (auto npc : npcs)
    {
        npc->buy_list();
    }

    co_return true;
}

async::task<bool> fb::game::session::inline_sell_price(const std::string& message, const std::vector<fb::game::npc*>& npcs)
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

async::task<bool> fb::game::session::inline_buy_price(const std::string& message, const std::vector<fb::game::npc*>& npcs)
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

async::task<bool> fb::game::session::inline_show_deposited_money(const std::string& message, const std::vector<fb::game::npc*>& npcs)
{
    if (fb::model::const_value::regex::match_deposited_money(message) == false)
        co_return false;

    for (auto npc : npcs)
    {
        if(npc->deposited_money(*this))
            co_return true;
    }

    co_return false;
}

async::task<bool> fb::game::session::inline_rename_weapon(const std::string& message, const std::vector<fb::game::npc*>& npcs)
{
    std::string model_name, custom_name;
    if (fb::model::const_value::regex::match_rename_weapon(message, model_name, custom_name) == false)
        co_return false;

    auto model = this->context.model.item.name2item(model_name);
    for (auto npc : npcs)
    {
        if(npc->rename_weapon(*this, model, custom_name))
            co_return true;
    }

    co_return false;
}

async::task<bool> fb::game::session::inline_hold_item_list(const std::string& message, const std::vector<fb::game::npc*>& npcs)
{
    if (fb::model::const_value::regex::match_hold_item_list(message) == false)
        co_return false;

    for (auto npc : npcs)
    {
        if(npc->hold_item_list(*this))
            co_return true;
    }

    co_return false;
}

async::task<bool> fb::game::session::inline_hold_item_count(const std::string& message, const std::vector<fb::game::npc*>& npcs)
{
    auto name = std::string();
    if (fb::model::const_value::regex::match_hold_item_count(message, name) == false)
        co_return false;

    auto model = this->context.model.item.name2item(name);
    for (auto npc : npcs)
    {
        if(npc->hold_item_count(*this, model))
            co_return true;
    }

    co_return false;
}

async::task<bool> fb::game::session::inline_interaction(const std::string& message, const std::vector<fb::game::npc*>& npcs)
{
    if (npcs.size() == 0)
        co_return false;

    for(auto & fn : this->inline_interaction_funcs)
    {
        if (co_await fn(message, npcs))
            co_return true;
    }

    co_return false;
}

fb::protocol::db::Character fb::game::session::to_protocol() const
{
    auto dto = fb::protocol::db::Character();
    dto.id = this->_id;
    dto.name = UTF8(this->_name, PLATFORM::Windows);
    dto.last_login = datetime().to_string();
    dto.admin = this->_admin;
    dto.look = this->_look;
    dto.color = this->_color;
    dto.sex = (uint16_t)this->_sex;
    dto.nation = (uint16_t)this->_nation;
    dto.creature = (uint16_t)this->_creature;
    dto.map = this->_map != nullptr ? this->_map->model.id : 0;
    dto.position = fb::protocol::db::Position{ this->_position.x, this->_position.y };
    dto.direction = (uint8_t)this->_direction;
    dto.state = (uint8_t)this->_state;
    dto.class_type = (uint8_t)this->_class;
    dto.promotion = this->_promotion;
    dto.exp = this->_experience;
    dto.money = this->_money;
    dto.deposited_money = this->_deposited_money;
    dto.disguise = this->_disguise;
    dto.hp = this->_hp;
    dto.base_hp = this->_base_hp;
    dto.additional_hp = 0;
    dto.mp = this->_mp;
    dto.base_mp = this->_base_mp;
    dto.additional_mp = 0;
    dto.weapon_color = std::nullopt;
    dto.helmet_color = std::nullopt;
    dto.armor_color = this->_armor_color;
    dto.shield_color = std::nullopt;
    dto.ring_left_color = std::nullopt;
    dto.ring_right_color = std::nullopt;
    dto.aux_top_color = std::nullopt;
    dto.aux_bot_color = std::nullopt;
    dto.clan = std::nullopt;
    return dto;
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