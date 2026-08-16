#include <fb/gateway/protocol/connection_ack.h>
#include <cstring>

namespace fb::protocol::gateway::request {

#ifndef BOT
template <CLIENT_VERSION V>
void connection_ack<V>::deserialize(fb::stream_reader<big_endian>& reader)
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
}

template <>
void connection_ack<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    constexpr size_t max_name_len = 6;
    char             buf[max_name_len]{};
    size_t           n = reader.readable_size();
    if (n > max_name_len)
        n = max_name_len;
    if (n > 0)
        reader.read(buf, n);
    size_t len = 0;
    while (len < n && buf[len] != '\0')
        ++len;
    this->client_name.assign(buf, len);
    while (reader.readable_size() > 0)
        reader.read<uint8_t>();
}
#else
template <CLIENT_VERSION V>
void connection_ack<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    constexpr size_t max_name_len = 6;

    header::serialize(writer);
    writer.write<uint8_t>(opcode);

    const size_t len = std::min(this->client_name.size(), max_name_len - 1);
    writer.write(this->client_name.data(), len);

    uint8_t null_byte = 0;
    writer.write(&null_byte, 1);
}
#endif

template class connection_ack<CLIENT_VERSION::v550>;
template class connection_ack<CLIENT_VERSION::v565>;
template class connection_ack<CLIENT_VERSION::v651>;

} // namespace fb::protocol::gateway::request
