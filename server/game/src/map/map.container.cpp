#include <fb/config.h>
#include <fb/game/map/container.h>
#include <fb/game/npc.h>
#include <fb/game/server.h>
#include <fb/model/model.h>
#include <fb/stream_reader.h>

using namespace fb::game;
using table = fb::model::table;

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
    auto active     = (model.host == this->host);
    auto lazy_load  = fb::config<bool>("lazy_load_maps", false);
    auto binary     = std::vector<char>();
    auto blocks     = std::vector<fb::model::point16_t>();
    if (active && lazy_load == false)
    {
        if (load_data(model.id, binary) == false)
            throw std::runtime_error(std::format("{} ({})", _TEXT(MESSAGE_ASSET_CANNOT_LOAD_MAP_DATA), model.name));

        if (load_block(model.id, blocks) == false)
            fb::logger::warn("{} ({})", _TEXT(MESSAGE_ASSET_CANNOT_LOAD_MAP_BLOCK), model.name);
    }

    auto map = std::make_shared<fb::game::map>(this->server,
                                               model,
                                               active,
                                               binary.empty() ? nullptr : binary.data(),
                                               binary.size());
    for (const auto& block : blocks)
    {
        map->block(block.x, block.y, true);
    }

    {
        auto _ = std::lock_guard(this->_mutex);
        this->push(model.id, map);
    }
}

bool map_container::ensure_loaded(const std::shared_ptr<map>& map)
{
    if (map == nullptr)
        return false;

    if (map->loaded())
        return true;

    if (map->active == false)
        return false;

    auto _ = std::lock_guard(this->_mutex);

    if (map->loaded())
        return true;

    auto binary = std::vector<char>();
    auto blocks = std::vector<fb::model::point16_t>();
    if (load_data(map->model.id, binary) == false)
        return false;

    if (load_block(map->model.id, blocks) == false)
        fb::logger::warn("{} ({})", _TEXT(MESSAGE_ASSET_CANNOT_LOAD_MAP_BLOCK), map->model.name);

    map->load_tiles(binary.data(), binary.size());
    for (const auto& block : blocks)
    {
        map->block(block.x, block.y, true);
    }

    this->spawn_npcs(map);
    return true;
}

void map_container::spawn_npcs(const std::shared_ptr<map>& map)
{
    if (table::npc_spawn.contains(map->model.id) == false)
        return;

    for (auto& spawn : table::npc_spawn[map->model.id])
    {
        this->spawn_npc(spawn, map);
    }
}

void map_container::spawn_npc(const fb::model::npc_spawn& spawn)
{
    if (this->contains(spawn.parent) == false)
        return;

    auto map = (*this)[spawn.parent];
    if (map == nullptr || map->active == false)
        return;

    if (map->loaded() == false)
        return;

    this->spawn_npc(spawn, map);
}

void map_container::spawn_npc(const fb::model::npc_spawn& spawn, const std::shared_ptr<map>& map)
{
    auto& npc_model = table::npc[spawn.npc];
    auto  npc       = this->server.make<fb::game::npc>(npc_model);
    auto  weak      = npc->weak_from_this_as<fb::game::npc>();
    auto fn = [](std::shared_ptr<fb::game::npc>     npc,
                 std::shared_ptr<fb::game::map>    map,
                 const fb::model::npc_spawn& spawn_model) -> async::task<void> {
        std::ignore = co_await npc->map(map, spawn_model.position);
        npc->direction(spawn_model.direction);
    };
    this->server.threads.enqueue(weak, [fn, npc, map, &spawn](auto&) -> async::task<void> {
        co_await fn(npc, map, spawn);
    });
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
