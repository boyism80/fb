#ifndef FB_GAME_POLLING_STORAGE_PENDING_CHANNEL_H
#define FB_GAME_POLLING_STORAGE_PENDING_CHANNEL_H

#include <async/task.h>
#include <fb/game/storage.h>
#include <fb/locker.h>
#include <functional>
#include <vector>

namespace fb::game {

class server;
class storage_box;

class storage_pending_channel
{
private:
    server&                                           _owner;
    fb::locker<std::vector<storage_box::pending_box>> _data;

public:
    explicit storage_pending_channel(server& owner);

    [[nodiscard]] async::task<void> fetch();
    void                            read(std::function<void(const std::vector<storage_box::pending_box>&)> fn);
    [[nodiscard]] async::task<void> read_async(std::function<async::task<void>(const std::vector<storage_box::pending_box>&)> fn);
    void                            write(std::function<void(std::vector<storage_box::pending_box>&)> fn);
};

} // namespace fb::game

#endif // FB_GAME_POLLING_STORAGE_PENDING_CHANNEL_H
