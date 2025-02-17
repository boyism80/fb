#include <fb/game/protocol/dialog/dialog_ch_list.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_ch_list::dialog_ch_list(const fb::model::npc&           npc,
                               const fb::game::character&      ch,
                               uint16_t                        face,
                               const std::vector<std::string>& list,
                               const std::string&              message,
                               bool                            button_prev,
                               fb::game::dialog::interaction   interaction) :
    npc(npc),
    ch(ch),
    face(face),
    list(list),
    message(message),
    button_prev(button_prev),
    interaction(interaction)
{ }
#endif

#ifndef BOT
async::task<void> dialog_ch_list::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(2);
    writer.write<uint8_t>(static_cast<uint8_t>(interaction));
    writer.write<uint32_t>(this->npc.id);
    writer.write<uint8_t>(0x01);
    writer.write<uint8_t>(0x00); // character = 0x00, npc = 0x01
    writer.write<uint8_t>(static_cast<uint8_t>(this->ch.sex()));
    writer.write<uint8_t>(static_cast<uint8_t>(this->ch.state()));
    writer.write<uint16_t>(this->face);
    writer.write<uint8_t>(this->ch.color());

    auto armor = this->ch.items.armor();
    writer.write<uint8_t>(armor != nullptr ? armor->based<fb::model::armor>().dress : 0xFF);
    writer.write<uint8_t>(this->ch.current_armor_color());

    auto weapon = this->ch.items.weapon();
    writer.write<uint16_t>(weapon != nullptr ? weapon->based<fb::model::weapon>().dress : 0xFFFF);
    writer.write<uint8_t>(weapon != nullptr ? weapon->color() : 0x00);

    auto shield = this->ch.items.shield();
    writer.write<uint16_t>(shield != nullptr ? shield->based<fb::model::shield>().dress : 0xFFFF);
    writer.write<uint8_t>(shield != nullptr ? shield->color() : 0x00);

    writer.write<uint8_t>(0);
    writer.write<uint8_t>(0);
    writer.write<uint8_t>(0);
    writer.write<uint8_t>(0);
    writer.write<uint8_t>(0);
    writer.write<uint8_t>(0);
    writer.write<uint8_t>(1);
    writer.write<uint8_t>(0);
    writer.write<uint8_t>(0);
    writer.write<std::string, uint16_t>(message);
    writer.write<uint8_t>(list.size());
    for (int i = 0; i < list.size(); i++)
    {
        writer.write<std::string>(list[i]);
    }
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> dialog_ch_list::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: deserialize bytes
}
#endif

} // namespace fb::protocol::game::response
