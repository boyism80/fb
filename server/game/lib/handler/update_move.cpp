#include <fb/game/handler/update_move.h>
#include <fb/game/handler/move.h>
#include <fb/game/server.h>

using namespace fb::game::handler;

update_move::update_move(fb::game::server& server) :
    fb::handler<fb::game::server, fb::protocol::game::request::update_move>(server)
{ }

async::task<bool> update_move::handle(fb::socket<character>& session, fb::protocol::game::request::update_move& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return true;

    auto move_handler = fb::game::handler::move(this->server);
    if (co_await move_handler.handle(session, request.direction, request.position))
    {
        auto hash = static_cast<uint64_t>(map->model.id) << 48 | static_cast<uint64_t>(request.begin.x) << 32 |
                    static_cast<uint64_t>(request.begin.y) << 16 | static_cast<uint64_t>(request.size.width) << 8 |
                    static_cast<uint64_t>(request.size.height);

        this->server.map_update_cache.write(
            hash,
            [ch, crc = request.crc](auto& cache_bytes) {
                if (cache_bytes.crc != crc)
                    ch->send(fb::stream(cache_bytes.bytes.data(), cache_bytes.bytes.size()));
            },
            [&server = this->server, map, &request, hash]() {
                auto bytes = map::cache_bytes();
                bytes.hash = hash;
                bytes.crc  = 0;

                auto writer = fb::stream_writer<big_endian>(bytes.bytes);
                auto resp   = fb::protocol::game::response::map_update(*map, request.begin, request.size);
                std::ignore = resp.serialize(writer);
                bytes.crc   = resp.crc;
                return bytes;
            });
    }

    co_return true;
}
