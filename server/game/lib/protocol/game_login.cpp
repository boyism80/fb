#include <fb/game/protocol/login.h>

namespace fb::protocol::game::request {

#ifdef BOT
login::login(const fb::stream& params)
{
    auto clone  = fb::stream{params};
    auto reader = fb::stream_reader<big_endian>{clone};
    async::awaitable_get(this->deserialize(reader));
}
#endif

async::task<void> login::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(this->enc_type);
    writer.write<uint8_t>(this->key_size);
    writer.write((void*)this->enc_key, this->key_size);
    writer.write<uint8_t>(static_cast<uint8_t>(this->from));
    writer.write<uint32_t>(this->id);
    writer.write<std::string, uint8_t>(this->name);
    writer.write<bool>(this->transfer.has_value());

    if (transfer.has_value())
    {
        writer.write<uint16_t>(this->transfer.value().map);
        writer.write<uint16_t>(this->transfer.value().position.x);
        writer.write<uint16_t>(this->transfer.value().position.y);
    }
}

async::task<void> login::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // base
    this->enc_type = reader.read<uint8_t>();
    this->key_size = reader.read<uint8_t>();
    reader.read((void*)this->enc_key, this->key_size);
#ifndef BOT
    this->from = static_cast<fb::protocol::internal::Service>(reader.read<uint8_t>());
#else
    this->from = reader.read<uint8_t>();
#endif

    // additional parameters
    this->id   = reader.read<uint32_t>();
    this->name = reader.read<std::string, uint8_t>();
    if (reader.read<bool>())
    {
        auto map       = reader.read<uint16_t>();
        auto x         = reader.read<uint16_t>();
        auto y         = reader.read<uint16_t>();
        this->transfer = transfer_param{.map = map, .position = fb::model::point<uint16_t>(x, y)};
    }
}

} // namespace fb::protocol::game::request