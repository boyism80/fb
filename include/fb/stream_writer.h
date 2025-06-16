#ifndef __STREAM_WRITER_H__
#define __STREAM_WRITER_H__

#include <fb/stream.h>
#include <fb/endian.h>
#include <fb/encoding.h>

namespace fb {

/**
 * @brief      Binary stream writer with endian-aware data serialization capabilities.
 *
 *             This template class provides a convenient interface for writing binary data
 *             to byte streams with automatic endian conversion. It supports writing
 *             primitive types, strings with length prefixes, and raw binary data.
 *
 * @tparam     EndianType  The endian type for writing data (big_endian or little_endian).
 */
#ifdef LITTLE_ENDIAN
template <typename EndianType = little_endian>
#else
template <typename EndianType = big_endian>
#endif
class stream_writer
{
private:
    std::vector<uint8_t>& _stream;

public:
    /**
     * @brief      Constructs a new stream writer for the specified byte stream.
     *
     *             Creates a writer that will append data to the given stream.
     *             All write operations will add data to the end of the stream.
     *
     * @param[in]  stream  The byte stream to write data to
     */
    stream_writer(std::vector<uint8_t>& stream) :
        _stream(stream)
    { }

public:
    /**
     * @brief      Writes a value of the specified type to the stream.
     *
     *             Serializes the value to the end of the stream with proper endian
     *             conversion. For strings, uses uint8_t length prefix. For fb::stream
     *             objects, appends all bytes directly.
     *
     * @param[in]  value      The value to write to the stream
     *
     * @tparam     ValueType  The type of value to write to the stream
     *
     * @return     Reference to this stream writer for method chaining
     */
    template <typename ValueType>
    stream_writer& write(const ValueType& value)
    {
        if constexpr (std::is_same_v<ValueType, std::string>)
        {
            this->write<std::string, uint8_t>(value);
            return *this;
        }
        else if constexpr (std::is_same_v<ValueType, fb::stream>)
        {
            this->_stream.insert(this->_stream.end(), value.begin(), value.end());
            return *this;
        }
        else
        {
            auto seek = this->_stream.size();
            for (int i = 0; i < sizeof(ValueType); i++)
            {
                this->_stream.push_back(0);
            }
            EndianType::template put<ValueType>(value, this->_stream.data() + seek);
            return *this;
        }
    }

    /**
     * @brief      Writes raw binary data from a buffer to the stream.
     *
     *             Copies the specified number of bytes from the source buffer
     *             directly to the end of the stream without any conversion.
     *
     * @param[in]  buffer  The source buffer containing data to write
     * @param[in]  size    The number of bytes to write from the buffer
     *
     * @return     Reference to this stream writer for method chaining
     */
    stream_writer& write(const void* buffer, size_t size)
    {
        this->_stream.insert(this->_stream.end(), (uint8_t*)buffer, (uint8_t*)buffer + size);
        return *this;
    }

    /**
     * @brief      Writes a string with custom length prefix type to the stream.
     *
     *             Serializes a string by first writing a length prefix of the specified
     *             type, then writing the string bytes. Handles CP949 encoding conversion
     *             on non-Windows platforms.
     *
     * @param[in]  value  The string value to write to the stream
     *
     * @tparam     T1     The string type (must be std::string)
     * @tparam     T2     The type of the length prefix (uint8_t, uint16_t, etc.)
     *
     * @return     Reference to this stream writer for method chaining
     * @throws     std::runtime_error if T1 is not std::string
     */
    template <typename T1, typename T2>
    stream_writer& write(const T1& value)
    {
        if constexpr (std::is_same_v<T1, std::string>)
        {
#ifdef _WIN32
            auto& target = value;
#else
            auto target = cp949(value);
#endif
            this->write<T2>(target.size());
            for (int i = 0; i < target.size(); i++)
            {
                this->_stream.push_back(target[i]);
            }

            return *this;
        }
        else
        {
            throw std::runtime_error("cannot call this");
        }
    }
};

} // namespace fb

#endif