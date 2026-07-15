#include <fb/bot/integration/dialog_bot.h>
#include <fb/stream.h>

namespace fb::bot::integration {

void dialog_bot::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    uint8_t type_byte = reader.read<uint8_t>();
    this->type        = static_cast<dialog_type>(type_byte);

    switch (this->type)
    {
    case dialog_type::input:
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
        this->message = reader.read<std::string, uint16_t>();
        reader.read<uint8_t>(); // trailing 0x00
    }
    break;

    case dialog_type::item:
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
        this->message      = reader.read<std::string, uint16_t>();
        this->item_pursuit = reader.read<uint16_t>();

        // Read items
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
            this->item_items.push_back(item);
        }
        reader.read<uint8_t>(); // trailing 0x00
    }
    break;

    case dialog_type::menu:
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
        this->message = reader.read<std::string, uint16_t>();

        // Read menus
        uint16_t menu_count = reader.read<uint16_t>();
        this->menu_menus.clear();
        for (uint16_t i = 0; i < menu_count; i++)
        {
            this->menu_menus.push_back(reader.read<std::string, uint8_t>());
            reader.read<uint16_t>(); // menu index
        }
        reader.read<uint8_t>(); // trailing 0x00
    }
    break;

    case dialog_type::slot:
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
        this->message = reader.read<std::string, uint16_t>();
        reader.read<uint16_t>(); // 0xFFFF

        // Read slots
        uint8_t slot_count = reader.read<uint8_t>();
        this->slot_slots.clear();
        for (uint8_t i = 0; i < slot_count; i++)
        {
            this->slot_slots.push_back(reader.read<uint8_t>());
        }
        reader.read<uint8_t>(); // trailing 0x00
    }
    break;

    default:
        throw std::runtime_error("Unknown dialog type: " + std::to_string(type_byte));
    }
}

} // namespace fb::bot::integration