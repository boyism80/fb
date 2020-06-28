#include "gateway.h"

fb::login::gateway::gateway(const std::string& name, const std::string& desc, uint32_t ip, uint16_t port) : 
    _name(name), _desc(desc), _ip(ip), _port(port)
{ }

fb::login::gateway::gateway(const std::string & name, const std::string & desc, const std::string & ip, uint16_t port) : 
    gateway(name, desc, inet_addr(ip.c_str()), port)
{ }

fb::login::gateway::~gateway()
{ }

const std::string& fb::login::gateway::name() const
{
    return this->_name;
}

const std::string& fb::login::gateway::dest() const
{
    return this->_desc;
}

uint32_t fb::login::gateway::ip() const
{
    return this->_ip;
}

uint16_t fb::login::gateway::port() const
{
    return this->_port;
}

fb::login::gateways::gateways()
{
}

fb::login::gateways::~gateways()
{
    for(auto x : *this)
        delete x;
}

fb::ostream fb::login::gateways::make_stream() const
{
    // ���������� ���̳ʸ� �������� ��ȯ
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

    // ���̳ʸ� ������ ����
    auto                        compressed = formats.compress();

    // ��Ŷ �������� ����
    fb::ostream                 ostream;
    ostream.write_u8(0x56)
        .write_u16(compressed.size())
        .write(compressed.data(), compressed.size() + 1);

    return ostream;
}
