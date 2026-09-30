#ifndef __FB_STREAM_H__
#define __FB_STREAM_H__

#include <fb/logger.h>

#include <zlib.h>

#include <cstddef>
#include <cstdint>
#include <vector>

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
