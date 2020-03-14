#include "lua.h"
using namespace fb::game::lua;


main* main::_instance;

state::state(lua_State* lua) : _lua(lua)
{
}

state::~state()
{
}

state::operator lua_State* () const
{
	return this->_lua;
}

thread::thread() : 
	state(lua_newthread(main::get())),
	_ref(luaL_ref(main::get(), LUA_REGISTRYINDEX))
{ }

thread::~thread()
{
	luaL_unref(main::get(), LUA_REGISTRYINDEX, this->_ref);
}

main::main() : state(lua_open())
{
	luaL_openlibs(*this);
}

main::~main()
{
	lua_close(*this);
}

main& main::get()
{
	if(main::_instance == nullptr)
		main::_instance = new main();

	return *_instance;
}

void main::release()
{
	if(main::_instance == nullptr)
		return;
	
	delete main::_instance;
	main::_instance = nullptr;
}

void fb::game::lua::release()
{
	main::release();
}