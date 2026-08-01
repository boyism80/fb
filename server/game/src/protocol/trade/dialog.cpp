#include <fb/game/protocol/trade/dialog.h>

using table = fb::model::table;
using namespace fb::protocol::game::response;

#ifndef BOT
trade_dialog::trade_dialog(const fb::game::character& me) :
    me(me)
{ }
#endif

#ifndef BOT
void trade_dialog::serialize(fb::stream_writer<big_endian>& writer) const
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
#else
void trade_dialog::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // SUBTYPE_OPEN
    this->oid  = reader.read<uint32_t>();
    this->name = reader.read<std::string, uint8_t>();
}
#endif
