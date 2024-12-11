#include <fb/stream.h>

using namespace fb;

stream::stream(const uint8_t* data, size_t size)
{
    std::vector<uint8_t>::reserve(size);
    std::vector<uint8_t>::assign(data, data + size);
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

stream stream::decompress() const
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