#include <fb/game/protocol/user_list.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
void user_list<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->unused);
}
#else
template <CLIENT_VERSION V>
void user_list<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    // Optional filler; real client often omits it (opcode-only).
    if (reader.readable_size() > 0)
        this->unused = reader.read<uint8_t>();
}
#endif

template class user_list<CLIENT_VERSION::v550>;
template class user_list<CLIENT_VERSION::v565>;
template class user_list<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

#ifndef BOT
user_list::user_list(std::vector<user_data>&& users, SORT_TYPE sort) :
    sort(sort),
    users(std::move(users))
{ }

void user_list::serialize(fb::stream_writer<big_endian>& writer) const
{
    constexpr auto head_size = sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint8_t);

    auto entries      = std::vector<uint8_t>();
    auto entry_writer = fb::stream_writer<big_endian>(entries);
    auto count        = uint16_t(0);
    for (const auto& user : this->users)
    {
        auto before = entries.size();
        entry_writer.write<uint8_t>(0x10 * user.nation + user.cls);
        entry_writer.write<uint8_t>(0x10 * user.promotion + user.level);
        entry_writer.write<uint8_t>(user.color);
        entry_writer.write<std::string, uint8_t>(user.name);
        if (head_size + entries.size() > fb::socket<>::MAX_PAYLOAD_SIZE)
        {
            entries.resize(before);
            break;
        }

        count++;
    }

    // The client shows the first count as the online total and parses exactly the second count of entries.
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>(static_cast<uint16_t>(std::min<size_t>(this->users.size(), UINT16_MAX)));
    writer.write<uint16_t>(count);
    writer.write<uint8_t>(static_cast<uint8_t>(this->sort));
    writer.write(entries.data(), entries.size());
}
#else
void user_list::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint16_t>(); // online total
    auto user_count = reader.read<uint16_t>();
    this->sort      = static_cast<SORT_TYPE>(reader.read<uint8_t>());

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
