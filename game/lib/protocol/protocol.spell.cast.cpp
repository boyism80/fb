#include <fb/game/protocol/spell/spell_cast.h>

namespace fb::protocol::game::request {

#ifdef BOT
async::task<void> spell_cast::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(this->slot);

    switch (this->type)
    {
    case SPELL_TYPE::INPUT:
#ifdef _WIN32
        writer.write(this->message.c_str(), this->message.size());
#else
        writer.write(cp949(this->message.c_str()), this->message.size());
#endif
        break;

    case SPELL_TYPE::TARGET:
        writer.write<uint32_t>(this->oid);
        writer.write<uint16_t>(this->position.x);
        writer.write<uint16_t>(this->position.y);
        break;
    }
}
#else
async::task<void> spell_cast::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->slot = reader.read<uint8_t>();

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
        this->oid        = reader.read<uint32_t>();
        this->position.x = reader.read<uint16_t>();
        this->position.y = reader.read<uint16_t>();
    }
    break;
    }
}
#endif
} // namespace fb::protocol::game::request
