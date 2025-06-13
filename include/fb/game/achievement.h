#ifndef __TRACE_H__
#define __TRACE_H__

#include <fb/model/model.h>
#include <fb/lua.h>

namespace fb::game {

class achievement : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    struct builtin;

public:
    const fb::model::achievement&    model;
    const std::optional<std::string> text;

public:
    achievement(const fb::model::achievement& model, const std::optional<std::string>& text = std::nullopt);
    ~achievement() = default;
};

/**
 * @brief      { struct_description }
 */
struct achievement::builtin
{
    /**
     * @brief      { function_description }
     *
     * @param      L     { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_model(lua_State* L);
    /**
     * @brief      { function_description }
     *
     * @param      L     { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_text(lua_State* L);
};

} // namespace fb::game

#endif
