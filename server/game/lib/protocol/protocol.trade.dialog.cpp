#include <fb/game/protocol/trade/trade_dialog.h>

namespace fb::protocol::game::response {

#ifndef BOT
trade_dialog::trade_dialog(const fb::game::character& me, const fb::model::model& model) :
    me(me),
    model(model)
{ }
#endif

#ifndef BOT
async::task<void> trade_dialog::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    const auto& cname = model.promotion[me.cls()][me.promotion()].name;

    std::stringstream sstream;
    sstream << this->me.name() << '(' << cname.c_str() << ')';

    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x00);
    writer.write<uint32_t>(this->me.oid());
    writer.write<std::string>(sstream.str());
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> trade_dialog::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    reader.read<uint8_t>(); // 0x00
    this->oid  = reader.read<uint32_t>();
    this->name = reader.read<std::string, uint8_t>();
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
