#include <fb/game/protocol/board/board_sections.h>

namespace fb::protocol::game::response {

#ifndef BOT
board_sections::board_sections(const fb::model::model& model) :
    model(model)
{ }
#endif

#ifndef BOT
async::task<void> board_sections::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    auto size = this->model.board.size();

    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x01);
    writer.write<uint16_t>(size);

    for (const auto& [k, v] : this->model.board)
    {
        writer.write<uint16_t>(k);
        writer.write<std::string>(v.name);
    }
}
#else
async::task<void> board_sections::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    reader.read<uint8_t>();
    auto size = reader.read<uint16_t>();
    for (auto i = 0; i < size; i++)
    {
        auto id    = reader.read<uint16_t>();
        auto title = reader.read<std::string, uint8_t>();

        this->boards.push_back(fb::bot::board(id, title));
    }
}
#endif

} // namespace fb::protocol::game::response
