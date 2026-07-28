#include <fb/game/protocol/dialog/list.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_list::dialog_list(const fb::game::object&         object,
                         const std::vector<std::string>& lists,
                         std::string_view                message,
                         bool                            button_prev,
                         uint32_t                        oid) :
    appearance(fb::game::appearance_factory::create(object)),
    lists(lists),
    message(std::string(message)),
    button_prev(button_prev),
    oid(oid)
{ }

dialog_list::dialog_list(const fb::model::object&        object,
                         const std::vector<std::string>& lists,
                         std::string_view                message,
                         bool                            button_prev,
                         uint32_t                        oid) :
    appearance(fb::game::appearance_factory::create(object)),
    lists(lists),
    message(std::string(message)),
    button_prev(button_prev),
    oid(oid)
{ }

dialog_list::dialog_list(appearance_ptr&&                appearance,
                         const std::vector<std::string>& lists,
                         std::string_view                message,
                         bool                            button_prev,
                         uint32_t                        oid) :
    appearance(std::move(appearance)),
    lists(lists),
    message(std::string(message)),
    button_prev(button_prev),
    oid(oid)
{ }
#endif

#ifndef BOT
void dialog_list::serialize(fb::stream_writer<big_endian>& writer) const
{
    constexpr auto type_value = static_cast<uint8_t>(type);

    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(type_value);
    writer.write<uint8_t>(type_value);
    writer.write<uint32_t>(this->oid);
    this->appearance->serialize(writer);
    writer.write<uint32_t>(1);
    writer.write<bool>(this->button_prev); // button prev
    writer.write<uint8_t>(1);
    writer.write<std::string, uint16_t>(message);

    writer.write<uint8_t>(lists.size());
    for (int i = 0; i < lists.size(); i++)
    {
        writer.write<std::string>(lists[i]);
    }
    writer.write<uint8_t>(0x00);
}
#else
void dialog_list::deserialize(fb::stream_reader<big_endian>& reader)
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
    reader.read<uint32_t>(); // 0x00000001
    this->button_prev = reader.read<bool>();
    reader.read<uint8_t>(); // 0x01
    this->message = reader.read<std::string, uint16_t>();

    uint8_t list_count = reader.read<uint8_t>();
    this->lists.clear();
    for (int i = 0; i < list_count; i++)
    {
        this->lists.push_back(reader.read<std::string, uint8_t>());
    }
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
