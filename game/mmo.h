#ifndef __MMO_TYPE_H__
#define __MMO_TYPE_H__

#include <stdint.h>
#include <string>
#include <vector>
#include "socket.h"
#include "stream.h"

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
enum action    : uint8_t { ATTACK = 0x01, EMOTION = 0x0B, PICKUP = 0x04, PICKUP_SILENT = 0x05, CAST_SPELL = 0x06, EAT = 0x08, };
enum state     : uint8_t { NORMAL = 0x00, GHOST = 0x01, TRANSLUCENCY = 0x02, RIDING = 0x03, DISGUISE = 0x04, HALF_CLOACK = 0x05, CLOACK = 0x06, };

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

    fb::ostream             make_stream(uint32_t time, type type = timer::type::DECREASE);
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

    fb::ostream             make_stream(type weather);
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
        BLUE        = 0x01,
        STATE       = 0x03,
        SHOUT       = 0x04,
        WORLD       = 0x05,
        POPUP       = 0x08,
        YELLOW      = 0x0B,
        BROWN       = 0x0C,
    };

    namespace exception
    {
        static const std::string RIDDING = "말을 타고는 할 수 없습니다.";
        static const std::string GHOST = "귀신은 할 수 없습니다.";
        static const std::string REQUIRE_CLASS = "직업을 선택해야 합니다.";
        static const std::string NO_CONVEYANCE = "탈 것이 없습니다.";
        static const std::string DISGUISE = "변신 중에는 할 수 없습니다.";
        static const std::string CANNOT_DROP_ITEM = "버릴 수 없는 물건입니다.";
        static const std::string CANNOT_THROW_ITEM = "던질 수 없는 물건입니다.";
        static const std::string INVENTORY_OVERFLOW = "소지품이 꽉 찼습니다.";
        static const std::string INVALID_TARGET = "대상이 올바르지 않습니다.";
    };

    namespace trade
    {
        static const std::string SUCCESS = "교환에 성공했습니다.";
        static const std::string FAILED = "교환에 실패했습니다.";
        static const std::string NOTIFY_LOCK_TO_PARTNER = "상대방이 확인을 눌렀습니다.";
        static const std::string CANCELLED_BY_ME = "내가 교환을 취소했습니다.";
        static const std::string CANCELLED_BY_PARTNER = "상대방이 교환을 취소했습니다.";
        static const std::string INVALID_COUNT = "갯수가 올바르지 않습니다.";
        static const std::string NOT_ALLOWED_TO_TRADE = "교환이 불가능한 아이템입니다.";
        static const std::string REFUSED_BY_ME = "교환 거부 상태입니다.";
        static const std::string REFUSED_BY_PARTNER = "님은 교환 거부 상태입니다.";
        static const std::string PARTNER_ALREADY_TRADING = "님은 이미 교환 중입니다.";
        static const std::string PARTNER_INVISIBLE = "대상이 보이지 않습니다.";
        static const std::string PARTNER_TOO_FAR = "님과 너무 멀리 떨어져 있습니다.";
    }

    namespace mix
    {
        static const std::string SUCCESS = "성공하였습니다.";
        static const std::string FAILED = "실패하였습니다.";
    }

    namespace ride
    {
        static const std::string ON = "말에 탔습니다.";
        static const std::string OFF = "말에서 내렸습니다.";
    }

    namespace money
    {
        static const std::string DROP = "돈을 버렸습니다.";
        static const std::string FULL = "더 이상 돈을 가질 수 없습니다.";
    }

    namespace level
    {
        static const std::string UP = "레벨이 올랐습니다";
    }

    namespace board
    {
        static const std::string ARTICLE_NOT_EXIST = "게시글이 존재하지 않습니다.";
        static const std::string SECTION_NOT_EXIST = "섹션이 존재하지 않습니다.";
        static const std::string NOT_AUTH = "권한이 없습니다.";
    }


    fb::ostream             make_stream(const std::string& message, message::type types);
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
    DURATION_EAT            = 0x27,
    DURATION_ATTACK         = 0x14,
    DURATION_PICKUP         = 0x32,
    DURATION_EMOTION        = 0x4E,
    DURATION_SPELL          = 0x20,
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