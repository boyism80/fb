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

using handle_callback       = std::function<void(const std::string&, double)>;
using handle_error          = std::function<void(const std::string&, const std::string&)>;
using handle_complete       = std::function<void(uint32_t)>;

bool                        load(const std::string& path, Json::Value& json);
uint32_t                    load(const std::string& path, const std::function<void(Json::Value&, Json::Value&, double)>& callback, const std::function<void(Json::Value&, Json::Value&, const std::string& error)> error, bool async = true);

} }

#endif // !__FB_TABLE_H__
