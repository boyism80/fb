#include <fb/game/protocol/spell/spell_buff.h>

namespace fb::protocol::game::response {

#ifndef BOT
spell_buff::spell_buff(std::string_view name, uint32_t time) :
    name(std::string(name)),
    time(time)
{ }
spell_buff::spell_buff(const fb::game::buff& buff) :
    name(buff.model.name),
    time(buff.time())
{ }
#endif

#ifndef BOT
async::task<void> spell_buff::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<std::string>(this->name);
    writer.write<uint32_t>(static_cast<uint32_t>(this->time.count() / 1000));
}
#else
async::task<void> spell_buff::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->name = reader.read<std::string, uint8_t>();
    this->time = std::chrono::milliseconds(reader.read<uint32_t>() * 1000);
}
#endif

} // namespace fb::protocol::game::response
