#include <fb/game/protocol/update_cc.h>

namespace fb::protocol::game::response {

#ifndef BOT
void update_cc::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<bool>(this->life.cc.contains(CROWD_CONTROL::DIRECTION));
    writer.write<bool>(this->life.cc.contains(CROWD_CONTROL::SIGHT));
    writer.write<bool>(this->life.cc.contains(CROWD_CONTROL::HEAR));
    writer.write<bool>(this->life.cc.contains(CROWD_CONTROL::CHAT));
}
#else
void update_cc::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    auto d   = reader.read<bool>();
    auto s   = reader.read<bool>();
    auto h   = reader.read<bool>();
    auto c   = reader.read<bool>();
    this->cc = static_cast<CROWD_CONTROL>((d ? static_cast<uint32_t>(CROWD_CONTROL::DIRECTION) : 0) |
                                          (s ? static_cast<uint32_t>(CROWD_CONTROL::SIGHT) : 0) |
                                          (h ? static_cast<uint32_t>(CROWD_CONTROL::HEAR) : 0) |
                                          (c ? static_cast<uint32_t>(CROWD_CONTROL::CHAT) : 0));
}
#endif

} // namespace fb::protocol::game::response
