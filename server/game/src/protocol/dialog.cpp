#include <fb/game/protocol/dialog.h>

namespace fb::protocol::game::request {

#ifndef BOT

void dialog::deserialize(fb::stream_reader<big_endian>& reader)
{
    using dialog_type = fb::game::dialog::type;

    header::deserialize(reader);
    this->type = static_cast<dialog_type>(reader.read<uint8_t>());

    switch (this->type)
    {
    case dialog_type::INPUT:
    {
        // subtype 3: oid | pursuit | ext | text
        this->oid     = reader.read<uint32_t>();
        this->pursuit = reader.read<uint16_t>();
        this->seq     = this->pursuit;
        this->ext     = reader.read<std::string, uint8_t>();
        this->message = reader.read<std::string, uint8_t>();
        this->action  = 0x02;
        break;
    }

    case dialog_type::INPUT_NO_EXT:
    {
        // subtype 2: oid | pursuit | text  (no ext)
        this->oid     = reader.read<uint32_t>();
        this->pursuit = reader.read<uint16_t>();
        this->seq     = this->pursuit;
        this->message = reader.read<std::string, uint8_t>();
        this->action  = 0x02;
        break;
    }

    case dialog_type::MENU:
    {
        // subtype 1: oid | index | ext
        this->oid   = reader.read<uint32_t>();
        this->index = reader.read<uint16_t>() + 1;
        this->ext   = reader.read<std::string, uint8_t>();
        break;
    }

    case dialog_type::MENU_NO_EXT:
    {
        // subtype 0: oid | index  (no ext)
        this->oid   = reader.read<uint32_t>();
        this->index = reader.read<uint16_t>() + 1;
        break;
    }

    case dialog_type::ITEM:
    case dialog_type::PURSUIT:
    case dialog_type::DUAL_FIELD:
    {
        this->oid     = reader.read<uint32_t>();
        this->pursuit = reader.read<uint16_t>();
        this->name    = reader.read<std::string, uint8_t>();
        break;
    }

    case dialog_type::SLOT:
    case dialog_type::SPELL:
    {
        this->oid     = reader.read<uint32_t>();
        this->pursuit = reader.read<uint16_t>();
        this->index   = reader.read<uint8_t>();
        break;
    }
    }
}

void dialog_list::deserialize(fb::stream_reader<big_endian>& reader)
{
    using list_type = fb::game::dialog::list_type;

    header::deserialize(reader);
    this->type = static_cast<list_type>(reader.read<uint8_t>());

    switch (this->type)
    {
    case list_type::TEXT:
    case list_type::TEXT_NO_MSG:
    {
        this->oid    = reader.read<uint32_t>();
        this->seq    = reader.read<uint16_t>();
        this->action = static_cast<uint8_t>(reader.read<uint16_t>());
        break;
    }

    case list_type::INPUT:
    case list_type::INPUT_NO_MSG:
    case list_type::INPUT_PASSWORD:
    case list_type::INPUT_PASSWORD_NO_MSG:
    case list_type::EMAIL:
    {
        this->oid = reader.read<uint32_t>();
        this->seq = reader.read<uint16_t>();
        if (reader.readable_size() > 2)
        {
            reader.read<uint16_t>();
            this->action = reader.read<uint8_t>();
            if (this->action == 0x02 && reader.readable_size() > 0)
                this->message = reader.read<std::string, uint8_t>();
        }
        else
        {
            this->action = static_cast<uint8_t>(reader.read<uint16_t>());
        }
        break;
    }

    case list_type::LIST:
    case list_type::LIST_NO_MSG:
    {
        this->oid    = reader.read<uint32_t>();
        this->button = static_cast<DIALOG_RESULT>(reader.read<uint32_t>());
        switch (this->button)
        {
        case DIALOG_RESULT::PREV:
        case DIALOG_RESULT::QUIT:
            break;

        case DIALOG_RESULT::NEXT:
            reader.read<uint8_t>();
            this->index = reader.read<uint8_t>();
            break;
        }
        break;
    }
    }
}

#else

void dialog::serialize(fb::stream_writer<big_endian>& writer) const
{
    using dialog_type = fb::game::dialog::type;

    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->type));

    switch (this->type)
    {
    case dialog_type::INPUT:
        writer.write<uint32_t>(this->oid);
        writer.write<uint16_t>(this->pursuit != 0 ? this->pursuit : this->seq);
        writer.write<std::string, uint8_t>(this->ext);
        writer.write<std::string, uint8_t>(this->message);
        break;

    case dialog_type::INPUT_NO_EXT:
        writer.write<uint32_t>(this->oid);
        writer.write<uint16_t>(this->pursuit != 0 ? this->pursuit : this->seq);
        writer.write<std::string, uint8_t>(this->message);
        break;

    case dialog_type::MENU:
        writer.write<uint32_t>(this->oid);
        writer.write<uint16_t>(this->index - 1);
        writer.write<std::string, uint8_t>(this->ext);
        break;

    case dialog_type::MENU_NO_EXT:
        writer.write<uint32_t>(this->oid);
        writer.write<uint16_t>(this->index - 1);
        break;

    case dialog_type::SLOT:
    case dialog_type::SPELL:
        writer.write<uint32_t>(this->oid);
        writer.write<uint16_t>(this->pursuit);
        writer.write<uint8_t>(static_cast<uint8_t>(this->index));
        break;

    case dialog_type::ITEM:
    case dialog_type::PURSUIT:
    case dialog_type::DUAL_FIELD:
        writer.write<uint32_t>(this->oid);
        writer.write<uint16_t>(this->pursuit);
        writer.write<std::string, uint8_t>(this->name);
        break;
    }
}

void dialog_list::serialize(fb::stream_writer<big_endian>& writer) const
{
    using list_type = fb::game::dialog::list_type;

    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->type));

    switch (this->type)
    {
    case list_type::TEXT:
    case list_type::TEXT_NO_MSG:
        writer.write<uint32_t>(this->oid);
        writer.write<uint16_t>(this->seq);
        writer.write<uint16_t>(this->action);
        break;

    case list_type::INPUT:
    case list_type::INPUT_NO_MSG:
    case list_type::INPUT_PASSWORD:
    case list_type::INPUT_PASSWORD_NO_MSG:
    case list_type::EMAIL:
        writer.write<uint32_t>(this->oid);
        writer.write<uint16_t>(this->seq);
        if (this->action == 0x02)
        {
            writer.write<uint16_t>(0);
            writer.write<uint8_t>(0x02);
            writer.write<std::string, uint8_t>(this->message);
        }
        else
        {
            writer.write<uint16_t>(this->action);
        }
        break;

    case list_type::LIST:
    case list_type::LIST_NO_MSG:
        writer.write<uint32_t>(this->oid);
        writer.write<uint32_t>(static_cast<uint32_t>(this->button));
        if (this->button == DIALOG_RESULT::NEXT)
        {
            writer.write<uint8_t>(0x01); // client sends 0x01 before index
            writer.write<uint8_t>(this->index);
        }
        break;
    }
}

#endif

} // namespace fb::protocol::game::request
