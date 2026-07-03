#include <fb/gateway/protocol/connection_ack.h>
#include <cstring>

namespace fb::protocol::gateway::request {

#ifndef BOT
async::task<void> connection_ack::deserialize(fb::stream_reader<big_endian>& reader)
{
    constexpr size_t max_name_len = 6;
    char             buf[max_name_len];

    reader.read(buf, max_name_len);
    size_t len = 0;
    while (len < max_name_len && buf[len] != '\0')
    {
        ++len;
    }
    this->client_name.assign(buf, len);
    co_return;
}
#else
async::task<void> connection_ack::serialize(fb::stream_writer<big_endian>& writer) const
{
    constexpr size_t max_name_len = 6;

    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);

    const size_t len = std::min(this->client_name.size(), max_name_len - 1);
    writer.write(this->client_name.data(), len);

    uint8_t null_byte = 0;
    writer.write(&null_byte, 1);
    co_return;
}
#endif

} // namespace fb::protocol::gateway::request
