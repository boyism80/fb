#include <fb/game/protocol/dialog/dialog_menu.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_menu::dialog_menu(const fb::model::object&        obj,
                         const std::vector<std::string>& menus,
                         const std::string&              message,
                         uint32_t                        oid,
                         fb::game::dialog::interaction   interaction) :
    obj(obj),
    menus(menus),
    message(message),
    oid(oid),
    interaction(interaction)
{ }
#endif

#ifndef BOT
async::task<void> dialog_menu::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x01);
    writer.write<uint8_t>(static_cast<uint8_t>(interaction));
    writer.write<uint32_t>(this->oid);
    writer.write<uint8_t>(this->obj.look > 0xBFFF ? 0x02 : 0x01);
    writer.write<uint8_t>(0x01);
    writer.write<uint16_t>(this->obj.look);
    writer.write<uint8_t>(this->obj.color);
    writer.write<uint8_t>(this->obj.look > 0xBFFF ? 0x02 : 0x01);
    writer.write<uint16_t>(this->obj.look);
    writer.write<uint8_t>(this->obj.color);
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
async::task<void> dialog_menu::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
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
