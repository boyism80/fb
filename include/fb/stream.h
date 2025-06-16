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
 * @brief      Binary data stream with compression and checksum capabilities.
 *
 *             This class extends std::vector<uint8_t> to provide a convenient interface
 *             for handling binary data streams. It includes built-in support for zlib
 *             compression/decompression and CRC32 checksum calculation when available.
 */
class stream : public std::vector<uint8_t>
{
public:
    /**
     * @brief      Constructs an empty stream.
     *
     *             Creates a new stream with no initial data.
     */
    stream() = default;

    /**
     * @brief      Constructs a stream from raw binary data.
     *
     *             Creates a new stream by copying the specified binary data.
     *
     * @param[in]  data  Pointer to the source data to copy
     * @param[in]  size  Number of bytes to copy from the source data
     */
    stream(const uint8_t* data, size_t size);

    /**
     * @brief      Constructs a stream by moving data from a vector.
     *
     *             Creates a new stream by taking ownership of the data from
     *             an existing vector, avoiding unnecessary copying.
     *
     * @param[in]  v     The vector to move data from
     */
    stream(std::vector<uint8_t>&& v);

    /**
     * @brief      Copy constructor for creating a stream from another stream.
     *
     *             Creates a new stream by copying all data from the source stream.
     *
     * @param[in]  right  The source stream to copy from
     */
    stream(const stream& right);

    /**
     * @brief      Destroys the stream and releases its resources.
     */
    ~stream() = default;

#ifdef ZLIB_VERSION
    /**
     * @brief      Calculates the CRC32 checksum of the stream data.
     *
     *             Computes a CRC32 checksum over all bytes in the stream,
     *             useful for data integrity verification.
     *
     * @return     The CRC32 checksum value
     */
    uint32_t crc() const;

    /**
     * @brief      Compresses the stream data using zlib deflate algorithm.
     *
     *             Creates a new stream containing the compressed version of this
     *             stream's data using zlib compression.
     *
     * @return     A new stream containing the compressed data
     * @throws     std::runtime_error if compression fails
     */
    stream compress() const;

    /**
     * @brief      Decompresses the stream data using zlib inflate algorithm.
     *
     *             Creates a new stream containing the decompressed version of this
     *             stream's data, assuming it was previously compressed with zlib.
     *
     * @return     A new stream containing the decompressed data
     * @throws     std::runtime_error if decompression fails
     */
    stream decompress() const;
#endif
};

} // namespace fb

#endif // !__FB_STREAM_H__
