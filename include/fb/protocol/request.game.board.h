#ifndef __PROTOCOL_RESPONSE_BOARD_H__
#define __PROTOCOL_RESPONSE_BOARD_H__

#include <fb/protocol/protocol.h>

namespace fb { namespace protocol { namespace game { namespace request { namespace board {

class board : public fb::protocol::base::header
{
public:
    uint8_t                 action;
    uint16_t                section;
    uint16_t                article;
    uint16_t                offset;
    std::string             title;
    std::string             contents;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->action = in_stream.read_u8();

        switch(this->action)
        {
        case 0x02:
            this->section = in_stream.read_u16();
            this->offset = in_stream.read_u16();
            break;

        case 0x03:
            this->section = in_stream.read_u16();
            this->article = in_stream.read_u16();
            break;

        case 0x04:
            this->section = in_stream.read_u16();
            this->title = in_stream.readstr_u8();
            this->contents = in_stream.readstr_u16();
            break;

        case 0x05:
            this->section = in_stream.read_u16();
            this->article = in_stream.read_u16();
            break;
        }
    }
};

} } } } }

#endif // !__PROTOCOL_RESPONSE_BOARD_H__