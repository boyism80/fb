#ifndef __STREAM_WRITER_H__
#define __STREAM_WRITER_H__

#include <fb/stream.h>
#include <fb/endian.h>
#include <fb/encoding.h>

namespace fb {

/**
 * @brief      This class describes a stream writer.
 *
 * @tparam     EndianType  { description }
 * @tparam     EndianType  { description }
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
     * @param      stream  The stream
     */
    stream_writer(std::vector<uint8_t>& stream) :
        _stream(stream)
    { }

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  value      The value
     *
     * @tparam     ValueType  { description }
     *
     * @return     { description_of_the_return_value }
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
            auto size = sizeof(ValueType);
            auto seek = this->_stream.size();
            for (int i = 0; i < size; i++)
            {
                this->_stream.push_back(0);
            }
            EndianType::template put<ValueType>(value, this->_stream.data() + seek);
            return *this;
        }
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  buffer  The buffer
     * @param[in]  size    The size
     *
     * @return     { description_of_the_return_value }
     */
    stream_writer& write(const void* buffer, size_t size)
    {
        this->_stream.insert(this->_stream.end(), (uint8_t*)buffer, (uint8_t*)buffer + size);
        return *this;
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @tparam     T1     { description }
     * @tparam     T2     { description }
     *
     * @return     { description_of_the_return_value }
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