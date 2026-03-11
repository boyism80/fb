#include <fb/game/protocol/dialog/dialog_input.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_input::dialog_input(const fb::model::object&      obj,
                           std::string_view              message,
                           uint32_t                      oid,
                           fb::game::dialog::interaction interaction) :
    appearance(fb::game::appearance_factory::create(obj)),
    message(std::string(message)),
    oid(oid),
    interaction(interaction)
{ }

dialog_input::dialog_input(const fb::game::object&       object,
                           std::string_view              message,
                           uint32_t                      oid,
                           fb::game::dialog::interaction interaction) :
    appearance(fb::game::appearance_factory::create(object)),
    message(std::string(message)),
    oid(oid),
    interaction(interaction)
{ }
#endif

#ifndef BOT
async::task<void> dialog_input::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0x03);
    writer.write<uint8_t>(static_cast<uint8_t>(this->interaction));
    writer.write<uint32_t>(this->oid);
    this->appearance->serialize(writer);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> dialog_input::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    reader.read<uint8_t>(); // 0x03
    this->interaction = reader.read<uint8_t>();
    this->oid         = reader.read<uint32_t>();
    reader.read<uint8_t>(); // obj type flag
    reader.read<uint8_t>(); // 0x01
    this->look  = reader.read<uint16_t>();
    this->color = reader.read<uint8_t>();
    reader.read<uint8_t>();  // obj type flag
    reader.read<uint16_t>(); // look (duplicate)
    reader.read<uint8_t>();  // color (duplicate)
    this->message = reader.read<std::string, uint16_t>();
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
