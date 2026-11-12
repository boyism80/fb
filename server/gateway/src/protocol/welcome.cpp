#include <fb/gateway/protocol/welcome.h>

namespace fb::protocol::gateway::response {

void welcome::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->code = reader.read<uint8_t>();
    this->message.clear();
    while (reader.readable_size() > 0)
    {
        this->message.push_back(static_cast<char>(reader.read<uint8_t>()));
    }
}

} // namespace fb::protocol::gateway::response
