#include <fb/table.h>
#include <fb/locker.h>

using namespace fb::table;

bool fb::table::load(const std::string& path, Json::Value& json)
{
    std::ifstream ifstream;
    ifstream.open(path);
    if (ifstream.is_open() == false)
        return false;

    Json::Reader reader;
    if (reader.parse(ifstream, json) == false)
    {
        ifstream.close();
        return false;
    }

    ifstream.close();
    return true;
}

uint32_t fb::table::load(const std::string& path, const handle_callback& callback, const handle_error& error, bool async)
{
    auto data = Json::Value{};
    if (fb::table::load(path, data) == false)
        return 0;

    auto count = data.size();

    struct work_data
    {
        std::queue<std::pair<Json::Value, std::unique_ptr<Json::Value>>> queue;
        int                                                              read = 0;
    };

    fb::locker<work_data> work;

    for (auto i = data.begin(); i != data.end(); i++)
    {
        work.write([&i](auto& w) {
            w.queue.push(std::make_pair(i.key(), std::make_unique<Json::Value>(*i)));
        });
    }

    auto fn = [&]() {
        while (true)
        {
            auto key  = Json::Value{};
            auto data = Json::Value{};

            bool queue_empty = work.write([&key, &data](auto& w) -> bool {
                if (w.queue.empty())
                    return true;

                auto& entity = w.queue.front();
                key          = entity.first;
                data         = *entity.second;
                w.queue.pop();
                return false;
            });

            if (queue_empty)
                break;

            auto percentage = 0.0;
            try
            {
                percentage = work.write([count](auto& w) {
                    return (w.read++ * 100) / double(count);
                });

                callback(key, data, percentage);
            }
            catch (std::exception& e)
            {
                error(key, data, e.what());
            }
        }
    };

    if (async)
    {
        auto tasks = std::queue<std::future<void>>();
        for (int i = 0; i < std::thread::hardware_concurrency(); i++)
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