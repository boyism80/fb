#include <fb/game/map/container.h>

using namespace fb::game;

map_container::map_container(fb::game::context& context, uint32_t host) :
    context(context),
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

bool map_container::load_block(uint32_t id, Json::Value& buffer)
{
    auto          fname = std::format("maps/{:06}.block", id);
    std::ifstream file(fname);
    if (file.is_open() == false)
        return false;

    Json::Reader reader;
    if (reader.parse(file, buffer) == false)
        return false;

    file.close();
    return true;
}

void map_container::load(const fb::model::map& model)
{
    auto active = (model.host == this->host);
    auto binary = std::vector<char>();
    auto blocks = Json::Value();
    if (active)
    {
        if (load_data(model.id, binary) == false)
            throw std::runtime_error(
                std::format("{} ({})", const_value::string::MESSAGE_ASSET_CANNOT_LOAD_MAP_DATA, model.name));

        if (load_block(model.id, blocks) == false)
            throw std::runtime_error(
                std::format("{} ({})", const_value::string::MESSAGE_ASSET_CANNOT_LOAD_MAP_BLOCK, model.name));
    }

    auto map = new fb::game::map(this->context, model, active, binary.data(), binary.size());
    for (const auto& block : blocks)
    {
        map->block(block["x"].asInt(), block["y"].asInt(), true);
    }

    {
        auto _ = std::lock_guard(this->_mutex);
        this->push(model.id, map);
    }
}

fb::game::map* map_container::name2map(const std::string& name) const
{
    for (const auto& [id, map] : *this)
    {
        if (map.model.name == name)
            return &map;
    }

    return nullptr;
}