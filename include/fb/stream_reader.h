#ifndef __STREAM_READER_H__
#define __STREAM_READER_H__

#include <fb/stream.h>
#include <fb/endian.h>
#include <fb/encoding.h>

namespace fb {

/**
 * @brief      Binary stream reader with endian-aware data extraction capabilities.
 *
 *             This template class provides a convenient interface for reading binary data
 *             from byte streams with automatic endian conversion. It supports reading
 *             primitive types, strings with length prefixes, and raw binary data.
 *
 * @tparam     EndianType  The endian type for reading data (big_endian or little_endian).
 */
#ifdef LITTLE_ENDIAN
template <typename EndianType = little_endian>
#else
template <typename EndianType = big_endian>
#endif
class stream_reader
{
private:
    std::vector<uint8_t>& _stream;
    uint32_t              _seek = 0;

public:
    /**
     * @brief      Constructs a new stream reader for the specified byte stream.
     *
     *             Creates a reader that will extract data from the given stream
     *             starting at position 0. The reader maintains its own seek position.
     *
     * @param[in]  stream  The byte stream to read data from
     */
    stream_reader(std::vector<uint8_t>& stream) :
        _stream(stream)
    { }

public:
    /**
     * @brief      Reads a value of the specified type from the stream.
     *
     *             Extracts a value from the current seek position, automatically
     *             handling endian conversion. For strings, uses uint8_t length prefix.
     *
     * @tparam     ValueType  The type of value to read from the stream
     *
     * @return     The value read from the stream with proper endian conversion
     * @throws     std::runtime_error if not enough data is available
     */
    template <typename ValueType>
    ValueType read()
    {
        if constexpr (std::is_same_v<ValueType, std::string>)
        {
            return this->read<std::string, uint8_t>();
        }
        else
        {
            auto size = sizeof(ValueType);
            if (this->readable_size() < size)
                throw std::runtime_error("stream_reader::read: out of range");

            auto value   = EndianType::template get<ValueType>(this->_stream.data() + this->_seek);
            this->_seek += size;
            return value;
        }
    }

    /**
     * @brief      Reads a string with custom length prefix type from the stream.
     *
     *             Extracts a string by first reading a length prefix of the specified
     *             type, then reading that many bytes as string data. Handles UTF-8
     *             conversion on non-Windows platforms.
     *
     * @tparam     T1    The string type (must be std::string)
     * @tparam     T2    The type of the length prefix (uint8_t, uint16_t, etc.)
     *
     * @return     The string read from the stream with proper encoding
     * @throws     std::runtime_error if not enough data is available
     */
    template <typename T1, typename T2>
    T1 read()
    {
        if constexpr (std::is_same_v<T1, std::string>)
        {
            auto len = this->read<T2>();
            if (this->readable_size() < len)
                throw std::runtime_error("stream_reader::read: out of range");

            auto str = std::string(this->_stream.data() + this->_seek, this->_stream.data() + this->_seek + len);

            this->_seek += len;
#ifndef _WIN32
            return utf8(str);
#else
            return str;
#endif
        }
        else
        {
            throw std::runtime_error("cannot call this");
        }
    }

public:
    /**
     * @brief      Reads raw binary data into a provided buffer.
     *
     *             Copies the specified number of bytes from the current stream
     *             position into the destination buffer. Advances the seek position.
     *
     * @param[out] buffer  The destination buffer to copy data into (can be nullptr to skip)
     * @param[in]  size    The number of bytes to read from the stream
     * @throws     std::runtime_error if not enough data is available
     */
    void read(void* buffer, size_t size)
    {
        if (this->readable_size() < size)
            throw std::runtime_error("stream_reader::read: out of range");

        if (buffer != nullptr)
            memcpy(buffer, this->_stream.data() + this->_seek, size);
        this->_seek += size;
    }

    /**
     * @brief      Gets the number of bytes available for reading from current position.
     *
     *             Calculates how many bytes remain in the stream from the current
     *             seek position to the end of the stream.
     *
     * @return     The number of bytes that can still be read
     */
    uint32_t readable_size() const
    {
        return std::max<uint32_t>(0, this->_stream.size() - this->_seek);
    }

    /**
     * @brief      Gets the current read position in the stream.
     *
     * @return     The current seek position (offset from stream beginning)
     */
    uint32_t seek() const
    {
        return this->_seek;
    }

    /**
     * @brief      Sets the read position in the stream.
     *
     *             Moves the seek position to the specified offset from the
     *             beginning of the stream for subsequent read operations.
     *
     * @param[in]  value  The new seek position (offset from stream beginning)
     */
    void seek(uint32_t value)
    {
        this->_seek = value;
    }

    /**
     * @brief      Removes processed data from the beginning of the stream.
     *
     *             Erases all bytes from the stream beginning up to the current
     *             seek position, then resets the seek position to 0. This is
     *             useful for freeing memory after processing data.
     */
    void flush()
    {
        this->_stream.erase(this->_stream.begin(), this->_stream.begin() + this->_seek);
        this->_seek = 0;
    }

    /**
     * @brief      Clears all data from the stream and resets position.
     *
     *             Removes all bytes from the stream and resets the seek
     *             position to 0, effectively starting with an empty stream.
     */
    void clear()
    {
        this->_stream.clear();
        this->_seek = 0;
    }
};

} // namespace fb

#endif