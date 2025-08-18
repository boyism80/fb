#ifndef __FB_STREAM_H__
#define __FB_STREAM_H__

#include <stdexcept>
#include <vector>
#include <zlib.h>
#include <cstdint>
#include <memory>
#include <fb/logger.h>
#ifdef __linux__
#include <sys/types.h>
#endif

namespace fb {

class stream : public std::vector<uint8_t>
{
public:
    stream() = default;
    stream(const uint8_t* data, size_t size);
    stream(std::vector<uint8_t>&& v);
    stream(const stream& right);
    ~stream() = default;

#ifdef ZLIB_VERSION
    uint32_t crc() const;
    stream   compress() const;
    stream   decompress() const;
#endif
};

} // namespace fb

#endif // !__FB_STREAM_H__
