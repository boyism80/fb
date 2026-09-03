#include <fb/game/protocol/login.h>
#include <stdexcept>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
login<V>::login(const fb::stream& params)
{
    auto clone  = fb::stream{params};
    auto reader = fb::stream_reader<big_endian>{clone};
    this->deserialize(reader);
}
#endif

template <CLIENT_VERSION V>
void login<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->enc_type);
    writer.write<uint8_t>(this->key_size);
    writer.write((void*)this->enc_key, this->key_size);
    writer.write<uint8_t>(static_cast<uint8_t>(this->from));
    writer.write<uint16_t>(static_cast<uint16_t>(this->client_version));
    writer.write<uint32_t>(this->id);
    writer.write<std::string, uint8_t>(this->name);
    writer.write<bool>(this->transfer.has_value());

    if (transfer.has_value())
    {
        writer.write<uint32_t>(this->transfer.value().world);
        writer.write<uint16_t>(this->transfer.value().map);
        writer.write<uint16_t>(this->transfer.value().position.x);
        writer.write<uint16_t>(this->transfer.value().position.y);
    }

    if (this->client_version == CLIENT_VERSION::v651 && this->ui_mode == CLIENT_UI_MODE::NEW)
        writer.write<uint8_t>(static_cast<uint8_t>(this->ui_mode));
}

template <CLIENT_VERSION V>
void login<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->enc_type = reader.read<uint8_t>();
    this->key_size = reader.read<uint8_t>();
    reader.read((void*)this->enc_key, this->key_size);
#ifndef BOT
    this->from = static_cast<fb::protocol::internal::Service>(reader.read<uint8_t>());
#else
    this->from = reader.read<uint8_t>();
#endif

    auto packed = reader.read<uint16_t>();
    if (is_supported(packed) == false)
        throw std::runtime_error("unsupported client version in login transfer");
    this->client_version = static_cast<CLIENT_VERSION>(packed);

    this->id   = reader.read<uint32_t>();
    this->name = reader.read<std::string, uint8_t>();
    if (reader.read<bool>())
    {
        auto world     = reader.read<uint32_t>();
        auto map       = reader.read<uint16_t>();
        auto x         = reader.read<uint16_t>();
        auto y         = reader.read<uint16_t>();
        this->transfer = transfer_param{.world = world, .map = map, .position = fb::model::point<uint16_t>(x, y)};
    }

    if (this->client_version == CLIENT_VERSION::v651 && reader.readable_size() >= 1)
    {
        this->ui_mode = static_cast<CLIENT_UI_MODE>(reader.read<uint8_t>());
    }
    else
    {
        this->ui_mode = CLIENT_UI_MODE::OLD;
    }
}

template class login<CLIENT_VERSION::v550>;
template class login<CLIENT_VERSION::v565>;
template class login<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
