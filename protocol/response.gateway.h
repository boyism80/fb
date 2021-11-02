#ifndef __PROTOCOL_RESPONSE_GATEWAY_H__
#define __PROTOCOL_RESPONSE_GATEWAY_H__

#include "protocol/protocol.h"
#include "model/gateway/gateway.h"
#include "module/cryptor/cryptor.h"

namespace fb { namespace protocol { namespace gateway { namespace response {

class crt : public fb::protocol::base::header
{
public:
    const fb::cryptor       cryptor;
    const uint32_t          entry_crc;

public:
    crt(const fb::cryptor& cryptor, uint32_t entry_crc) : 
        cryptor(cryptor), entry_crc(entry_crc)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x00)      // cmd : 0x00
                  .write_u8(0x00)
                  .write_u32(this->entry_crc)
                  .write_u8(this->cryptor.type())
                  .write_u8(0x09)
                  .write(this->cryptor.key(), 0x09)
                  .write_u8(0x00);
    }
};

class hosts : public fb::protocol::base::header
{
public:
    const std::vector<fb::gateway::entry>&  entries;

public:
    hosts(const std::vector<fb::gateway::entry>& entries) : 
        entries(entries)
    { }

    void serialize(fb::ostream& out_stream) const
    {
        // 서버정보를 바이너리 형식으로 변환
        fb::ostream                 formats;
        char                        buffer[256];
        formats.write_u8((uint8_t)this->entries.size());
        for(uint32_t i = 0; i < this->entries.size(); i++)
        {
            auto                    gateway = this->entries.at(i);
            uint32_t                len = sprintf(buffer, "%s;%s", gateway.name().c_str(), gateway.dst().c_str()) + 1;

            formats.write_u8(i)
                .write_u32(gateway.ip())
                .write_u16(gateway.port())
                .write(buffer, len);
        }

        // 바이너리 데이터 압축
        auto                        compressed = formats.compress();

        // 패킷 형식으로 저장
        out_stream.write_u8(0x56)
                  .write_u16(compressed.size())
                  .write(compressed.data(), compressed.size() + 1);
    }
};

} } } }

#endif // !__PROTOCOL_RESPONSE_GATEWAY_H__