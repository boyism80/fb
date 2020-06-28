#ifndef __BUILTIN_FUNCTION_H__
#define __BUILTIN_FUNCTION_H__

#include "model/mmo/mmo.h"
#include "model/lua/lua.h"

template <typename T>
void to_lua(lua_State* lua, const T* self);

template <typename T>
int builtin_dialog(lua_State* lua);

template <typename T>
int builtin_menu_dialog(lua_State* lua);

template <typename T>
int builtin_item_dialog(lua_State* lua);

template <typename T>
int builtin_input_dialog(lua_State* lua);

#include "builtin_function.hpp"

#endif // !__BUILTIN_FUNCTION_H__
