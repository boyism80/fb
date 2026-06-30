#include <fb/config.h>
#include <fb/game/character.h>
#include <fb/game/map.h>
#include <fb/game/npc.h>
#include <fb/game/server.h>
#include <fb/game/thread_params.h>
#include <fb/model/model.h>
#include <fb/stream_reader.h>
#include <fb/stream_writer.h>

using namespace fb::game;
using table         = fb::model::table;
namespace game_resp = fb::protocol::game::response;

map::container::container(fb::game::server& server, uint32_t host) :
    _update_cache(
        [](const fb::game::map::cache_bytes& cache_bytes) -> uint64_t {
            return cache_bytes.hash;
        },
        1024),
    server(server),
    host(host)
{ }

map::container::~container()
{ }

bool map::container::load_data(uint32_t id, std::vector<char>& buffer)
{
    auto fname = std::format("maps/{:06}.map", id);
    auto file  = std::ifstream(fname, std::ios::binary);
    if (file.is_open() == false)
        return false;

    buffer = std::vector<char>(std::istreambuf_iterator<char>(file), {});
    file.close();

    return true;
}

bool map::container::load_block(uint32_t id, std::vector<fb::model::point16_t>& buffer)
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

void map::container::load(const fb::model::map& model)
{
    auto active    = (model.host == this->host);
    auto lazy_load = fb::config<bool>("lazy_load_maps", false);
    auto binary    = std::vector<char>();
    auto blocks    = std::vector<fb::model::point16_t>();
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

bool map::container::ensure_loaded(const std::shared_ptr<fb::game::map>& map)
{
    if (map == nullptr)
        return false;

    if (map->loaded())
        return true;

    if (map->active == false)
        return false;

    auto loaded_now = false;
    {
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

        loaded_now = true;
    }

    if (loaded_now)
    {
        this->spawn_npcs(map);
        this->invoke_init_script(map);
    }

    return true;
}

bool map::container::try_mark_init_script(const std::shared_ptr<fb::game::map>& map)
{
    if (map == nullptr || map->active == false || map->loaded() == false)
        return false;

    auto expected = false;
    return map->_init_script_invoked.compare_exchange_strong(expected, true);
}

async::task<void> map::container::run_init_script(const std::shared_ptr<fb::game::map>& map)
{
    auto path = std::format("scripts/map/{}.lua", map->model.id);
    auto func = std::format("ON_MAP_INIT_{}", map->model.id);

    auto lua = this->server.lua.new_ctx_guard(path, func);
    if (!lua)
        co_return;

    lua->pushobject(map);
    std::ignore = lua->call(1);

    co_return;
}

void map::container::invoke_init_script(const std::shared_ptr<fb::game::map>& map)
{
    if (this->try_mark_init_script(map) == false)
        return;

    auto builder = map->thread()->new_builder<void>();
    builder.func = [this, map](auto&) -> async::task<void> {
        co_await this->run_init_script(map);
    };
    builder.enqueue();
}

async::task<void> map::container::invoke_init_script_wait(const std::shared_ptr<fb::game::map>& map)
{
    if (this->try_mark_init_script(map) == false)
        co_return;

    auto builder = map->thread()->new_builder<void>();
    builder.func = [this, map](auto&) -> async::task<void> {
        co_await this->run_init_script(map);
    };
    co_await builder.dispatch();
}

void map::container::spawn_npcs(const std::shared_ptr<fb::game::map>& map)
{
    if (table::npc_spawn.contains(map->model.id) == false)
        return;

    for (auto& spawn : table::npc_spawn[map->model.id])
    {
        this->spawn_npc(spawn, map);
    }
}

void map::container::spawn_npc(const fb::model::npc_spawn& spawn)
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

void map::container::spawn_npc(const fb::model::npc_spawn& spawn, const std::shared_ptr<fb::game::map>& map)
{
    auto& npc_model = table::npc[spawn.npc];
    auto  npc       = this->server.make<fb::game::npc>(npc_model);
    auto  weak      = npc->weak_from_this_as<fb::game::npc>();
    auto  fn        = [](std::shared_ptr<fb::game::npc> npc,
                 std::shared_ptr<fb::game::map> map,
                 const fb::model::npc_spawn&    spawn_model) -> async::task<void> {
        std::ignore = co_await npc->map(map, spawn_model.position);
        npc->direction(spawn_model.direction);
    };
    auto builder = this->server.threads.new_builder(weak);
    builder.func = [fn, npc, map, &spawn](auto&) -> async::task<void> {
        co_await fn(npc, map, spawn);
    };
    builder.enqueue();
}

std::shared_ptr<fb::game::map> map::container::name2map(std::string_view name) const
{
    for (const auto& [id, map] : *this)
    {
        if (map->model.name == name)
            return map;
    }

    return nullptr;
}

void map::container::rezen_force()
{
    for (auto& [id, thread] : this->server.threads)
    {
        auto builder = thread->new_builder<void>();
        builder.func = [](auto& thread) -> async::task<void> {
            auto params = thread.template data<thread_params>();
            for (auto& rezen : params->rezens)
            {
                rezen.force_spawn(thread.id());
            }
            co_return;
        };
        builder.enqueue();
    }
}

void map::container::erase_map_cache(uint32_t map_id, const fb::model::point16_t& point)
{
    std::unique_lock lock(this->_update_cache_mutex);

    std::vector<uint64_t> to_erase;
    for (auto hash : this->_update_cache.keys())
    {
        const auto entry_map_id = static_cast<uint32_t>(hash >> 48);
        if (entry_map_id != map_id)
            continue;

        const uint16_t pos_x  = (hash >> 32) & 0xFFFF;
        const uint16_t pos_y  = (hash >> 16) & 0xFFFF;
        const uint8_t  width  = (hash >> 8) & 0xFF;
        const uint8_t  height = hash & 0xFF;
        auto           area   = fb::model::area<uint16_t>(pos_x, pos_y, pos_x + width, pos_y + height);
        if (area.contains(point))
            to_erase.push_back(hash);
    }

    for (uint64_t hash : to_erase)
    {
        this->_update_cache.erase(hash);
    }
}

void map::container::send_map_cache(character&                  ch,
                                    const fb::game::map&        map,
                                    const fb::model::point16_t& position,
                                    const fb::model::size8_t&   size,
                                    uint16_t                    crc)
{
    const auto hash = static_cast<uint64_t>(map.model.id) << 48 | static_cast<uint64_t>(position.x) << 32 |
                      static_cast<uint64_t>(position.y) << 16 | static_cast<uint64_t>(size.width) << 8 |
                      static_cast<uint64_t>(size.height);

    auto send_cache_bytes = [&ch, crc](const auto& cache_bytes) {
        if (cache_bytes.crc == crc)
            return;

        ch.send(fb::stream(cache_bytes.bytes.data(), cache_bytes.bytes.size()));
    };

    {
        std::shared_lock lock(this->_update_cache_mutex);
        if (this->_update_cache.try_read(hash, send_cache_bytes))
            return;
    }

    std::unique_lock lock(this->_update_cache_mutex);
    this->_update_cache.write(hash, send_cache_bytes, [&map, &position, &size, hash]() {
        auto bytes = fb::game::map::cache_bytes();
        bytes.hash = hash;
        bytes.crc  = 0;

        auto writer = fb::stream_writer<big_endian>(bytes.bytes);
        auto resp   = game_resp::map_update(map, position, size);
        std::ignore = resp.serialize(writer);
        bytes.crc   = resp.crc;
        return bytes;
    });
}

void map::container::update_map_cache(uint32_t map_id, const fb::model::area<uint16_t>& area)
{
    {
        std::unique_lock lock(this->_update_cache_mutex);

        std::vector<uint64_t> to_erase;
        for (auto hash : _update_cache.keys())
        {
            const auto entry_map_id = static_cast<uint32_t>(hash >> 48);
            if (entry_map_id != map_id)
                continue;

            const uint16_t e_left     = (hash >> 32) & 0xFFFF;
            const uint16_t e_top      = (hash >> 16) & 0xFFFF;
            const uint8_t  e_w        = (hash >> 8) & 0xFF;
            const uint8_t  e_h        = hash & 0xFF;
            const uint16_t e_right    = static_cast<uint16_t>(e_left + e_w);
            const uint16_t e_bottom   = static_cast<uint16_t>(e_top + e_h);
            const auto     entry_rect = fb::model::area<uint16_t>(e_left, e_top, e_right, e_bottom);

            if (entry_rect.intersects(area))
                to_erase.push_back(hash);
        }

        for (uint64_t hash : to_erase)
        {
            this->_update_cache.erase(hash);
        }
    }

    auto map_ptr = this->find(map_id);
    if (map_ptr == nullptr)
        return;

    const auto& map     = *map_ptr;
    auto        viewers = std::vector<std::shared_ptr<character>>{};
    for (const auto& [fd, obj] : map.objects)
    {
        if (obj->is(OBJECT_TYPE::CHARACTER) == false)
            continue;

        if (obj->sight_area().intersects(area))
            viewers.push_back(std::static_pointer_cast<character>(obj));
    }

    const uint16_t             w = area.right > area.left ? static_cast<uint16_t>(area.right - area.left) : 0;
    const uint16_t             h = area.bottom > area.top ? static_cast<uint16_t>(area.bottom - area.top) : 0;
    const fb::model::point16_t begin(area.left, area.top);
    const fb::model::size8_t   size(static_cast<uint8_t>(w > 255 ? 255 : w), static_cast<uint8_t>(h > 255 ? 255 : h));

    for (const auto& ch : viewers)
    {
        ch->update_map(map, begin, size);
    }
}
