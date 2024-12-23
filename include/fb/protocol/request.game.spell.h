#ifndef __PROTOCOL_REQUEST_GAME_SPELL_H__
#define __PROTOCOL_REQUEST_GAME_SPELL_H__

#include <fb/protocol/protocol.h>
#ifndef BOT
#include <spell.h>
#endif

namespace fb::protocol::game::request::spell {

class use : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x0F;

private:
    fb::stream_reader<big_endian>* _reader;

public:
    mutable uint8_t         slot;
    mutable std::string     message;
    mutable uint32_t        fd;
    mutable point<uint16_t> position;

public:
    use() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->_reader = &reader;
        this->slot    = reader.read<uint8_t>() - 1;
    }

    void parse(SPELL_TYPE type) const
    {
        switch (type)
        {
        case SPELL_TYPE::INPUT:
        {
            this->message = this->_reader->read<std::string, uint8_t>();
            break;
        }

        case SPELL_TYPE::TARGET:
        {
            this->fd         = this->_reader->read<uint32_t>();
            this->position.x = this->_reader->read<uint16_t>();
            this->position.y = this->_reader->read<uint16_t>();
            break;
        }
        }
    }
};

} // namespace fb::protocol::game::request::spell

#endif // !__PROTOCOL_REQUEST_GAME_SPELL_H__