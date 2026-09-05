#include <fb/game/protocol/dialog/slot.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
dialog_slot<V>::dialog_slot(const fb::model::object&    obj,
                            const std::vector<uint8_t>& slots,
                            std::string_view            message,
                            uint32_t                    oid) :
    appearance(fb::game::appearance_factory::create<V>(obj)),
    slots(slots),
    message(std::string(message)),
    oid(oid)
{ }

template <CLIENT_VERSION V>
dialog_slot<V>::dialog_slot(const fb::game::object&     object,
                            const std::vector<uint8_t>& slots,
                            std::string_view            message,
                            uint32_t                    oid) :
    appearance(fb::game::appearance_factory::create<V>(object)),
    slots(slots),
    message(std::string(message)),
    oid(oid)
{ }

template <CLIENT_VERSION V>
dialog_slot<V>::dialog_slot(appearance_ptr&&            appearance,
                            const std::vector<uint8_t>& slots,
                            std::string_view            message,
                            uint32_t                    oid) :
    appearance(std::move(appearance)),
    slots(slots),
    message(std::string(message)),
    oid(oid)
{ }

template <CLIENT_VERSION V>
void dialog_slot<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    constexpr auto type_value = static_cast<uint8_t>(type);

    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(type_value);
    writer.write<uint8_t>(type_value);
    writer.write<uint32_t>(this->oid);
    this->appearance->serialize(writer);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<uint16_t>(0xFFFF);
    writer.write<uint8_t>((uint8_t)this->slots.size());

    for (auto slot : this->slots)
    {
        writer.write<uint8_t>(slot);
    }
}

template <>
void dialog_slot<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    constexpr auto type_value = static_cast<uint8_t>(type);

    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(type_value);
    writer.write<uint8_t>(type_value);
    writer.write<uint32_t>(this->oid);
    this->appearance->serialize(writer);
    writer.write<uint32_t>(0);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<uint16_t>(0xFFFF);
    writer.write<uint8_t>((uint8_t)this->slots.size());

    for (auto slot : this->slots)
    {
        writer.write<uint8_t>(slot);
    }
}

template class dialog_slot<CLIENT_VERSION::v550>;
template class dialog_slot<CLIENT_VERSION::v565>;
template class dialog_slot<CLIENT_VERSION::v651>;
#else
void dialog_slot::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>();
    this->type_echo = reader.read<uint8_t>();
    this->oid       = reader.read<uint32_t>();
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
}
#endif

} // namespace fb::protocol::game::response
