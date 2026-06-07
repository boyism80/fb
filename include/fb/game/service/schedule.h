#ifndef __FB_GAME_SERVICE_SCHEDULE_H__
#define __FB_GAME_SERVICE_SCHEDULE_H__

#include <async/task.h>
#include <fb/model/datetime.h>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace fb::game {
class server;
} // namespace fb::game

namespace fb::game::service {

class schedule
{
public:
    fb::game::server& server;

private:
    std::unordered_map<uint32_t, fb::model::datetime> _tasks;

public:
    explicit schedule(fb::game::server& server);

    void              init();
    async::task<void> poll();
};

} // namespace fb::game::service

#endif
