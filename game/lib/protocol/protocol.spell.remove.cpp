#include <fb/game/protocol/spell/spell_remove.h>

namespace fb::protocol::game::response {

#ifndef BOT
spell_remove::spell_remove(const fb::game::life& me, uint8_t index) :
    me(me),
    index(index)
{ }
#endif

#ifndef BOT
async::task<void> spell_remove::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    auto spell = this->me.spells.at(index);
    if (spell != nullptr)
        co_return;

    writer.write<uint8_t>(header);
    writer.write<uint8_t>(this->index + 1);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> spell_remove::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: deserialize bytes
}
#endif

} // namespace fb::protocol::game::response
