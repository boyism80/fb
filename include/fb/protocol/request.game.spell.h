#ifndef __PROTOCOL_REQUEST_GAME_SPELL_H__
#define __PROTOCOL_REQUEST_GAME_SPELL_H__

#include <fb/protocol/protocol.h>
#include <fb/game/spell.h>

namespace fb { namespace protocol { namespace game { namespace request { namespace spell {

class use : public fb::protocol::base::header
{
private:
    mutable fb::istream             _in_stream;

public:
    mutable uint8_t                 slot;
    mutable std::string             message;
    mutable uint32_t                fd;
    mutable point<uint16_t>               position;

public:
    use() : fb::protocol::base::header(0x0F)
    { }

public:
    void deserialize(fb::istream& in_stream)
    {
        this->_in_stream = in_stream;
        this->slot = this->_in_stream.read_u8() - 1;
    }

    void parse(SPELL_TYPE type) const
    {
        switch(type)
        {
        case SPELL_TYPE::INPUT:
        {
            this->message = _in_stream.readstr();
            break;
        }

        case SPELL_TYPE::TARGET:
        {
            this->fd = this->_in_stream.read_u32();
            this->position.x = this->_in_stream.read_u16();
            this->position.y = this->_in_stream.read_u16();
            break;
        }
        }
    }
};

} } } } }

#endif // !__PROTOCOL_REQUEST_GAME_SPELL_H__