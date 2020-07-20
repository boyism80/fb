#include "gateway.h"

fb::gateway::host_entry::host_entry(const std::string& name, const std::string& desc, uint32_t ip, uint16_t port) : 
    _name(name), _desc(desc), _ip(ip), _port(port)
{ }

fb::gateway::host_entry::host_entry(const std::string & name, const std::string & desc, const std::string & ip, uint16_t port) : 
    host_entry(name, desc, inet_addr(ip.c_str()), port)
{ }

fb::gateway::host_entry::~host_entry()
{ }

const std::string& fb::gateway::host_entry::name() const
{
    return this->_name;
}

const std::string& fb::gateway::host_entry::dest() const
{
    return this->_desc;
}

uint32_t fb::gateway::host_entry::ip() const
{
    return this->_ip;
}

uint16_t fb::gateway::host_entry::port() const
{
    return this->_port;
}

fb::gateway::host_entries::host_entries()
{
}

fb::gateway::host_entries::~host_entries()
{
    for(auto x : *this)
        delete x;
}

fb::ostream fb::gateway::host_entries::make_stream() const
{
    // 서버정보를 바이너리 형식으로 변환
    ostream                     formats;
    char                        buffer[256];
    formats.write_u8((uint8_t)this->size());
    for(uint32_t i = 0; i < this->size(); i++)
    {
        auto                    gateway = this->at(i);
        uint32_t                len = sprintf(buffer, "%s;%s", gateway->name().c_str(), gateway->dest().c_str()) + 1;

        formats.write_u8(i)
            .write_u32(gateway->ip())
            .write_u16(gateway->port())
            .write(buffer, len);
    }

    // 바이너리 데이터 압축
    auto                        compressed = formats.compress();

    // 패킷 형식으로 저장
    fb::ostream                 ostream;
    ostream.write_u8(0x56)
        .write_u16(compressed.size())
        .write(compressed.data(), compressed.size() + 1);

    return ostream;
}
