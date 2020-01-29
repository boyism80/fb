#ifndef __NPC_H__
#define	__NPC_H__

#include "object.h"

namespace fb { namespace game {

class npc : public object
{
public:
	class core : public object::core
	{
		friend class npc;

	public:
		core(uint32_t id, const std::string& name, uint16_t look, uint8_t color = 0);
		~core();

	public:
		object::types			type() const { return object::types::NPC; }
		object*					make() const;
	};

private:
	std::string				_script;

public:
	npc(const core* core);
	npc(const npc& right);
	~npc();

public:
	const std::string&		script() const;
	void					script(const std::string& value);

public:
	npc*					make();
	object::types			type() const;
};


} }

#endif // !__NPC_H__
