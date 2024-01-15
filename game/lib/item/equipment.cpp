#include <algorithm>
#include <fb/game/item.h>
#include <fb/game/context.h>

fb::game::equipment::model::model(const fb::game::equipment::model::config& config) : 
    fb::game::item::model(config),
    dress(config.dress),
    durability(config.durability),
    repair(config.repair),
    rename(config.rename),
    dress_script(config.dress_script),
    undress_script(config.undress_script),
    hit(config.hit),
    damage(config.damage),
    strength(config.strength),
    intelligence(config.intelligence),
    dexteritry(config.dexteritry),
    base_hp(config.base_hp),
    base_mp(config.base_mp),
    hp_percentage(config.hp_percentage),
    mp_percentage(config.mp_percentage),
    healing_cycle(config.healing_cycle),
    defensive(config.defensive)
{ }

fb::game::equipment::model::~model()
{ }

fb::game::item::attrs fb::game::equipment::model::attr() const
{
    return item::attrs::EQUIPMENT;
}

fb::game::equipment::equipment(fb::game::context& context, const equipment::model* model) : 
    item(context, model)
{
    this->_durability = model->durability;
}

fb::game::equipment::equipment(const equipment& right) :
    item(right)
{ }

fb::game::equipment::~equipment()
{ }


const std::string fb::game::equipment::name_trade() const
{
    std::stringstream       sstream;
    auto                    model = this->based<fb::game::equipment>();
    float                   percentage = this->_durability / float(model->durability) * 100;
    sstream << this->name() << '(' << std::fixed << std::setprecision(1) << percentage << "%)";

    return sstream.str();
}

bool fb::game::equipment::active()
{
    fb::game::item*         before = nullptr;
    auto                    slot = equipment::slot::UNKNOWN_SLOT;
    switch(this->attr())
    {
    case item::attrs::WEAPON:
        before = this->_owner->items.weapon(static_cast<fb::game::weapon*>(this));
        slot = equipment::slot::WEAPON_SLOT;
        break;

    case item::attrs::ARMOR:
        before = this->_owner->items.armor(static_cast<fb::game::armor*>(this));
        slot = equipment::slot::ARMOR_SLOT;
        break;

    case item::attrs::SHIELD:
        before = this->_owner->items.shield(static_cast<fb::game::shield*>(this));
        slot = equipment::slot::SHIELD_SLOT;
        break;

    case item::attrs::HELMET:
        before = this->_owner->items.helmet(static_cast<fb::game::helmet*>(this));
        slot = equipment::slot::HELMET_SLOT;
        break;

    case item::attrs::RING:
        if(this->_owner->items.ring(equipment::position::LEFT) == nullptr)
        {
            slot = equipment::slot::LEFT_HAND_SLOT;
        }
        else
        {
            slot = equipment::slot::RIGHT_HAND_SLOT;
        }

        before = this->_owner->items.ring(static_cast<fb::game::ring*>(this));
        break;


    case item::attrs::AUXILIARY:
        if(this->_owner->items.auxiliary(equipment::position::LEFT) == nullptr)
        {
            slot = equipment::slot::LEFT_AUX_SLOT;
        }
        else
        {
            slot = equipment::slot::RIGHT_AUX_SLOT;
        }

        before = this->_owner->items.auxiliary(static_cast<fb::game::auxiliary*>(this));
        break;

    default:
        throw equipment::not_equipment_exception();
    }

    // 인벤토리에서는 사라지지만 소유상태는 유지되므로
    // id를 유지시켜줘야 한다.
    this->_owner->items.remove(*this);

    this->_owner->items.add(before);

    auto listener = this->_owner->get_listener<fb::game::session>();
    listener->on_equipment_on(*this->_owner, *this, slot);

    return true;
}

std::optional<uint16_t> fb::game::equipment::durability() const
{
    return this->_durability;
}

void fb::game::equipment::durability(uint16_t value)
{
    auto model = this->based<fb::game::equipment>();
    this->_durability = std::max(uint16_t(0), std::min(model->durability, value));
}

std::string fb::game::equipment::mid_message() const
{
    return std::string();
}

std::string fb::game::equipment::tip_message() const
{
    std::stringstream           sstream;
    auto                        model = this->based<fb::game::equipment>();

    sstream << this->name() << std::endl;
    sstream << "내구성: " << std::to_string(this->_durability) << '/' << std::to_string(model->durability) << ' ' << std::fixed << std::setprecision(1) << (this->_durability / (float)model->durability) * 100 << '%' << std::endl;
    sstream << this->mid_message();
    sstream << "무장:   " << std::to_string(model->defensive.physical) << " Hit:  " << std::to_string(model->hit) << " Dam:  " << std::to_string(model->damage);

    if(model->base_hp)
        sstream << std::left << std::setw(14) << std::endl << "체력치 상승:" << std::to_string(model->base_hp);

    if(model->base_mp)
        sstream << std::left << std::setw(14) << std::endl << "마력치 상승:" << std::to_string(model->base_mp);

    if(model->strength)
        sstream << std::left << std::setw(14) << std::endl << "힘 상승:" << std::to_string(model->strength);

    if(model->dexteritry)
        sstream << std::left << std::setw(14) << std::endl << "민첩성 상승:" << std::to_string(model->dexteritry);

    if(model->intelligence)
        sstream << std::left << std::setw(14) << std::endl << "지력 상승:" << std::to_string(model->intelligence);

    if(model->healing_cycle)
        sstream << std::left << std::setw(14) << std::endl << "재생력 상승:" << std::to_string(model->healing_cycle);

    std::stringstream class_stream;

    class_stream << std::endl << std::setw(14) << std::left;
    
    if(model->condition.cls == 0)
        class_stream << "직업제한무";
    else
        class_stream << fb::game::model::classes.class2name(model->condition.cls, 0) << "용";
    sstream << class_stream.str() << "레벨 " << std::to_string(model->condition.level) << " 이상";

    if(model->desc.empty() == false)
        sstream << std::endl << std::endl << model->desc;

    return sstream.str();
}

const std::string equipment::column(equipment::slot slot)
{
    switch(slot)
    {
    case fb::game::equipment::slot::WEAPON_SLOT:
        return "weapon";

    case fb::game::equipment::slot::ARMOR_SLOT:
        return "armor";

    case fb::game::equipment::slot::SHIELD_SLOT:
        return "shield";

    case fb::game::equipment::slot::HELMET_SLOT:
        return "helmet";

    case fb::game::equipment::slot::LEFT_HAND_SLOT:
        return "ring_left";

    case fb::game::equipment::slot::RIGHT_HAND_SLOT:
        return "ring_right";

    case fb::game::equipment::slot::LEFT_AUX_SLOT:
        return "aux_top";

    case fb::game::equipment::slot::RIGHT_AUX_SLOT:
        return "aux_bot";

    default:
        throw std::runtime_error("invalid equipment slot");
    }
}