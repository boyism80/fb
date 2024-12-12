#ifndef __STREAM_READER_H__
#define __STREAM_READER_H__

#include <fb/stream.h>
#include <fb/endian.h>
#include <fb/encoding.h>

namespace fb {

/**
 * @brief      This class describes a stream reader.
 *
 * @tparam     EndianType  { description }
 * @tparam     EndianType  { description }
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
     * @param      stream  The stream
     */
    stream_reader(std::vector<uint8_t>& stream) :
        _stream(stream)
    { }

public:
    /**
     * @brief      { function_description }
     *
     * @tparam     ValueType  { description }
     *
     * @return     The value type.
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
            auto size    = sizeof(ValueType);
            auto value   = EndianType::template get<ValueType>(this->_stream.data() + this->_seek);
            this->_seek += size;
            return value;
        }
    }

    /**
     * @brief      { function_description }
     *
     * @tparam     T1    { description }
     * @tparam     T2    { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <typename T1, typename T2>
    T1 read()
    {
        if constexpr (std::is_same_v<T1, std::string>)
        {
            auto len     = this->read<T2>();
            auto str     = std::string(this->_stream.data() + this->_seek, this->_stream.data() + this->_seek + len);
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
     * @brief      { function_description }
     *
     * @param      buffer  The buffer
     * @param[in]  size    The size
     */
    void read(void* buffer, size_t size)
    {
        if (buffer == nullptr)
            return;

        memcpy(buffer, this->_stream.data() + this->_seek, size);
        this->_seek += size;
    }

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t readable_size() const
    {
        return std::max(uint32_t(0), uint32_t(this->_stream.size() - this->_seek));
    }

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t seek() const
    {
        return this->_seek;
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void seek(uint32_t value)
    {
        this->_seek = value;
    }

    /**
     * @brief      Flushes the object.
     */
    void flush()
    {
        this->_stream.erase(this->_stream.begin(), this->_stream.begin() + this->_seek);
        this->_seek = 0;
    }

    /**
     * @brief      Clears the object.
     */
    void clear()
    {
        this->_stream.clear();
        this->_seek = 0;
    }
};

} // namespace fb

#endif