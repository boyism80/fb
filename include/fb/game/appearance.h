#ifndef FB_GAME_APPEARANCE_H
#define FB_GAME_APPEARANCE_H

#include <fb/model/model.h>
#include <fb/protocol/client_version.h>
#include <fb/stream_writer.h>
#include <fb/lua.h>
#include <optional>

namespace fb::game {

using namespace fb::model::enum_value;

inline uint8_t walk_delay_from_speed(uint8_t speed)
{
    if (speed > 16)
        return 0;
    return static_cast<uint8_t>(5 * (16 - speed));
}

class object;
class character;

class appearance
{
protected:
    appearance() = default;

public:
    virtual ~appearance() = default;

public:
    virtual void serialize(fb::stream_writer<big_endian>& writer) const = 0;
    virtual void to_lua(fb::lua::context* lua) const                    = 0;
};

template <fb::protocol::CLIENT_VERSION V = fb::protocol::CLIENT_VERSION::v550>
class character_appearance : public appearance
{
public:
    GENDER                  gender       = GENDER::MALE;
    std::optional<STATE>    state        = std::nullopt;
    uint16_t                hair         = 0;
    std::optional<uint8_t>  hair_color   = std::nullopt;
    std::optional<uint16_t> weapon       = std::nullopt;
    std::optional<uint8_t>  weapon_color = std::nullopt;
    std::optional<uint8_t>  armor        = std::nullopt;
    std::optional<uint8_t>  armor_color  = std::nullopt;
    std::optional<uint8_t>  shield       = std::nullopt;
    std::optional<uint8_t>  shield_color = std::nullopt;
    std::optional<uint16_t> disguise     = std::nullopt;
    uint8_t                 speed        = 0;

public:
    static character_appearance from(const character& ch);

public:
    character_appearance() = default;
    character_appearance(GENDER                  gender,
                         std::optional<STATE>    state,
                         uint16_t                hair,
                         std::optional<uint8_t>  hair_color,
                         std::optional<uint16_t> weapon,
                         std::optional<uint8_t>  weapon_color,
                         std::optional<uint8_t>  armor,
                         std::optional<uint8_t>  armor_color,
                         std::optional<uint8_t>  shield,
                         std::optional<uint8_t>  shield_color,
                         std::optional<uint16_t> disguise);
    character_appearance(const character_appearance&) = default;

    template <fb::protocol::CLIENT_VERSION Other>
    character_appearance(const character_appearance<Other>& right) :
        gender(right.gender),
        state(right.state),
        hair(right.hair),
        hair_color(right.hair_color),
        weapon(right.weapon),
        weapon_color(right.weapon_color),
        armor(right.armor),
        armor_color(right.armor_color),
        shield(right.shield),
        shield_color(right.shield_color),
        disguise(right.disguise),
        speed(right.speed)
    { }

    ~character_appearance() = default;

public:
    void serialize(fb::stream_writer<big_endian>& writer) const override;
    void to_lua(fb::lua::context* lua) const override;
};

template <>
class character_appearance<fb::protocol::CLIENT_VERSION::v651> : public appearance
{
public:
    GENDER                       gender          = GENDER::MALE;
    std::optional<STATE>         state           = std::nullopt;
    uint16_t                     hair            = 0;
    std::optional<uint8_t>       hair_color      = std::nullopt;
    std::optional<uint16_t>      weapon          = std::nullopt;
    std::optional<uint8_t>       weapon_color    = std::nullopt;
    std::optional<uint8_t>       armor           = std::nullopt;
    std::optional<uint8_t>       armor_color     = std::nullopt;
    std::optional<uint8_t>       shield          = std::nullopt;
    std::optional<uint8_t>       shield_color    = std::nullopt;
    std::optional<uint16_t>      disguise        = std::nullopt;
    uint8_t                      speed           = 0;
    uint16_t                     ridable_id      = 0;
    uint8_t                      face            = 0;
    uint8_t                      hair_style      = 0;
    uint8_t                      face_hair_tint  = 0;
    uint8_t                      body_color      = 0;
    uint8_t                      hair_to_hat     = 0;
    std::optional<uint8_t>       helmet          = std::nullopt;
    std::optional<uint8_t>       helmet_color    = std::nullopt;
    uint16_t                     accessory_pack  = 0xFFFF;
    uint8_t                      accessory_color = 0;
    fb::protocol::CLIENT_UI_MODE ui_mode         = fb::protocol::CLIENT_UI_MODE::OLD;

public:
    static character_appearance from(const character& ch);

public:
    character_appearance() = default;
    character_appearance(GENDER                  gender,
                         std::optional<STATE>    state,
                         uint16_t                hair,
                         std::optional<uint8_t>  hair_color,
                         std::optional<uint16_t> weapon,
                         std::optional<uint8_t>  weapon_color,
                         std::optional<uint8_t>  armor,
                         std::optional<uint8_t>  armor_color,
                         std::optional<uint8_t>  shield,
                         std::optional<uint8_t>  shield_color,
                         std::optional<uint16_t> disguise);
    character_appearance(const character_appearance&) = default;

    template <fb::protocol::CLIENT_VERSION Other>
    character_appearance(const character_appearance<Other>& right) :
        gender(right.gender),
        state(right.state),
        hair(right.hair),
        hair_color(right.hair_color),
        weapon(right.weapon),
        weapon_color(right.weapon_color),
        armor(right.armor),
        armor_color(right.armor_color),
        shield(right.shield),
        shield_color(right.shield_color),
        disguise(right.disguise),
        speed(right.speed)
    { }

    ~character_appearance() = default;

public:
    void serialize(fb::stream_writer<big_endian>& writer) const override;
    void serialize(fb::stream_writer<big_endian>& writer, fb::protocol::CLIENT_UI_MODE ui_mode) const;
    void to_lua(fb::lua::context* lua) const override;
};

template <fb::protocol::CLIENT_VERSION V = fb::protocol::CLIENT_VERSION::v550>
class object_appearance : public appearance
{
public:
    uint16_t look  = 0;
    uint8_t  color = 0;

public:
    object_appearance() = default;
    object_appearance(uint16_t look, uint8_t color);
    object_appearance(const object_appearance& right);
    ~object_appearance() = default;

public:
    void serialize(fb::stream_writer<big_endian>& writer) const override;
    void to_lua(fb::lua::context* lua) const override;
};

template <>
void object_appearance<fb::protocol::CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;

class appearance_factory
{
public:
    static std::unique_ptr<appearance> create(const fb::model::object& obj);
    static std::unique_ptr<appearance> create(const fb::game::object& obj);
    template <fb::protocol::CLIENT_VERSION V>
    static std::unique_ptr<appearance> create(const fb::model::object& obj);
    template <fb::protocol::CLIENT_VERSION V>
    static std::unique_ptr<appearance> create(const fb::game::object& obj);
    static std::unique_ptr<appearance> create(const fb::model::object& obj, const fb::game::character& viewer);
    static std::unique_ptr<appearance> create(const fb::game::object& obj, const fb::game::character& viewer);

private:
    appearance_factory() = default;
};

} // namespace fb::game

#endif
