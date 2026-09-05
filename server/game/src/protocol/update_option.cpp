#include <fb/game/protocol/update_option.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
update_option<V>::update_option(OPTION option, bool ride) :
    option(option),
    ride(ride)
{ }

update_option<CLIENT_VERSION::v651>::update_option(OPTION option, bool ride) :
    options{option},
    ride(ride)
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void update_option<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->option));
    if (this->option == OPTION::EXTENSION)
        writer.write<bool>(this->ride);
}

void update_option<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->options.size()));
    for (auto option : this->options)
        writer.write<uint8_t>(static_cast<uint8_t>(option));
    if (this->options.size() == 1 && this->options.front() == OPTION::EXTENSION)
        writer.write<bool>(this->ride);
}
#else
template <CLIENT_VERSION V>
void update_option<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->option = static_cast<OPTION>(reader.read<uint8_t>());
    if (this->option == OPTION::EXTENSION)
        this->ride = reader.read<bool>();
}

void update_option<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->options.clear();
    this->ride = false;

    auto count = reader.read<uint8_t>();
    if (count == 0 || reader.readable_size() < count)
        return;

    this->options.reserve(count);
    for (uint8_t i = 0; i < count; ++i)
        this->options.push_back(static_cast<OPTION>(reader.read<uint8_t>()));

    if (this->options.size() == 1 && this->options.front() == OPTION::EXTENSION && reader.readable_size() >= 1)
        this->ride = reader.read<bool>();
    else
    {
        while (reader.readable_size() > 0)
            reader.read<uint8_t>();
    }
}
#endif

template class update_option<CLIENT_VERSION::v550>;
template class update_option<CLIENT_VERSION::v565>;

} // namespace fb::protocol::game::request
