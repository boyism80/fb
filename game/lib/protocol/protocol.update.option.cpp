#include <fb/game/protocol/update_option.h>

namespace fb::protocol::game::request {

[[nodiscard]] async::task<void> update_option::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->option = static_cast<OPTION>(reader.read<uint8_t>());
    if (this->option == OPTION::EXTENSION)
    {
        this->ride = reader.read<bool>();
    }
}

} // namespace fb::protocol::game::request
