#ifndef __FB_STREAM_H__
#define __FB_STREAM_H__

#include <stdexcept>
#include <vector>
#include <zlib.h>
#include <cstdint>
#ifdef __linux__
#include <sys/types.h>
#endif

namespace fb {

/**
 * @brief      This class describes a stream.
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
     * @param[in]  data  The data
     * @param[in]  size  The size
     */
    stream(const uint8_t* data, size_t size);

    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The right
     */
    stream(const stream& right);

    /**
     * @brief      Destroys the object.
     */
    ~stream() = default;

#ifdef ZLIB_VERSION
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t crc() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    stream compress() const;

    /**
     * @brief      Decompresses the object.
     *
     * @return     { description_of_the_return_value }
     */
    stream decompress() const;
#endif
};

} // namespace fb

#endif // !__FB_STREAM_H__
