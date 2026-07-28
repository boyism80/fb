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

    case dialog_type::MENU:
    {
        this->oid   = reader.read<uint32_t>();
        this->seq   = reader.read<uint16_t>();
        this->index = reader.read<uint16_t>() + 1;
        break;
    }

    case dialog_type::ITEM:
    {
        this->oid     = reader.read<uint32_t>();
        this->pursuit = reader.read<uint16_t>();
        this->name    = reader.read<std::string, uint8_t>();
        break;
    }

    case dialog_type::SLOT:
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
    {
        this->oid    = reader.read<uint32_t>();
        this->seq    = reader.read<uint16_t>();
        this->action = static_cast<uint8_t>(reader.read<uint16_t>());
        break;
    }

    case list_type::INPUT:
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
        writer.write<uint16_t>(this->seq);
        if (this->message.empty() == false)
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

    case dialog_type::MENU:
        writer.write<uint32_t>(this->oid);
        writer.write<uint16_t>(this->seq);
        writer.write<uint16_t>(this->index - 1);
        break;

    case dialog_type::SLOT:
        writer.write<uint32_t>(this->oid);
        writer.write<uint16_t>(this->pursuit);
        writer.write<uint8_t>(static_cast<uint8_t>(this->index));
        break;

    case dialog_type::ITEM:
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
        writer.write<uint32_t>(this->oid);
        writer.write<uint16_t>(this->seq);
        writer.write<uint16_t>(this->action);
        break;

    case list_type::INPUT:
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
        writer.write<uint32_t>(this->oid);
        writer.write<uint32_t>(static_cast<uint32_t>(this->button));
        if (this->button == DIALOG_RESULT::NEXT)
        {
            writer.write<uint8_t>(0x00);
            writer.write<uint8_t>(this->index);
        }
        break;
    }
}

#endif

} // namespace fb::protocol::game::request
