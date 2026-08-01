#include <fb/game/protocol/dialog/dialog_pursuit.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_pursuit::dialog_pursuit(const fb::model::object&        obj,
                               const std::vector<std::string>& options,
                               std::string_view                message,
                               uint32_t                        oid,
                               uint16_t                        pursuit) :
    appearance(fb::game::appearance_factory::create(obj)),
    options(options),
    message(std::string(message)),
    pursuit(pursuit),
    oid(oid)
{ }

dialog_pursuit::dialog_pursuit(const fb::game::object&         object,
                               const std::vector<std::string>& options,
                               std::string_view                message,
                               uint32_t                        oid,
                               uint16_t                        pursuit) :
    appearance(fb::game::appearance_factory::create(object)),
    options(options),
    message(std::string(message)),
    pursuit(pursuit),
    oid(oid)
{ }

void dialog_pursuit::serialize(fb::stream_writer<big_endian>& writer) const
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
    writer.write<uint16_t>((uint16_t)options.size());
    for (auto& name : options)
    {
        writer.write<uint32_t>(0); // unused skip 4 bytes
        writer.write<std::string>(name);
    }
}
#else
void dialog_pursuit::deserialize(fb::stream_reader<big_endian>& reader)
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

    uint16_t option_count = reader.read<uint16_t>();
    this->options.clear();
    for (uint16_t i = 0; i < option_count; i++)
    {
        reader.read<uint32_t>(); // unused
        this->options.push_back(reader.read<std::string, uint8_t>());
    }
}
#endif

} // namespace fb::protocol::game::response
