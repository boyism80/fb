#ifndef __PARALLEL_WORKER_H__
#define __PARALLEL_WORKER_H__

#include <atomic>
#include <memory>
#include <optional>
#include <queue>
#include <vector>
#include <fb/async_executor.h>
#include <fb/generator.h>
#include <fb/synchronized.h>
#include <async/task.h>

namespace fb {

template <typename T, typename R = void>
class parallel_worker
{
protected:
    fb::async_executor& _executor;

    explicit parallel_worker(fb::async_executor& executor) :
        _executor(executor)
    { }

    virtual fb::generator<T> on_ready()              = 0;
    virtual fb::generator<R> on_work(const T& value) = 0;
    virtual void             on_worked(const T& input, const R& output, double percent)
    { }
    virtual void on_error(const T& input, std::exception& e)
    { }
    virtual void on_finish(const std::vector<R>& result)
    { }

    struct queued_item
    {
        T        item;
        uint32_t index;
    };

public:
    async::task<void> run(std::vector<R>& result)
    {
        auto queue = fb::synchronized<std::queue<queued_item>>{};
        auto count = uint32_t{0};
        {
            auto gen = this->on_ready();
            while (gen.next())
            {
                auto guard = queue.enter_write();
                guard.value().push({std::move(gen.value()), count++});
            }
        }

        if (count == 0)
        {
            this->on_finish(result);
            co_return;
        }

        auto processed      = std::atomic<uint32_t>{0};
        auto outputs_buffer = std::vector<std::vector<R>>(count);
        auto worker         = [this, &queue, count, &processed, &outputs_buffer]() -> async::task<void> {
            while (true)
            {
                auto work = std::optional<queued_item>{};
                {
                    auto  guard = queue.enter_write();
                    auto& q     = guard.value();
                    if (q.empty())
                        break;

                    work = std::move(q.front());
                    q.pop();
                }

                try
                {
                    auto outputs = std::vector<R>{};
                    auto gen     = this->on_work(work->item);
                    while (gen.next())
                        outputs.push_back(gen.value());

                    for (auto& output : outputs)
                        this->on_worked(work->item, output, (++processed * 100.0) / static_cast<double>(count));

                    outputs_buffer[work->index] = std::move(outputs);
                }
                catch (std::exception& e)
                {
                    processed++;
                    this->on_error(work->item, e);
                }
            }
            co_return;
        };

        const auto thread_count = this->_executor.threads.count();
        if (thread_count == 0)
        {
            co_await worker();
        }
        else
        {
            auto tasks = std::vector<async::task<void>>{};
            tasks.reserve(thread_count);

            for (uint8_t i = 0; i < thread_count; ++i)
            {
                auto& thread = *this->_executor.threads.at(i);
                tasks.push_back(thread.dispatch([&worker](auto&) -> async::task<void> {
                    co_return co_await worker();
                }));
            }

            for (auto& task : tasks)
                co_await task;
        }

        for (auto& outputs : outputs_buffer)
        {
            for (auto& output : outputs)
                result.push_back(std::move(output));
        }

        this->on_finish(result);
        co_return;
    }
};

template <typename T>
class parallel_worker<T, void>
{
protected:
    fb::async_executor& _executor;

    explicit parallel_worker(fb::async_executor& executor) :
        _executor(executor)
    { }

    virtual fb::generator<T>  on_ready()              = 0;
    virtual async::task<void> on_work(const T& value) = 0;
    virtual void              on_worked(const T& input, double percent)
    { }
    virtual void on_error(const T& input, std::exception& e)
    { }
    virtual void on_finish()
    { }

public:
    async::task<void> run()
    {
        auto queue = fb::synchronized<std::queue<T>>{};
        auto total = uint32_t{0};
        {
            auto gen = this->on_ready();
            while (gen.next())
            {
                auto guard = queue.enter_write();
                guard.value().push(std::move(gen.value()));
                total++;
            }
        }

        const auto count = static_cast<double>(total);
        if (count == 0)
        {
            this->on_finish();
            co_return;
        }

        auto processed = std::atomic<uint32_t>{0};
        auto worker    = [this, &queue, count, &processed]() -> async::task<void> {
            while (true)
            {
                auto item = std::optional<T>{};
                {
                    auto  guard = queue.enter_write();
                    auto& q     = guard.value();
                    if (q.empty())
                        break;

                    item = std::move(q.front());
                    q.pop();
                }

                try
                {
                    co_await this->on_work(*item);
                    this->on_worked(*item, (++processed * 100.0) / count);
                }
                catch (std::exception& e)
                {
                    processed++;
                    this->on_error(*item, e);
                }
            }
            co_return;
        };

        const auto thread_count = this->_executor.threads.count();
        if (thread_count == 0)
        {
            co_await worker();
        }
        else
        {
            auto tasks = std::vector<async::task<void>>{};
            tasks.reserve(thread_count);

            for (uint8_t i = 0; i < thread_count; ++i)
            {
                auto& thread = *this->_executor.threads.at(i);
                tasks.push_back(thread.dispatch([&worker](auto&) -> async::task<void> {
                    co_return co_await worker();
                }));
            }

            for (auto& task : tasks)
                co_await task;
        }

        this->on_finish();
        co_return;
    }
};

} // namespace fb

#endif
