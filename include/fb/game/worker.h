#ifndef __WORKER_H__
#define __WORKER_H__

#include <fb/model/loader.h>
#include <filesystem>

namespace fb { namespace game {

/**
 * @brief      Forward declaration of the game context class.
 */
class context;

/**
 * @brief      Parallel worker for loading and processing game maps.
 *
 *             This class extends the parallel worker framework to handle map loading operations
 *             in a multi-threaded environment. It processes map data from the model system and
 *             initializes them for use in the game world. The worker handles map file loading,
 *             validation, and integration with the game context.
 */
class map_loader : public fb::parallel_worker<std::reference_wrapper<fb::model::map>>
{
public:
    using input_type = std::reference_wrapper<fb::model::map>;

private:
    fb::game::context& _context;

public:
    /**
     * @brief      Constructs a new map loader worker.
     *
     *             Initializes the map loader with a reference to the game context
     *             that will manage the loaded maps.
     *
     * @param      context  The game context that will own the loaded maps.
     */
    map_loader(fb::game::context& context);
    /**
     * @brief      Destroys the map loader worker.
     */
    ~map_loader() = default;

protected:
    /**
     * @brief      Generates the sequence of maps to be loaded.
     *
     *             Called when the worker is ready to begin processing. Returns a generator
     *             that yields references to all maps that need to be loaded from the model system.
     *
     * @return     A generator yielding map references to be processed.
     */
    fb::generator<input_type> on_ready() override final;
    /**
     * @brief      Processes a single map loading operation.
     *
     *             Performs the actual map loading work including file I/O, data validation,
     *             and map initialization. This method is called for each map in parallel.
     *
     * @param[in]  value  The map reference to load and process.
     */
    void on_work(const input_type& value) override final;
    /**
     * @brief      Reports progress during map loading operations.
     *
     *             Called periodically to report the progress of map loading operations.
     *             Can be used to update loading screens or progress indicators.
     *
     * @param[in]  input    The map reference being processed.
     * @param[in]  percent  The completion percentage (0.0 to 1.0).
     */
    void on_worked(const input_type& input, double percent) override final;
    /**
     * @brief      Handles errors that occur during map loading.
     *
     *             Called when an error occurs while loading a specific map. Logs the error
     *             and may attempt recovery or skip the problematic map.
     *
     * @param[in]  input  The map reference that failed to load.
     * @param      e      The exception that occurred during loading.
     */
    void on_error(const input_type& input, std::exception& e) override final;
    /**
     * @brief      Finalizes the map loading process.
     *
     *             Called when all map loading operations are complete. Performs any
     *             necessary cleanup and finalization of the loaded map data.
     */
    void on_finish() override final;
};

/**
 * @brief      Parallel worker for spawning NPCs in the game world.
 *
 *             This class handles the spawning of NPCs (Non-Player Characters) across all maps
 *             in a multi-threaded environment. It processes NPC spawn data from the model system
 *             and creates NPC instances in their designated locations within the game world.
 */
class npc_spawner : public fb::parallel_worker<std::reference_wrapper<fb::model::npc_spawn>>
{
public:
    using input_type = std::reference_wrapper<fb::model::npc_spawn>;

private:
    fb::game::context& _context;

public:
    /**
     * @brief      Constructs a new NPC spawner worker.
     *
     *             Initializes the NPC spawner with a reference to the game context
     *             that will manage the spawned NPCs.
     *
     * @param      context  The game context that will own the spawned NPCs.
     */
    npc_spawner(fb::game::context& context);
    /**
     * @brief      Destroys the NPC spawner worker.
     */
    ~npc_spawner() = default;

protected:
    /**
     * @brief      Generates the sequence of NPC spawns to be processed.
     *
     *             Called when the worker is ready to begin spawning NPCs. Returns a generator
     *             that yields references to all NPC spawn configurations from the model system.
     *
     * @return     A generator yielding NPC spawn references to be processed.
     */
    fb::generator<input_type> on_ready() override final;
    /**
     * @brief      Processes a single NPC spawning operation.
     *
     *             Creates an NPC instance based on the spawn configuration and places it
     *             in the appropriate location within the game world. This method is called
     *             for each NPC spawn in parallel.
     *
     * @param[in]  value  The NPC spawn configuration to process.
     */
    void on_work(const input_type& value) override final;
    /**
     * @brief      Reports progress during NPC spawning operations.
     *
     *             Called periodically to report the progress of NPC spawning operations.
     *             Can be used to update loading screens or progress indicators.
     *
     * @param[in]  input    The NPC spawn configuration being processed.
     * @param[in]  percent  The completion percentage (0.0 to 1.0).
     */
    void on_worked(const input_type& input, double percent) override final;
    /**
     * @brief      Handles errors that occur during NPC spawning.
     *
     *             Called when an error occurs while spawning a specific NPC. Logs the error
     *             and may attempt recovery or skip the problematic NPC spawn.
     *
     * @param[in]  input  The NPC spawn configuration that failed.
     * @param      e      The exception that occurred during spawning.
     */
    void on_error(const input_type& input, std::exception& e) override final;
    /**
     * @brief      Finalizes the NPC spawning process.
     *
     *             Called when all NPC spawning operations are complete. Performs any
     *             necessary cleanup and finalization of the spawned NPC data.
     */
    void on_finish() override final;
};

/**
 * @brief      Parallel worker for loading and executing Lua scripts.
 *
 *             This class handles the loading and execution of Lua scripts in a multi-threaded
 *             environment. It processes script loading tasks and executes them within the
 *             appropriate Lua contexts, enabling dynamic game logic and content loading.
 */
class script_loader : public fb::parallel_worker<std::function<async::task<void>()>>
{
private:
    fb::game::context& _context;

public:
    /**
     * @brief      Constructs a new script loader worker.
     *
     *             Initializes the script loader with a reference to the game context
     *             that provides the Lua execution environment.
     *
     * @param      context  The game context that provides the Lua environment.
     */
    script_loader(fb::game::context& context);
    /**
     * @brief      Destroys the script loader worker.
     */
    ~script_loader() = default;

protected:
    /**
     * @brief      Generates the sequence of script loading tasks to be executed.
     *
     *             Called when the worker is ready to begin loading scripts. Returns a generator
     *             that yields async task functions for loading and executing Lua scripts.
     *
     * @return     A generator yielding script loading task functions.
     */
    fb::generator<std::function<async::task<void>()>> on_ready() override final;
    /**
     * @brief      Executes a single script loading task.
     *
     *             Executes the provided async task function which typically loads and
     *             runs a Lua script within the game context. This method is called
     *             for each script loading task in parallel.
     *
     * @param[in]  value  The script loading task function to execute.
     */
    void on_work(const std::function<async::task<void>()>& value) override final;
    /**
     * @brief      Reports progress during script loading operations.
     *
     *             Called periodically to report the progress of script loading operations.
     *             Can be used to update loading screens or progress indicators.
     *
     * @param[in]  input    The script loading task being processed.
     * @param[in]  percent  The completion percentage (0.0 to 1.0).
     */
    void on_worked(const std::function<async::task<void>()>& input, double percent) override final;
    /**
     * @brief      Handles errors that occur during script loading.
     *
     *             Called when an error occurs while loading or executing a script. Logs the error
     *             and may attempt recovery or skip the problematic script.
     *
     * @param[in]  input  The script loading task that failed.
     * @param      e      The exception that occurred during script loading.
     */
    void on_error(const std::function<async::task<void>()>& input, std::exception& e) override final;
    /**
     * @brief      Finalizes the script loading process.
     *
     *             Called when all script loading operations are complete. Performs any
     *             necessary cleanup and finalization of the script loading process.
     */
    void on_finish() override final;
};

}} // namespace fb::game

#endif