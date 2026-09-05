#include <fb/game/protocol/trade/dialog.h>
#include <sstream>

using table = fb::model::table;

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
trade_dialog<V>::trade_dialog(const fb::game::character& me) :
    me(me)
{ }
#endif

#ifndef BOT
template <CLIENT_VERSION V>
void trade_dialog<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    const auto& cname = table::promotion[me.cls()][me.promotion()].name;

    std::stringstream sstream;
    sstream << this->me.name() << '(' << cname.c_str() << ')';

    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(SUBTYPE_OPEN);
    writer.write<uint32_t>(this->me.oid());
    writer.write<std::string>(sstream.str());
}

template <>
void trade_dialog<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    const auto& cname = table::promotion[me.cls()][me.promotion()].name;

    std::stringstream sstream;
    sstream << this->me.name() << '(' << cname.c_str() << ')';

    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(SUBTYPE_OPEN);
    writer.write<uint32_t>(this->me.oid());
    writer.write<std::string>(sstream.str());
    writer.write<int16_t>(this->me.reputation());
}
#else
template <CLIENT_VERSION V>
void trade_dialog<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // SUBTYPE_OPEN
    this->oid  = reader.read<uint32_t>();
    this->name = reader.read<std::string, uint8_t>();
}

template <>
void trade_dialog<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // SUBTYPE_OPEN
    this->oid        = reader.read<uint32_t>();
    this->name       = reader.read<std::string, uint8_t>();
    this->reputation = reader.read<int16_t>();
}
#endif

template class trade_dialog<CLIENT_VERSION::v550>;
template class trade_dialog<CLIENT_VERSION::v565>;
template class trade_dialog<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response
