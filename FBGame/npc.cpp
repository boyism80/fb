#include "npc.h"

fb::game::npc::core::core(const std::string& name, uint16_t look, uint8_t color) : 
    fb::game::object::core(name, look, color)
{}

fb::game::npc::core::~core()
{}

fb::game::object* fb::game::npc::core::make() const
{
    return new npc(this);
}

fb::game::npc::npc(const core* core) : 
    fb::game::object(core)
{
}

fb::game::npc::npc(const npc& right) : 
    object(right)
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

fb::game::object::types fb::game::npc::type() const
{
    return static_cast<const core*>(this->_core)->type();
}
