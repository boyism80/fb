#include <fb/stream.h>

using namespace fb;

stream::stream(const uint8_t* data, size_t size)
{
    this->std::vector<uint8_t>::assign(data, data + size);
}

stream::stream(std::vector<uint8_t>&& v)
{
    this->swap(v);
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
    auto src_size = static_cast<uLongf>(this->size());
    auto dst_size = compressBound(src_size);
    auto buffer   = std::unique_ptr<uint8_t[]>(new uint8_t[dst_size + 16]);
    std::memset(buffer.get(), 0xCC, dst_size + 16);

    if (compress2(buffer.get(), &dst_size, this->data(), src_size, Z_BEST_COMPRESSION) != Z_OK)
        throw std::runtime_error("compress failed");

    return stream(std::vector<uint8_t>(buffer.get(), buffer.get() + dst_size));
}

stream stream::decompress() const
{
    auto src_size = static_cast<uLongf>(this->size());
    auto buffer   = std::unique_ptr<uint8_t[]>();
    auto dst_size = src_size * 2;

    while (true)
    {
        buffer = std::make_unique<uint8_t[]>(dst_size + 16);
        std::memset(buffer.get(), 0xCC, dst_size + 16);

        auto result = uncompress(buffer.get(), &dst_size, this->data(), src_size);
        if (result == Z_OK)
            break;

        if (result != Z_BUF_ERROR)
            throw std::runtime_error("decompress failed");

        dst_size *= 2;
    }

    return stream(std::vector<uint8_t>(buffer.get(), buffer.get() + dst_size));
}