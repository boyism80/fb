#include <fb/gateway/protocol/endpoint.h>

namespace fb::protocol::gateway::request {

#ifdef BOT
endpoint::endpoint(uint8_t action, uint8_t index) :
    action(action),
    index(index)
{ }
#endif

#ifdef BOT
async::task<void> endpoint::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(this->action);

    if (this->action == 0x00)
        writer.write<uint8_t>(this->index);
}
#else
async::task<void> endpoint::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->action = reader.read<uint8_t>();
    if (action == 0x00)
        this->index = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::gateway::request

namespace fb::protocol::gateway::response {

#ifndef BOT
endpoint::endpoint(const std::vector<fb::protocol::gateway::endpoint>& entries) :
    entries(entries)
{ }
#endif

#ifndef BOT
async::task<void> endpoint::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    // 서버정보를 바이너리 형식으로 변환
    auto formats = fb::stream();
    {
        auto writer = fb::stream_writer<big_endian>(formats);
        writer.write<uint8_t>((uint8_t)this->entries.size());
        for (uint32_t i = 0; i < this->entries.size(); i++)
        {
            auto gateway = this->entries.at(i);
            auto buffer  = std::format("{};{}", gateway.name, gateway.desc);

            writer.write<uint8_t>(i);
            writer.write<uint32_t>(gateway.ip);
            writer.write<uint16_t>(gateway.port);
            writer.write(buffer.c_str(), buffer.size() + 1);
        }
    }

    // 바이너리 데이터 압축
    auto compressed = formats.compress();

    // 패킷 형식으로 저장
    writer.write<uint8_t>(header);
    writer.write<uint16_t>(compressed.size());
    writer.write(compressed.data(), compressed.size() + 1);
}
#else
async::task<void> endpoint::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: 파싱해서 데이터 적재
    auto count = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::gateway::response