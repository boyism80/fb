#ifndef __ITEM_EQUIPMENT_H__
#define __ITEM_EQUIPMENT_H__

#include "model/item/item.base.h"

namespace fb { namespace game {

class equipment : public item
{
#pragma region enum
public:
    enum slot : uint8_t
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

    enum position : uint8_t { EQUIPMENT_LEFT = 0, EQUIPMENT_RIGHT = 1, };
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
    uint16_t                            _durability;
#pragma endregion

#pragma region constructor / destructor
public:
    equipment(const fb::game::equipment::master* master, fb::game::equipment::listener* listener);
    equipment(const fb::game::equipment& right);
    virtual ~equipment();
#pragma endregion

#pragma region public method
public:
    const std::string                   name_trade() const;
    bool                                active();

public:
    uint16_t                            dress() const;

    std::optional<uint16_t>             durability() const;
    void                                durability(std::optional<uint16_t> value);
    uint16_t                            durability_base() const;

    bool                                repair_enabled() const;
    float                               repair_price() const;

    bool                                rename_enabled() const;
    uint32_t                            rename_price() const;

    int16_t                             defensive_physical() const;
    int16_t                             defensive_magical() const;


    uint8_t                             hit() const;
    uint8_t                             damage() const;
    
    uint8_t                             strength() const;
    uint8_t                             intelligence() const;
    uint8_t                             dexteritry() const;

    uint32_t                            base_hp() const;
    uint32_t                            base_mp() const;

    float                               hp_percentage() const;
    float                               mp_percentage() const;

    uint8_t                             healing_cycle() const;
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
    bool                                _enabled;
    float                               _price;

public:
    explicit repair(bool enabled = true, float price = 0) : _enabled(enabled), _price(price) {}

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
    bool                                _enabled;
    uint32_t                            _price;

public:
    explicit rename(bool enabled = true, uint32_t price = 0) : _enabled(enabled), _price(price) {}

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

protected:
    uint16_t                            _dress;
    uint16_t                            _durability;
    fb::game::equipment::repair         _repair;
    fb::game::equipment::rename         _rename;
    std::string                         _dress_script, _undress_script;

    uint8_t                             _hit, _damage;
    uint8_t                             _strength, _intelligence, _dexteritry;
    uint32_t                            _base_hp, _base_mp;
    float                               _hp_percentage, _mp_percentage;
    uint8_t                             _healing_cycle;
    fb::game::defensive                 _defensive;

public:
    master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color = 0, uint16_t durability = 100);
    ~master();

public:
    virtual fb::game::item::attrs       attr() const;
    virtual fb::game::item*             make(fb::game::item::listener* listener) const;

public:
    uint16_t                            dress() const;

    std::optional<uint16_t>             durability() const;
    void                                durability(uint16_t value);

    bool                                repair_enabled() const;
    void                                repair_enabled(bool value);

    float                               repair_price() const;
    void                                repair_price(float value);

    bool                                rename_enabled() const;
    void                                rename_enabled(bool value);

    uint32_t                            rename_price() const;
    void                                rename_price(uint32_t value);

    int16_t                             defensive_physical() const;
    void                                defensive_physical(int16_t value);

    int16_t                             defensive_magical() const;
    void                                defensive_magical(int16_t value);


    uint8_t                             hit() const;
    void                                hit(uint8_t value);

    uint8_t                             damage() const;
    void                                damage(uint8_t value);

    uint8_t                             strength() const;
    void                                strength(uint8_t value);

    uint8_t                             intelligence() const;
    void                                intelligence(uint8_t value);

    uint8_t                             dexteritry() const;
    void                                dexteritry(uint8_t value);

    uint32_t                            base_hp() const;
    void                                base_hp(uint32_t value);

    uint32_t                            base_mp() const;
    void                                base_mp(uint32_t value);

    float                               hp_percentage() const;
    void                                hp_percentage(float value);

    float                               mp_percentage() const;
    void                                mp_percentage(float value);

    uint8_t                             healing_cycle() const;
    void                                healing_cycle(uint8_t value);

    const std::string&                  dress_script() const;
    void                                dress_script(const std::string& value);

    const std::string&                  undress_script() const;
    void                                undress_script(const std::string& value);
};
#pragma endregion

} }

#endif // !__ITEM_EQUIPMENT_H__
