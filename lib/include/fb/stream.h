#ifndef __FB_STREAM_H__
#define __FB_STREAM_H__

#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <zlib.h>
#include <cstring> // std::memcpy
#ifdef __linux__
#include <sys/types.h>
#endif
#include <fb/encoding.h>

#if defined(__s390x__)
#define LITTLE_ENDIAN 0
#endif // __s390x__
#if !defined(LITTLE_ENDIAN)
#if defined(__GNUC__) || defined(__clang__) || defined(__ICCARM__)
#if (defined(__BIG_ENDIAN__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__))
#define LITTLE_ENDIAN 0
#else
#define LITTLE_ENDIAN 1
#endif // __BIG_ENDIAN__
#elif defined(_MSC_VER)
#if defined(_M_PPC)
#define LITTLE_ENDIAN 0
#else
#define LITTLE_ENDIAN 1
#endif
#else
#error Unable to determine endianness, define LITTLE_ENDIAN.
#endif
#endif // !defined(LITTLE_ENDIAN)

#ifdef min
#undef min
#endif

#ifdef max
#undef max
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
    stream(const uint8_t* data, size_t size)
    {
        std::vector<uint8_t>::reserve(size);
        std::vector<uint8_t>::assign(data, data + size);
    }

    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The right
     */
    stream(const stream& right) :
        stream(right.data(), right.size())
    { }

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
    uint32_t crc() const
    {
        return crc32(0, vector<uint8_t>::data(), (uint32_t)this->size());
    }

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    stream compress() const
    {
        uint32_t src_size = this->size();
        uint32_t dst_size = this->size() * 2;
        uint8_t* buffer   = new uint8_t[dst_size];

        if (compress2(buffer,
                      (uLongf*)&dst_size,
                      vector<uint8_t>::data(),
                      uint32_t(this->size()),
                      Z_BEST_COMPRESSION) == Z_STREAM_ERROR)
            throw std::runtime_error("cannot compress data");

        auto compressed = stream(buffer, dst_size);
        delete[] buffer;
        return compressed;
    }

    /**
     * @brief      Decompresses the object.
     *
     * @return     { description_of_the_return_value }
     */
    stream decompress() const
    {
        uint32_t src_size = this->size();
        uint32_t dst_size = this->size() * 2;
        uint8_t* buffer   = new uint8_t[dst_size];

        if (uncompress(buffer, (uLongf*)&dst_size, vector<uint8_t>::data(), uint32_t(this->size())) != Z_OK)
            throw std::runtime_error("cannot compress data");

        auto decompressed = stream(buffer, dst_size);
        delete[] buffer;
        return decompressed;
    }
#endif
};

class little_endian
{
public:
    template <typename ValueType>
    static ValueType get(const uint8_t* p)
    {
        auto value = ValueType();
        auto ptr   = (uint8_t*)(ValueType*)&value;
#ifndef LITTLE_ENDIAN
        for (int i = 0; i < sizeof(ValueType); i++)
        {
            ptr[sizeof(ValueType) - i - 1] = p[i];
        }
#else
        std::memcpy(ptr, p, sizeof(ValueType));
#endif
        return value;
    }

public:
    template <typename ValueType>
    static void put(const ValueType& value, uint8_t* p)
    {
        auto ptr = (uint8_t*)(ValueType*)&value;
        for (int i = 0; i < sizeof(ValueType); i++)
        {
#ifdef LITTLE_ENDIAN
            p[i] = ptr[i];
#else
            p[sizeof(ValueType) - i - 1] = ptr[i];
#endif
        }
    }
};

class big_endian
{
public:
    template <typename ValueType>
    static ValueType get(const uint8_t* p)
    {
        auto value = ValueType();
        auto ptr   = (uint8_t*)(ValueType*)&value;
#ifdef LITTLE_ENDIAN
        for (int i = 0; i < sizeof(ValueType); i++)
        {
            ptr[sizeof(ValueType) - i - 1] = p[i];
        }
#else
        std::memcpy(ptr, p, sizeof(ValueType));
#endif
        return value;
    }

public:
    template <typename ValueType>
    static void put(const ValueType& value, uint8_t* p)
    {
        auto ptr = (uint8_t*)(ValueType*)&value;
        for (int i = 0; i < sizeof(ValueType); i++)
        {
#ifndef LITTLE_ENDIAN
            p[i] = ptr[i];
#else
            p[sizeof(ValueType) - i - 1] = ptr[i];
#endif
        }
    }
};

/**
 * @brief      This class describes a stream reader.
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
            return str;
        }
        else
        {
            throw std::runtime_error("cannot call this");
        }
    }

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

/**
 * @brief      This class describes a stream writer.
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

#endif // !__FB_STREAM_H__
