#ifndef __PROTOCOL_RESPONSE_GATEWAY_H__
#define __PROTOCOL_RESPONSE_GATEWAY_H__

#include <fb/protocol/protocol.h>
#include <fb/cryptor.h>
#include <format>
#ifndef _WIN32
#include <arpa/inet.h>
#endif

namespace fb { namespace protocol { namespace gateway {

class entry
{
public:
    std::string name;
    std::string desc;
    uint32_t    ip   = 0;
    uint16_t    port = 0;

public:
    entry(const std::string& name, const std::string& desc, uint32_t ip, uint16_t port) :
        name(name),
        desc(desc),
        ip(ip),
        port(port)
    { }

    entry(const std::string& name, const std::string& desc, const std::string& ip, uint16_t port) :
        entry(name, desc, inet_addr(ip.c_str()), port)
    { }

    entry(const entry& right) :
        entry(right.name, right.desc, right.ip, right.port)
    { }

    ~entry()
    { }
};

}}} // namespace fb::protocol::gateway

namespace fb { namespace protocol { namespace gateway { namespace response {

class welcome : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x7E;

public:
    welcome() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader)
    { }
};

class crt : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x00;

public:
#ifdef BOT
    fb::cryptor cryptor;
    uint32_t    entry_crc;
#else
    const fb::cryptor cryptor;
    const uint32_t    entry_crc;
#endif

public:
#ifdef BOT
    crt() = default;
#else
    crt(const fb::cryptor& cryptor, uint32_t entry_crc) :
        cryptor(cryptor),
        entry_crc(entry_crc)
    { }
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x00);
        writer.write<uint32_t>(this->entry_crc);
        writer.write<uint8_t>(this->cryptor.type());
        writer.write<uint8_t>(0x09);
        writer.write(this->cryptor.key(), 0x09);
        writer.write<uint8_t>(0x00);
    }
#else
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        reader.read<uint8_t>();

        auto entry_crc = reader.read<uint32_t>();
        auto enc_type  = reader.read<uint8_t>();
        auto size      = reader.read<uint8_t>();
        auto enc_key   = new uint8_t[size];
        reader.read(enc_key, size);
        this->cryptor = fb::cryptor(enc_type, enc_key);
        delete[] enc_key;
    }
#endif
};

class hosts : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x56;

public:
#ifdef BOT
    std::vector<entry> entries;
#else
    const std::vector<entry>& entries;
#endif

public:
#ifdef BOT
    hosts() = default;
#else
    hosts(const std::vector<entry>& entries) :
        entries(entries)
    { }
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
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
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        // TODO: 파싱해서 데이터 적재
        auto count = reader.read<uint8_t>();
    }
#endif
};

}}}} // namespace fb::protocol::gateway::response

#endif // !__PROTOCOL_RESPONSE_GATEWAY_H__