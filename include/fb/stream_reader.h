#ifndef __STREAM_READER_H__
#define __STREAM_READER_H__

#include <fb/stream.h>
#include <fb/endian.h>
#include <fb/encoding.h>

namespace fb {

/**
 * @brief      This class describes a stream reader.
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
     * @brief      Constructs a new instance.
     *
     * @param      stream  The stream to read from.
     */
    stream_reader(std::vector<uint8_t>& stream) :
        _stream(stream)
    { }

public:
    /**
     * @brief      Reads a value of the specified type from the stream.
     *
     * @tparam     ValueType  The type of value to read.
     *
     * @return     The read value.
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
     * @brief      Reads a string with length prefix from the stream.
     *
     * @tparam     T1    The string type (must be std::string).
     * @tparam     T2    The type of the length prefix.
     *
     * @return     The read string.
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
     * @brief      Reads raw data into a buffer.
     *
     * @param      buffer  The buffer to read data into.
     * @param[in]  size    The number of bytes to read.
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
     * @brief      Gets the number of bytes available for reading.
     *
     * @return     The number of readable bytes.
     */
    uint32_t readable_size() const
    {
        return std::max<uint32_t>(0, this->_stream.size() - this->_seek);
    }

    /**
     * @brief      Gets the current seek position.
     *
     * @return     The current seek position.
     */
    uint32_t seek() const
    {
        return this->_seek;
    }

    /**
     * @brief      Sets the seek position.
     *
     * @param[in]  value  The new seek position.
     */
    void seek(uint32_t value)
    {
        this->_seek = value;
    }

    /**
     * @brief      Flushes the stream.
     */
    void flush()
    {
        this->_stream.erase(this->_stream.begin(), this->_stream.begin() + this->_seek);
        this->_seek = 0;
    }

    /**
     * @brief      Clears the stream.
     */
    void clear()
    {
        this->_stream.clear();
        this->_seek = 0;
    }
};

} // namespace fb

#endif