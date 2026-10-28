#ifndef __BOT_LOAD_TRACE_H__
#define __BOT_LOAD_TRACE_H__

#include <json/json.h>

#include <fstream>
#include <mutex>
#include <string>

namespace fb::bot::load {

class trace
{
private:
    std::mutex    _mutex;
    std::ofstream _file;

public:
    void open(const std::string& path);
    void write(const std::string& bot, Json::Value entry);
};

} // namespace fb::bot::load

#endif
