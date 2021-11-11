#ifndef __ITEM_EQUIPMENT_H__
#define __ITEM_EQUIPMENT_H__

#include "model/item/item.base.h"

namespace fb { namespace game {

class equipment : public item
{
#pragma region enum
public:
    enum class slot : uint8_t
    {
        UNKNOWN_SLOT                    = 0,
        WEAPON_SLOT                     = 1,
        ARMOR_SLOT                      = 2,
        SHIELD_SLOT                     = 3,
        HELMET_SLOT                     = 4,
        LEFT_HAND_SLOT                  = 7,
        RIGHT_HAND_SLOT                 = 8,
        LEFT_AUX_SLOT                   = 20,
        RIGHT_AUX_SLOT                  = 21,
    };

    enum class position : uint8_t { LEFT = 0, RIGHT = 1, };
#pragma endregion

#pragma region forward nested declaration
public:
    interface listener;

public:
    class master;

public:
    struct repair;
    struct rename;
#pragma endregion

#pragma region type definition
public:
    DECLARE_EXCEPTION(not_equipment_exception, "입을 수 없는 물건입니다.")
#pragma endregion
    
#pragma region protected field
protected:
    uint16_t                            _durability = 0;
#pragma endregion

#pragma region constructor / destructor
public:
    equipment(fb::game::context& context, const fb::game::equipment::master* master);
    equipment(const fb::game::equipment& right);
    virtual ~equipment();
#pragma endregion

#pragma region public method
public:
    const std::string                   name_trade() const;
    bool                                active();

public:
    std::optional<uint16_t>             durability() const;
    void                                durability(std::optional<uint16_t> value);
#pragma endregion


#pragma region virtual method
protected:
    virtual std::string                 mid_message() const;

public:
    virtual std::string                 tip_message() const;
#pragma endregion

#pragma region static method
public:
    static const std::string            column(equipment::slot slot);
#pragma endregion
};


#pragma region nested interface
interface equipment::listener : public virtual fb::game::item::listener
{
    virtual void on_equipment_on(session& me, item& item, equipment::slot slot) = 0;
    virtual void on_equipment_off(session& me, equipment::slot slot, uint8_t index) = 0;
};
#pragma endregion

#pragma region nested structure
struct equipment::repair
{
public:
    friend class equipment;

private:
    bool                                _enabled = false;
    float                               _price   = 0.0f;

public:
    repair(bool enabled = true, float price = 0) : _enabled(enabled), _price(price) { }

public:
    bool                                enabled() const { return this->_enabled; }
    void                                enabled(bool value) { this->_enabled = value; }

    float                               price() const { return this->_price; }
    void                                price(float value) { this->_price = value; }
};

struct equipment::rename
{
public:
    friend class equipment;

private:
    bool                                _enabled = false;
    uint32_t                            _price   = 0;

public:
    rename(bool enabled = true, uint32_t price = 0) : _enabled(enabled), _price(price) { }

public:
    bool                                enabled() const { return this->_enabled; }
    void                                enabled(bool value) { this->_enabled = value; }

    uint32_t                            price() const { return this->_price; }
    void                                price(uint32_t value) { this->_price = value; }
};
#pragma endregion

#pragma region nested class
class equipment::master : public fb::game::item::master
{
public:
    friend class equipment;

public:
    const uint16_t                      dress;
    const uint16_t                      durability;
    const fb::game::equipment::repair   repair;
    const fb::game::equipment::rename   rename;
    const std::string                   dress_script, undress_script;

    const uint8_t                       hit, damage;
    const uint8_t                       strength, intelligence, dexteritry;
    const uint32_t                      base_hp, base_mp;
    const float                         hp_percentage, mp_percentage;
    const uint8_t                       healing_cycle;
    const fb::game::defensive           defensive;

public:
    master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color = 0, uint16_t durability = 100);
    master(const std::string&                  name, 
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
           const fb::game::defensive&          defensive);
    ~master();

public:
    virtual fb::game::item::attrs       attr() const;
};
#pragma endregion

} }

#endif // !__ITEM_EQUIPMENT_H__
