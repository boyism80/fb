#ifndef __TYPE_H__
#define __TYPE_H__

namespace fb { namespace game {

enum class NATION_TYPE    : uint8_t { GOGURYEO = 0x01, BUYEO = 0x02 };
enum class CREATURE_TYPE  : uint8_t { PHOENIX = 0x00, TIGER = 0x01, TURTLE = 0x02, DRAGON = 0x03 };
enum class DIRECTION_TYPE : uint8_t { TOP = 0x00, RIGHT = 0x01, BOTTOM = 0x02, LEFT = 0x03 };
enum class CONDITION_TYPE : uint8_t { NONE = 0x00, MOVE = 0x01, SIGHT = 0x02, HEAR = 0x04, ORAL = 0x08, MAP = 0x10 };
enum class SEX_TYPE       : uint8_t { MAN = 0x00, WOMAN = 0x01, BOTH = 0x02};
enum class ACTION_TYPE    : uint8_t { ATTACK = 0x01, ARROW = 0x03, EMOTION = 0x0B, PICKUP = 0x04, PICKUP_SILENT = 0x05, CAST_SPELL = 0x06, EAT = 0x08, };
enum class STATE_TYPE     : uint8_t { NORMAL = 0x00, GHOST = 0x01, TRANSLUCENCY = 0x02, RIDING = 0x03, DISGUISE = 0x04, HALF_CLOACK = 0x05, CLOACK = 0x06, };

struct point8_t
{
public:
    uint8_t x = 0;
    uint8_t y = 0;

public:
    point8_t() = default;
    point8_t(uint8_t x, uint8_t y) : x(x), y(y)
    { }

};

struct size8_t
{
public:
    uint8_t width  = 0;
    uint8_t height = 0;

public:
    size8_t() = default;
    size8_t(uint8_t width, uint8_t height) : width(width), height(height)
    { }

public:
    bool empty() { return this->width == 0 || this->height == 0; }

};

struct point16_t
{
public:
    uint16_t x = 0;
    uint16_t y = 0;

public:
    point16_t() = default;
    point16_t(uint16_t x, uint16_t y) { this->x = x; this->y = y; }

public:
    struct point16_t& move(DIRECTION_TYPE direction, uint16_t step = 1)
    {
        switch(direction)
        {
        case fb::game::DIRECTION_TYPE::TOP:
            this->y -= step;
            break;

        case fb::game::DIRECTION_TYPE::BOTTOM:
            this->y += step;
            break;

        case fb::game::DIRECTION_TYPE::LEFT:
            this->x -= step;
            break;

        case fb::game::DIRECTION_TYPE::RIGHT:
            this->x += step;
            break;
        }

        return *this;
    }

    struct point16_t& forward(DIRECTION_TYPE direction, uint16_t step = 1)
    {
        return this->move(direction, step);
    }

    struct point16_t& backward(DIRECTION_TYPE direction, uint16_t step = 1)
    {
        auto newdir = direction;
        switch(direction)
        {
        case fb::game::DIRECTION_TYPE::TOP:
            newdir = fb::game::DIRECTION_TYPE::BOTTOM;
            break;

        case fb::game::DIRECTION_TYPE::BOTTOM:
            newdir = fb::game::DIRECTION_TYPE::TOP;
            break;

        case fb::game::DIRECTION_TYPE::LEFT:
            newdir = fb::game::DIRECTION_TYPE::RIGHT;
            break;

        case fb::game::DIRECTION_TYPE::RIGHT:
            newdir = fb::game::DIRECTION_TYPE::LEFT;
            break;
        }

        return this->move(newdir, step);
    }

    struct point16_t& left(DIRECTION_TYPE direction, uint16_t step = 1)
    {
        auto newdir = direction;
        switch(direction)
        {
        case fb::game::DIRECTION_TYPE::TOP:
            newdir = fb::game::DIRECTION_TYPE::LEFT;
            break;

        case fb::game::DIRECTION_TYPE::BOTTOM:
            newdir = fb::game::DIRECTION_TYPE::RIGHT;
            break;

        case fb::game::DIRECTION_TYPE::LEFT:
            newdir = fb::game::DIRECTION_TYPE::BOTTOM;
            break;

        case fb::game::DIRECTION_TYPE::RIGHT:
            newdir = fb::game::DIRECTION_TYPE::TOP;
            break;
        }

        return this->move(newdir, step);
    }

    struct point16_t& right(DIRECTION_TYPE direction, uint16_t step = 1)
    {
        auto newdir = direction;
        switch(direction)
        {
        case fb::game::DIRECTION_TYPE::TOP:
            newdir = fb::game::DIRECTION_TYPE::RIGHT;
            break;

        case fb::game::DIRECTION_TYPE::BOTTOM:
            newdir = fb::game::DIRECTION_TYPE::LEFT;
            break;

        case fb::game::DIRECTION_TYPE::LEFT:
            newdir = fb::game::DIRECTION_TYPE::TOP;
            break;

        case fb::game::DIRECTION_TYPE::RIGHT:
            newdir = fb::game::DIRECTION_TYPE::BOTTOM;
            break;
        }

        return this->move(newdir, step);
    }

public:
    bool operator == (const struct point16_t& right) const
    {
        return (this->x == right.x) && (this->y == right.y);
    }

    bool operator != (const struct point16_t& right) const
    {
        return !(this->operator==(right));
    }
};

struct size16_t
{
public:
    uint16_t width  = 0;
    uint16_t height = 0;

public:
    size16_t() = default;
    size16_t(uint16_t width, uint16_t height) : width(width), height(height)
    { }
    size16_t(const struct size16_t& right) : width(right.width), height(right.height)
    { }

public:
    bool empty() { return this->width == 0 || this->height == 0; }

};

struct mutable_uint16_t
{
public:
    uint16_t model  = 0;
    uint16_t current = 0;

public:
    mutable_uint16_t() = default;
    mutable_uint16_t(uint16_t value) : model(value), current(value) 
    { }
    mutable_uint16_t(uint16_t model, uint16_t current) : model(model), current(current)
    { }
};

struct mutable_uint32_t
{
public:
    uint32_t model  = 0;
    uint32_t current = 0;

public:
    mutable_uint32_t() = default;
    mutable_uint32_t(uint32_t value) : model(value), current(value)
    { }
    mutable_uint32_t(uint32_t model, uint32_t current) : model(model), current(current)
    { }
};

struct range8_t
{
public:
    uint8_t min = 0;
    uint8_t max = 0;

public:
    range8_t() = default;
    range8_t(uint8_t min, uint8_t max) : min(min), max(max)
    { }
};

struct range16_t
{
public:
    uint16_t min = 0;
    uint16_t max = 0;

public:
    range16_t() = default;
    range16_t(uint16_t min, uint16_t max) : min(min), max(max)
    { }
};

struct range32_t
{
public:
    uint32_t min = 0;
    uint32_t max = 0;

public:
    range32_t() = default;
    range32_t(uint32_t min, uint32_t max) : min(min), max(max)
    { }
};

} }

#endif // !__TYPE_H__