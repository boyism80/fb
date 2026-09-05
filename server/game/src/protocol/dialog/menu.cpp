#include <fb/game/protocol/dialog/menu.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
dialog_menu<V>::dialog_menu(const fb::model::object&        obj,
                            const std::vector<std::string>& menus,
                            std::string_view                message,
                            uint32_t                        oid,
                            std::optional<std::string>      ext) :
    appearance(fb::game::appearance_factory::create<V>(obj)),
    menus(menus),
    message(std::string(message)),
    oid(oid),
    ext(std::move(ext))
{ }

template <CLIENT_VERSION V>
dialog_menu<V>::dialog_menu(const fb::game::object&         object,
                            const std::vector<std::string>& menus,
                            std::string_view                message,
                            uint32_t                        oid,
                            std::optional<std::string>      ext) :
    appearance(fb::game::appearance_factory::create<V>(object)),
    menus(menus),
    message(std::string(message)),
    oid(oid),
    ext(std::move(ext))
{ }

template <CLIENT_VERSION V>
dialog_menu<V>::dialog_menu(appearance_ptr&&                appearance,
                            const std::vector<std::string>& menus,
                            std::string_view                message,
                            uint32_t                        oid,
                            std::optional<std::string>      ext) :
    appearance(std::move(appearance)),
    menus(menus),
    message(std::string(message)),
    oid(oid),
    ext(std::move(ext))
{ }

template <CLIENT_VERSION V>
void dialog_menu<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    auto type_value = static_cast<uint8_t>(this->ext.has_value() ? fb::game::dialog::type::MENU
                                                                 : fb::game::dialog::type::MENU_NO_EXT);

    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(type_value);
    writer.write<uint8_t>(type_value);
    writer.write<uint32_t>(this->oid);
    this->appearance->serialize(writer);
    writer.write<std::string, uint16_t>(message);

    if (this->ext.has_value())
        writer.write<std::string, uint8_t>(*this->ext);

    writer.write<uint8_t>((uint8_t)menus.size());
    for (int i = 0; i < menus.size(); i++)
    {
        writer.write<std::string>(menus[i]);
        writer.write<uint16_t>(i);
    }
}

template <>
void dialog_menu<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    auto type_value = static_cast<uint8_t>(this->ext.has_value() ? fb::game::dialog::type::MENU
                                                                 : fb::game::dialog::type::MENU_NO_EXT);

    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(type_value);
    writer.write<uint8_t>(type_value);
    writer.write<uint32_t>(this->oid);
    this->appearance->serialize(writer);
    writer.write<uint32_t>(0);
    writer.write<std::string, uint16_t>(message);

    if (this->ext.has_value())
        writer.write<std::string, uint8_t>(*this->ext);

    writer.write<uint8_t>((uint8_t)menus.size());
    for (int i = 0; i < menus.size(); i++)
    {
        writer.write<std::string>(menus[i]);
        writer.write<uint16_t>(i);
    }
}

template class dialog_menu<CLIENT_VERSION::v550>;
template class dialog_menu<CLIENT_VERSION::v565>;
template class dialog_menu<CLIENT_VERSION::v651>;
#else
void dialog_menu::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // 0x01
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

    this->ext = std::nullopt;
    if (this->type_echo == static_cast<uint8_t>(fb::game::dialog::type::MENU))
        this->ext = reader.read<std::string, uint8_t>();

    uint8_t menu_count = reader.read<uint8_t>();
    this->menus.clear();
    for (int i = 0; i < menu_count; i++)
    {
        this->menus.push_back(reader.read<std::string, uint8_t>());
        reader.read<uint16_t>(); // menu index
    }
}
#endif

} // namespace fb::protocol::game::response
