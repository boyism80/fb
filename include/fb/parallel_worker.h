#ifndef __PARALLEL_WORKER_H__
#define __PARALLEL_WORKER_H__

#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <future>
#include <algorithm>
#include <fb/generator.h>
#include <fb/locker.h>

namespace fb {

template <typename T, typename R = void>
class parallel_worker
{
protected:
    virtual fb::generator<T> on_ready()              = 0;
    virtual fb::generator<R> on_work(const T& value) = 0;
    virtual void             on_worked(const T& input, const R& output, double percent)
    { }
    virtual void on_error(const T& input, std::exception& e)
    { }
    virtual void on_finish(const std::vector<R>& result)
    { }

public:
    void run(std::vector<R>& result)
    {
        auto indices   = std::unordered_map<T*, int>();
        auto processed = std::atomic<int>(0);

        auto queue  = fb::locker<std::queue<T>>();
        auto buffer = fb::locker<std::unordered_map<uint32_t, std::unique_ptr<std::vector<R>>>>();

        auto gen_ready = this->on_ready();
        while (gen_ready.next())
        {
            auto input = gen_ready.value();
            {
                auto  guard = queue.enter_write();
                auto& q     = guard.value();
                q.push(input);
                auto& added = q.back();
                indices.insert({&added, indices.size()});
            }
        }

        double count = 0;
        {
            auto guard = queue.enter_read();
            count      = double(guard.value().size());
        }
        auto fn = [&, this]() {
            while (true)
            {
                T*   input       = nullptr;
                bool queue_empty = false;
                {
                    auto  guard = queue.enter_write();
                    auto& q     = guard.value();
                    if (q.empty())
                    {
                        queue_empty = true;
                    }
                    else
                    {
                        input = &q.front();
                        q.pop();
                    }
                }

                if (queue_empty)
                    break;

                auto index = indices.at(input);
                {
                    auto guard = buffer.enter_write();
                    guard.value().insert({index, std::make_unique<std::vector<R>>()});
                }

                try
                {
                    auto gen_work = this->on_work(*input);
                    while (gen_work.next())
                    {
                        auto output = gen_work.value();
                        {
                            auto guard = buffer.enter_write();
                            guard.value()[index]->push_back(output);
                        }
                    }

                    {
                        auto guard = buffer.enter_read();
                        for (auto& output : *guard.value().at(index))
                        {
                            this->on_worked(*input, output, (++processed * 100) / count);
                        }
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

        auto keys = std::vector<uint32_t>{};
        {
            auto guard = buffer.enter_read();
            for (auto& [k, _] : guard.value())
            {
                keys.push_back(k);
            }
        }
        std::sort(keys.begin(), keys.end());

        {
            auto guard = buffer.enter_read();
            for (auto k : keys)
            {
                for (auto& output : *guard.value().at(k))
                {
                    result.push_back(output);
                }
            }
        }

        this->on_finish(result);
    }
};

template <typename T>
class parallel_worker<T, void>
{
protected:
    virtual fb::generator<T> on_ready()              = 0;
    virtual void             on_work(const T& value) = 0;
    virtual void             on_worked(const T& input, double percent)
    { }
    virtual void on_error(const T& input, std::exception& e)
    { }
    virtual void on_finish()
    { }

public:
    void run()
    {
        fb::locker<std::queue<T>> queue;
        std::atomic<int>          processed{0};

        auto gen = this->on_ready();
        while (gen.next())
        {
            {
                auto guard = queue.enter_write();
                guard.value().push(gen.value());
            }
        }

        double count = 0;
        {
            auto guard = queue.enter_read();
            count      = double(guard.value().size());
        }
        auto fn = [&, this]() {
            while (true)
            {
                auto input       = std::optional<T>{};
                bool queue_empty = false;
                {
                    auto  guard = queue.enter_write();
                    auto& q     = guard.value();
                    if (q.empty())
                    {
                        queue_empty = true;
                    }
                    else
                    {
                        input = std::move(q.front());
                        q.pop();
                    }
                }

                if (queue_empty)
                    break;

                try
                {
                    this->on_work(input.value());
                    auto current_progress = (++processed * 100) / count;
                    this->on_worked(input.value(), current_progress);
                }
                catch (std::exception& e)
                {
                    processed++;
                    this->on_error(input.value(), e);
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

} // namespace fb

#endif