#ifndef __FB_STREAM_H__
#define __FB_STREAM_H__

/**
 * @file    stream.h
 * @brief   Binary data stream with compression and checksum capabilities
 * @author  FB Development Team
 *
 * @details This file implements a binary data stream class that extends std::vector<uint8_t>
 *          to provide specialized functionality for handling binary data in the FB 2D MMORPG
 *          server. The stream supports compression, decompression, and integrity checking
 *          capabilities essential for network communication and data storage.
 *
 *          Key features:
 *          - Binary data stream based on std::vector<uint8_t> for efficient memory management
 *          - zlib compression and decompression support for data size optimization
 *          - CRC32 checksum calculation for data integrity verification
 *          - Multiple construction methods for flexible data initialization
 *          - Move semantics support for efficient data transfer without copying
 *          - Exception-safe operations with proper error handling
 *          - Cross-platform compatibility with appropriate header inclusions
 *          - Integration with the logging system for error reporting
 *          - Memory-efficient data handling for large binary payloads
 *          - Support for protocol data serialization and network transmission
 *
 * @note    This stream class is fundamental to the server's data handling infrastructure,
 *          providing the foundation for protocol serialization, network communication,
 *          and data storage operations throughout the FB 2D MMORPG server.
 */

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
