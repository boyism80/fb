#ifndef FB_GAME_APPEARANCE_H
#define FB_GAME_APPEARANCE_H

#include <fb/model/model.h>
#include <fb/stream_writer.h>
#include <fb/lua.h>

namespace fb::game {

using namespace fb::model::enum_value;

class object;

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

class character_appearance : public appearance
{
public:
    GENDER                  gender       = GENDER::MAN;
    STATE                   state        = STATE::NORMAL;
    uint16_t                hair         = 0;
    std::optional<uint8_t>  hair_color   = std::nullopt;
    std::optional<uint16_t> weapon       = std::nullopt;
    std::optional<uint8_t>  weapon_color = std::nullopt;
    std::optional<uint8_t>  armor        = std::nullopt;
    std::optional<uint8_t>  armor_color  = std::nullopt;
    std::optional<uint8_t>  shield       = std::nullopt;
    std::optional<uint8_t>  shield_color = std::nullopt;
    std::optional<uint16_t> disguise     = std::nullopt;

public:
    character_appearance() = default;
    character_appearance(GENDER                  gender,
                         STATE                   state,
                         uint16_t                hair,
                         std::optional<uint8_t>  hair_color,
                         std::optional<uint16_t> weapon,
                         std::optional<uint8_t>  weapon_color,
                         std::optional<uint8_t>  armor,
                         std::optional<uint8_t>  armor_color,
                         std::optional<uint8_t>  shield,
                         std::optional<uint8_t>  shield_color,
                         std::optional<uint16_t> disguise);
    character_appearance(const character_appearance& right);
    ~character_appearance() = default;

public:
    void serialize(fb::stream_writer<big_endian>& writer) const override;
    void to_lua(fb::lua::context* lua) const override;
};

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

class appearance_factory
{
public:
    static std::unique_ptr<appearance> create(const fb::model::object& obj);
    static std::unique_ptr<appearance> create(const fb::game::object& obj);

private:
    appearance_factory() = default;
};

} // namespace fb::game

#endif
