#ifndef __WORKER_H__
#define __WORKER_H__

#include <fb/parallel_worker.h>
#include <model.h>

namespace fb { namespace game {

class context;

class model_loader : public fb::parallel_worker<std::reference_wrapper<fb::model::container>>
{
public:
    using input_type = std::reference_wrapper<fb::model::container>;

private:
    fb::game::context& _context;

public:
    model_loader(fb::game::context& context);
    ~model_loader() = default;

protected:
    fb::generator<input_type>   on_ready();
    void                        on_work(const input_type& value);
    void                        on_worked(const input_type& input, double percent);
    void                        on_error(const input_type& input, std::exception& e);
    void                        on_finish();
};

class map_loader : public fb::parallel_worker<std::reference_wrapper<fb::model::map>>
{
public:
    using input_type = std::reference_wrapper<fb::model::map>;

private:
    fb::game::context& _context;

public:
    map_loader(fb::game::context& context);
    ~map_loader() = default;

protected:
    fb::generator<input_type>   on_ready();
    void                        on_work(const input_type& value);
    void                        on_worked(const input_type& input, double percent);
    void                        on_error(const input_type& input, std::exception& e);
    void                        on_finish();
};

class npc_spawner : public fb::parallel_worker<std::reference_wrapper<fb::model::npc_spawn>>
{
public:
    using input_type = std::reference_wrapper<fb::model::npc_spawn>;

private:
    fb::game::context& _context;

public:
    npc_spawner(fb::game::context& context);
    ~npc_spawner() = default;

protected:
    fb::generator<input_type>   on_ready();
    void                        on_work(const input_type& value);
    void                        on_worked(const input_type& input, double percent);
    void                        on_error(const input_type& input, std::exception& e);
    void                        on_finish();
};

} }

#endif