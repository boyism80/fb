#include <fb/game/protocol/character/position.h>

namespace fb::protocol::game::response {

#ifndef BOT
position::position(const fb::game::character& ch) :
    ch(ch)
{ }
#endif

#ifndef BOT
async::task<void> position::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint16_t>(this->ch.x()); // 실제 x 좌표
    writer.write<uint16_t>(this->ch.y()); // 실제 y 좌표

    auto map = this->ch.map();
    if (map->width() < fb::game::map::MAX_SCREEN_WIDTH)
        writer.write<uint16_t>(this->ch.x() + fb::game::map::HALF_SCREEN_WIDTH - (map->width() / 2));
    else if (this->ch.x() < fb::game::map::HALF_SCREEN_WIDTH)
        writer.write<uint16_t>(this->ch.x());
    else if (this->ch.x() >= map->width() - fb::game::map::HALF_SCREEN_WIDTH)
        writer.write<uint16_t>(this->ch.x() + fb::game::map::MAX_SCREEN_WIDTH - map->width());
    else
        writer.write<uint16_t>(fb::game::map::HALF_SCREEN_WIDTH);

    // 스크린에서의 y 좌표
    if (map->height() < fb::game::map::MAX_SCREEN_HEIGHT)
        writer.write<uint16_t>(this->ch.y() + fb::game::map::HALF_SCREEN_HEIGHT - (map->height() / 2));
    else if (this->ch.y() < fb::game::map::HALF_SCREEN_HEIGHT)
        writer.write<uint16_t>(this->ch.y());
    else if (this->ch.y() >= (map->height() - fb::game::map::HALF_SCREEN_HEIGHT))
        writer.write<uint16_t>(this->ch.y() + fb::game::map::MAX_SCREEN_HEIGHT - map->height());
    else
        writer.write<uint16_t>(fb::game::map::HALF_SCREEN_HEIGHT);

    writer.write<uint8_t>(0x00);
}
#else
async::task<void> position::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->abs.x = reader.read<uint16_t>();
    this->abs.y = reader.read<uint16_t>();
    this->rel.x = reader.read<uint16_t>();
    this->rel.y = reader.read<uint16_t>();
}
#endif

} // namespace fb::protocol::game::response
