#ifndef FB_GAME_CHANNEL_STORAGE_PENDING_CHANNEL_H
#define FB_GAME_CHANNEL_STORAGE_PENDING_CHANNEL_H

#include <async/task.h>
#include <fb/game/storage.h>
#include <fb/locker.h>
#include <functional>
#include <unordered_map>
#include <vector>

namespace fb::game {

class server;
class storage_box;

class storage_pending_channel
{
    using pending_map = std::unordered_map<std::string, storage_box::pending_box>;

private:
    server&                 _owner;
    fb::locker<pending_map> _data;

public:
    explicit storage_pending_channel(server& owner);

    async::task<void> fetch();
    void              read(std::function<void(const pending_map&)> fn);
    async::task<void> read_async(std::function<async::task<void>(const pending_map&)> fn);
    void              write(const std::function<void(pending_map&)>& fn);
};

} // namespace fb::game

#endif // FB_GAME_CHANNEL_STORAGE_PENDING_CHANNEL_H
