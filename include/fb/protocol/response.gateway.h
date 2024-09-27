#ifndef __PROTOCOL_RESPONSE_GATEWAY_H__
#define __PROTOCOL_RESPONSE_GATEWAY_H__

#include <fb/protocol/protocol.h>
#include <fb/cryptor.h>
#ifndef _WIN32
#include <arpa/inet.h>
#endif

namespace fb { namespace protocol { namespace gateway { 

class entry
{
public:
    std::string             name;
    std::string             desc;
    uint32_t                ip   = 0;
    uint16_t                port = 0;

public:
    entry(const std::string& name, const std::string& desc, uint32_t ip, uint16_t port) : 
        name(name), desc(desc), ip(ip), port(port)
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

} } }

namespace fb { namespace protocol { namespace gateway { namespace response {

class welcome : public fb::protocol::base::header
{
public:
    welcome() : fb::protocol::base::header(0x7E)
    {}

public:
    void deserialize(fb::istream& in_stream)
    {
        
    }
};

class crt : public fb::protocol::base::header
{
public:
#ifdef BOT
    fb::cryptor             cryptor;
    uint32_t                entry_crc;
#else
    const fb::cryptor       cryptor;
    const uint32_t          entry_crc;
#endif

public:
#ifdef BOT
    crt() : fb::protocol::base::header(0x00)
    { }
#else
    crt(const fb::cryptor& cryptor, uint32_t entry_crc) : fb::protocol::base::header(0x00),
        cryptor(cryptor), entry_crc(entry_crc)
    { }
#endif

public:
#ifndef BOT
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(0x00)
                  .write_u32(this->entry_crc)
                  .write_u8(this->cryptor.type())
                  .write_u8(0x09)
                  .write(this->cryptor.key(), 0x09)
                  .write_u8(0x00);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        in_stream.read_u8();

        auto entry_crc = in_stream.read_u32();
        auto enc_type = in_stream.read_u8();
        auto size = in_stream.read_u8();
        auto enc_key = new uint8_t[size];
        in_stream.read(enc_key, size);
        this->cryptor = fb::cryptor(enc_type, enc_key);
        delete[] enc_key;
    }
#endif
};

class hosts : public fb::protocol::base::header
{
public:
#ifdef BOT
    std::vector<entry>         entries;
#else
    const std::vector<entry>&  entries;
#endif

public:
#ifdef BOT
    hosts() : fb::protocol::base::header(0x56)
    { }
#else
    hosts(const std::vector<entry>& entries) : fb::protocol::base::header(0x56),
        entries(entries)
    { }
#endif

public:
#ifndef BOT
    void serialize(fb::ostream& out_stream) const
    {
        // 서버정보를 바이너리 형식으로 변환
        fb::ostream                 formats;
        char                        buffer[256];
        formats.write_u8((uint8_t)this->entries.size());
        for(uint32_t i = 0; i < this->entries.size(); i++)
        {
            auto                    gateway = this->entries.at(i);
            uint32_t                len = sprintf(buffer, "%s;%s", gateway.name.c_str(), gateway.desc.c_str()) + 1;

            formats.write_u8(i)
                .write_u32(gateway.ip)
                .write_u16(gateway.port)
                .write(buffer, len);
        }

        // 바이너리 데이터 압축
        auto                        compressed = formats.compress();

        // 패킷 형식으로 저장
        base::header::serialize(out_stream);
        out_stream.write_u16(compressed.size())
                  .write(compressed.data(), compressed.size() + 1);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        // TODO: 파싱해서 데이터 적재
        auto count = in_stream.read_u8();
    }
#endif
};

} } } }

#endif // !__PROTOCOL_RESPONSE_GATEWAY_H__