#include <fb/game/protocol/dialog/dialog_ch_list.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_ch_list::dialog_ch_list(const fb::model::object&        obj,
                               const fb::game::dialog::preset& preset,
                               const std::vector<std::string>& list,
                               const std::string&              message,
                               bool                            button_prev,
                               uint32_t                        oid,
                               fb::game::dialog::interaction   interaction) :
    obj(obj),
    preset(preset),
    list(list),
    message(message),
    button_prev(button_prev),
    oid(oid),
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
    writer.write<uint32_t>(this->oid);
    writer.write<uint8_t>(0x01);
    writer.write<uint8_t>(0x00); // character = 0x00, obj = 0x01
    writer.write<uint8_t>(static_cast<uint8_t>(this->preset.sex));
    writer.write<uint8_t>(static_cast<uint8_t>(this->preset.state));
    writer.write<uint16_t>(this->preset.face);
    writer.write<uint8_t>(this->preset.hair_color);
    writer.write<uint8_t>(this->preset.armor.value_or(static_cast<uint8_t>(this->preset.sex)));
    writer.write<uint8_t>(this->preset.armor_color.value_or(0x00));
    writer.write<uint16_t>(this->preset.weapon.value_or(0xFFFF));
    writer.write<uint8_t>(this->preset.weapon_color.value_or(0x00));
    writer.write<uint16_t>(this->preset.shield.value_or(0xFFFF));
    writer.write<uint8_t>(this->preset.shield_color.value_or(0x00));
    writer.write<uint32_t>(0);
    writer.write<uint16_t>(0);
    writer.write<uint8_t>(1);
    writer.write<uint8_t>(this->button_prev);
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
    reader.read<uint8_t>(); // 2
    this->interaction = reader.read<uint8_t>();
    this->oid         = reader.read<uint32_t>();
    reader.read<uint8_t>(); // 0x01
    reader.read<uint8_t>(); // 0x00
    this->sex          = reader.read<uint8_t>();
    this->state        = reader.read<uint8_t>();
    this->face         = reader.read<uint16_t>();
    this->hair_color   = reader.read<uint8_t>();
    this->armor        = reader.read<uint8_t>();
    this->armor_color  = reader.read<uint8_t>();
    this->weapon       = reader.read<uint16_t>();
    this->weapon_color = reader.read<uint8_t>();
    this->shield       = reader.read<uint16_t>();
    this->shield_color = reader.read<uint8_t>();
    reader.read<uint32_t>(); // 0
    reader.read<uint16_t>(); // 0
    reader.read<uint8_t>();  // 1
    reader.read<uint16_t>(); // 0
    this->message      = reader.read<std::string, uint16_t>();
    uint8_t list_count = reader.read<uint8_t>();

    this->list.clear();
    for (int i = 0; i < list_count; i++)
    {
        this->list.push_back(reader.read<std::string, uint8_t>());
    }
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
