#include <fb/gateway/protocol/server_list.h>

namespace fb::protocol::gateway::request {

#ifndef BOT
async::task<void> endpoint::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->action = reader.read<uint8_t>();
    if (action == 0x00)
        this->index = reader.read<uint8_t>();
}
#else
endpoint::endpoint(uint8_t action, uint8_t index) :
    action(action),
    index(index)
{ }

async::task<void> endpoint::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(this->action);

    if (this->action == 0x00)
        writer.write<uint8_t>(this->index);
}
#endif

} // namespace fb::protocol::gateway::request

namespace fb::protocol::gateway::response {

#ifndef BOT
server_list::server_list(const std::vector<fb::protocol::gateway::endpoint>& servers) :
    servers(servers)
{ }
#endif

#ifndef BOT
async::task<void> server_list::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    // 서버정보를 바이너리 형식으로 변환
    auto formats = fb::stream();
    {
        auto writer = fb::stream_writer<big_endian>(formats);
        writer.write<uint8_t>((uint8_t)this->servers.size());
        for (uint32_t i = 0; i < this->servers.size(); i++)
        {
            auto gateway = this->servers.at(i);
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
    writer.write(compressed.data(), compressed.size());
    writer.write<uint8_t>(0);
}
#else
async::task<void> server_list::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: 파싱해서 데이터 적재
    auto count = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::gateway::response
