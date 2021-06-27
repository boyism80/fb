#ifndef __FB_TABLE_H__
#define __FB_TABLE_H__

#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <jsoncpp/json/json.h>
#include <queue>
#include <mutex>
#include <future>
#include <thread>

namespace fb { namespace table {

typedef std::function<void(const std::string&, double)>                 handle_callback;
typedef std::function<void(const std::string&, const std::string&)>     handle_error;
typedef std::function<void(uint32_t)>                                   handle_complete;

bool                        load(const std::string& path, Json::Value& json);
uint32_t                    load(const std::string& path, std::function<void(Json::Value&, Json::Value&, double)> callback, std::function<void(Json::Value&, Json::Value&, const std::string& error)> error, bool async = true);

} }

#endif // !__FB_TABLE_H__
