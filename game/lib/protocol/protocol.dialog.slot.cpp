#include <fb/game/protocol/dialog/dialog_slot.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_slot::dialog_slot(const fb::model::object&      obj,
                         const std::vector<uint8_t>&   slots,
                         const std::string&            message,
                         uint32_t                      sequence,
                         fb::game::dialog::interaction interaction) :
    obj(obj),
    slots(slots),
    message(message),
    sequence(sequence),
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
    writer.write<uint32_t>(this->sequence);
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
    // TODO: deserialize bytes
}
#endif

} // namespace fb::protocol::game::response
