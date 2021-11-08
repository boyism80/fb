#include "model/item/item.equipment.h"
#include "model/context/context.game.h"

fb::game::equipment::master::master(const std::string&                  name, 
                                    uint16_t                            look, 
                                    uint8_t                             color,
                                    uint32_t                            id,
                                    uint32_t                            price,
                                    const fb::game::item::conditions&   condition,
                                    penalties                           penalty,
                                    uint16_t                            capacity,
                                    const fb::game::item::trade&        trade,
                                    const fb::game::item::storage&      storage,
                                    std::string                         desc,
                                    std::string                         active_script,
                                    uint16_t                            dress,
                                    uint16_t                            durability,
                                    const fb::game::equipment::repair&  repair,
                                    const fb::game::equipment::rename&  rename,
                                    const std::string&                  dress_script,
                                    const std::string&                  undress_script,
                                    uint8_t                             hit,
                                    uint8_t                             damage,
                                    uint8_t                             strength,
                                    uint8_t                             intelligence,
                                    uint8_t                             dexteritry,
                                    uint32_t                            base_hp,
                                    uint32_t                            base_mp,
                                    float                               hp_percentage,
                                    float                               mp_percentage,
                                    uint8_t                             healing_cycle,
                                    const fb::game::defensive&          defensive) : fb::game::item::master(name, 
                                                                                                            look, 
                                                                                                            color, 
                                                                                                            id, 
                                                                                                            price, 
                                                                                                            condition, 
                                                                                                            penalty, 
                                                                                                            capacity, 
                                                                                                            trade, 
                                                                                                            storage, 
                                                                                                            desc, 
                                                                                                            active_script),
                                                                                     dress(dress),
                                                                                     durability(durability),
                                                                                     repair(repair),
                                                                                     rename(rename),
                                                                                     dress_script(dress_script),
                                                                                     undress_script(undress_script),
                                                                                     hit(hit),
                                                                                     damage(damage),
                                                                                     strength(strength),
                                                                                     intelligence(intelligence),
                                                                                     dexteritry(dexteritry),
                                                                                     base_hp(base_hp),
                                                                                     base_mp(base_mp),
                                                                                     hp_percentage(hp_percentage),
                                                                                     mp_percentage(mp_percentage),
                                                                                     healing_cycle(healing_cycle),
                                                                                     defensive(defensive)
{ }

fb::game::equipment::master::~master()
{ }

fb::game::item::attrs fb::game::equipment::master::attr() const
{
    return item::attrs::EQUIPMENT;
}

fb::game::item* fb::game::equipment::master::make(fb::game::context* context) const
{
    return new fb::game::equipment(context, this);
}

fb::game::equipment::equipment(fb::game::context* context, const equipment::master* master) : 
    item(context, master)
{
    this->_durability = master->durability;
}

fb::game::equipment::equipment(const equipment& right) :
    item(right)
{ }

fb::game::equipment::~equipment()
{ }


const std::string fb::game::equipment::name_trade() const
{
    std::stringstream       sstream;
    auto                    master = this->based<fb::game::equipment>();
    float                   percentage = this->_durability / float(master->durability) * 100;
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

void fb::game::equipment::durability(std::optional<uint16_t> value)
{
    this->_durability = value.value_or(0);
}

std::string fb::game::equipment::mid_message() const
{
    return std::string();
}

std::string fb::game::equipment::tip_message() const
{
    std::stringstream           sstream;
    auto                        master = this->based<fb::game::equipment>();

    sstream << this->name() << std::endl;
    sstream << "내구성: " << std::to_string(this->_durability) << '/' << std::to_string(master->durability) << ' ' << std::fixed << std::setprecision(1) << (this->_durability / (float)master->durability) * 100 << '%' << std::endl;
    sstream << this->mid_message();
    sstream << "무장:   " << std::to_string(master->defensive.physical) << " Hit:  " << std::to_string(master->hit) << " Dam:  " << std::to_string(master->damage);

    if(master->base_hp)
        sstream << std::left << std::setw(14) << std::endl << "체력치 상승:" << std::to_string(master->base_hp);

    if(master->base_mp)
        sstream << std::left << std::setw(14) << std::endl << "마력치 상승:" << std::to_string(master->base_mp);

    if(master->strength)
        sstream << std::left << std::setw(14) << std::endl << "힘 상승:" << std::to_string(master->strength);

    if(master->dexteritry)
        sstream << std::left << std::setw(14) << std::endl << "민첩성 상승:" << std::to_string(master->dexteritry);

    if(master->intelligence)
        sstream << std::left << std::setw(14) << std::endl << "지력 상승:" << std::to_string(master->intelligence);

    if(master->healing_cycle)
        sstream << std::left << std::setw(14) << std::endl << "재생력 상승:" << std::to_string(master->healing_cycle);

    std::stringstream class_stream;

    class_stream << std::endl << std::setw(14) << std::left;
    
    if(master->condition.cls == 0)
        class_stream << "직업제한무";
    else
        class_stream << fb::game::table::classes.class2name(master->condition.cls, 0) << "용";
    sstream << class_stream.str() << "레벨 " << std::to_string(master->condition.level) << " 이상";

    if(master->desc.empty() == false)
        sstream << std::endl << std::endl << master->desc;

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