#include <fb/game/protocol/spell/spell_update.h>

namespace fb::protocol::game::response {

#ifndef BOT
spell_update::spell_update(const fb::game::life& me, uint8_t index) :
    me(me),
    index(index)
{ }
#endif

#ifndef BOT
async::task<void> spell_update::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    auto spell = this->me.spells.at(index);
    if (spell == nullptr)
        co_return;

    writer.write<uint8_t>(header);
    writer.write<uint8_t>(this->index + 1);
    writer.write<uint8_t>(static_cast<uint8_t>(spell->model.type));
    writer.write<std::string>(spell->model.name);

    if (static_cast<int>(spell->model.type) < 3)
        writer.write<std::string>(spell->model.message);
}
#else
async::task<void> spell_update::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->index = reader.read<uint8_t>() - 1;
    this->type  = reader.read<uint8_t>();
    this->name  = reader.read<std::string, uint8_t>();
    if (type < 3)
        this->message = reader.read<std::string, uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
