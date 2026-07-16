#include <fb/game/protocol/user_list.h>

namespace fb::protocol::game::request {

#ifdef BOT
void user_list::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0);
}
#else
void user_list::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    auto unknown = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

#ifndef BOT
user_list::user_list(std::vector<user_data>&& users) :
    users(std::move(users))
{ }

void user_list::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>((uint16_t)this->users.size());
    writer.write<uint16_t>((uint16_t)this->users.size());
    writer.write<uint8_t>(0x00);

    for (const auto& user : this->users)
    {
        writer.write<uint8_t>(0x10 * user.nation + user.cls);
        writer.write<uint8_t>(0x10 * user.promotion + user.level);
        writer.write<uint8_t>(user.color);
        writer.write<std::string, uint8_t>(user.name);
    }
}
#else
void user_list::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    auto user_count = reader.read<uint16_t>();
    reader.read<uint16_t>(); // user_count (duplicate)
    reader.read<uint8_t>();  // 0x00

    this->users.clear();
    for (int i = 0; i < user_count; i++)
    {
        user_data user;
        auto      nation_cls = reader.read<uint8_t>();
        user.nation          = nation_cls / 0x10;
        user.cls             = nation_cls % 0x10;

        auto promotion_level = reader.read<uint8_t>();
        user.promotion       = promotion_level / 0x10;
        user.level           = promotion_level % 0x10;

        user.color = reader.read<uint8_t>();
        user.name  = reader.read<std::string, uint8_t>();
        this->users.push_back(user);
    }
}
#endif

} // namespace fb::protocol::game::response
