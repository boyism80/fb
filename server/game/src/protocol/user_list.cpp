#include <fb/game/protocol/user_list.h>

namespace fb::protocol::game::request {

#ifdef BOT
void user_list::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->unused);
}
#else
void user_list::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    // Optional filler; real client often omits it (opcode-only).
    if (reader.readable_size() > 0)
        this->unused = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

#ifndef BOT
user_list::user_list(std::vector<user_data>&& users, SORT_TYPE sort) :
    sort(sort),
    users(std::move(users))
{ }

void user_list::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>((uint16_t)this->users.size());
    writer.write<uint16_t>((uint16_t)this->users.size());
    writer.write<uint8_t>(static_cast<uint8_t>(this->sort));

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
    this->sort = static_cast<SORT_TYPE>(reader.read<uint8_t>());

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
