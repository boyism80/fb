#ifndef __MMO_TYPE_H__
#define __MMO_TYPE_H__

#include <stdint.h>
#include <string>
#include <vector>

#define DECLARE_EXCEPTION(name, msg)    class name : public std::runtime_error\
                                        {\
                                        public:\
                                            name() : std::runtime_error(msg) {}\
                                        };

namespace fb { namespace game {

enum nation    : uint8_t { GOGURYEO = 0x01, BUYEO = 0x02 };
enum creature  : uint8_t { PHOENIX = 0x00, TIGER = 0x01, TURTLE = 0x02, DRAGON = 0x03 };
enum direction : uint8_t { TOP = 0x00, RIGHT = 0x01, BOTTOM = 0x02, LEFT = 0x03 };
enum condition : uint8_t { NONE = 0x00, MOVE = 0x01, SIGHT = 0x02, HEAR = 0x04, ORAL = 0x08, MAP = 0x10 };
enum sex       : uint8_t { MAN = 0x00, WOMAN = 0x01, BOTH = 0x02};
enum action    : uint8_t { ATTACK = 0x01, EMOTION = 0x0B, PICKUP = 0x04, PICKUP_SILENT = 0x05, EAT = 0x08, };
enum state     : uint8_t { NORMAL = 0x00, GHOST = 0x01, TRANSLUCENCY = 0x02, RIDING = 0x03, DISGUISE = 0x04, TRANSPARENCY = 0x06, };

namespace message
{
    enum type : uint8_t
    {
        MESSAGE_STATE = 0x03,
        MESSAGE_TRADE = 0x08,
    };
}

enum options : uint8_t
{
    RIDE                    = 0x00,
    WHISPER                 = 0x01,
    GROUP                   = 0x02,
    ROAR                    = 0x03,
    ROAR_WORLDS             = 0x04,
    MAGIC_EFFECT            = 0x05,
    WEATHER_EFFECT          = 0x06,
    FIXED_MOVE              = 0x07,
    TRADE                   = 0x08,
    FAST_MOVE               = 0x09,
    EFFECT_SOUND            = 0x0A,
    PK                      = 0x0B,
};

enum action_sounds : uint16_t
{
    SOUND_EAT               = 0x0006,
    SOUND_EQUIPMENT_OFF     = 0x019A,
    SOUND_EQUIPMENT_ON      = 0x019B,
};

enum duration : uint8_t
{
    DURATION_EAT            = 0x27,
    DURATION_ATTACK         = 0x14,
    DURATION_PICKUP         = 0x32,
    DURATION_EMOTION        = 0x4E,
};

enum state_level : uint8_t
{
    BASED = 0x40, HP_MP = 0x20, EXP_MONEY = 0x10, CONDITION = 0x08,
    LEVEL_MAX = BASED | HP_MP | EXP_MONEY | CONDITION,
    LEVEL_MIDDLE = HP_MP | EXP_MONEY | CONDITION,
    LEVEL_MIN = EXP_MONEY | CONDITION,
};

typedef struct _legend
{
    uint8_t                 look;
    uint8_t                 color;
    std::string             content;

public:
    _legend() : look(0), color(0) {}
    _legend(uint8_t look, uint8_t color, const std::string& content) : look(look), color(color), content(content) {}
    _legend(const _legend& right) : _legend(right.look, right.color, right.content) {}
} legend;


typedef struct _defensive
{
public:
    int16_t physical, magical;

public:
    _defensive() : _defensive(100, 0) {}
    _defensive(int16_t physical, int16_t magical) : physical(physical), magical(magical) {}
} defensive;

typedef struct _point8
{
public:
    uint8_t x, y;

public:
    _point8() { x = y = 0; }
    _point8(uint8_t x, uint8_t y) { this->x = x; this->y = y; }

} point8_t;




typedef struct _size8
{
public:
    uint8_t width, height;

public:
    _size8() { width = height = 0; }
    _size8(uint8_t width, uint8_t height) { this->width = width; this->height = height; }

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
    _size16() { width = height = 0; }
    _size16(uint16_t width, uint16_t height) { this->width = width; this->height = height; }

public:
    bool empty() { return this->width == 0 || this->height == 0; }

} size16_t;



typedef struct _mutable_uint8_t
{
public:
    uint8_t core, current;

public:
    _mutable_uint8_t(uint8_t value) { this->core = this->current = value; }

} mutable_uint8_t;


typedef struct _mutable_uint16_t
{
public:
    uint16_t core, current;

public:
    _mutable_uint16_t(uint16_t value) { this->core = this->current = value; }
    _mutable_uint16_t(uint16_t core, uint16_t current) { this->core = core; this->current = current; }

} mutable_uint16_t;


typedef struct _mutable_uint32_t
{
public:
    uint32_t core, current;

public:
    _mutable_uint32_t(uint32_t value) { this->core = this->current = value; }
    _mutable_uint32_t(uint32_t core, uint32_t current) { this->core = core; this->current = current; }

} mutable_uint32_t;

typedef struct _range8_t
{
public:
    uint8_t min, max;

public:
    _range8_t(uint8_t min, uint8_t max) { this->min = min; this->max = max; }
} range8_t;


typedef struct _range16_t
{
public:
    uint16_t min, max;

public:
    _range16_t(uint16_t min, uint16_t max) { this->min = min; this->max = max; }
} range16_t;


typedef struct _range32_t
{
public:
    uint32_t min, max;

public:
    _range32_t(uint32_t min, uint32_t max) { this->min = min; this->max = max; }
} range32_t;

typedef struct _level_ability
{
public:
    uint8_t strength, intelligence, dexteritry;
    uint32_t base_hp, base_mp, exp;

public:
    _level_ability(uint8_t strength, uint8_t intelligence, uint8_t dexteritry, uint32_t base_hp, uint32_t base_mp, uint32_t exp);
    ~_level_ability();

} level_ability;

class class_data
{
public:
    std::vector<std::string> promotions;
    std::vector<level_ability*> level_abilities;

public:
    class_data();
    ~class_data();

public:
    void add_level_ability(level_ability* data);
    void add_promotion(const std::string& name);
};

} }

#endif // !__MMO_TYPE_H__