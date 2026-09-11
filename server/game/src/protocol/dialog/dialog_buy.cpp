#include <fb/game/protocol/dialog/dialog_buy.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
dialog_buy<V>::dialog_buy(const fb::model::object&             obj,
                          const fb::game::dialog::buy_entries& entries,
                          std::string_view                     message,
                          uint32_t                             oid,
                          uint16_t                             pursuit) :
    appearance(fb::game::appearance_factory::create<V>(obj)),
    entries(entries),
    message(std::string(message)),
    pursuit(pursuit),
    oid(oid)
{ }

template <CLIENT_VERSION V>
dialog_buy<V>::dialog_buy(const fb::game::object&              object,
                          const fb::game::dialog::buy_entries& entries,
                          std::string_view                     message,
                          uint32_t                             oid,
                          uint16_t                             pursuit) :
    appearance(fb::game::appearance_factory::create<V>(object)),
    entries(entries),
    message(std::string(message)),
    pursuit(pursuit),
    oid(oid)
{ }

template <CLIENT_VERSION V>
dialog_buy<V>::dialog_buy(appearance_ptr&&                     appearance,
                          const fb::game::dialog::buy_entries& entries,
                          std::string_view                     message,
                          uint32_t                             oid,
                          uint16_t                             pursuit) :
    appearance(std::move(appearance)),
    entries(entries),
    message(std::string(message)),
    pursuit(pursuit),
    oid(oid)
{ }

template <CLIENT_VERSION V>
void dialog_buy<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    constexpr auto type_value = static_cast<uint8_t>(type);

    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(type_value);
    writer.write<uint8_t>(type_value);
    writer.write<uint32_t>(this->oid);
    this->appearance->serialize(writer);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<uint16_t>(this->pursuit);
    writer.write<uint16_t>((uint16_t)this->entries.size());
    for (auto& entry : this->entries)
    {
        writer.write<std::string>(entry.model.name);
        writer.write<std::string>(std::to_string(entry.price));
    }
}

template <>
void dialog_buy<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    // NEW ui_mode only. Listener sends PURSUIT for v651 OLD (type 10 is NEW_ONLY on client).
    constexpr auto type_value = static_cast<uint8_t>(type);

    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(type_value);
    writer.write<uint8_t>(type_value);
    writer.write<uint32_t>(this->oid);
    this->appearance->serialize(writer);
    writer.write<uint32_t>(0);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<uint16_t>(this->pursuit);
    writer.write<uint16_t>((uint16_t)this->entries.size());
    for (uint16_t i = 0; i < this->entries.size(); i++)
    {
        const auto& entry   = this->entries[i];
        auto        id      = static_cast<uint32_t>(i + 1);
        auto        percent = entry.percent.has_value() ? *entry.percent : uint8_t{0xFF};

        writer.write<uint32_t>(id);
        writer.write<uint16_t>(entry.model.look);
        writer.write<uint8_t>(entry.model.color);
        writer.write<uint32_t>(entry.price);
        writer.write<uint8_t>(entry.count);
        writer.write<std::string>(entry.model.name);
        writer.write<uint8_t>(percent);
    }
}

template class dialog_buy<CLIENT_VERSION::v550>;
template class dialog_buy<CLIENT_VERSION::v565>;
template class dialog_buy<CLIENT_VERSION::v651>;
#else
void dialog_buy::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // first type echo
    this->type_echo = reader.read<uint8_t>();
    this->oid       = reader.read<uint32_t>();
    reader.read<uint8_t>();  // obj type flag
    reader.read<uint8_t>();  // 0x01
    reader.read<uint16_t>(); // look
    reader.read<uint8_t>();  // color
    reader.read<uint8_t>();  // obj type flag
    reader.read<uint16_t>(); // look (duplicate)
    reader.read<uint8_t>();  // color (duplicate)
    this->message = reader.read<std::string, uint16_t>();
    this->pursuit = reader.read<uint16_t>();

    uint16_t entry_count = reader.read<uint16_t>();
    this->entries.clear();
    for (uint16_t i = 0; i < entry_count; i++)
    {
        entry_data entry;
        entry.name  = reader.read<std::string, uint8_t>();
        entry.value = reader.read<std::string, uint8_t>();
        this->entries.push_back(std::move(entry));
    }
}
#endif

} // namespace fb::protocol::game::response
