#include <fb/game/protocol/spell/spell_buff.h>

namespace fb::protocol::game::response {

#ifndef BOT
spell_buff::spell_buff(std::string_view name, const fb::model::timespan& duration) :
    name(std::string(name)),
    duration(duration)
{ }
spell_buff::spell_buff(const fb::game::buff& buff) :
    name(buff.model.name),
    duration(buff.remaining())
{ }
#endif

#ifndef BOT
async::task<void> spell_buff::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<std::string>(this->name);
    auto duration_ms = this->duration.total_milliseconds();
    if (duration_ms < 0)
        duration_ms = 0;
    writer.write<uint32_t>(static_cast<uint32_t>(duration_ms / 1000));
}
#else
async::task<void> spell_buff::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->name     = reader.read<std::string, uint8_t>();
    this->duration = fb::model::timespan{std::chrono::seconds(reader.read<uint32_t>())};
}
#endif

} // namespace fb::protocol::game::response
