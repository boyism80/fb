#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <string>
#include <iostream>
#include <memory>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <sstream>
#include <mutex>

namespace fb {

class config
{
private:
    Json::Value             _json;

private:
    config(const char* env = nullptr);

public:
    static const Json::Value& get(const char* env = nullptr);
};

}

#endif // !__CONFIG_H__
