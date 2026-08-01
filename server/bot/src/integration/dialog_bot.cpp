#include <fb/bot/integration/dialog_bot.h>
#include <fb/stream.h>

namespace fb::bot::integration {

namespace {

void read_appearance(fb::stream_reader<big_endian>& reader, dialog_bot& self)
{
    self.interaction = reader.read<uint8_t>();
    self.oid         = reader.read<uint32_t>();
    reader.read<uint8_t>(); // obj type flag
    reader.read<uint8_t>(); // 0x01
    self.look  = reader.read<uint16_t>();
    self.color = reader.read<uint8_t>();
    reader.read<uint8_t>();  // obj type flag (duplicate)
    reader.read<uint16_t>(); // look (duplicate)
    reader.read<uint8_t>();  // color (duplicate)
}

} // namespace

void dialog_bot::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    uint8_t type_byte = reader.read<uint8_t>();
    this->type        = static_cast<dialog_type>(type_byte);

    switch (this->type)
    {
    case dialog_type::input:
    case dialog_type::input_no_ext:
    {
        read_appearance(reader, *this);
        this->message = reader.read<std::string, uint16_t>();
        if (this->type == dialog_type::input)
            reader.read<std::string, uint8_t>(); // ext blob
        this->pursuit = reader.read<uint16_t>();
    }
    break;

    case dialog_type::item:
    {
        read_appearance(reader, *this);
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
        read_appearance(reader, *this);
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
        read_appearance(reader, *this);
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

    case dialog_type::dual_field:
    {
        read_appearance(reader, *this);
        this->message = reader.read<std::string, uint16_t>();
        this->pursuit = reader.read<uint16_t>();

        uint16_t pair_count = reader.read<uint16_t>();
        this->dual_pairs.clear();
        this->menu_menus.clear();
        for (uint16_t i = 0; i < pair_count; i++)
        {
            auto label = reader.read<std::string, uint8_t>();
            auto value = reader.read<std::string, uint8_t>();
            this->menu_menus.push_back(label);
            this->dual_pairs.emplace_back(std::move(label), std::move(value));
        }
    }
    break;

    case dialog_type::slot:
    {
        read_appearance(reader, *this);
        this->message = reader.read<std::string, uint16_t>();
        this->pursuit = reader.read<uint16_t>();

        uint8_t slot_count = reader.read<uint8_t>();
        this->slot_slots.clear();
        for (uint8_t i = 0; i < slot_count; i++)
            this->slot_slots.push_back(reader.read<uint8_t>());
    }
    break;

    case dialog_type::spell:
    {
        // Client picks from local spellbook; packet carries message + pursuit only.
        read_appearance(reader, *this);
        this->message = reader.read<std::string, uint16_t>();
        this->pursuit = reader.read<uint16_t>();
    }
    break;

    default:
        throw std::runtime_error("Unknown dialog type: " + std::to_string(type_byte));
    }
}

} // namespace fb::bot::integration
