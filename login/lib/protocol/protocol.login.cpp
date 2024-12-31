#include <fb/login/protocol/login.h>

namespace fb::protocol::login::request {

#ifdef BOT
login::login(const std::string& id, const std::string& pw) :
    id(id),
    pw(pw)
{ }
#endif

#ifdef BOT
async::task<void> login::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<std::string, uint8_t>(this->id);
    writer.write<std::string, uint8_t>(this->pw);
}
#else
async::task<void> login::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->id = reader.read<std::string, uint8_t>();
    this->pw = reader.read<std::string, uint8_t>();
}
#endif

} // namespace fb::protocol::login::request
