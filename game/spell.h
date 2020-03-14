#ifndef __SPELL_H__
#define __SPELL_H__

#include "mmo.h"
#include "stream.h"
#include "container.h"

namespace fb { namespace game {

class spell
{
public:
    static const uint32_t   MAX_SLOT = 52;

private:
    uint8_t                 _type;
    std::string             _name;
    std::string             _cast, _uncast, _concast;
    std::string             _message;

public:
    spell(uint8_t type, const std::string& name, const std::string& cast, const std::string& uncast, const std::string& concast, const std::string& message);
    ~spell();

public:
    uint8_t                 type() const;
    const std::string&      name() const;
    const std::string&      cast() const;
    const std::string&      uncast() const;
    const std::string&      concast() const;
    const std::string&      message() const;

public:
    ostream                 make_show_stream(uint8_t slot) const;
    static ostream          make_delete_stream(uint8_t slot);
    static fb::ostream      make_buff_stream(const std::string& message, uint32_t time);
};


class spells : public container<fb::game::spell>
{
public:
	spells(session& owner);
	~spells();
};

} }

#endif // !__SPELL_H__
