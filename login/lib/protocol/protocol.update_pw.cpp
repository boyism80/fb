#include <fb/login/protocol/update_pw.h>

namespace fb::protocol::login::request {

#ifdef BOT
update_pw::update_pw(const std::string& name, const std::string& pw, const std::string& new_pw, uint32_t birthday) :
    name(name),
    pw(pw),
    new_pw(new_pw),
    birthday(birthday)
{ }
#endif

#ifdef BOT
async::task<void> update_pw::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<std::string, uint8_t>(this->name);
    writer.write<std::string, uint8_t>(this->pw);
    writer.write<std::string, uint8_t>(this->new_pw);
    writer.write<uint32_t>(this->birthday);
}
#else
async::task<void> update_pw::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->name     = reader.read<std::string, uint8_t>();
    this->pw       = reader.read<std::string, uint8_t>();
    this->new_pw   = reader.read<std::string, uint8_t>();
    this->birthday = reader.read<uint32_t>();
}
#endif

} // namespace fb::protocol::login::request
