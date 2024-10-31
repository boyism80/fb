#ifndef __MMO_TYPE_H__
#define __MMO_TYPE_H__

#include <stdint.h>
#include <string>
#include <vector>
#include <type_traits>
#include <fb/socket.h>
#include <fb/stream.h>

#ifndef interface
#define interface struct
#endif

/**
 * @brief      { function_description }
 *
 * @param      name  The name
 * @param      msg   The message
 *
 * @return     { description_of_the_return_value }
 */
#define DECLARE_EXCEPTION(name, msg)       \
    class name : public std::runtime_error \
    {                                      \
    public:                                \
        name() :                           \
            std::runtime_error(msg)        \
        { }                                \
    };

/**
 * @brief      Bitwise 'or' operator.
 *
 * @param[in]  flag1      The flag 1
 * @param[in]  flag2      The flag 2
 *
 * @tparam     T          { description }
 * @tparam     <unnamed>  { description }
 *
 * @return     The result of the bitwise 'or'
 */
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator| (T flag1, T flag2)
{
    return static_cast<T>(static_cast<int>(flag1) | static_cast<int>(flag2));
}

/**
 * @brief      Bitwise 'or' assignment operator.
 *
 * @param      flag1      The flag 1
 * @param[in]  flag2      The flag 2
 *
 * @tparam     T          { description }
 * @tparam     <unnamed>  { description }
 *
 * @return     The result of the bitwise 'or' assignment
 */
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator|= (T& flag1, const T& flag2)
{
    flag1 = static_cast<T>(static_cast<int>(flag1) | static_cast<int>(flag2));
    return flag1;
}

/**
 * @brief      Logical 'and' operator.
 *
 * @param[in]  flag1      The flag 1
 * @param[in]  flag2      The flag 2
 *
 * @tparam     T1         { description }
 * @tparam     T2         { description }
 * @tparam     <unnamed>  { description }
 * @tparam     <unnamed>  { description }
 *
 * @return     The result of the logical 'and'
 */
template <typename T1,
          typename T2,
          typename = typename std::enable_if<std::is_enum<T1>::value, T1>::type,
          typename = typename std::enable_if<std::is_enum<T2>::value, T2>::type>
constexpr bool operator&& (const T1& flag1, const T2& flag2)
{
    return static_cast<int>(flag1) && static_cast<int>(flag2);
}

/**
 * @brief      Bitwise 'and' operator.
 *
 * @param[in]  flag1      The flag 1
 * @param[in]  flag2      The flag 2
 *
 * @tparam     T          { description }
 * @tparam     <unnamed>  { description }
 *
 * @return     The result of the bitwise 'and'
 */
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator& (T flag1, T flag2)
{
    return static_cast<T>(static_cast<int>(flag1) & static_cast<int>(flag2));
}

/**
 * @brief      Bitwise 'and' assignment operator.
 *
 * @param      flag1      The flag 1
 * @param[in]  flag2      The flag 2
 *
 * @tparam     T          { description }
 * @tparam     <unnamed>  { description }
 *
 * @return     The result of the bitwise 'and' assignment
 */
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator&= (T& flag1, const T& flag2)
{
    flag1 = static_cast<T>(static_cast<int>(flag1) & static_cast<int>(flag2));
    return flag1;
}

/**
 * @brief      Bitwise 'one's complement' operator.
 *
 * @param      flag       The flag
 *
 * @tparam     T          { description }
 * @tparam     <unnamed>  { description }
 *
 * @return     The result of the bitwise 'one's complement'
 */
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator~(T& flag)
{
    flag = static_cast<T>(~static_cast<int>(flag));
    return flag;
}

/**
 * @brief      { function_description }
 *
 * @param[in]  src        The source
 * @param[in]  value      The value
 *
 * @tparam     T          { description }
 * @tparam     <unnamed>  { description }
 *
 * @return     { description_of_the_return_value }
 */
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
bool enum_in(T src, T value)
{
    return (static_cast<int>(src) & static_cast<int>(value)) == static_cast<int>(value);
}

namespace fb { namespace game {

/**
 * @brief      { struct_description }
 */
struct legend
{
public:
    const uint8_t     look;
    const uint8_t     color;
    const std::string content;

public:
    /**
     * @brief      Constructs a new instance.
     */
    legend() :
        look(0),
        color(0)
    { }
    legend(uint8_t look, uint8_t color, const std::string& content) :
        look(look),
        color(color),
        content(content)
    { }
    legend(const legend& right) :
        legend(right.look, right.color, right.content)
    { }
};

/**
 * @brief      This class describes a legend container.
 */
class legend_container : private std::vector<legend>
{
public:
    using std::vector<legend>::begin;
    using std::vector<legend>::end;
    using std::vector<legend>::size;

public:
    /**
     * @brief      Constructs a new instance.
     */
    legend_container();
    /**
     * @brief      Destroys the object.
     */
    ~legend_container();

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  legend  The legend
     *
     * @return     { description_of_the_return_value }
     */
    legend_container& push(const legend& legend);
    /**
     * @brief      { function_description }
     *
     * @param[in]  look     The look
     * @param[in]  color    The color
     * @param[in]  content  The content
     *
     * @return     { description_of_the_return_value }
     */
    legend_container& push(uint8_t look, uint8_t color, const std::string& content);
};

/**
 * @brief      { struct_description }
 */
struct defensive
{
public:
    int16_t physical, magical;

public:
    /**
     * @brief      Constructs a new instance.
     */
    defensive() :
        /**
         * @brief      Constructs a new instance.
         *
         * @param[in]  <unnamed>  { parameter_description }
         * @param[in]  <unnamed>  { parameter_description }
         */
        defensive(100, 0)
    { }
    defensive(int16_t physical, int16_t magical) :
        physical(physical),
        magical(magical)
    { }
};

/**
 * @brief      { struct_description }
 */
struct ability
{
public:
    uint8_t  strength, intelligence, dexteritry;
    uint32_t base_hp, base_mp, exp;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  strength      The strength
     * @param[in]  intelligence  The intelligence
     * @param[in]  dexteritry    The dexteritry
     * @param[in]  base_hp       The base hp
     * @param[in]  base_mp       The base mp
     * @param[in]  exp           The exponent
     */
    ability(uint8_t  strength,
            uint8_t  intelligence,
            uint8_t  dexteritry,
            uint32_t base_hp,
            uint32_t base_mp,
            uint32_t exp);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The right
     */
    ability(const ability& right);
    /**
     * @brief      Destroys the object.
     */
    ~ability();
};

/**
 * @brief      This class describes a class data.
 */
class class_data
{
public:
    std::vector<std::string>              promotions;
    std::vector<std::unique_ptr<ability>> abilities;

public:
    class_data();
    /**
     * @brief      Destroys the object.
     */
    ~class_data();

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  ability  The ability
     */
    void push(const ability& ability);
    /**
     * @brief      { function_description }
     *
     * @param[in]  name  The name
     */
    void push(const std::string& name);
};

}} // namespace fb::game

#endif // !__MMO_TYPE_H__