#ifndef __PROTOCOL_RESPONSE_GAME_SPELL_H__
#define __PROTOCOL_RESPONSE_GAME_SPELL_H__

#include <protocol/protocol.h>
#include <model/spell/spell.h>

namespace fb { namespace protocol { namespace response { namespace game { namespace spell {

class buff : public fb::protocol::base::response
{
public:
    const std::string       name;
    const uint32_t          time;

public:
    buff(const std::string& name, uint32_t time) : 
        name(name), time(time)
    {}
    buff(const fb::game::buff& buff) : 
        name(buff.spell().name()), time(buff.time())
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x3A)
                  .write(this->name)
                  .write_u32(this->time);
    }
};

} } } } }

#endif // !__PROTOCOL_RESPONSE_GAME_SPELL_H__