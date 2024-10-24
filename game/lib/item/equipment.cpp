#include <algorithm>
#include <context.h>
#include <item.h>

fb::game::equipment::equipment(fb::game::context& context, const fb::model::equipment& model) :
    item(context, model)
{
    this->_durability = model.durability;
}

fb::game::equipment::equipment(const equipment& right) :
    item(right)
{ }

fb::game::equipment::~equipment()
{ }

std::string fb::game::equipment::trade_name() const
{
    std::stringstream sstream;
    auto&             model      = this->based<fb::model::equipment>();
    float             percentage = this->_durability / float(model.durability) * 100;
    sstream << model.name << '(' << std::fixed << std::setprecision(1) << percentage << "%)";

    return sstream.str();
}

bool fb::game::equipment::active()
{
    fb::game::item* before = nullptr;
    auto            parts  = EQUIPMENT_PARTS::UNKNOWN;
    auto&           model  = this->based<fb::model::equipment>();
    switch (model.attr())
    {
    case ITEM_ATTRIBUTE::WEAPON:
        before = this->_owner->items.weapon(static_cast<fb::game::weapon*>(this));
        parts  = EQUIPMENT_PARTS::WEAPON;
        break;

    case ITEM_ATTRIBUTE::ARMOR:
        before = this->_owner->items.armor(static_cast<fb::game::armor*>(this));
        parts  = EQUIPMENT_PARTS::ARMOR;
        break;

    case ITEM_ATTRIBUTE::SHIELD:
        before = this->_owner->items.shield(static_cast<fb::game::shield*>(this));
        parts  = EQUIPMENT_PARTS::SHIELD;
        break;

    case ITEM_ATTRIBUTE::HELMET:
        before = this->_owner->items.helmet(static_cast<fb::game::helmet*>(this));
        parts  = EQUIPMENT_PARTS::HELMET;
        break;

    case ITEM_ATTRIBUTE::RING:
        if (this->_owner->items.ring(EQUIPMENT_POSITION::LEFT) == nullptr)
        {
            parts = EQUIPMENT_PARTS::LEFT_HAND;
        }
        else
        {
            parts = EQUIPMENT_PARTS::RIGHT_HAND;
        }

        before = this->_owner->items.ring(static_cast<fb::game::ring*>(this));
        break;

    case ITEM_ATTRIBUTE::AUXILIARY:
        if (this->_owner->items.auxiliary(EQUIPMENT_POSITION::LEFT) == nullptr)
        {
            parts = EQUIPMENT_PARTS::LEFT_AUX;
        }
        else
        {
            parts = EQUIPMENT_PARTS::RIGHT_AUX;
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

    auto listener = this->_owner->get_listener<fb::game::character>();
    if (listener != nullptr)
        listener->on_equipment_on(*this->_owner, *this, parts);

    return true;
}

std::optional<uint32_t> fb::game::equipment::durability() const
{
    return this->_durability;
}

void fb::game::equipment::durability(uint32_t value)
{
    auto& model       = this->based<fb::model::equipment>();
    this->_durability = std::max(uint32_t(0), std::min(model.durability, value));
}

std::string fb::game::equipment::mid_message() const
{
    return std::string();
}

std::string fb::game::equipment::tip_message() const
{
    std::stringstream sstream;
    auto&             model = this->based<fb::model::equipment>();

    sstream << this->name() << std::endl;
    sstream << "내구성: " << std::to_string(this->_durability) << '/' << std::to_string(model.durability) << ' ' << std::fixed << std::setprecision(1)
            << (this->_durability / (float)model.durability) * 100 << '%' << std::endl;
    sstream << this->mid_message();
    sstream << "무장:   " << std::to_string(model.defensive_physical) << " Hit:  " << std::to_string(model.hit) << " Dam:  " << std::to_string(model.damage);

    if (model.base_hp)
        sstream << std::left << std::setw(14) << std::endl << "체력치 상승:" << std::to_string(model.base_hp);

    if (model.base_mp)
        sstream << std::left << std::setw(14) << std::endl << "마력치 상승:" << std::to_string(model.base_mp);

    if (model.strength)
        sstream << std::left << std::setw(14) << std::endl << "힘 상승:" << std::to_string(model.strength);

    if (model.dexteritry)
        sstream << std::left << std::setw(14) << std::endl << "민첩성 상승:" << std::to_string(model.dexteritry);

    if (model.intelligence)
        sstream << std::left << std::setw(14) << std::endl << "지력 상승:" << std::to_string(model.intelligence);

    if (model.healing_cycle)
        sstream << std::left << std::setw(14) << std::endl << "재생력 상승:" << std::to_string(model.healing_cycle);

    std::stringstream class_stream;

    class_stream << std::endl << std::setw(14) << std::left;

    auto cls   = CLASS::NONE;
    auto level = uint8_t(0);
    for (auto& dsl : model.condition)
    {
        switch (dsl.header)
        {
        case DSL::class_t:
            cls = dsl::class_t(dsl.params).value;
            break;

        case DSL::level:
            level = dsl::level(dsl.params).min.value_or(0);
            break;
        }
    }

    sstream << std::endl;
    switch (cls)
    {
    case CLASS::NONE:
        sstream << "직업제한무";
        break;

    default:
        sstream << this->context.model.promotion[cls][0].name << "용";
        break;
    }

    sstream << " 레벨 " << std::to_string(level) << " 이상";

    if (model.desc.empty() == false)
        sstream << std::endl << std::endl << model.desc;

    return sstream.str();
}

const std::string fb::game::equipment::column(EQUIPMENT_PARTS parts)
{
    switch (parts)
    {
    case EQUIPMENT_PARTS::WEAPON:
        return "weapon";

    case EQUIPMENT_PARTS::ARMOR:
        return "armor";

    case EQUIPMENT_PARTS::SHIELD:
        return "shield";

    case EQUIPMENT_PARTS::HELMET:
        return "helmet";

    case EQUIPMENT_PARTS::LEFT_HAND:
        return "ring_left";

    case EQUIPMENT_PARTS::RIGHT_HAND:
        return "ring_right";

    case EQUIPMENT_PARTS::LEFT_AUX:
        return "aux_top";

    case EQUIPMENT_PARTS::RIGHT_AUX:
        return "aux_bot";

    default:
        throw std::runtime_error("invalid equipment parts");
    }
}