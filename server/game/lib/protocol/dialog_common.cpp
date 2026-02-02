#include <fb/game/protocol/dialog/dialog.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog::dialog(const fb::model::object&      object,
               std::string_view               message,
               bool                          button_prev,
               bool                          button_next,
               uint32_t                      oid,
               fb::game::dialog::interaction interaction) :
    portrait(fb::game::portrait_factory::create(object)),
    message(std::string(message)),
    button_prev(button_prev),
    button_next(button_next),
    oid(oid),
    interaction(interaction)
{ }

dialog::dialog(const fb::game::object&       object,
               std::string_view               message,
               bool                          button_prev,
               bool                          button_next,
               uint32_t                      oid,
               fb::game::dialog::interaction interaction) :
    portrait(fb::game::portrait_factory::create(object)),
    message(std::string(message)),
    button_prev(button_prev),
    button_next(button_next),
    oid(oid),
    interaction(interaction)
{ }
#endif

#ifndef BOT
async::task<void> dialog::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x00);                                    // unknown
    writer.write<uint8_t>(static_cast<uint8_t>(this->interaction)); // interaction
    writer.write<uint32_t>(this->oid);
    this->portrait->serialize(writer);
    writer.write<uint32_t>(0x01);
    writer.write<uint8_t>(this->button_prev);
    writer.write<uint8_t>(this->button_next);
    writer.write<std::string, uint16_t>(this->message);
}
#else
async::task<void> dialog::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    reader.read<uint8_t>(); // 0x00
    this->interaction = reader.read<uint8_t>();
    this->oid         = reader.read<uint32_t>();
    reader.read<uint8_t>(); // obj type flag
    reader.read<uint8_t>(); // 0x01
    this->look  = reader.read<uint16_t>();
    this->color = reader.read<uint8_t>();
    reader.read<uint8_t>();  // obj type flag
    reader.read<uint16_t>(); // look (duplicate)
    reader.read<uint8_t>();  // color (duplicate)
    reader.read<uint32_t>(); // 0x01
    this->button_prev = reader.read<uint8_t>();
    this->button_next = reader.read<uint8_t>();
    this->message     = reader.read<std::string, uint16_t>();
}
#endif

} // namespace fb::protocol::game::response
