#include <fb/game/protocol/bulletin/bulletin_sections.h>

using table = fb::model::table;

namespace fb::protocol::game::response {

#ifndef BOT
bulletin_sections::bulletin_sections()
{ }
#endif

#ifndef BOT
async::task<void> bulletin_sections::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    auto size = table::bulletin.size();

    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x01);
    writer.write<uint16_t>(size);

    for (const auto& [k, v] : table::bulletin)
    {
        writer.write<uint16_t>(k);
        writer.write<std::string>(v.name);
    }
}
#else
async::task<void> bulletin_sections::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    reader.read<uint8_t>();
    auto size = reader.read<uint16_t>();
    for (auto i = 0; i < size; i++)
    {
        auto id    = reader.read<uint16_t>();
        auto title = reader.read<std::string, uint8_t>();

        this->bulletins.push_back(fb::bot::bulletin(id, title));
    }
}
#endif

} // namespace fb::protocol::game::response