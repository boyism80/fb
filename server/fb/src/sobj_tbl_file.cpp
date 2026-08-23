#include <fb/sobj_tbl_file.h>
#include <fb/stream_reader.h>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <format>

namespace fb {

void sobj_tbl_file::load(std::string_view path)
{
    this->_collision.clear();

    auto file = std::filesystem::path(std::string(path));
    if (std::filesystem::exists(file) == false)
        throw std::runtime_error(std::format("SObj.tbl not found: {}", file.string()));

    auto size = std::filesystem::file_size(file);
    auto buf  = std::vector<uint8_t>(static_cast<size_t>(size));
    auto in   = std::ifstream(file, std::ios::binary);
    if (!in.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(size)))
        throw std::runtime_error(std::format("failed to read SObj.tbl: {}", file.string()));

    auto reader = fb::stream_reader<little_endian>(buf);
    auto count  = reader.read<uint32_t>();
    reader.read<uint16_t>();

    this->_collision.assign(count, 0);
    for (uint32_t id = 1; id < count; ++id)
    {
        reader.read<uint32_t>();
        reader.read<uint8_t>();
        this->_collision[id] = reader.read<uint8_t>();

        auto extra_len = reader.read<uint8_t>();
        for (uint8_t i = 0; i < extra_len; ++i)
        {
            reader.read<uint16_t>();
        }
    }
}

uint8_t sobj_tbl_file::collision(uint16_t object_id) const
{
    if (object_id >= this->_collision.size())
        return 0;

    return this->_collision[object_id];
}

bool sobj_tbl_file::fully_blocked(uint16_t object_id) const
{
    return (this->collision(object_id) & ALL) == ALL;
}

} // namespace fb
