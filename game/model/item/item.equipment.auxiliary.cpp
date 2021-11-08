#include "model/item/item.equipment.auxiliary.h"

fb::game::auxiliary::master::master(const std::string&                  name, 
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
                                    const fb::game::defensive&          defensive) : fb::game::equipment::master(name,
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
                                                                                                                 active_script,
                                                                                                                 dress,
                                                                                                                 durability,
                                                                                                                 repair,
                                                                                                                 rename,
                                                                                                                 dress_script,
                                                                                                                 undress_script,
                                                                                                                 hit,
                                                                                                                 damage,
                                                                                                                 strength,
                                                                                                                 intelligence,
                                                                                                                 dexteritry,
                                                                                                                 base_hp,
                                                                                                                 base_mp,
                                                                                                                 hp_percentage,
                                                                                                                 mp_percentage,
                                                                                                                 healing_cycle,
                                                                                                                 defensive)
{ }

fb::game::auxiliary::master::~master()
{ }

fb::game::item::attrs fb::game::auxiliary::master::attr() const
{
    return item::attrs::AUXILIARY;
}

fb::game::item* fb::game::auxiliary::master::make(fb::game::context* context) const
{
    return new fb::game::auxiliary(context, this);
}



fb::game::auxiliary::auxiliary(fb::game::context* context, const master* master) : 
    equipment(context, master)
{ }

fb::game::auxiliary::auxiliary(const auxiliary& right) : 
    equipment(right)
{ }

fb::game::auxiliary::~auxiliary()
{ }