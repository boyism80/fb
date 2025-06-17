#include <fb/game/protocol/bulletin/bulletin_article.h>

namespace fb::protocol::game::response {

#ifndef BOT
bulletin_article::bulletin_article(const fb::game::bulletin::article& value, BULLETIN_BUTTON_ENABLE flag) :
    value(value),
    flag(flag)
{ }
#endif

#ifndef BOT
async::task<void> bulletin_article::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x03);
    writer.write<uint8_t>(static_cast<uint8_t>(flag));
    writer.write<uint8_t>(0x00);
    writer.write<uint16_t>(this->value.id);
    writer.write<std::string>(this->value.uname);
    writer.write<uint8_t>(this->value.month);
    writer.write<uint8_t>(this->value.day);
    writer.write<std::string>(this->value.title);
    writer.write<std::string, uint16_t>(this->value.contents);
    writer.write<uint8_t>(0x00);
}
#endif

} // namespace fb::protocol::game::response