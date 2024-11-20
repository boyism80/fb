#ifndef __MMO_TYPE_H__
#define __MMO_TYPE_H__

#include <stdint.h>
#include <fb/socket.h>

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