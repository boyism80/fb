#include <fb/bot/integration/dialog_bot.h>

#include <fb/stream.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

namespace fb::bot::integration {

void dialog_bot::read_appearance(fb::stream_reader<big_endian>& reader)
{
    this->interaction = reader.read<uint8_t>();
    this->oid         = reader.read<uint32_t>();
    reader.read<uint8_t>(); // obj type flag
    reader.read<uint8_t>(); // 0x01
    this->look  = reader.read<uint16_t>();
    this->color = reader.read<uint8_t>();
    reader.read<uint8_t>();  // obj type flag (duplicate)
    reader.read<uint16_t>(); // look (duplicate)
    reader.read<uint8_t>();  // color (duplicate)
}

void dialog_bot::read_appearance_v651(fb::stream_reader<big_endian>& reader)
{
    this->interaction = reader.read<uint8_t>();
    this->oid         = reader.read<uint32_t>();
    reader.read<uint8_t>();
    if (reader.read<uint8_t>() == 0x00)
    {
        this->look  = 0;
        this->color = 0;
        for (auto i = 0; i < 23; i++)
        {
            reader.read<uint8_t>();
        }
    }
    else
    {
        this->look  = reader.read<uint16_t>();
        this->color = reader.read<uint8_t>();
    }
    reader.read<uint32_t>();
}

void dialog_bot::deserialize(fb::stream_reader<big_endian>& reader)
{
    this->deserialize(reader, fb::protocol::CLIENT_VERSION::v550);
}

void dialog_bot::deserialize(fb::stream_reader<big_endian>& reader, fb::protocol::CLIENT_VERSION version)
{
    header::deserialize(reader);
    uint8_t type_byte = reader.read<uint8_t>();
    this->type        = static_cast<dialog_type>(type_byte);

    switch (this->type)
    {
    case dialog_type::input:
    case dialog_type::input_no_ext:
    case dialog_type::item:
    case dialog_type::menu:
    case dialog_type::menu_no_ext:
    case dialog_type::pursuit:
    case dialog_type::buy:
    case dialog_type::slot:
    case dialog_type::spell:
        break;

    default:
        throw std::runtime_error("Unknown dialog type: " + std::to_string(type_byte));
    }

    if (version == fb::protocol::CLIENT_VERSION::v651)
        this->read_appearance_v651(reader);
    else
        this->read_appearance(reader);

    switch (this->type)
    {
    case dialog_type::input:
    case dialog_type::input_no_ext:
    {
        this->message = reader.read<std::string, uint16_t>();
        if (this->type == dialog_type::input)
            reader.read<std::string, uint8_t>(); // ext blob
        this->pursuit = reader.read<uint16_t>();
    }
    break;

    case dialog_type::item:
    {
        this->message      = reader.read<std::string, uint16_t>();
        this->item_pursuit = reader.read<uint16_t>();
        this->pursuit      = this->item_pursuit;

        uint16_t item_count = reader.read<uint16_t>();
        this->item_items.clear();
        for (uint16_t i = 0; i < item_count; i++)
        {
            dialog_item_data item;
            item.look  = reader.read<uint16_t>();
            item.color = reader.read<uint8_t>();
            item.value = reader.read<uint32_t>();
            item.name  = reader.read<std::string, uint8_t>();
            item.desc  = reader.read<std::string, uint8_t>();
            this->item_items.push_back(std::move(item));
        }
    }
    break;

    case dialog_type::menu:
    case dialog_type::menu_no_ext:
    {
        this->message = reader.read<std::string, uint16_t>();
        if (this->type == dialog_type::menu)
            reader.read<std::string, uint8_t>(); // ext blob

        uint8_t menu_count = reader.read<uint8_t>();
        this->menu_menus.clear();
        for (uint8_t i = 0; i < menu_count; i++)
        {
            this->menu_menus.push_back(reader.read<std::string, uint8_t>());
            reader.read<uint16_t>(); // menu index
        }
    }
    break;

    case dialog_type::pursuit:
    {
        this->message = reader.read<std::string, uint16_t>();
        this->pursuit = reader.read<uint16_t>();

        uint16_t option_count = reader.read<uint16_t>();
        this->menu_menus.clear();
        for (uint16_t i = 0; i < option_count; i++)
        {
            reader.read<uint32_t>(); // unused
            this->menu_menus.push_back(reader.read<std::string, uint8_t>());
        }
    }
    break;

    case dialog_type::buy:
    {
        this->message = reader.read<std::string, uint16_t>();
        this->pursuit = reader.read<uint16_t>();

        uint16_t entry_count = reader.read<uint16_t>();
        this->buy_entries.clear();
        this->menu_menus.clear();
        for (uint16_t i = 0; i < entry_count; i++)
        {
            if (version == fb::protocol::CLIENT_VERSION::v651)
            {
                reader.read<uint32_t>();
                reader.read<uint16_t>();
                reader.read<uint8_t>();
                auto price = reader.read<uint32_t>();
                reader.read<uint8_t>();
                auto name = reader.read<std::string, uint8_t>();
                reader.read<uint8_t>();
                this->menu_menus.push_back(name);
                this->buy_entries.emplace_back(std::move(name), std::to_string(price));
            }
            else
            {
                auto name  = reader.read<std::string, uint8_t>();
                auto value = reader.read<std::string, uint8_t>();
                this->menu_menus.push_back(name);
                this->buy_entries.emplace_back(std::move(name), std::move(value));
            }
        }
    }
    break;

    case dialog_type::slot:
    {
        this->message = reader.read<std::string, uint16_t>();
        this->pursuit = reader.read<uint16_t>();

        uint8_t slot_count = reader.read<uint8_t>();
        this->slot_slots.clear();
        for (uint8_t i = 0; i < slot_count; i++)
        {
            this->slot_slots.push_back(reader.read<uint8_t>());
        }
    }
    break;

    case dialog_type::spell:
    {
        // Client picks from local spellbook; packet carries message + pursuit only.
        this->message = reader.read<std::string, uint16_t>();
        this->pursuit = reader.read<uint16_t>();
    }
    break;
    }
}

} // namespace fb::bot::integration
