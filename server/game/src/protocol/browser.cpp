#include <fb/game/protocol/browser.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
browser<V>::browser(uint8_t type) :
    type(type)
{ }

template <CLIENT_VERSION V>
void browser<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->type);
}
#else
template <CLIENT_VERSION V>
void browser<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->type = 0;
    if (reader.readable_size() > 0)
        this->type = reader.read<uint8_t>();
}
#endif

template class browser<CLIENT_VERSION::v550>;
template class browser<CLIENT_VERSION::v565>;
template class browser<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

#ifndef BOT
browser<CLIENT_VERSION::v651>::browser(uint8_t type, std::string url, std::string key, std::string cookie) :
    type(type),
    url(std::move(url)),
    key(std::move(key)),
    cookie(std::move(cookie))
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void browser<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
}

void browser<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->type   = reader.read<uint8_t>();
    this->url    = reader.read<std::string, uint8_t>();
    this->key    = reader.read<std::string, uint8_t>();
    this->cookie = reader.read<std::string, uint8_t>();
}
#else
template <CLIENT_VERSION V>
void browser<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
}

void browser<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->type);
    writer.write<std::string, uint8_t>(this->url);
    writer.write<std::string, uint8_t>(this->key);
    writer.write<std::string, uint8_t>(this->cookie);
}
#endif

template class browser<CLIENT_VERSION::v550>;
template class browser<CLIENT_VERSION::v565>;

} // namespace fb::protocol::game::response
