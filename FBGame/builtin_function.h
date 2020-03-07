#ifndef __BUILTIN_FUNCTION_H__
#define __BUILTIN_FUNCTION_H__

#include "mmo.h"
#include "lua.h"

template <typename T>
int builtin_dialog(lua_State* lua);

template <typename T>
int builtin_input_dialog_ext(lua_State* lua);

template <typename T>
int builtin_menu_dialog(lua_State* lua);

template <typename T>
int builtin_item_dialog(lua_State* lua);

template <typename T>
int builtin_input_dialog(lua_State* lua);

#include "builtin_function.hpp"

#endif // !__BUILTIN_FUNCTION_H__
