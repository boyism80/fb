#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <string>
#include <iostream>
#include <memory>
#include <fstream>
#include <json/json.h>
#include <sstream>
#include <mutex>
#include <format>
#include <fb/console.h>

namespace fb {

class config
{
private:
    Json::Value _json;

private:
    config(const char* env = nullptr);

public:
    static const Json::Value& get();
};

} // namespace fb

#endif // !__CONFIG_H__
