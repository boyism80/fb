#ifndef __STREAM_READER_H__
#define __STREAM_READER_H__

#include <fb/stream.h>
#include <fb/endian.h>
#include <fb/encoding.h>

namespace fb {

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
    stream_reader(std::vector<uint8_t>& stream) :
        _stream(stream)
    { }

public:
    template <typename ValueType> ValueType read()
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

    template <typename T1, typename T2> T1 read()
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
    void read(void* buffer, size_t size)
    {
        if (this->readable_size() < size)
            throw std::runtime_error("stream_reader::read: out of range");

        if (buffer != nullptr)
            memcpy(buffer, this->_stream.data() + this->_seek, size);
        this->_seek += size;
    }

    uint32_t readable_size() const
    {
        return std::max<uint32_t>(0, this->_stream.size() - this->_seek);
    }

    uint32_t seek() const
    {
        return this->_seek;
    }

    void seek(uint32_t value)
    {
        this->_seek = value;
    }

    void flush()
    {
        this->_stream.erase(this->_stream.begin(), this->_stream.begin() + this->_seek);
        this->_seek = 0;
    }

    void clear()
    {
        this->_stream.clear();
        this->_seek = 0;
    }
};

} // namespace fb

#endif