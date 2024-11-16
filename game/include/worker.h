#ifndef __WORKER_H__
#define __WORKER_H__

#include <fb/model/loader.h>

namespace fb { namespace game {

/**
 * @brief      This class describes a context.
 */
class context;

/**
 * @brief      This class describes a map loader.
 */
class map_loader : public fb::parallel_worker<std::reference_wrapper<fb::model::map>>
{
public:
    using input_type = std::reference_wrapper<fb::model::map>;

private:
    fb::game::context& _context;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     */
    map_loader(fb::game::context& context);
    /**
     * @brief      Destroys the object.
     */
    ~map_loader() = default;

protected:
    /**
     * @brief      Called on ready.
     *
     * @return     { description_of_the_return_value }
     */
    fb::generator<input_type> on_ready();
    /**
     * @brief      Called on work.
     *
     * @param[in]  value  The value
     */
    void on_work(const input_type& value);
    /**
     * @brief      Called when worked.
     *
     * @param[in]  input    The input
     * @param[in]  percent  The percent
     */
    void on_worked(const input_type& input, double percent);
    /**
     * @brief      Called on error.
     *
     * @param[in]  input  The input
     * @param      e      { parameter_description }
     */
    void on_error(const input_type& input, std::exception& e);
    /**
     * @brief      Called on finish.
     */
    void on_finish();
};

/**
 * @brief      This class describes a npc spawner.
 */
class npc_spawner : public fb::parallel_worker<std::reference_wrapper<fb::model::npc_spawn>>
{
public:
    using input_type = std::reference_wrapper<fb::model::npc_spawn>;

private:
    fb::game::context& _context;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     */
    npc_spawner(fb::game::context& context);
    /**
     * @brief      Destroys the object.
     */
    ~npc_spawner() = default;

protected:
    /**
     * @brief      Called on ready.
     *
     * @return     { description_of_the_return_value }
     */
    fb::generator<input_type> on_ready();
    /**
     * @brief      Called on work.
     *
     * @param[in]  value  The value
     */
    void on_work(const input_type& value);
    /**
     * @brief      Called when worked.
     *
     * @param[in]  input    The input
     * @param[in]  percent  The percent
     */
    void on_worked(const input_type& input, double percent);
    /**
     * @brief      Called on error.
     *
     * @param[in]  input  The input
     * @param      e      { parameter_description }
     */
    void on_error(const input_type& input, std::exception& e);
    /**
     * @brief      Called on finish.
     */
    void on_finish();
};

}} // namespace fb::game

#endif