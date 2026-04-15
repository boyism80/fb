#include <fb/game/map/container.h>
#include <fb/game/server.h>
#include <fb/stream_reader.h>

using namespace fb::game;

map_container::map_container(fb::game::server& server, uint32_t host) :
    server(server),
    host(host)
{ }

map_container::~map_container()
{ }

bool map_container::load_data(uint32_t id, std::vector<char>& buffer)
{
    auto fname = std::format("maps/{:06}.map", id);
    auto file  = std::ifstream(fname, std::ios::binary);
    if (file.is_open() == false)
        return false;

    buffer = std::vector<char>(std::istreambuf_iterator<char>(file), {});
    file.close();

    return true;
}

bool map_container::load_block(uint32_t id, std::vector<fb::model::point16_t>& buffer)
{
    auto fname = std::format("maps/{:06}.block", id);
    auto file  = std::ifstream(fname, std::ios::binary);
    if (file.is_open() == false)
        return false;

    auto bytes = std::vector<uint8_t>(std::istreambuf_iterator<char>(file), {});
    file.close();

    try
    {
        auto reader = fb::stream_reader<little_endian>(bytes);
        auto count  = reader.read<uint32_t>();

        buffer.clear();
        buffer.reserve(count);

        for (uint32_t i = 0; i < count; ++i)
        {
            auto x = reader.read<uint16_t>();
            auto y = reader.read<uint16_t>();
            buffer.emplace_back(x, y);
        }

        if (reader.readable_size() != 0)
            return false;
    }
    catch (const std::runtime_error&)
    {
        return false;
    }

    return true;
}

void map_container::load(const fb::model::map& model)
{
    auto active = (model.host == this->host);
    auto binary = std::vector<char>();
    auto blocks = std::vector<fb::model::point16_t>();
    if (active)
    {
        if (load_data(model.id, binary) == false)
            throw std::runtime_error(std::format("{} ({})", _TEXT(MESSAGE_ASSET_CANNOT_LOAD_MAP_DATA), model.name));

        if (load_block(model.id, blocks) == false)
            fb::logger::warn("{} ({})", _TEXT(MESSAGE_ASSET_CANNOT_LOAD_MAP_BLOCK), model.name);
    }

    auto map = std::make_shared<fb::game::map>(this->server, model, active, binary.data(), binary.size());
    for (const auto& block : blocks)
    {
        map->block(block.x, block.y, true);
    }

    {
        auto _ = std::lock_guard(this->_mutex);
        this->push(model.id, map);
    }
}

std::shared_ptr<fb::game::map> map_container::name2map(std::string_view name) const
{
    for (const auto& [id, map] : *this)
    {
        if (map->model.name == name)
            return map;
    }

    return nullptr;
}