#ifndef __STREAM_WRITER_H__
#define __STREAM_WRITER_H__

#include <fb/stream.h>
#include <fb/endian.h>
#include <fb/encoding.h>

namespace fb {

/**
 * @brief      This class describes a stream writer.
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
     * @brief      Constructs a new instance.
     *
     * @param      stream  The stream to write data to.
     */
    stream_writer(std::vector<uint8_t>& stream) :
        _stream(stream)
    { }

public:
    /**
     * @brief      Writes a value of the specified type to the stream.
     *
     * @param[in]  value      The value to write.
     *
     * @tparam     ValueType  The type of value to write.
     *
     * @return     Reference to this stream writer for chaining.
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
     * @brief      Writes raw data from a buffer to the stream.
     *
     * @param[in]  buffer  The buffer containing data to write.
     * @param[in]  size    The number of bytes to write.
     *
     * @return     Reference to this stream writer for chaining.
     */
    stream_writer& write(const void* buffer, size_t size)
    {
        this->_stream.insert(this->_stream.end(), (uint8_t*)buffer, (uint8_t*)buffer + size);
        return *this;
    }

    /**
     * @brief      Writes a string with length prefix to the stream.
     *
     * @param[in]  value  The string value to write.
     *
     * @tparam     T1     The string type (must be std::string).
     * @tparam     T2     The type of the length prefix.
     *
     * @return     Reference to this stream writer for chaining.
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