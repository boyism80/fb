#include <fb/game/protocol/dialog/menu.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_menu::dialog_menu(const fb::model::object&        obj,
                         const std::vector<std::string>& menus,
                         std::string_view                message,
                         uint32_t                        oid,
                         fb::game::dialog::interaction   interaction) :
    appearance(fb::game::appearance_factory::create(obj)),
    menus(menus),
    message(std::string(message)),
    oid(oid),
    interaction(interaction)
{ }

dialog_menu::dialog_menu(const fb::game::object&         object,
                         const std::vector<std::string>& menus,
                         std::string_view                message,
                         uint32_t                        oid,
                         fb::game::dialog::interaction   interaction) :
    appearance(fb::game::appearance_factory::create(object)),
    menus(menus),
    message(std::string(message)),
    oid(oid),
    interaction(interaction)
{ }
#endif

#ifndef BOT
void dialog_menu::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0x01);
    writer.write<uint8_t>(static_cast<uint8_t>(interaction));
    writer.write<uint32_t>(this->oid);
    this->appearance->serialize(writer);
    writer.write<std::string, uint16_t>(message);

    writer.write<uint16_t>((uint16_t)menus.size());
    for (int i = 0; i < menus.size(); i++)
    {
        writer.write<std::string>(menus[i]);
        writer.write<uint16_t>(i);
    }

    writer.write<uint8_t>(0x00);
}
#else
void dialog_menu::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // 0x01
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

    uint16_t menu_count = reader.read<uint16_t>();
    this->menus.clear();
    for (int i = 0; i < menu_count; i++)
    {
        this->menus.push_back(reader.read<std::string, uint8_t>());
        reader.read<uint16_t>(); // menu index
    }
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
