#ifndef __WORKER_H__
#define __WORKER_H__

#include <fb/model/loader.h>
#include <filesystem>

namespace fb::game {

class server;

class map_loader : public fb::parallel_worker<std::reference_wrapper<fb::model::map>>
{
public:
    using input_type = std::reference_wrapper<fb::model::map>;

private:
    fb::game::server& _server;

public:
    map_loader(fb::game::server& server);
    ~map_loader() = default;

protected:
    fb::generator<input_type> on_ready() override final;
    void                      on_work(const input_type& value) override final;
    void                      on_worked(const input_type& input, double percent) override final;
    void                      on_error(const input_type& input, std::exception& e) override final;
    void                      on_finish() override final;
};

class npc_spawner : public fb::parallel_worker<std::reference_wrapper<fb::model::npc_spawn>>
{
public:
    using input_type = std::reference_wrapper<fb::model::npc_spawn>;

private:
    fb::game::server& _server;

public:
    npc_spawner(fb::game::server& server);
    ~npc_spawner() = default;

protected:
    fb::generator<input_type> on_ready() override final;
    void                      on_work(const input_type& value) override final;
    void                      on_worked(const input_type& input, double percent) override final;
    void                      on_error(const input_type& input, std::exception& e) override final;
    void                      on_finish() override final;
};

class script_loader : public fb::parallel_worker<std::function<async::task<void>()>>
{
    using ready_func_generator = fb::generator<std::function<async::task<void>()>>;

private:
    fb::game::server& _server;

public:
    script_loader(fb::game::server& server);
    ~script_loader() = default;

protected:
    ready_func_generator on_ready() override final;
    void                 on_work(const std::function<async::task<void>()>& value) override final;
    void                 on_worked(const std::function<async::task<void>()>& input, double percent) override final;
    void                 on_error(const std::function<async::task<void>()>& input, std::exception& e) override final;
    void                 on_finish() override final;
};

} // namespace fb::game

#endif // __WORKER_H__