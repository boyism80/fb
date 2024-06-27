#ifndef __PARALLEL_WORKER_H__
#define __PARALLEL_WORKER_H__

#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <future>
#include <fb/core/coroutine.h>

namespace fb {

template <typename T, typename R = void>
class parallel_worker
{
protected:
    virtual fb::generator<T> on_ready() = 0;
    virtual fb::generator<R> on_work(const T& value) = 0;
    virtual void on_worked(const T& input, const R& output, double percent)
    {}
    virtual void on_error(const T& input, std::exception& e)
    {}
    virtual void on_finish(const std::vector<R>& result)
    {}

public:
    void run(std::vector<R>& result)
    {
        auto queue = std::queue<T>();
        auto indices = std::unordered_map<T*, int>();
        auto buffer = std::unordered_map<uint32_t, std::unique_ptr<std::vector<R>>>();
        auto processed = std::atomic<int>(0);

        std::mutex mutex_queue, mutex_buffer;

        auto gen_ready = this->on_ready();
        while (gen_ready.next())
        {
            auto input = gen_ready.value();
            queue.push(input);

            auto& added = queue.back();
            indices.insert({&added, indices.size()});
        }

        auto count = double(queue.size());
        auto fn = [&, this]()
        {
            while (true)
            {
                T* input = nullptr;
                {
                    auto _ = std::lock_guard(mutex_queue);
                    if (queue.empty())
                        break;

                    input = &queue.front();
                    queue.pop();
                }

                auto index = indices.at(input);
                mutex_buffer.lock();
                buffer.insert({index, std::make_unique<std::vector<R>>()});
                mutex_buffer.unlock();

                try
                {
                    auto gen_work = this->on_work(*input);
                    while (gen_work.next())
                    {
                        auto output = gen_work.value();
                        buffer[index]->push_back(output);
                    }

                    for (auto& output : *buffer[index])
                    {
                        this->on_worked(*input, output, (++processed * 100) / count);
                    }
                }
                catch (std::exception& e)
                {
                    this->on_error(*input, e);
                    processed++;
                }
            }
        };

        auto tasks = std::vector<std::future<void>>();
        for (int i = 0; i < std::thread::hardware_concurrency(); i++)
        {
            tasks.push_back(std::async(std::launch::async, fn));
        }

        for (auto& task : tasks)
        {
            task.wait();
        }

        auto keys = std::vector<uint32_t>();
        for (auto& [k, _] : buffer)
        {
            keys.push_back(k);
        }
        std::sort(keys.begin(), keys.end());

        for (auto k : keys)
        {
            for (auto& output : *buffer[k])
            {
                result.push_back(output);
            }
        }

        this->on_finish(result);
    }
};

template <typename T>
class parallel_worker<T, void>
{
protected:
    virtual fb::generator<T> on_ready() = 0;
    virtual void on_work(const T& value) = 0;
    virtual void on_worked(const T& input, double percent)
    {}
    virtual void on_error(const T& input, std::exception& e)
    {}
    virtual void on_finish()
    {}

public:
    void run()
    {
        auto queue = std::queue<T>();
        auto mutex_queue = std::mutex();
        auto processed = std::atomic<int>(0);

        auto gen = this->on_ready();
        while (gen.next())
        {
            queue.push(gen.value());
        }

        auto count = double(queue.size());
        auto fn = [&, this]() 
        {
            while (true)
            {
                T* input = nullptr;
                {
                    auto _ = std::lock_guard(mutex_queue);
                    if (queue.empty())
                        break;

                    input = &queue.front();
                    queue.pop();
                }

                try
                {
                    this->on_work(*input);

                    this->on_worked(*input, (++processed * 100) / count);
                }
                catch (std::exception& e)
                {
                    processed++;
                    this->on_error(*input, e);
                }
            }
        };

        auto tasks = std::vector<std::future<void>>();
        for (int i = 0; i < std::thread::hardware_concurrency(); i++)
        {
            tasks.push_back(std::async(std::launch::async, fn));
        }

        for (auto& task : tasks)
        {
            task.wait();
        }

        this->on_finish();
    }
};

}

#endif