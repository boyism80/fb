#include <fb/config.h>
#include <fb/game/character.h>
#include <fb/game/instance_map.h>
#include <fb/game/map.h>
#include <fb/game/mob.h>
#include <fb/game/npc.h>
#include <fb/game/server.h>
#include <fb/game/thread_params.h>
#include <fb/logger.h>
#include <fb/model/model.h>
#include <fb/stream_reader.h>
#include <fb/stream_writer.h>
#include <algorithm>
#include <tuple>

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

bool map::container::contains(uint32_t id) const
{
    return this->_maps.read([&](const registry& registry) {
        return registry.contains(id);
    });
}

std::shared_ptr<fb::game::map> map::container::find(uint32_t id) const
{
    return this->_maps.read([&](const registry& registry) {
        return registry.find(id);
    });
}

std::shared_ptr<fb::game::map> map::container::operator[] (uint32_t id) const
{
    auto found = this->find(id);
    if (found == nullptr)
        throw std::runtime_error(std::format("map {} does not exists", id));

    return found;
}

std::shared_ptr<const map::container::snapshot_t> map::container::snapshot() const
{
    return this->_snapshot.load();
}

void map::container::append_snapshot(const std::shared_ptr<fb::game::map>& map)
{
    auto prev = this->_snapshot.load();
    auto next = std::make_shared<snapshot_t>(*prev);
    next->push_back(map);
    this->_snapshot.store(next);
}

void map::container::remove_snapshot(uint32_t id)
{
    auto prev = this->_snapshot.load();
    auto next = std::make_shared<snapshot_t>();
    next->reserve(prev->size());
    for (const auto& map : *prev)
    {
        if (map->id != id)
            next->push_back(map);
    }
    this->_snapshot.store(next);
}

void map::container::insert(const std::shared_ptr<fb::game::map>& map)
{
    this->_maps.write([&](registry& registry) {
        registry.push(map->id, map);
        this->_sequence = std::max(this->_sequence, map->id + 1);
    });
    this->append_snapshot(map);
}

void map::container::erase(uint32_t id)
{
    std::shared_ptr<fb::game::map> erased;
    this->_maps.write([&](registry& registry) {
        erased = registry.find(id);
        if (erased != nullptr && erased->is_instance())
            this->release_slot(erased->model.id, erased->slot(), erased);

        registry.erase(id);
        this->_available_seq.push(id);
    });
    this->remove_snapshot(id);

    if (erased != nullptr && erased->is_instance())
        this->unregister_group_instance(erased);
}

uint32_t map::container::allocate_id(registry& registry)
{
    while (this->_available_seq.empty() == false)
    {
        auto id = this->_available_seq.front();
        this->_available_seq.pop();
        if (registry.contains(id) == false)
            return id;
    }

    return this->_sequence++;
}

uint32_t map::container::allocate_slot(slot_pool& pool)
{
    while (pool.available.empty() == false)
    {
        auto slot = *pool.available.begin();
        pool.available.erase(pool.available.begin());
        if (pool.by_slot.contains(slot) == false)
            return slot;
    }

    return pool.next++;
}

void map::container::release_slot(uint32_t model_id, uint32_t slot, const std::shared_ptr<fb::game::map>& map)
{
    auto& pool = this->_slot_pools[model_id];
    auto  it   = pool.by_slot.find(slot);
    if (it == pool.by_slot.end() || it->second != map)
        return;

    pool.by_slot.erase(it);
    pool.available.insert(slot);
}

void map::container::register_slot(uint32_t model_id, uint32_t slot, const std::shared_ptr<fb::game::map>& map)
{
    auto& pool = this->_slot_pools[model_id];
    pool.available.erase(slot);
    pool.by_slot[slot] = map;
    pool.next          = std::max(pool.next, slot + 1);
}

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
                                               model.id,
                                               model,
                                               active,
                                               binary.empty() ? nullptr : binary.data(),
                                               binary.size());
    for (const auto& block : blocks)
    {
        map->block(block.x, block.y, true);
    }

    this->insert(map);
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
        auto _ = std::lock_guard(this->_load_mutex);

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

    auto lua = this->server.lua.open(path, func);
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
    auto  position  = spawn.position;
    auto  direction = spawn.direction;
    auto  fn        = [](std::shared_ptr<fb::game::npc> npc,
                 std::shared_ptr<fb::game::map> map,
                 fb::model::point16_t           position,
                 DIRECTION                      direction) -> async::task<void> {
        std::ignore = co_await npc->map(map, position);
        npc->direction(direction);
    };
    auto builder = this->server.threads.new_builder(weak);
    builder.func = [fn, npc, map, position, direction](auto&) -> async::task<void> {
        co_await fn(npc, map, position, direction);
    };
    builder.enqueue();
}

async::task<void> map::container::cleanup()
{
    auto maps_division = std::unordered_map<fb::thread*, std::vector<std::shared_ptr<fb::game::map>>>{};
    for (const auto& map : *this->snapshot())
    {
        if (map->loaded() == false)
            continue;

        auto thread = map->thread();
        if (thread == nullptr)
            continue;

        maps_division[thread].push_back(map);
    }

    auto tasks = std::vector<async::task<void>>{};
    for (auto& [thread, maps] : maps_division)
    {
        auto builder = thread->new_builder<void>();
        builder.func = [maps = std::move(maps)](auto&) -> async::task<void> {
            for (const auto& map : maps)
            {
                auto objects = std::vector<std::shared_ptr<fb::game::object>>{};
                for (auto& [seq, obj] : map->objects)
                {
                    std::ignore = seq;
                    if (obj->is(OBJECT_TYPE::CHARACTER))
                        continue;

                    objects.push_back(obj);
                }

                for (auto& obj : objects)
                {
                    co_await obj->destroy();
                }
            }
            co_return;
        };
        tasks.push_back(builder.dispatch());
    }

    for (auto& task : tasks)
    {
        co_await task;
    }
    co_return;
}

std::shared_ptr<fb::game::map> map::container::name2map(std::string_view name) const
{
    for (const auto& map : *this->snapshot())
    {
        if (map->model.name == name)
            return map;
    }

    return nullptr;
}

std::shared_ptr<fb::game::map> map::container::create_instance(const std::shared_ptr<fb::game::map>& source,
                                                               uint32_t                              slot)
{
    auto created = false;
    auto map     = this->_maps.write([&](registry& registry) {
        auto& pool = this->_slot_pools[source->model.id];
        auto  it   = pool.by_slot.find(slot);
        if (it != pool.by_slot.end() && it->second != nullptr && it->second->closing() == false)
            return it->second;

        if (it != pool.by_slot.end() && it->second != nullptr && it->second->closing())
            pool.by_slot.erase(it);

        auto id  = this->allocate_id(registry);
        auto map = std::make_shared<fb::game::instance_map>(this->server, id, slot, source);
        registry.push(id, map);
        this->_sequence = std::max(this->_sequence, id + 1);
        this->register_slot(source->model.id, slot, map);
        created = true;
        return std::static_pointer_cast<fb::game::map>(map);
    });

    if (created == false)
        return map;

    this->append_snapshot(map);

    auto builder = map->thread()->new_builder<void>();
    builder.func = [this, map](auto& thread) -> async::task<void> {
        auto params = thread.template data<thread_params>();
        params->add_map(map);
        if (table::mob_spawn.contains(map->model.id))
        {
            for (auto& spawn : table::mob_spawn[map->model.id])
            {
                params->rezens.push_back(std::make_unique<fb::game::rezen>(this->server, spawn, map));
            }
        }
        co_return;
    };
    builder.enqueue();

    this->spawn_npcs(map);
    this->invoke_init_script(map);

    return map;
}

std::shared_ptr<fb::game::map> map::container::clone(const std::shared_ptr<fb::game::map>& source)
{
    if (source == nullptr)
        return nullptr;

    auto root = source->is_instance() ? source->source() : source;
    if (root == nullptr)
        return nullptr;

    if (this->ensure_loaded(root) == false)
        return nullptr;

    auto slot = this->_maps.write([&](registry&) {
        return this->allocate_slot(this->_slot_pools[root->model.id]);
    });

    return this->create_instance(root, slot);
}

std::shared_ptr<fb::game::map> map::container::ensure_instance(const std::shared_ptr<fb::game::map>& source,
                                                               uint32_t                              slot)
{
    if (source == nullptr || slot == 0)
        return nullptr;

    auto root = source->is_instance() ? source->source() : source;
    if (root == nullptr)
        return nullptr;

    auto existing = this->_maps.write([&](registry&) -> std::shared_ptr<fb::game::map> {
        auto& pool = this->_slot_pools[root->model.id];
        auto  it   = pool.by_slot.find(slot);
        if (it == pool.by_slot.end() || it->second == nullptr)
            return nullptr;

        if (it->second->closing())
        {
            pool.by_slot.erase(it);
            return nullptr;
        }

        return it->second;
    });
    if (existing != nullptr)
        return existing;

    if (this->ensure_loaded(root) == false)
        return nullptr;

    return this->create_instance(root, slot);
}

void map::container::unregister_group_instance(const std::shared_ptr<fb::game::map>& map)
{
    auto lock = std::lock_guard(this->_entry_mutex);
    for (auto it = this->_group_instances.begin(); it != this->_group_instances.end();)
    {
        if (it->second == map)
            it = this->_group_instances.erase(it);
        else
            ++it;
    }
}

std::shared_ptr<fb::game::map> map::container::choice_by_capacity(const std::shared_ptr<fb::game::map>& source)
{
    auto capacity = source->model.instance_capacity;
    if (capacity.has_value() == false || capacity.value() == 0)
        return source;

    auto limit = capacity.value();
    if (source->character_count() < limit)
        return source;

    auto existing = this->_maps.write([&](registry&) -> std::shared_ptr<fb::game::map> {
        auto& pool = this->_slot_pools[source->model.id];
        for (uint32_t slot = 1; slot < pool.next; ++slot)
        {
            auto it = pool.by_slot.find(slot);
            if (it == pool.by_slot.end() || it->second == nullptr)
                continue;

            if (it->second->closing())
                continue;

            if (it->second->character_count() < limit)
                return it->second;
        }
        return nullptr;
    });
    if (existing != nullptr)
        return existing;

    return this->clone(source);
}

std::shared_ptr<fb::game::map> map::container::choice_by_group(character&                            ch,
                                                               const std::shared_ptr<fb::game::map>& source)
{
    auto group_id = ch.group_id();
    if (group_id.has_value() == false)
        return nullptr;

    auto gid = group_id.value();
    {
        auto lock = std::lock_guard(this->_entry_mutex);
        auto it   = this->_group_instances.find(gid);
        if (it != this->_group_instances.end() && it->second != nullptr && it->second->closing() == false)
            return it->second;
    }

    auto created = this->clone(source);
    if (created == nullptr)
        return nullptr;

    {
        auto lock = std::lock_guard(this->_entry_mutex);
        auto it   = this->_group_instances.find(gid);
        if (it != this->_group_instances.end() && it->second != nullptr && it->second->closing() == false)
        {
            auto winner = it->second;
            // Lost the creation race; drop the unused empty instance.
            auto builder = created->thread()->new_builder<void>();
            builder.func = [this, created](auto&) -> async::task<void> {
                co_await this->destroy(created);
            };
            builder.enqueue();
            return winner;
        }

        this->_group_instances[gid] = created;
    }

    return created;
}

std::shared_ptr<fb::game::map> map::container::choice_entry(character& ch, const std::shared_ptr<fb::game::map>& dest)
{
    if (dest == nullptr)
        return nullptr;

    // Explicit instance target bypasses systemic routing.
    if (dest->is_instance())
        return dest;

    auto source = dest;
    switch (source->model.instance_rule)
    {
    case fb::model::enum_value::INSTANCE_RULE_TYPE::NONE:
        return source;

    case fb::model::enum_value::INSTANCE_RULE_TYPE::CAPACITY:
        return this->choice_by_capacity(source);

    case fb::model::enum_value::INSTANCE_RULE_TYPE::GROUP:
        return this->choice_by_group(ch, source);

    default:
        return source;
    }
}

async::task<void> map::container::destroy(const std::shared_ptr<fb::game::map>& map)
{
    if (map == nullptr || map->is_instance() == false)
        co_return;

    if (map->begin_destroy() == false)
        co_return;

    auto thread = map->thread();
    if (thread == nullptr)
        co_return;

    auto characters = std::make_shared<std::vector<std::shared_ptr<fb::game::character>>>();
    {
        auto builder = thread->new_builder<void>();
        builder.func = [map, characters](auto&) -> async::task<void> {
            auto objects = std::vector<std::shared_ptr<fb::game::object>>{};
            for (auto& [seq, obj] : map->objects)
            {
                objects.push_back(obj);
            }

            for (auto& obj : objects)
            {
                if (obj->is(OBJECT_TYPE::CHARACTER))
                    characters->push_back(std::static_pointer_cast<fb::game::character>(obj));
                else
                    co_await obj->destroy();
            }
            co_return;
        };
        co_await builder.dispatch();
    }

    auto source = map->source();
    for (auto& character : *characters)
    {
        auto weak    = character->weak_from_this_as<fb::game::object>();
        auto builder = this->server.threads.new_builder(weak);
        builder.func = [character, source](auto&) -> async::task<void> {
            map_options opts;
            opts.skip_instance_rule = true;
            std::ignore             = co_await character->map(source, std::nullopt, opts);
        };
        co_await builder.dispatch();
    }

    {
        auto builder = thread->new_builder<void>();
        builder.func = [map](auto& thread) -> async::task<void> {
            auto  params = thread.template data<thread_params>();
            auto& rezens = params->rezens;
            rezens.erase(std::remove_if(rezens.begin(),
                                        rezens.end(),
                                        [id = map->id](const auto& rezen) {
                                            return rezen->map_id() == id;
                                        }),
                         rezens.end());
            params->remove_map(map->id);
            co_return;
        };
        co_await builder.dispatch();
    }

    this->erase(map->id);
    co_return;
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
                rezen->force_spawn(thread.id());
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

std::optional<fb::stream> map::container::map_update_stream(character&                  ch,
                                                            const fb::game::map&        map,
                                                            const fb::model::point16_t& position,
                                                            const fb::model::size8_t&   size,
                                                            uint16_t                    crc)
{
    if (map.is_instance())
    {
        auto bytes  = std::vector<uint8_t>();
        auto writer = fb::stream_writer<big_endian>(bytes);
        auto resp   = game_resp::map_update(map, position, size);
        resp.serialize(writer);
        if (resp.crc == crc)
            return std::nullopt;

        return fb::stream(bytes.data(), bytes.size());
    }

    const auto hash = static_cast<uint64_t>(map.model.id) << 48 | static_cast<uint64_t>(position.x) << 32 |
                      static_cast<uint64_t>(position.y) << 16 | static_cast<uint64_t>(size.width) << 8 |
                      static_cast<uint64_t>(size.height);

    std::optional<fb::stream> stream;
    auto                      apply_cache_bytes = [&stream, crc](const auto& cache_bytes) {
        if (cache_bytes.crc == crc)
            return;

        stream = fb::stream(cache_bytes.bytes.data(), cache_bytes.bytes.size());
    };

    {
        std::shared_lock lock(this->_update_cache_mutex);
        if (this->_update_cache.try_read(hash, apply_cache_bytes))
            return stream;
    }

    std::unique_lock lock(this->_update_cache_mutex);
    this->_update_cache.write(hash, apply_cache_bytes, [&map, &position, &size, hash]() {
        auto bytes = fb::game::map::cache_bytes();
        bytes.hash = hash;
        bytes.crc  = 0;

        auto writer = fb::stream_writer<big_endian>(bytes.bytes);
        auto resp   = game_resp::map_update(map, position, size);
        resp.serialize(writer);
        bytes.crc = resp.crc;
        return bytes;
    });

    return stream;
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
