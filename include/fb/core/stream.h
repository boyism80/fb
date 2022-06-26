#ifndef __FB_STREAM_H__
#define __FB_STREAM_H__

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <zlib.h>
#include <cstring> // std::memcpy
#ifdef __linux__
#include <sys/types.h>
#endif
#include <fb/core/encoding.h>

namespace fb {

class buffer : public std::vector<uint8_t>
{
public:
    enum class endian { LITTLE = 0, BIG = 1};

protected:
    uint32_t                _offset = 0;

public:
    buffer(size_t size = 1024);
    buffer(const uint8_t* data, size_t size);
    buffer(const buffer& right);
    virtual ~buffer();

public:
    uint32_t                offset() const;
    const void*             data() const;

public:
    uint32_t                crc() const;
    buffer                  compress() const;
    void                    clear();
};

class istream : public buffer
{
public:
    istream(size_t size = 1024);
    istream(const uint8_t* data, size_t size);
    ~istream();

public:
    int8_t                  read_8();
    uint8_t                 read_u8();
    int16_t                 read_16(buffer::endian endian = buffer::endian::BIG);
    uint16_t                read_u16(buffer::endian endian = buffer::endian::BIG);
    int32_t                 read_32(buffer::endian endian = buffer::endian::BIG);
    uint32_t                read_u32(buffer::endian endian = buffer::endian::BIG);
    void                    read(void* buffer, size_t size);
    std::string             readstr(bool encoding = true);
    std::string             readstr_u8(bool encoding = true);
    std::string             readstr_u16(bool encoding = true, buffer::endian endian = buffer::endian::BIG);
    std::string             readstr_u32(bool encoding = true, buffer::endian endian = buffer::endian::BIG);
    uint32_t                readable_size() const;
    void                    reset();
    void                    shift(size_t size);
    void                    flush();
};


class ostream : public buffer
{
public:
    using iterator = std::vector<uint8_t>::iterator;

public:
    ostream(size_t size = 1024);
    ostream(const uint8_t* data, size_t size);
    ~ostream();

public:
    ostream&                write_8(int8_t value);
    template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
    ostream&                write_8(T value)
    {
        return write_8(static_cast<int8_t>(value));
    }

    ostream&                write_u8(uint8_t value);
    template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
    ostream&                write_u8(T value)
    {
        return write_u8(static_cast<uint8_t>(value));
    }

    ostream&                write_16(int16_t value, buffer::endian endian = buffer::endian::BIG);
    template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
    ostream&                write_16(T value, buffer::endian endian = buffer::endian::BIG)
    {
        return write_16(static_cast<int16_t>(value), endian);
    }

    ostream&                write_u16(uint16_t value, buffer::endian endian = buffer::endian::BIG);
    template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
    ostream&                write_u16(T value, buffer::endian endian = buffer::endian::BIG)
    {
        return write_u16(static_cast<uint16_t>(value), endian);
    }

    ostream&                write_32(int32_t value, buffer::endian endian = buffer::endian::BIG);
    template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
    ostream&                write_32(T value, buffer::endian endian = buffer::endian::BIG)
    {
        return write_32(static_cast<int32_t>(value), endian);
    }

    ostream&                write_u32(uint32_t value, buffer::endian endian = buffer::endian::BIG);
    template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
    ostream&                write_u32(T value, buffer::endian endian = buffer::endian::BIG)
    {
        return write_u32(static_cast<uint32_t>(value), endian);
    }

    ostream&                writestr_u8(const std::string& value, bool encoding = true);
    ostream&                writestr_u16(const std::string& value, buffer::endian endian = buffer::endian::BIG, bool encoding = true);
    ostream&                writestr_u32(const std::string& value, buffer::endian endian = buffer::endian::BIG, bool encoding = true);
    ostream&                write(const void* buffer, size_t size);
    ostream&                write(const buffer& wb);
    ostream&                write(const std::string& str, bool uint16 = false, bool encoding = true);
};

}

#endif // !__FB_STREAM_H__
