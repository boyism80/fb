#ifndef __PORTRAIT_H__
#define __PORTRAIT_H__

#include <fb/model/model.h>
#include <fb/stream_writer.h>

using namespace fb::model::enum_value;

namespace fb::game {

class object;

class portrait
{
protected:
    portrait() = default;

public:
    virtual ~portrait() = default;

public:
    virtual void serialize(fb::stream_writer<big_endian>& writer) const = 0;
};

class character_portrait : public portrait
{
public:
    SEX                     sex          = SEX::MAN;
    STATE                   state        = STATE::NORMAL;
    uint16_t                hair         = 0;
    uint8_t                 hair_color   = 0;
    std::optional<uint16_t> weapon       = std::nullopt;
    std::optional<uint8_t>  weapon_color = std::nullopt;
    std::optional<uint8_t>  armor        = std::nullopt;
    std::optional<uint8_t>  armor_color  = std::nullopt;
    std::optional<uint8_t>  shield       = std::nullopt;
    std::optional<uint8_t>  shield_color = std::nullopt;
    std::optional<uint16_t> disguise     = std::nullopt;

public:
    character_portrait() = default;
    character_portrait(SEX                     sex,
                       STATE                   state,
                       uint16_t                hair,
                       uint8_t                 hair_color,
                       std::optional<uint16_t> weapon,
                       std::optional<uint8_t>  weapon_color,
                       std::optional<uint8_t>  armor,
                       std::optional<uint8_t>  armor_color,
                       std::optional<uint8_t>  shield,
                       std::optional<uint8_t>  shield_color,
                       std::optional<uint16_t> disguise);
    character_portrait(const character_portrait& right);
    ~character_portrait() = default;

public:
    void serialize(fb::stream_writer<big_endian>& writer) const;
};

class object_portrait : public portrait
{
public:
    uint16_t look  = 0;
    uint8_t  color = 0;

public:
    object_portrait() = default;
    object_portrait(uint16_t look, uint8_t color);
    object_portrait(const object_portrait& right);
    ~object_portrait() = default;

public:
    void serialize(fb::stream_writer<big_endian>& writer) const;
};

class portrait_factory
{
public:
    static std::unique_ptr<portrait> create(fb::model::model& model, const fb::model::object& obj);
    static std::unique_ptr<portrait> create(const fb::game::object& obj);

private:
    portrait_factory() = default;
};

} // namespace fb::game

#endif // !__PORTRAIT_H__