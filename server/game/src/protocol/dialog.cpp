#include <fb/game/protocol/dialog.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
dialog<V>::dialog(fb::game::dialog::type type,
                  uint8_t                action,
                  std::string            message,
                  uint16_t               index,
                  uint16_t               pursuit,
                  std::string            name,
                  uint32_t               oid,
                  uint16_t               seq,
                  std::string            ext) :
    type(type),
    oid(oid),
    seq(seq),
    action(action),
    message(std::move(message)),
    index(index),
    pursuit(pursuit),
    name(std::move(name)),
    ext(std::move(ext))
{ }

dialog<CLIENT_VERSION::v651>::dialog(fb::game::dialog::type type,
                                     uint8_t                action,
                                     std::string            message,
                                     uint16_t               index,
                                     uint16_t               pursuit,
                                     std::string            name,
                                     uint32_t               oid,
                                     uint16_t               seq,
                                     std::string            ext,
                                     uint32_t               item_value) :
    type(type),
    oid(oid),
    seq(seq),
    action(action),
    message(std::move(message)),
    index(index),
    pursuit(pursuit),
    name(std::move(name)),
    ext(std::move(ext)),
    item_value(item_value)
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
dialog_list<V>::dialog_list(fb::game::dialog::list_type type,
                            uint8_t                     action,
                            std::string                 message,
                            uint16_t                    index,
                            DIALOG_RESULT               button,
                            uint32_t                    oid,
                            uint16_t                    seq) :
    type(type),
    oid(oid),
    seq(seq),
    action(action),
    message(std::move(message)),
    index(index),
    button(button)
{ }
#endif

#ifndef BOT

template <CLIENT_VERSION V>
void dialog<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    using dialog_type = fb::game::dialog::type;

    header::deserialize(reader);
    this->type = static_cast<dialog_type>(reader.read<uint8_t>());

    switch (this->type)
    {
    case dialog_type::INPUT:
    {
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
        this->oid     = reader.read<uint32_t>();
        this->pursuit = reader.read<uint16_t>();
        this->seq     = this->pursuit;
        this->message = reader.read<std::string, uint8_t>();
        this->action  = 0x02;
        break;
    }

    case dialog_type::MENU:
    {
        this->oid   = reader.read<uint32_t>();
        this->index = reader.read<uint16_t>() + 1;
        this->ext   = reader.read<std::string, uint8_t>();
        break;
    }

    case dialog_type::MENU_NO_EXT:
    {
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

void dialog<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    using dialog_type = fb::game::dialog::type;

    header::deserialize(reader);
    this->type = static_cast<dialog_type>(reader.read<uint8_t>());

    switch (this->type)
    {
    case dialog_type::INPUT:
    {
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
        this->oid     = reader.read<uint32_t>();
        this->pursuit = reader.read<uint16_t>();
        this->seq     = this->pursuit;
        this->message = reader.read<std::string, uint8_t>();
        this->action  = 0x02;
        break;
    }

    case dialog_type::MENU:
    {
        this->oid   = reader.read<uint32_t>();
        this->index = reader.read<uint16_t>() + 1;
        this->ext   = reader.read<std::string, uint8_t>();
        break;
    }

    case dialog_type::MENU_NO_EXT:
    {
        this->oid   = reader.read<uint32_t>();
        this->index = reader.read<uint16_t>() + 1;
        break;
    }

    case dialog_type::ITEM:
    {
        this->oid     = reader.read<uint32_t>();
        this->pursuit = reader.read<uint16_t>();
        reader.read<uint8_t>();
        this->item_value = reader.read<uint32_t>();
        this->index      = reader.read<uint8_t>();
        break;
    }

    case dialog_type::PURSUIT:
    {
        this->oid   = reader.read<uint32_t>();
        this->index = reader.read<uint16_t>() + 1;
        break;
    }

    case dialog_type::DUAL_FIELD:
    {
        this->oid     = reader.read<uint32_t>();
        this->pursuit = reader.read<uint16_t>();
        reader.read<uint8_t>();
        this->item_value = reader.read<uint32_t>();
        this->index      = reader.read<uint8_t>();
        this->name       = std::to_string(this->item_value);
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

template <CLIENT_VERSION V>
void dialog_list<V>::deserialize(fb::stream_reader<big_endian>& reader)
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

template <>
void dialog_list<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
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
        // 6.51: [oid u32][seq u16][u16] then NEXT adds [1][index u8] (send 12).
        this->oid = reader.read<uint32_t>();
        this->seq = reader.read<uint16_t>();
        auto code = reader.read<uint16_t>();
        if (reader.readable_size() >= 2)
        {
            reader.read<uint8_t>();
            this->index  = reader.read<uint8_t>();
            this->button = DIALOG_RESULT::NEXT;
        }
        else if (code == static_cast<uint16_t>(DIALOG_RESULT::PREV) ||
                 code == static_cast<uint16_t>(DIALOG_RESULT::QUIT) ||
                 code == static_cast<uint16_t>(DIALOG_RESULT::NEXT))
        {
            this->button = static_cast<DIALOG_RESULT>(code);
        }
        else
        {
            this->button = DIALOG_RESULT::QUIT;
        }
        break;
    }
    }
}

#else

template <CLIENT_VERSION V>
void dialog<V>::serialize(fb::stream_writer<big_endian>& writer) const
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

void dialog<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
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
        writer.write<uint32_t>(this->oid);
        writer.write<uint16_t>(this->pursuit);
        writer.write<uint8_t>(1);
        writer.write<uint32_t>(this->item_value);
        writer.write<uint8_t>(static_cast<uint8_t>(this->index));
        break;

    case dialog_type::PURSUIT:
        writer.write<uint32_t>(this->oid);
        writer.write<uint16_t>(this->index - 1);
        break;

    case dialog_type::DUAL_FIELD:
        writer.write<uint32_t>(this->oid);
        writer.write<uint16_t>(this->pursuit);
        writer.write<uint8_t>(1);
        writer.write<uint32_t>(this->item_value);
        writer.write<uint8_t>(static_cast<uint8_t>(this->index));
        break;
    }
}

template <CLIENT_VERSION V>
void dialog_list<V>::serialize(fb::stream_writer<big_endian>& writer) const
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
            writer.write<uint8_t>(0x01);
            writer.write<uint8_t>(this->index);
        }
        break;
    }
}

#endif

template class dialog<CLIENT_VERSION::v550>;
template class dialog<CLIENT_VERSION::v565>;
template class dialog_list<CLIENT_VERSION::v550>;
template class dialog_list<CLIENT_VERSION::v565>;
template class dialog_list<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
