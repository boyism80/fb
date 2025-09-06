#ifndef __FB_ENCRYPTION_H__
#define __FB_ENCRYPTION_H__

#include <fb/stream.h>
#include <algorithm>
#include <cstring>
#include <memory>

namespace fb {

class encryption
{
public:
    static uint32_t           HEX_TABLE[10][0x100];
    static constexpr uint32_t KEY_SIZE = 0x09;

private:
    uint8_t                    _pattern = 0;
    std::unique_ptr<uint8_t[]> _iv;
    uint8_t                    _sequence = 0;

public:
    encryption();
    encryption(uint8_t pattern, const uint8_t* iv);
    encryption(const encryption& encryption);
    ~encryption() = default;

private:
    void crypt(const uint8_t* source, uint8_t* dest, uint32_t size, const uint8_t* key, uint32_t ksize);

public:
    inline uint8_t pattern() const
    {
        return this->_pattern;
    }
    inline const uint8_t* iv() const
    {
        return this->_iv.get();
    }

public:
    uint32_t encrypt(fb::stream& data, uint32_t offset, uint32_t size);
    uint32_t encrypt(fb::stream& data);
    uint32_t decrypt(fb::stream& data, uint32_t offset, uint32_t size);
    uint32_t decrypt(fb::stream& data);
    uint32_t wrap(fb::stream& data, uint32_t offset) const;
    uint32_t wrap(fb::stream& data) const;
    uint32_t unwrap(fb::stream& data, uint32_t offset) const;
    uint32_t unwrap(fb::stream& data) const;

public:
    encryption& operator= (const encryption&);

public:
    static encryption generate();
    static bool       validate(uint8_t type, const uint8_t* key, uint8_t ksize);
};

} // namespace fb

#endif // !__FB_ENCRYPTION_H__
