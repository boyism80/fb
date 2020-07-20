#include "config.h"

namespace fb {

const Json::Value& config()
{
    static Json::Value  instance;
    static bool			loaded = false;

    if(loaded == false)
    {
        std::ifstream ifstream;
        ifstream.open("config.json");
        if(ifstream.is_open() == false)
            throw std::runtime_error("cannot load conf_login.json file");

        Json::Reader reader;
        if(reader.parse(ifstream, instance) == false)
            throw std::runtime_error("cannot parse conf_login.json file");
        ifstream.close();
    }

    return instance;
}

}