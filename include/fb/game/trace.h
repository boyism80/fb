#ifndef __TRACE_H__
#define __TRACE_H__

#include <fb/model/model.h>
#include <fb/lua.h>

namespace fb::game {

class trace : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    const fb::model::trace&          model;
    const std::optional<std::string> text;

public:
    trace(const fb::model::trace& model, const std::optional<std::string>& text = std::nullopt);
    ~trace() = default;

public:
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_model(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_text(lua_State* L);
};

} // namespace fb::game

#endif
