#include <fb/config.h>

fb::config::config(const char* env)
{
    std::ifstream ifstream;
    try
    {
        std::stringstream sstream;
        if (env == nullptr)
            sstream << "config/config.json";
        else
            sstream << "config/config." << env << ".json";

        ifstream.open(sstream.str());
        if (ifstream.is_open() == false)
        {
            sstream.str("");
            sstream << "cannot load config." << env << " file.";
            throw std::runtime_error(sstream.str());
        }

        Json::Reader reader;
        if (reader.parse(ifstream, this->_json) == false)
        {
            sstream.str("");
            sstream << "cannot parse config." << env << " file.";
            throw std::runtime_error(sstream.str());
        }

        ifstream.close();
    }
    catch (std::exception& e)
    {
        if (ifstream.is_open())
            ifstream.close();

        throw e;
    }
}

const Json::Value& fb::config::get()
{
    static std::once_flag              flag;
    static std::unique_ptr<fb::config> ist;

    std::call_once(flag, [] {
        const char* env = std::getenv("KINGDOM_OF_WIND_ENVIRONMENT");
        if (env == nullptr)
            env = "dev";

        ist = std::unique_ptr<fb::config>(new fb::config(env));
    });
    return ist->_json;
}