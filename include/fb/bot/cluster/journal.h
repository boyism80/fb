#ifndef __FB_BOT_CLUSTER_JOURNAL_H__
#define __FB_BOT_CLUSTER_JOURNAL_H__

#include <json/json.h>

#include <atomic>
#include <mutex>

namespace fb::bot::cluster {

class journal
{
private:
    std::atomic<bool> _enabled = false;
    std::mutex        _mutex;
    Json::Value       _events{Json::arrayValue};

public:
    void        enable();
    void        record(Json::Value event);
    Json::Value drain();
};

} // namespace fb::bot::cluster

#endif
