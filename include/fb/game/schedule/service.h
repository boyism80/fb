#ifndef __SCHEDULE_SERVICE_H__
#define __SCHEDULE_SERVICE_H__

#include <async/task.h>
#include <fb/model/datetime.h>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace fb::game {

class server;

class schedule_service
{
public:
    fb::game::server& server;

private:
    std::unordered_map<uint32_t, fb::model::datetime> _tasks;

public:
    explicit schedule_service(fb::game::server& server);

    void              init();
    async::task<void> poll();
};

} // namespace fb::game

#endif
