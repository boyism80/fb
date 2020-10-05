#ifndef __CONFIG_H__
#define	__CONFIG_H__

#include <string>
#include <iostream>
#include <memory>
#include <fstream>
#include <jsoncpp/json/json.h>

namespace fb {

class config
{
public:
    static const Json::Value& get();
};

}

#endif // !__CONFIG_H__
