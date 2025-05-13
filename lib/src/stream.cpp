#include <fb/stream.h>

using namespace fb;

stream::stream(const uint8_t* data, size_t size)
{
    this->std::vector<uint8_t>::assign(data, data + size);
}

stream::stream(const stream& right) :
    stream(right.data(), right.size())
{ }

uint32_t stream::crc() const
{
    return crc32(0, vector<uint8_t>::data(), (uint32_t)this->size());
}

stream stream::compress() const
{
    auto src_size = uint32_t(this->size());
    auto dst_size = uint32_t(src_size * 2);
    auto buffer   = std::unique_ptr<uint8_t[]>(new uint8_t[dst_size]);

    if (compress2(buffer.get(), (uLongf*)&dst_size, this->data(), src_size, Z_BEST_COMPRESSION) == Z_STREAM_ERROR)
        throw std::runtime_error("cannot compress data");

    return stream(buffer.get(), dst_size);
}

stream stream::decompress() const
{
    auto src_size = uint32_t(this->size());
    auto dst_size = uint32_t(src_size * 2);
    auto buffer   = std::unique_ptr<uint8_t[]>(new uint8_t[dst_size]);

    if (uncompress(buffer.get(), (uLongf*)&dst_size, this->data(), src_size) != Z_OK)
        throw std::runtime_error("cannot decompress data");

    return stream(buffer.get(), dst_size);
}