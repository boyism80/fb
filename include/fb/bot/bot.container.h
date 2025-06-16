#ifndef __BOT_CONTAINER_H__
#define __BOT_CONTAINER_H__

#include <fb/bot/bot.h>
#include <fb/thread_container.h>

namespace fb::bot {

/**
 * @brief      Thread-specific parameters for bot management.
 *
 *             This class stores bot instances associated with a specific thread.
 *             Each worker thread maintains its own collection of bots to ensure
 *             thread-safe operations and proper resource management.
 */
class bot_thread_params
{
public:
    std::unordered_map<uint32_t, std::shared_ptr<base_bot>> bots; ///< Map of bot ID to bot instance for this thread
};

/**
 * @brief      Container for managing multiple bot instances across threads.
 *
 *             This class provides a centralized management system for automated
 *             game client bots. It handles bot creation, lifecycle management,
 *             thread distribution, and provides utilities for bot coordination.
 *             The container supports different types of bots (gateway, login, game)
 *             and manages their execution across multiple worker threads.
 */
class bot_container : public fb::context
{
private:
    uint32_t                 _remained_count;        ///< Number of bots remaining to be spawned
    uint32_t                 _sequence          = 0; ///< Sequence counter for generating unique bot IDs
    uint32_t                 _gateway_bot_count = 0; ///< Number of gateway bots to create
    uint32_t                 _login_bot_count   = 0; ///< Number of login bots to create
    uint32_t                 _game_bot_count    = 0; ///< Number of game bots to create
    boost::asio::io_context& _context;               ///< Reference to the I/O context for network operations
    bool                     _exit = false;          ///< Flag indicating if the container is shutting down
    std::mutex               _mutex;                 ///< Mutex for thread-safe operations

public:
    /**
     * @brief      Constructs a new bot container.
     *
     * @param      context  The boost::asio I/O context for network operations.
     */
    bot_container(boost::asio::io_context& context);

    /**
     * @brief      Destroys the bot container and cleans up all bots.
     */
    ~bot_container();

private:
    /**
     * @brief      Handles the spawning of new bot instances.
     *
     *             Internal method that manages the creation and initialization
     *             of bot instances based on the configured bot counts.
     *
     * @return     An async task that completes when bot spawning is finished.
     */
    async::task<void> handle_bot_spawn();

public:
    /**
     * @brief      Gets the I/O context used by this container.
     *
     * @return     Reference to the boost::asio I/O context for network operations.
     */
    boost::asio::io_context& context() const;

    /**
     * @brief      Handles incoming data for bot sockets.
     *
     *             Delegates data processing to the appropriate bot instance.
     *             This method is called by the socket framework when data arrives.
     *
     * @param      socket  The socket that received the data.
     * @param      stream  The incoming data stream.
     *
     * @return     An async task that completes when data processing is finished.
     */
    async::task<void> on_receive(fb::socket<>& socket, fb::stream& stream)
    {
        co_await static_cast<base_bot&>(socket).on_receive(stream);
    }

    /**
     * @brief      Handles socket closure events for bot connections.
     *
     *             Performs cleanup when a bot's connection is closed and removes
     *             the bot from the thread-local bot collection.
     *
     * @param      socket  The socket that was closed.
     *
     * @return     An async task that completes when cleanup is finished.
     */
    async::task<void> on_closed(fb::socket<>& socket)
    {
        auto& bot   = static_cast<base_bot&>(socket);
        std::ignore = bot.thread()->dispatch([&bot](auto& thread) -> async::task<void> {
            co_await bot.on_closed();
            auto params = thread.template data<bot_thread_params>();
            params->bots.erase(bot.id);
        });
        co_return;
    }

    /**
     * @brief      Creates a new bot instance of the specified type.
     *
     *             Instantiates a bot with a unique ID and registers it with
     *             the appropriate worker thread for execution.
     *
     * @tparam     T     The bot type to create (must inherit from base_bot).
     *
     * @return     Shared pointer to the newly created bot instance.
     */
    template <typename T>
    std::shared_ptr<T> create()
    {
        this->_mutex.lock();
        auto id = this->_sequence++;
        this->_mutex.unlock();
        auto bot    = std::make_shared<T>(*this, id);
        std::ignore = bot->thread()->dispatch([id, bot](auto& thread) -> async::task<void> {
            auto params = thread.template data<bot_thread_params>();
            params->bots.insert({id, bot});
            co_return;
        });
        return bot;
    }

    /**
     * @brief      Creates a new bot instance with initialization parameters.
     *
     *             Instantiates a bot with a unique ID and custom parameters,
     *             then registers it with the appropriate worker thread.
     *
     * @param[in]  params  Initialization parameters for the bot.
     *
     * @tparam     T       The bot type to create (must inherit from base_bot).
     *
     * @return     Shared pointer to the newly created bot instance.
     */
    template <typename T>
    std::shared_ptr<T> create(const fb::stream& params)
    {
        this->_mutex.lock();
        auto id = this->_sequence++;
        this->_mutex.unlock();
        auto bot    = std::make_shared<T>(*this, id, params);
        std::ignore = bot->thread()->dispatch([id, bot](auto& thread) -> async::task<void> {
            auto params = thread.template data<bot_thread_params>();
            params->bots.insert({id, bot});
            co_return;
        });
        return bot;
    }

    /**
     * @brief      Handles timer events for all bots in a specific thread.
     *
     *             Distributes timer notifications to all bot instances
     *             managed by the specified thread.
     *
     * @param[in]  now  The current date and time.
     * @param[in]  id   The thread identifier.
     *
     * @return     An async task that completes when all bots have processed the timer.
     */
    async::task<void> handle_timer(const fb::model::datetime& now, std::thread::id id);

    /**
     * @brief      Dispatches a function to a specific bot's thread.
     *
     *             Executes the given function on the thread that manages
     *             the bot with the specified ID.
     *
     * @param[in]  id  The bot ID whose thread should execute the function.
     * @param      fn  The function to execute on the bot's thread.
     *
     * @return     An async task that completes when the function execution is finished.
     */
    async::task<void> dispatch(uint32_t id, std::function<async::task<void>(fb::thread&)>&& fn);

    /**
     * @brief      Displays statistics about spawned bots.
     *
     *             Static utility method that outputs information about
     *             the current state of bot instances across all containers.
     */
    static void display_spawned_bots();
};

} // namespace fb::bot

#endif