#ifndef __FB_TABLE_H__
#define __FB_TABLE_H__

#include <string>
#include <string_view>
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
using handle_error    = std::function<void(Json::Value&, Json::Value&, std::string_view error)>;

bool     load(std::string_view path, Json::Value& json);
uint32_t load(std::string_view path, const handle_callback& callback, const handle_error& error, bool async = true);

} // namespace fb::table

#endif // !__FB_TABLE_H__
