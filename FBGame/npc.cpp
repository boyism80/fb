#include "npc.h"

fb::game::npc::npc(const std::string& name, uint16_t look, uint8_t color) : 
	object(0xFFFFFFFF, name, look, color)
{
}

fb::game::npc::npc(const npc& right) : 
	object(right),
	_script(right._script)
{
}

fb::game::npc::~npc()
{
}

const std::string& fb::game::npc::script() const
{
	return this->_script;
}

void fb::game::npc::script(const std::string& value)
{
	this->_script = value;
}

fb::game::npc* fb::game::npc::make()
{
	return new npc(*this);
}
