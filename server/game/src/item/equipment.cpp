#include <fb/game/item.h>
#include <fb/game/server.h>

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

using table = fb::model::table;

fb::game::equipment::equipment(fb::game::server&           server,
                               const fb::model::equipment& model,
                               const initial_params&       params) :
    item(server, model, params)
{
    this->_durability = model.durability;
}

fb::game::equipment::equipment(const equipment& right) :
    item(right)
{ }

fb::game::equipment::~equipment()
{ }

const fb::model::equipment& fb::game::equipment::model() const
{
    return static_cast<const fb::model::equipment&>(fb::model::table::item[this->_model_id]);
}

std::string fb::game::equipment::trade_name() const
{
    std::stringstream sstream;
    auto&             model      = this->model();
    float             percentage = this->_durability / float(model.durability) * 100;
    sstream << model.name << '(' << std::fixed << std::setprecision(1) << percentage << "%)";

    return sstream.str();
}

async::task<bool> fb::game::equipment::active()
{
    if (this->_container == nullptr)
        co_return false;

    auto owner = this->_container->owner();
    if (owner == nullptr)
        co_return false;

    auto& model = this->model();

    for (auto& dsl : model.condition)
    {
        switch (dsl.header)
        {
        case DSL::level:
        {
            auto params = fb::model::dsl::level(dsl.params);
            if (owner->level() < params.min)
                throw std::runtime_error(_TEXT(MESSAGE_EQUIPMENT_LEVEL_INSUFFICIENT));
        }

        break;

        case DSL::strength:
        {
            auto params = fb::model::dsl::strength(dsl.params);
            if (owner->stat.str() < params.value)
                throw std::runtime_error(_TEXT(MESSAGE_EQUIPMENT_TOO_HEAVY));
        }

        break;

        case DSL::dexterity:
        {
            auto params = fb::model::dsl::dexterity(dsl.params);
            if (owner->stat.dex() < params.value)
                throw std::runtime_error(_TEXT(MESSAGE_EQUIPMENT_DEXTERITY_INSUFFICIENT));
        }

        break;

        case DSL::intelligence:
        {
            auto params = fb::model::dsl::intelligence(dsl.params);
            if (owner->stat.intelligence() < params.value)
                throw std::runtime_error(_TEXT(MESSAGE_EQUIPMENT_INTELLIGENCE_INSUFFICIENT));
        }

        break;

        case DSL::class_t:
        {
            auto params = fb::model::dsl::class_t(dsl.params);
            if (owner->cls() != params.value)
                throw std::runtime_error(_TEXT(MESSAGE_EQUIPMENT_CANNOT_EQUIP));
        }

        break;

        case DSL::promotion:
        {
            auto params = fb::model::dsl::promotion(dsl.params);
            if (owner->promotion() < params.value)
                throw std::runtime_error(_TEXT(MESSAGE_EQUIPMENT_CANNOT_EQUIP));
        }

        break;

        case DSL::gender:
        {
            auto params = fb::model::dsl::gender(dsl.params);
            if (owner->gender() != params.value)
                throw std::runtime_error(_TEXT(MESSAGE_EQUIPMENT_CANNOT_EQUIP));
        }

        break;
        }
    }

    auto parts  = co_await owner->items.wear(this->shared_from_this_as<fb::game::equipment>());
    std::ignore = co_await fb::game::item::active();

    // Execute equipment activation script
    auto lua = this->server.lua.open("scripts/interaction.lua", "on_equipment_active");
    if (lua)
    {
        lua->pushobject(owner);
        lua->pushinteger(parts);
        lua->pushobject(*this);
        fb::lua::run_async(std::move(lua), 3);
    }

    co_return true;
}

std::optional<uint32_t> fb::game::equipment::durability() const
{
    return this->_durability;
}

void fb::game::equipment::durability(uint32_t value)
{
    auto& model       = this->model();
    this->_durability = std::max(uint32_t(0), std::min(model.durability, value));
}

bool fb::game::equipment::durability_down(uint32_t value)
{
    auto owner = this->owner();
    if (owner == nullptr)
        return false;

    auto before = this->_durability;

    if (value > this->_durability)
    {
        this->_durability = 0;
    }
    else
    {
        this->_durability -= value;
    }

    owner->listener.on_durability_down(*owner, *this, before, this->_durability);
    return this->_durability == 0;
}

std::string fb::game::equipment::mid_message() const
{
    return std::string();
}

std::string fb::game::equipment::tip_message() const
{
    std::stringstream sstream;
    auto&             model = this->model();

    sstream << this->name() << std::endl;
    sstream << "내구성: " << std::to_string(this->_durability) << '/' << std::to_string(model.durability) << ' '
            << std::fixed << std::setprecision(1) << (this->_durability / (float)model.durability) * 100 << '%'
            << std::endl;
    sstream << this->mid_message();
    sstream << "무장:   " << std::to_string(model.defensive_physical) << " Hit:  " << std::to_string(model.hit)
            << " Dam:  " << std::to_string(model.damage);

    if (model.base_hp)
        sstream << std::left << std::setw(14) << std::endl << "체력치 상승:" << std::to_string(model.base_hp);

    if (model.base_mp)
        sstream << std::left << std::setw(14) << std::endl << "마력치 상승:" << std::to_string(model.base_mp);

    if (model.hp_percentage != 0.0f)
        sstream << std::left << std::setw(14) << std::endl
                << "최대체력 퍼센트:" << std::fixed << std::setprecision(1) << model.hp_percentage << '%';

    if (model.mp_percentage != 0.0f)
        sstream << std::left << std::setw(14) << std::endl
                << "최대마력 퍼센트:" << std::fixed << std::setprecision(1) << model.mp_percentage << '%';

    if (model.strength)
        sstream << std::left << std::setw(14) << std::endl << "힘 상승:" << std::to_string(model.strength);

    if (model.dexterity)
        sstream << std::left << std::setw(14) << std::endl << "민첩성 상승:" << std::to_string(model.dexterity);

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
            cls = fb::model::dsl::class_t(dsl.params).value;
            break;

        case DSL::level:
            level = fb::model::dsl::level(dsl.params).min.value_or(0);
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
        sstream << table::promotion[cls][0].name << "용";
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