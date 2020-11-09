#ifndef __TYPE_H__
#define __TYPE_H__

namespace fb { namespace game {

enum nation    : uint8_t { GOGURYEO = 0x01, BUYEO = 0x02 };
enum creature  : uint8_t { PHOENIX = 0x00, TIGER = 0x01, TURTLE = 0x02, DRAGON = 0x03 };
enum direction : uint8_t { TOP = 0x00, RIGHT = 0x01, BOTTOM = 0x02, LEFT = 0x03 };
enum condition : uint8_t { NONE = 0x00, MOVE = 0x01, SIGHT = 0x02, HEAR = 0x04, ORAL = 0x08, MAP = 0x10 };
enum sex       : uint8_t { MAN = 0x00, WOMAN = 0x01, BOTH = 0x02};
enum action    : uint8_t { ATTACK = 0x01, ARROW = 0x03, EMOTION = 0x0B, PICKUP = 0x04, PICKUP_SILENT = 0x05, CAST_SPELL = 0x06, EAT = 0x08, };
enum state     : uint8_t { NORMAL = 0x00, GHOST = 0x01, TRANSLUCENCY = 0x02, RIDING = 0x03, DISGUISE = 0x04, HALF_CLOACK = 0x05, CLOACK = 0x06, };

typedef struct _point8
{
public:
    uint8_t x, y;

public:
    _point8() : x(0), y(0) 
    {}
    _point8(uint8_t x, uint8_t y) : x(x), y(y)
    {}

} point8_t;

typedef struct _size8
{
public:
    uint8_t width, height;

public:
    _size8() : width(0), height(0)
    {}
    _size8(uint8_t width, uint8_t height) : width(width), height(height)
    {}

public:
    bool empty() { return this->width == 0 || this->height == 0; }

} size8_t;

typedef struct _point16
{
public:
    uint16_t x, y;

public:
    _point16() { x = y = 0; }
    _point16(uint16_t x, uint16_t y) { this->x = x; this->y = y; }

public:
    struct _point16& move(direction direction, uint16_t step = 1)
    {
        switch(direction)
        {
        case fb::game::direction::TOP:
            this->y -= step;
            break;

        case fb::game::direction::BOTTOM:
            this->y += step;
            break;

        case fb::game::direction::LEFT:
            this->x -= step;
            break;

        case fb::game::direction::RIGHT:
            this->x += step;
            break;
        }

        return *this;
    }

    struct _point16& forward(direction direction, uint16_t step = 1)
    {
        return this->move(direction, step);
    }

    struct _point16& backward(direction direction, uint16_t step = 1)
    {
        auto newdir = direction;
        switch(direction)
        {
        case fb::game::direction::TOP:
            newdir = fb::game::direction::BOTTOM;
            break;

        case fb::game::direction::BOTTOM:
            newdir = fb::game::direction::TOP;
            break;

        case fb::game::direction::LEFT:
            newdir = fb::game::direction::RIGHT;
            break;

        case fb::game::direction::RIGHT:
            newdir = fb::game::direction::LEFT;
            break;
        }

        return this->move(newdir, step);
    }

    struct _point16& left(direction direction, uint16_t step = 1)
    {
        auto newdir = direction;
        switch(direction)
        {
        case fb::game::direction::TOP:
            newdir = fb::game::direction::LEFT;
            break;

        case fb::game::direction::BOTTOM:
            newdir = fb::game::direction::RIGHT;
            break;

        case fb::game::direction::LEFT:
            newdir = fb::game::direction::BOTTOM;
            break;

        case fb::game::direction::RIGHT:
            newdir = fb::game::direction::TOP;
            break;
        }

        return this->move(newdir, step);
    }

    struct _point16& right(direction direction, uint16_t step = 1)
    {
        auto newdir = direction;
        switch(direction)
        {
        case fb::game::direction::TOP:
            newdir = fb::game::direction::RIGHT;
            break;

        case fb::game::direction::BOTTOM:
            newdir = fb::game::direction::LEFT;
            break;

        case fb::game::direction::LEFT:
            newdir = fb::game::direction::TOP;
            break;

        case fb::game::direction::RIGHT:
            newdir = fb::game::direction::BOTTOM;
            break;
        }

        return this->move(newdir, step);
    }

public:
    bool operator == (const struct _point16& right) const
    {
        return (this->x == right.x) && (this->y == right.y);
    }

    bool operator != (const struct _point16& right) const
    {
        return !(this->operator==(right));
    }
} point16_t;

typedef struct _size16
{
public:
    uint16_t width, height;

public:
    _size16() : width(width), height(height)
    {}
    _size16(uint16_t width, uint16_t height) : width(width), height(height)
    {}
    _size16(const struct _size16& right) : width(right.width), height(right.height)
    {}

public:
    bool empty() { return this->width == 0 || this->height == 0; }

} size16_t;

typedef struct _mutable_uint8_t
{
public:
    uint8_t master, current;

public:
    _mutable_uint8_t(uint8_t value) { this->master = this->current = value; }

} mutable_uint8_t;

typedef struct _mutable_uint16_t
{
public:
    uint16_t master, current;

public:
    _mutable_uint16_t(uint16_t value) : master(value), current(value) 
    {}
    _mutable_uint16_t(uint16_t master, uint16_t current) : master(master), current(current)
    {}

} mutable_uint16_t;

typedef struct _mutable_uint32_t
{
public:
    uint32_t master, current;

public:
    _mutable_uint32_t(uint32_t value) : master(value), current(value)
    {}
    _mutable_uint32_t(uint32_t master, uint32_t current) : master(master), current(current)
    {}

} mutable_uint32_t;

typedef struct _range8_t
{
public:
    uint8_t min, max;

public:
    _range8_t(uint8_t min, uint8_t max) : min(min), max(max)
    {}
} range8_t;

typedef struct _range16_t
{
public:
    uint16_t min, max;

public:
    _range16_t(uint16_t min, uint16_t max) : min(min), max(max)
    {}
} range16_t;

typedef struct _range32_t
{
public:
    uint32_t min, max;

public:
    _range32_t(uint32_t min, uint32_t max) : min(min), max(max)
    {}
} range32_t;

} }

#endif // !__TYPE_H__