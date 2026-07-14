#ifndef __ITEM_WEAPON_H__
#define __ITEM_WEAPON_H__

#include <fb/game/item/equipment.h>
#include <optional>

namespace fb::game {

class weapon : public equipment
{
public:
    LUA_PROTOTYPE

public:
    enum class types : uint8_t
    {
        NORMAL,
        SPEAR,
        BOW,
        FAN,
        UNKNOWN
    };

private:
    std::optional<std::string> _custom_name;

public:
    weapon(fb::game::server& server, const fb::model::weapon& model, const initial_params& params = {});
    weapon(const fb::game::weapon& right);
    ~weapon();

protected:
    std::string mid_message() const override final;

public:
    const std::string&                name() const override final;
    std::string                       inven_name() const override;
    std::string                       trade_name() const override;
    const std::optional<std::string>& custom_name() const;
    async::task<void>                 custom_name(std::string_view name);
    async::task<void>                 reset_custom_name();
    fb::protocol::internal::Item      to_protocol(EQUIPMENT_PARTS parts = EQUIPMENT_PARTS::UNKNOWN) const override;
};

} // namespace fb::game

#endif // !__ITEM_WEAPON_H__
