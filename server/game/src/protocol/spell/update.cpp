#include <fb/game/protocol/spell/update.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
spell_update<V>::spell_update(const fb::game::life& me, uint8_t index) :
    me(me),
    index(index)
{ }

template <CLIENT_VERSION V>
void spell_update<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    auto spell = this->me.spells.at(index);
    if (spell == nullptr)
        return;

    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->index + 1);
    writer.write<uint8_t>(static_cast<uint8_t>(spell->model().type));
    writer.write<std::string>(spell->model().name);

    if (static_cast<int>(spell->model().type) < 3)
        writer.write<std::string>(spell->model().message);
}

template <>
void spell_update<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    auto spell = this->me.spells.at(index);
    if (spell == nullptr)
        return;

    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->index + 1);
    writer.write<uint8_t>(static_cast<uint8_t>(spell->model().type));
    writer.write<std::string>(spell->model().name);
    writer.write<std::string>(spell->model().message);
}
#else
template <CLIENT_VERSION V>
void spell_update<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->index = reader.read<uint8_t>() - 1;
    this->type  = reader.read<uint8_t>();
    this->name  = reader.read<std::string, uint8_t>();
    if (type < 3)
        this->message = reader.read<std::string, uint8_t>();
}

template <>
void spell_update<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->index   = reader.read<uint8_t>() - 1;
    this->type    = reader.read<uint8_t>();
    this->name    = reader.read<std::string, uint8_t>();
    this->message = reader.read<std::string, uint8_t>();
}
#endif

template class spell_update<CLIENT_VERSION::v550>;
template class spell_update<CLIENT_VERSION::v565>;
template class spell_update<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response
