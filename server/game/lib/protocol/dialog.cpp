#include <fb/game/protocol/dialog.h>

namespace fb::protocol::game::request {

#ifndef BOT
async::task<void> dialog::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->interaction = static_cast<fb::game::dialog::interaction>(reader.read<uint8_t>());
    switch (static_cast<fb::game::dialog::interaction>(this->interaction))
    {
    case fb::game::dialog::interaction::NORMAL: // Normal dialog
    {
        reader.read(nullptr, 0x07); // Skip 7 bytes
        this->action = reader.read<uint8_t>();
        break;
    }

    case fb::game::dialog::interaction::INPUT:
    {
        auto unknown1 = reader.read<uint16_t>();
        auto unknown2 = reader.read<uint32_t>();
        this->message = reader.read<std::string, uint16_t>();
        break;
    }

    case fb::game::dialog::interaction::INPUT_EX:
    {
        reader.read(nullptr, 0x07); // Skip 7 bytes
        this->action = reader.read<uint8_t>();
        if (this->action == 0x02) // OK button
        {
            auto unknown1 = reader.read<uint8_t>();
            this->message = reader.read<std::string, uint8_t>();
        }
        break;
    }

    case fb::game::dialog::interaction::MENU:
    {
        auto unknown = reader.read<uint32_t>();
        this->index  = reader.read<uint16_t>();
        break;
    }

    case fb::game::dialog::interaction::LIST:
    {
        auto unknown1 = reader.read<uint32_t>();
        this->button  = static_cast<DIALOG_RESULT>(reader.read<uint32_t>());
        switch (this->button)
        {
        case DIALOG_RESULT::PREV:
        case DIALOG_RESULT::QUIT:
            break;

        case DIALOG_RESULT::NEXT:
            auto unknown2 = reader.read<uint8_t>();
            this->index   = reader.read<uint8_t>() - 1;
            break;
        }
        break;
    }

    case fb::game::dialog::interaction::ITEM:
    {
        auto unknown  = reader.read<uint32_t>();
        this->pursuit = reader.read<uint16_t>();
        this->name    = reader.read<std::string, uint8_t>();
        break;
    }

    case fb::game::dialog::interaction::SLOT:
    {
        auto unknown  = reader.read<uint32_t>();
        this->pursuit = reader.read<uint16_t>();
        this->index   = reader.read<uint8_t>();
        break;
    }
    }
}
#else
async::task<void> dialog::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(static_cast<uint8_t>(this->interaction));

    switch (this->interaction)
    {
    case INTERACTION::NORMAL:        // NORMAL
        writer.write<uint8_t>(0x00); // 7-byte padding
        writer.write<uint8_t>(0x00);
        writer.write<uint8_t>(0x00);
        writer.write<uint8_t>(0x00);
        writer.write<uint8_t>(0x00);
        writer.write<uint8_t>(0x00);
        writer.write<uint8_t>(0x00);
        writer.write<uint8_t>(this->action);
        break;

    case INTERACTION::INPUT:                // INPUT
        writer.write<uint16_t>(0x0000);     // unknown1
        writer.write<uint32_t>(0x00000000); // unknown2
        writer.write<std::string, uint16_t>(this->message);
        break;

    case INTERACTION::INPUT_EX:      // INPUT_EX
        writer.write<uint8_t>(0x00); // 7-byte padding
        writer.write<uint8_t>(0x00);
        writer.write<uint8_t>(0x00);
        writer.write<uint8_t>(0x00);
        writer.write<uint8_t>(0x00);
        writer.write<uint8_t>(0x00);
        writer.write<uint8_t>(0x00);
        writer.write<uint8_t>(this->action);
        if (this->action == 0x02)
        {
            writer.write<uint8_t>(0x00); // unknown1
            writer.write<std::string, uint8_t>(this->message);
        }
        break;

    case INTERACTION::MENU:                 // MENU
        writer.write<uint32_t>(0x00000000); // unknown
        writer.write<uint16_t>(this->index);
        break;

    case INTERACTION::LIST:                 // LIST
        writer.write<uint32_t>(0x00000000); // unknown1
        writer.write<uint32_t>(static_cast<uint32_t>(this->button));
        if (this->button == DIALOG_RESULT::NEXT)
        {
            writer.write<uint8_t>(0x00); // unknown2
            writer.write<uint8_t>(this->index + 1);
        }
        break;

    case INTERACTION::SLOT:                 // SLOT
        writer.write<uint32_t>(0x00000000); // unknown
        writer.write<uint16_t>(this->pursuit);
        writer.write<std::string, uint8_t>(this->name);
        break;

    case INTERACTION::ITEM:                 // ITEM
        writer.write<uint32_t>(0x00000000); // unknown
        writer.write<uint16_t>(this->pursuit);
        writer.write<uint8_t>(this->index);
        break;
    }
}
#endif

} // namespace fb::protocol::game::request
