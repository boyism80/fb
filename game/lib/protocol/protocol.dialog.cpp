#include <fb/game/protocol/dialog.h>

namespace fb::protocol::game::request {

#ifdef BOT
async::task<void> dialog::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    // TODO: serialize bytes
}
#else
async::task<void> dialog::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->interaction = static_cast<fb::game::dialog::interaction>(reader.read<uint8_t>());
    switch (static_cast<fb::game::dialog::interaction>(this->interaction))
    {
    case fb::game::dialog::interaction::NORMAL: // 일반 다이얼로그
    {
        reader.read(nullptr, 0x07); // 7바이트 무시
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
        reader.read(nullptr, 0x07); // 7바이트 무시
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
#endif

} // namespace fb::protocol::game::request
