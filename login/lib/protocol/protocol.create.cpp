#include <fb/login/protocol/create.h>

namespace fb::protocol::login::request {

#ifndef BOT
async::task<void> create::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->id = reader.read<std::string, uint8_t>();
    this->pw = reader.read<std::string, uint8_t>();
}
#else
create::create(const std::string& id, const std::string& pw) :
    id(id),
    pw(pw)
{ }

async::task<void> create::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<std::string, uint8_t>(this->id);
    writer.write<std::string, uint8_t>(this->pw);
}
#endif

} // namespace fb::protocol::login::request
