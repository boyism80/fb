#include <fb/bot/integration/dialog_ext_bot.h>
#include <fb/stream.h>

namespace fb::bot::integration {

async::task<void> dialog_ext_bot::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    uint8_t type_byte = reader.read<uint8_t>();
    this->type        = static_cast<dialog_ext_type>(type_byte);

    switch (this->type)
    {
    case dialog_ext_type::normal:
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
        reader.read<uint32_t>(); // 0x01
        this->normal_button_prev = reader.read<uint8_t>();
        this->normal_button_next = reader.read<uint8_t>();
        this->message            = reader.read<std::string, uint16_t>();
    }
    break;

    case dialog_ext_type::list:
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
        reader.read<uint32_t>(); // 1
        this->list_button_prev = reader.read<uint8_t>();
        reader.read<uint8_t>(); // 1
        this->message = reader.read<std::string, uint16_t>();

        // Read lists
        uint8_t list_count = reader.read<uint8_t>();
        this->list_lists.clear();
        for (uint8_t i = 0; i < list_count; i++)
        {
            this->list_lists.push_back(reader.read<std::string, uint8_t>());
        }
        reader.read<uint8_t>(); // trailing 0x00
    }
    break;

    case dialog_ext_type::input_ext:
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
        reader.read<uint32_t>(); // 0x00000001
        this->input_ext_button_prev = reader.read<uint8_t>();
        reader.read<uint8_t>(); // 0x00
        this->message          = reader.read<std::string, uint16_t>();
        this->input_ext_top    = reader.read<std::string, uint8_t>();
        this->input_ext_maxlen = reader.read<uint8_t>();
        this->input_ext_bottom = reader.read<std::string, uint8_t>();
        reader.read<uint8_t>(); // trailing 0x00
    }
    break;

    default:
        throw std::runtime_error("Unknown dialog ext type: " + std::to_string(type_byte));
    }
}

} // namespace fb::bot::integration