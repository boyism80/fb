#include <fb/bot/integration/dialog_ext_bot.h>
#include <fb/stream.h>

namespace fb::bot::integration {

namespace {

void read_appearance(fb::stream_reader<big_endian>& reader, dialog_ext_bot& self)
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

void dialog_ext_bot::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    uint8_t type_byte = reader.read<uint8_t>();
    this->type        = static_cast<dialog_ext_type>(type_byte);

    switch (this->type)
    {
    case dialog_ext_type::normal:
    case dialog_ext_type::normal_no_msg:
    case dialog_ext_type::look:
    {
        read_appearance(reader, *this);
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
        read_appearance(reader, *this);
        reader.read<uint32_t>(); // 1
        this->list_button_prev = reader.read<uint8_t>() != 0;
        reader.read<uint8_t>(); // 1
        this->message.clear();
        if (this->type == dialog_ext_type::list)
            this->message = reader.read<std::string, uint16_t>();

        uint8_t list_count = reader.read<uint8_t>();
        this->list_lists.clear();
        for (uint8_t i = 0; i < list_count; i++)
            this->list_lists.push_back(reader.read<std::string, uint8_t>());
    }
    break;

    case dialog_ext_type::input_ext:
    case dialog_ext_type::input_ext_no_msg:
    case dialog_ext_type::input_password:
    case dialog_ext_type::input_password_no_msg:
    {
        read_appearance(reader, *this);
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
        read_appearance(reader, *this);
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

    default:
        throw std::runtime_error("Unknown dialog ext type: " + std::to_string(type_byte));
    }
}

} // namespace fb::bot::integration
