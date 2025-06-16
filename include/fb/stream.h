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

/**
 * @brief      This class describes a stream of bytes.
 */
class stream : public std::vector<uint8_t>
{
public:
    /**
     * @brief      Constructs a new instance.
     */
    stream() = default;

    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  data  The data to copy from.
     * @param[in]  size  The size of the data.
     */
    stream(const uint8_t* data, size_t size);

    /**
     * @brief      Constructs a new instance from a vector.
     *
     * @param      v     The vector to move data from.
     */
    stream(std::vector<uint8_t>&& v);

    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The right stream to copy from.
     */
    stream(const stream& right);

    /**
     * @brief      Destroys the object.
     */
    ~stream() = default;

#ifdef ZLIB_VERSION
    /**
     * @brief      Calculates the CRC32 checksum of the stream data.
     *
     * @return     The CRC32 checksum.
     */
    uint32_t crc() const;

    /**
     * @brief      Compresses the stream data using zlib.
     *
     * @return     A new stream containing the compressed data.
     */
    stream compress() const;

    /**
     * @brief      Decompresses the stream data using zlib.
     *
     * @return     A new stream containing the decompressed data.
     */
    stream decompress() const;
#endif
};

} // namespace fb

#endif // !__FB_STREAM_H__
