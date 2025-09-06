#ifndef __ITEM_EQUIPMENT_H__
#define __ITEM_EQUIPMENT_H__

#include <fb/game/item/base.h>

namespace fb::game {

class equipment : public item
{
public:
    LUA_PROTOTYPE

public:
    struct listener_t;

protected:
    uint32_t _durability = 0;

protected:
    equipment(fb::game::server& server, const fb::model::equipment& model);
    equipment(const fb::game::equipment& right);

public:
    virtual ~equipment();

public:
    bool active() override;

public:
    std::optional<uint32_t> durability() const;
    void                    durability(uint32_t value);

    bool durability_down(uint32_t value);

protected:
    virtual std::string mid_message() const;

public:
    virtual std::string tip_message() const;
    virtual std::string trade_name() const override;

public:
    static const std::string column(EQUIPMENT_PARTS parts);
};

struct equipment::listener_t : public virtual fb::game::item::listener_t
{
    virtual void on_equipment_on(character& me, item& item, EQUIPMENT_PARTS parts)                                  = 0;
    virtual void on_equipment_off(character& me, EQUIPMENT_PARTS parts, fb::game::equipment& equipment)             = 0;
    virtual void on_durability_down(character& me, fb::game::equipment& equipment, uint32_t before, uint32_t after) = 0;
};

} // namespace fb::game

#endif // !__ITEM_EQUIPMENT_H__
