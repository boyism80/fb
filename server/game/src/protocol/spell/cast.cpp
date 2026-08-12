#include <fb/game/protocol/spell/cast.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
void spell_cast<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->slot + 1);

    switch (this->type)
    {
    case SPELL_TYPE::INPUT:
#ifdef _WIN32
        writer.write((const void*)this->message.c_str(), this->message.size() + 1);
#else
        writer.write((const void*)cp949(this->message).c_str(), this->message.size() + 1);
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
template <CLIENT_VERSION V>
void spell_cast<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->slot = reader.read<uint8_t>() - 1;

    auto remained = reader.readable_size();
    auto buffer   = std::make_unique<uint8_t[]>(remained);
    reader.read(buffer.get(), remained);

    auto writer = fb::stream_writer(this->buffer);
    writer.write(buffer.get(), remained);
}

template <CLIENT_VERSION V>
void spell_cast<V>::parse(SPELL_TYPE type)
{
    auto reader = fb::stream_reader<big_endian>(this->buffer);
    switch (type)
    {
    case SPELL_TYPE::INPUT:
    {
        if (this->buffer.empty())
            throw std::runtime_error("spell_cast::parse: buffer is empty");

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

template class spell_cast<CLIENT_VERSION::v550>;
template class spell_cast<CLIENT_VERSION::v565>;
template class spell_cast<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
