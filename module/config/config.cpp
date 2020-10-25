#include "config.h"

const Json::Value& fb::config::get()
{
    static Json::Value  instance;
    static bool			loaded = false;

    if(loaded == false)
    {
        std::ifstream ifstream;
#if defined DEBUG | defined _DEBUG
        ifstream.open("config.dev.json");
#else
        ifstream.open("config.json");
#endif
        if(ifstream.is_open() == false)
            throw std::runtime_error("cannot load conf_login.json file");

        Json::Reader reader;
        if(reader.parse(ifstream, instance) == false)
            throw std::runtime_error("cannot parse conf_login.json file");
        ifstream.close();

        loaded = true;
    }

    return instance;
}