#ifndef __PROTOCOL_REQUEST_GAME_SPELL_H__
#define __PROTOCOL_REQUEST_GAME_SPELL_H__

#include <protocol/protocol.h>
#include <model/spell/spell.h>

namespace fb { namespace protocol { namespace request { namespace game { namespace spell {

class use : public fb::protocol::base::request
{
private:
    mutable fb::istream             _in_stream;

public:
    mutable uint8_t                 slot;
    mutable std::string             message;
    mutable uint32_t                fd;
    mutable point16_t               position;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->_in_stream = in_stream;
        this->slot = in_stream.read_u8();
    }

    void parse(fb::game::spell::types type) const
    {
        switch(type)
        {
        case fb::game::spell::types::INPUT:
        {
            auto size = strlen((const char*)this->_in_stream.data());
            auto raw = new char[size + 1];
            this->_in_stream.read(raw, size);
            raw[size] = 0x00;
            this->message = raw;
            delete[] raw;
            break;
        }

        case fb::game::spell::types::TARGET:
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