#include <fb/game/protocol/web.h>

namespace fb::protocol::game::response {

web::web(uint8_t type, std::string address, std::string message) :
    type(type),
    address(address),
    message(message)
{ }

async::task<void> web::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(this->type);
    writer.write<std::string, uint16_t>(this->address);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<uint8_t>(0x00);
}
} // namespace fb::protocol::game::response
