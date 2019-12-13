#ifndef __NPC_H__
#define	__NPC_H__

#include "object.h"

namespace fb { namespace game {

class npc : public object
{
private:
	std::string				_script;

public:
	npc(const std::string& name, uint16_t look, uint8_t color);
	npc(const npc& right);
	~npc();

public:
	const std::string&		script() const;
	void					script(const std::string& value);

public:
	npc*					make();
	object::types			type() const { return object::types::NPC; }
};


} }

#endif // !__NPC_H__
