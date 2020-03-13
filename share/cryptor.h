#ifndef __FB_CRYPTOR_H__
#define __FB_CRYPTOR_H__

#include "stream.h"

namespace fb {

class cryptor
{
public:
    static uint32_t         HEX_TABLE[10][0x100];

private:
    uint8_t                 _type;
    uint8_t*                _key;
    uint8_t                 _sequence;

public:
    cryptor();
    cryptor(uint8_t types, const uint8_t* key);
    cryptor(const cryptor& crt);
    ~cryptor();

private:
    void                    intercrypt(const uint8_t *core, uint8_t *dest, uint32_t size, const uint8_t *key, uint32_t ksize);

public:
    uint32_t                encrypt(buffer& data, uint32_t offset, uint32_t size);
    uint32_t                encrypt(buffer& data);
    uint32_t                decrypt(buffer& data, uint32_t offset, uint32_t size);
    uint32_t                decrypt(buffer& data);
    uint32_t                wrap(buffer& data, uint32_t offset);
    uint32_t                wrap(buffer& data);
    uint32_t                unwrap(buffer& data, uint32_t offset);
    uint32_t                unwrap(buffer& data);

public:
    uint8_t                 types() const;
    uint8_t*                key() const;

public:
    cryptor&                operator = (const cryptor& crt);
};

}

#endif // !__FB_CRYPTOR_H__
