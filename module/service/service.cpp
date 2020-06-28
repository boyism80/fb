#include "service.h"

fb::service::service()
{ }

fb::service::~service()
{ }

fb::ostream fb::service::make_transfer_stream(const fb::cryptor& crt, uint32_t ip, uint16_t port, const fb::ostream* parameter)
{
    fb::ostream             ostream;
    auto                    backward_size = parameter != nullptr ? 0x0B + parameter->size() : 0x0B;
    ostream.write_u8(0x03)
        .write_u32(ip)                      // ip
        .write_u16(port)                    // port
        .write_u8(uint8_t(backward_size))   // backward size
        .write_u8(crt.type())
        .write_u8(cryptor::KEY_SIZE)
        .write(crt.key(), cryptor::KEY_SIZE);

    if(parameter != nullptr)
        ostream.write(*parameter);

    return ostream;
}
