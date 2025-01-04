#include <fb/game/protocol/spell/spell_cast.h>

namespace fb::protocol::game::request {

async::task<void> spell_cast::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->_reader = &reader;
    this->slot    = reader.read<uint8_t>() - 1;
}

void spell_cast::parse(SPELL_TYPE type) const
{
    switch (type)
    {
    case SPELL_TYPE::INPUT:
    {
        this->message = this->_reader->read<std::string, uint8_t>();
    }
    break;

    case SPELL_TYPE::TARGET:
    {
        this->fd         = this->_reader->read<uint32_t>();
        this->position.x = this->_reader->read<uint16_t>();
        this->position.y = this->_reader->read<uint16_t>();
    }
    break;
    }
}
} // namespace fb::protocol::game::request
