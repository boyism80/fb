#include <fb/config.h>

fb::config::config(const char* env)
{
    std::ifstream ifstream;
    try
    {
        auto path = env != nullptr ? std::format("config/config.{}.json", env) : "config/config.json";
        ifstream.open(path);
        if (ifstream.is_open() == false)
        {
            throw std::runtime_error(std::format("cannot load file {}", path));
        }

        Json::Reader reader;
        if (reader.parse(ifstream, this->_json) == false)
        {
            throw std::runtime_error(std::format("cannot parse json file {}", path));
        }

        ifstream.close();
    }
    catch (std::exception& e)
    {
        if (ifstream.is_open())
            ifstream.close();

        fb::console::get().puts(e.what());
        throw e;
    }
}

const Json::Value& fb::config::get()
{
    static std::once_flag              flag;
    static std::unique_ptr<fb::config> ist;

    std::call_once(flag, [] {
        const char* env = std::getenv("KINGDOM_OF_WIND_ENVIRONMENT");
#if defined DEBUG || defined _DEBUG
        if (env == nullptr)
            env = "dev";
#endif

        ist = std::unique_ptr<fb::config>(new fb::config(env));
    });
    return ist->_json;
}