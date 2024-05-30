#ifndef __MMO_TYPE_H__
#define __MMO_TYPE_H__

#include <stdint.h>
#include <string>
#include <vector>
#include <type_traits>
#include <fb/core/socket.h>
#include <fb/core/stream.h>
#include <fb/core/type.h>

#ifndef interface
#define interface struct
#endif

#define DECLARE_EXCEPTION(name, msg)    class name : public std::runtime_error\
                                        {\
                                        public:\
                                            name() : std::runtime_error(msg) { }\
                                        };

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator | (T flag1, T flag2)
{
    return static_cast<T>(static_cast<int>(flag1) | static_cast<int>(flag2));
}

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator |= (T& flag1, const T& flag2)
{
    flag1 = static_cast<T>(static_cast<int>(flag1) | static_cast<int>(flag2));
    return flag1;
}

template <typename T1, typename T2, typename = typename std::enable_if<std::is_enum<T1>::value, T1>::type, typename = typename std::enable_if<std::is_enum<T2>::value, T2>::type>
constexpr bool operator && (const T1& flag1, const T2& flag2)
{
    return static_cast<int>(flag1) && static_cast<int>(flag2);
}

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator & (T flag1, T flag2)
{
    return static_cast<T>(static_cast<int>(flag1) & static_cast<int>(flag2));
}

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator &= (T& flag1, const T& flag2)
{
    flag1 = static_cast<T>(static_cast<int>(flag1) & static_cast<int>(flag2));
    return flag1;
}

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator ~ (T& flag)
{
    flag = static_cast<T>(~static_cast<int>(flag));
    return flag;
}

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
bool enum_in(T src, T value)
{
    return (static_cast<int>(src) & static_cast<int>(value)) == static_cast<int>(value);
}


namespace fb { namespace game {

enum class TIMER_TYPE : uint8_t
{
    INCREASE            = 0x01,
    DECREASE            = 0x02,
};

enum class WEATHER_TYPE : uint8_t
{
    NORMAL              = 0x00,
    RAIN                = 0x01,
    SNOW                = 0x02,
    BIRD                = 0x03,
};

enum class CHAT_TYPE : uint8_t
{
    NORMAL              = 0x00,
    SHOUT               = 0x01,
    BLUE                = 0x02,
    LIGHT_BLUE          = 0x03,
};

enum class SWAP_TYPE : uint8_t
{
    ITEM                = 0x00,
    SPELL               = 0x01,
};

enum class MESSAGE_TYPE : uint8_t
{
    NOTIFY              = 0x00,
    BLUE                = 0x01,
    STATE               = 0x03,
    SHOUT               = 0x04,
    WORLD               = 0x05,
    POPUP               = 0x08,
    YELLOW              = 0x0B,
    BROWN               = 0x0C,
};

enum class OPTION : uint8_t
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

enum class SOUND_TYPE : uint16_t
{
    SWING               = 0x014B,
    EAT                 = 0x0006,
    EQUIPMENT_OFF       = 0x019A,
    EQUIPMENT_ON        = 0x019B,
    DAMAGE              = 0x015D,
};

enum class DURATION : uint8_t
{
    FAST           = 0x0F,
    EAT            = 0x27,
    ATTACK         = 0x14,
    PICKUP         = 0x32,
    EMOTION        = 0x4E,
    SPELL          = 0x20,
    THROW          = 0x28,
};

enum class STATE_LEVEL : uint8_t
{
    BASED = 0x40, HP_MP = 0x20, EXP_MONEY = 0x10, CONDITION = 0x08,
    LEVEL_MAX = BASED | HP_MP | EXP_MONEY | CONDITION,
    LEVEL_MIDDLE = HP_MP | EXP_MONEY | CONDITION,
    LEVEL_MIN = EXP_MONEY | CONDITION,
};

struct legend
{
public:
    const uint8_t           look;
    const uint8_t           color;
    const std::string       content;

public:
    legend() : look(0), color(0) { }
    legend(uint8_t look, uint8_t color, const std::string& content) : look(look), color(color), content(content) { }
    legend(const legend& right) : legend(right.look, right.color, right.content) { }
};

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

struct defensive
{
public:
    int16_t physical, magical;

public:
    defensive() : defensive(100, 0) { }
    defensive(int16_t physical, int16_t magical) : physical(physical), magical(magical) { }
};

struct ability
{
public:
    uint8_t strength, intelligence, dexteritry;
    uint32_t base_hp, base_mp, exp;

public:
    ability(uint8_t strength, uint8_t intelligence, uint8_t dexteritry, uint32_t base_hp, uint32_t base_mp, uint32_t exp);
    ability(const ability& right);
    ~ability();

};

class class_data
{
public:
    std::vector<std::string> promotions;
    std::vector<std::unique_ptr<ability>> abilities;

public:
    class_data();
    ~class_data();

public:
    void push(const ability& ability);
    void push(const std::string& name);
};

} }

#endif // !__MMO_TYPE_H__