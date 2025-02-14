#include <fb/game/protocol/spell/spell_cast.h>

namespace fb::protocol::game::request {

async::task<void> spell_cast::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->slot = reader.read<uint8_t>() - 1;

    auto remained = reader.readable_size();
    auto buffer   = new uint8_t[remained];
    reader.read(buffer, remained);

    auto writer = fb::stream_writer(this->buffer);
    writer.write(buffer, remained);
    delete[] buffer;
}

void spell_cast::parse(SPELL_TYPE type)
{
    auto reader = fb::stream_reader<big_endian>(this->buffer);
    switch (type)
    {
    case SPELL_TYPE::INPUT:
    {
#ifdef _WIN32
        this->message = (const char*)this->buffer.data();
#else
        this->message = utf8((const char*)this->buffer.data());
#endif
    }
    break;

    case SPELL_TYPE::TARGET:
    {
        this->fd         = reader.read<uint32_t>();
        this->position.x = reader.read<uint16_t>();
        this->position.y = reader.read<uint16_t>();
    }
    break;
    }
}
} // namespace fb::protocol::game::request
