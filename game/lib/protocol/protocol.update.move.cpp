#include <fb/game/protocol/update_move.h>

namespace fb::protocol::game::request {

async::task<void> update_move::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    co_await move::deserialize(reader);

    this->begin.x     = reader.read<uint16_t>();
    this->begin.y     = reader.read<uint16_t>();
    this->size.width  = reader.read<uint8_t>();
    this->size.height = reader.read<uint8_t>();
    this->crc         = reader.read<uint16_t>();
}

} // namespace fb::protocol::game::request
