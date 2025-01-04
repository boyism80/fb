#include <fb/game/protocol/click.h>

namespace fb::protocol::game::request {

[[nodiscard]] async::task<void> click::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    auto unknown = reader.read<uint8_t>();
    this->fd     = reader.read<uint32_t>();
}

} // namespace fb::protocol::game::request
