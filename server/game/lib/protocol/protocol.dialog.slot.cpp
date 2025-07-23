#include <fb/game/protocol/dialog/dialog_slot.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_slot::dialog_slot(const fb::model::object&      obj,
                         const std::vector<uint8_t>&   slots,
                         const std::string&            message,
                         uint32_t                      oid,
                         fb::game::dialog::interaction interaction) :
    obj(obj),
    slots(slots),
    message(message),
    oid(oid),
    interaction(interaction)
{ }
#endif

#ifndef BOT
async::task<void> dialog_slot::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x05);
    writer.write<uint8_t>(static_cast<uint8_t>(this->interaction));
    writer.write<uint32_t>(this->oid);
    writer.write<uint8_t>(this->obj.look > 0xBFFF ? 0x02 : 0x01);
    writer.write<uint8_t>(0x01);
    writer.write<uint16_t>(this->obj.look);
    writer.write<uint8_t>(this->obj.color);
    writer.write<uint8_t>(this->obj.look > 0xBFFF ? 0x02 : 0x01);
    writer.write<uint16_t>(this->obj.look);
    writer.write<uint8_t>(this->obj.color);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<uint16_t>(0xFFFF);
    writer.write<uint8_t>((uint8_t)this->slots.size());

    for (auto slot : this->slots)
        writer.write<uint8_t>(slot);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> dialog_slot::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    reader.read<uint8_t>(); // 0x05
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
    reader.read<uint16_t>(); // 0xFFFF
    uint8_t slot_count = reader.read<uint8_t>();

    this->slots.clear();
    for (int i = 0; i < slot_count; i++)
    {
        this->slots.push_back(reader.read<uint8_t>());
    }
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
