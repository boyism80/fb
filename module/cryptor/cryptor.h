#ifndef __FB_CRYPTOR_H__
#define __FB_CRYPTOR_H__

#include "module/stream/stream.h"

namespace fb {

class cryptor
{
public:
    static uint32_t             HEX_TABLE[10][0x100];
    static constexpr uint32_t   KEY_SIZE = 0x09;

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
    void                    crypt(const uint8_t *source, uint8_t *dest, uint32_t size, const uint8_t *key, uint32_t ksize);

public:
    uint32_t                encrypt(buffer& data, uint32_t offset, uint32_t size);
    uint32_t                encrypt(buffer& data);
    uint32_t                decrypt(buffer& data, uint32_t offset, uint32_t size);
    uint32_t                decrypt(buffer& data);
    uint32_t                wrap(buffer& data, uint32_t offset) const;
    uint32_t                wrap(buffer& data) const;
    uint32_t                unwrap(buffer& data, uint32_t offset) const;
    uint32_t                unwrap(buffer& data) const;

public:
    uint8_t                 type() const;
    uint8_t*                key() const;

public:
    cryptor&                operator = (const cryptor& crt);

public:
    static cryptor          generate();
    static bool             validate(uint8_t type, const uint8_t* key, uint8_t ksize);
};

}

#endif // !__FB_CRYPTOR_H__
