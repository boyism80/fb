#include <fb/bot/integration/trade_bot.h>
#include <fb/stream.h>

namespace fb::bot::integration {

void trade_bot::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->type = static_cast<trade_type>(reader.read<uint8_t>());

    switch (this->type)
    {
    case trade_type::dialog:
    {
        this->dialog_oid  = reader.read<uint32_t>();
        this->dialog_name = reader.read<std::string, uint8_t>();
        reader.read<uint8_t>(); // trailing 0x00
        break;
    }

    case trade_type::bundle:
    {
        reader.read<uint8_t>(); // trailing 0x00
        break;
    }

    case trade_type::upload:
    {
        this->upload_mine  = reader.read<uint8_t>() == 0x00;
        this->upload_index = reader.read<uint8_t>();
        this->upload_look  = reader.read<uint16_t>();
        this->upload_color = reader.read<uint8_t>();
        this->upload_name  = reader.read<std::string, uint8_t>();
        reader.read<uint8_t>(); // trailing 0x00
        break;
    }

    case trade_type::money:
    {
        this->mine  = reader.read<uint8_t>() == 0x00;
        this->money = reader.read<uint32_t>();
        reader.read<uint8_t>(); // trailing 0x00
        break;
    }

    case trade_type::lock:
    {
        reader.read<uint8_t>(); // trailing 0x00
        break;
    }

    case trade_type::close:
    {
        this->close_message = reader.read<std::string, uint16_t>();
        break;
    }
    }
}

} // namespace fb::bot::integration