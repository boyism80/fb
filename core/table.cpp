#include <fb/table.h>

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

uint32_t fb::table::load(const std::string& path, const std::function<void(Json::Value&, Json::Value&, double)>& callback, const std::function<void(Json::Value&, Json::Value&, const std::string& error)> error, bool async)
{
    Json::Value             data;
    if(fb::table::load(path, data) == false)
        return 0;

    auto                    count = data.size();
    auto                    read = 0;
    auto                    queue = std::queue<std::pair<Json::Value, std::unique_ptr<Json::Value>>>();
    auto                    mutex = std::mutex();

    for(auto i = data.begin(); i != data.end(); i++)
    {
        queue.push(std::make_pair(i.key(), std::make_unique<Json::Value>(*i)));
    }

    auto                    fn = [&]()
    {
        while(true)
        {
            auto            key = Json::Value();
            auto            data = Json::Value();

            {   auto _ = std::lock_guard(mutex);
                
                if(queue.empty())
                    break;

                auto&       entity = queue.front();
                key                = entity.first;
                data               = *entity.second;
                queue.pop();
            }

            auto percentage = 0.0;
            try
            {
                {   auto _ = std::lock_guard(mutex);
                    percentage = (read++ * 100) / double(count);
                }

                callback(key, data, percentage);
            }
            catch(std::exception& e)
            {
                error(key, data, e.what());
            }
        }
    };

    if(async)
    {
        auto tasks = std::queue<std::future<void>>();
        for(int i = 0; i < std::thread::hardware_concurrency(); i++)
        {
            tasks.push(std::async(std::launch::async, fn));
        }
    }
    else
    {
        fn();
    }

    return count;
}