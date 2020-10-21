#include "table.h"

bool fb::table::load(const std::string& path, Json::Value& json)
{
    std::ifstream           ifstream;
    ifstream.open(path);
    if(ifstream.is_open() == false)
        return false;

    Json::Reader            reader;
    if(reader.parse(ifstream, json) == false)
    {
        ifstream.close();
        return false;
    }

    ifstream.close();
    return true;
}

uint32_t fb::table::load(const std::string& path, std::function<void(Json::Value::iterator&, double)> on_callback, std::function<void(Json::Value::iterator& i, const std::string& error)> on_error)
{
    Json::Value             data;
    if(fb::table::load(path, data) == false)
        return 0;

    auto                    count = data.size();
    auto                    read = 0;
    for(auto i = data.begin(); i != data.end(); i++)
    {
        try
        {
            auto percentage = (read++ * 100) / double(count);
            on_callback(i, percentage);
        }
        catch(std::exception& e)
        {
            on_error(i, e.what());
        }
    }

    return count;
}