#ifndef __FB_TABLE_H__
#define __FB_TABLE_H__

#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <jsoncpp/json/json.h>

namespace fb { namespace table {

typedef std::function<void(const std::string&, double)>                 handle_callback;
typedef std::function<void(const std::string&, const std::string&)>     handle_error;
typedef std::function<void(uint32_t)>                                   handle_complete;

bool                        load(const std::string& path, Json::Value& json);
uint32_t                    load(const std::string& path, std::function<void(Json::Value::iterator&, double)> on_callback, std::function<void(Json::Value::iterator& i, const std::string& error)> on_error);

} }

#endif // !__FB_TABLE_H__
