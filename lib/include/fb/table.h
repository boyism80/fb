#ifndef __FB_TABLE_H__
#define __FB_TABLE_H__

#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <json/json.h>
#include <queue>
#include <mutex>
#include <future>
#include <thread>

namespace fb::table {

using handle_callback = std::function<void(Json::Value&, Json::Value&, double)>;
using handle_error    = std::function<void(Json::Value&, Json::Value&, const std::string& error)>;

bool     load(const std::string& path, Json::Value& json);
uint32_t load(const std::string& path, const handle_callback& callback, const handle_error& error, bool async = true);

} // namespace fb::table

#endif // !__FB_TABLE_H__
