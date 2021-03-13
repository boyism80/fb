#ifndef __MMO_TYPE_H__
#define __MMO_TYPE_H__

#include <stdint.h>
#include <string>
#include <vector>
#include "module/socket/socket.h"
#include "module/stream/stream.h"
#include "module/common/type.h"

#define DECLARE_EXCEPTION(name, msg)    class name : public std::runtime_error\
                                        {\
                                        public:\
                                            name() : std::runtime_error(msg) {}\
                                        };

namespace fb { namespace game {

namespace dialog
{
    enum interaction : uint8_t
    {
        NORMAL              = 0x00,
        INPUT               = 0x01,
        INPUT_EX            = 0x02,
        MENU                = 0x04,
        SLOT                = 0x05,
        SELL                = 0x06,
    };
}

namespace timer
{
    enum type : uint8_t
    {
        INCREASE            = 0x01,
        DECREASE            = 0x02,
    };
}

namespace weather
{
    enum type : uint8_t
    {
        NORMAL              = 0x00,
        RAIN                = 0x01,
        SNOW                = 0x02,
        BIRD                = 0x03,
    };
}

namespace chat
{
    enum type : uint8_t
    {
        NORMAL              = 0x00,
        SHOUT               = 0x01,
        BLUE                = 0x02,
        LIGHT_BLUE          = 0x03,
    };
}

namespace swap
{
    enum type : uint8_t
    {
        ITEM                = 0x00,
        SPELL               = 0x01,
    };
}

namespace message
{
    enum type : uint8_t
    {
        NOTIFY      = 0x00,
        BLUE        = 0x01,
        STATE       = 0x03,
        SHOUT       = 0x04,
        WORLD       = 0x05,
        POPUP       = 0x08,
        YELLOW      = 0x0B,
        BROWN       = 0x0C,
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

namespace sound
{
    enum type : uint16_t
    {
        SWING               = 0x014B,
        EAT                 = 0x0006,
        EQUIPMENT_OFF       = 0x019A,
        EQUIPMENT_ON        = 0x019B,
        DAMAGE              = 0x015D,
    };
}

enum duration : uint8_t
{
    DURATION_FAST           = 0x0F,
    DURATION_EAT            = 0x27,
    DURATION_ATTACK         = 0x14,
    DURATION_PICKUP         = 0x32,
    DURATION_EMOTION        = 0x4E,
    DURATION_SPELL          = 0x20,
    DURATION_THROW          = 0x28,
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

class legend_container : private std::vector<legend>
{
public:
    using std::vector<legend>::begin;
    using std::vector<legend>::end;
    using std::vector<legend>::size;

public:
    legend_container();
    ~legend_container();

public:
    legend_container&       push(const legend& legend);
    legend_container&       push(uint8_t look, uint8_t color, const std::string& content);
};

typedef struct _defensive
{
public:
    int16_t physical, magical;

public:
    _defensive() : _defensive(100, 0) {}
    _defensive(int16_t physical, int16_t magical) : physical(physical), magical(magical) {}
} defensive;

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