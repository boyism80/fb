#include <fb/bot/integration/dialog_ext_bot.h>

#include <fb/stream.h>

#include <cstdint>
#include <stdexcept>
#include <string>

namespace fb::bot::integration {

void dialog_ext_bot::read_appearance(fb::stream_reader<big_endian>& reader)
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

void dialog_ext_bot::read_appearance_v651(fb::stream_reader<big_endian>& reader)
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

void dialog_ext_bot::deserialize(fb::stream_reader<big_endian>& reader)
{
    this->deserialize(reader, fb::protocol::CLIENT_VERSION::v550);
}

void dialog_ext_bot::deserialize(fb::stream_reader<big_endian>& reader, fb::protocol::CLIENT_VERSION version)
{
    header::deserialize(reader);
    uint8_t type_byte = reader.read<uint8_t>();
    this->type        = static_cast<dialog_ext_type>(type_byte);

    switch (this->type)
    {
    case dialog_ext_type::normal:
    case dialog_ext_type::normal_no_msg:
    case dialog_ext_type::look:
    case dialog_ext_type::list:
    case dialog_ext_type::list_no_msg:
    case dialog_ext_type::input_ext:
    case dialog_ext_type::input_ext_no_msg:
    case dialog_ext_type::input_password:
    case dialog_ext_type::input_password_no_msg:
    case dialog_ext_type::email:
        break;

    default:
        throw std::runtime_error("Unknown dialog ext type: " + std::to_string(type_byte));
    }

    if (version == fb::protocol::CLIENT_VERSION::v651)
        this->read_appearance_v651(reader);
    else
        this->read_appearance(reader);

    switch (this->type)
    {
    case dialog_ext_type::normal:
    case dialog_ext_type::normal_no_msg:
    case dialog_ext_type::look:
    {
        reader.read<uint32_t>(); // seq seed
        this->normal_button_prev = reader.read<uint8_t>() != 0;
        this->normal_button_next = reader.read<uint8_t>() != 0;
        this->message.clear();
        if (this->type == dialog_ext_type::normal || this->type == dialog_ext_type::look)
            this->message = reader.read<std::string, uint16_t>();
    }
    break;

    case dialog_ext_type::list:
    case dialog_ext_type::list_no_msg:
    {
        reader.read<uint32_t>(); // 1
        this->list_button_prev = reader.read<uint8_t>() != 0;
        reader.read<uint8_t>(); // 1
        this->message.clear();
        if (this->type == dialog_ext_type::list)
            this->message = reader.read<std::string, uint16_t>();

        uint8_t list_count = reader.read<uint8_t>();
        this->list_lists.clear();
        for (uint8_t i = 0; i < list_count; i++)
        {
            this->list_lists.push_back(reader.read<std::string, uint8_t>());
        }
    }
    break;

    case dialog_ext_type::input_ext:
    case dialog_ext_type::input_ext_no_msg:
    case dialog_ext_type::input_password:
    case dialog_ext_type::input_password_no_msg:
    {
        reader.read<uint32_t>(); // 1
        this->input_ext_button_prev = reader.read<uint8_t>() != 0;
        reader.read<uint8_t>(); // flag_next unused
        this->input_ext_password =
            (this->type == dialog_ext_type::input_password || this->type == dialog_ext_type::input_password_no_msg);

        this->message.clear();
        if (this->type == dialog_ext_type::input_ext || this->type == dialog_ext_type::input_password)
            this->message = reader.read<std::string, uint16_t>();

        this->input_ext_top    = reader.read<std::string, uint8_t>();
        this->input_ext_maxlen = reader.read<uint8_t>();
        this->input_ext_bottom = reader.read<std::string, uint8_t>();
    }
    break;

    case dialog_ext_type::email:
    {
        reader.read<uint32_t>(); // 1
        this->normal_button_prev = reader.read<uint8_t>() != 0;
        this->normal_button_next = reader.read<uint8_t>() != 0;
        this->message            = reader.read<std::string, uint16_t>();
        this->email_str1         = reader.read<std::string, uint8_t>();
        reader.read<uint8_t>(); // unused pad (InputExt maxlen slot)
        this->email_str2 = reader.read<std::string, uint8_t>();

        // Alias into input_ext fields for Lua convenience.
        this->input_ext_top         = this->email_str1;
        this->input_ext_bottom      = this->email_str2;
        this->input_ext_button_prev = this->normal_button_prev;
    }
    break;
    }
}

} // namespace fb::bot::integration
