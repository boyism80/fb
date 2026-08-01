#include <fb/game/protocol/dialog/dialog_dual_field.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_dual_field::dialog_dual_field(const fb::model::object& obj,
                                     const pair_list&         pairs,
                                     std::string_view         message,
                                     uint32_t                 oid,
                                     uint16_t                 pursuit) :
    appearance(fb::game::appearance_factory::create(obj)),
    pairs(pairs),
    message(std::string(message)),
    pursuit(pursuit),
    oid(oid)
{ }

dialog_dual_field::dialog_dual_field(const fb::game::object& object,
                                     const pair_list&        pairs,
                                     std::string_view        message,
                                     uint32_t                oid,
                                     uint16_t                pursuit) :
    appearance(fb::game::appearance_factory::create(object)),
    pairs(pairs),
    message(std::string(message)),
    pursuit(pursuit),
    oid(oid)
{ }

void dialog_dual_field::serialize(fb::stream_writer<big_endian>& writer) const
{
    constexpr auto type_value = static_cast<uint8_t>(type);

    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(type_value);
    writer.write<uint8_t>(type_value);
    writer.write<uint32_t>(this->oid);
    this->appearance->serialize(writer);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<uint16_t>(this->pursuit);
    writer.write<uint16_t>((uint16_t)pairs.size());
    for (auto& [label, value] : pairs)
    {
        writer.write<std::string>(label);
        writer.write<std::string>(value);
    }
}
#else
void dialog_dual_field::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // first type echo
    this->type_echo = reader.read<uint8_t>();
    this->oid       = reader.read<uint32_t>();
    reader.read<uint8_t>();  // obj type flag
    reader.read<uint8_t>();  // 0x01
    reader.read<uint16_t>(); // look
    reader.read<uint8_t>();  // color
    reader.read<uint8_t>();  // obj type flag
    reader.read<uint16_t>(); // look (duplicate)
    reader.read<uint8_t>();  // color (duplicate)
    this->message = reader.read<std::string, uint16_t>();
    this->pursuit = reader.read<uint16_t>();

    uint16_t pair_count = reader.read<uint16_t>();
    this->pairs.clear();
    for (uint16_t i = 0; i < pair_count; i++)
    {
        auto label = reader.read<std::string, uint8_t>();
        auto value = reader.read<std::string, uint8_t>();
        this->pairs.emplace_back(std::move(label), std::move(value));
    }
}
#endif

} // namespace fb::protocol::game::response
