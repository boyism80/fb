#ifndef __BOT_CONTAINER_H__
#define __BOT_CONTAINER_H__

#include <atomic>
#include <fb/bot/bot.h>
#include <fb/thread_container.h>

namespace fb::bot {

// Forward declarations for bot_controllers
class gateway_bot_controller;
class login_bot_controller;
class game_bot_controller;

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
    template <typename BotType>
    friend class bot_controller;

private:
    std::atomic<uint32_t>    _sequence          = 0; ///< Atomic sequence counter for generating unique bot IDs
    uint32_t                 _gateway_bot_count = 0; ///< Number of gateway bots to create
    uint32_t                 _login_bot_count   = 0; ///< Number of login bots to create
    uint32_t                 _game_bot_count    = 0; ///< Number of game bots to create
    boost::asio::io_context& _context;               ///< Reference to the I/O context for network operations
    bool                     _exit = false;          ///< Flag indicating if the container is shutting down

public:
    fb::model::model                        model;
    std::shared_ptr<gateway_bot_controller> gateway;
    std::shared_ptr<login_bot_controller>   login;
    std::shared_ptr<game_bot_controller>    game;

public:
    /**
     * @brief      Constructs a new bot container.
     *
     * @param      context  The boost::asio I/O context for network operations.
     */
    bot_container(boost::asio::io_context& context);

    /**
     * @brief      Sets the gateway bot bot_controller for this container.
     *
     * @param      bot_controller  The gateway bot bot_controller instance to use.
     */
    void set_gateway_bot_controller(std::shared_ptr<gateway_bot_controller> bot_controller);

    /**
     * @brief      Sets the login bot bot_controller for this container.
     *
     * @param      bot_controller  The login bot bot_controller instance to use.
     */
    void set_login_bot_controller(std::shared_ptr<login_bot_controller> bot_controller);

    /**
     * @brief      Sets the game bot bot_controller for this container.
     *
     * @param      bot_controller  The game bot bot_controller instance to use.
     */
    void set_game_bot_controller(std::shared_ptr<game_bot_controller> bot_controller);

    /**
     * @brief      Initializes the bot container after construction.
     *
     *             This method must be called after the object is fully constructed
     *             and managed by a shared_ptr. It sets up timers and begins bot spawning.
     */
    void initialize();

    /**
     * @brief      Destroys the bot container and cleans up all bots.
     */
    ~bot_container();

private:
    /**
     * @brief      Creates a new bot instance of the specified type.
     *
     *             Instantiates a bot with a unique ID and registers it with
     *             the appropriate worker thread for execution. This method
     *             can only be called by bot_controller classes.
     *
     * @tparam     T     The bot type to create (must inherit from base_bot).
     *
     * @return     Shared pointer to the newly created bot instance.
     */
    template <typename T>
    std::shared_ptr<T> create()
    {
        auto id     = this->_sequence++;
        auto bot    = std::make_shared<T>(*this, id);
        std::ignore = bot->thread()->dispatch([id, bot](auto& thread) -> async::task<void> {
            auto params = thread.template data<bot_thread_params>();
            params->bots.insert({id, bot});
            co_return;
        });
        return bot;
    }

    /**
     * @brief      Creates a new bot instance with a bot_controller.
     *
     *             Instantiates a bot with a unique ID and registers it with
     *             the appropriate worker thread for execution. This method
     *             can only be called by bot_controller classes.
     *
     * @tparam     T          The bot type to create (must inherit from base_bot).
     * @tparam     Controller The bot_controller type that will manage this bot.
     * @param[in]  bot_controller The bot bot_controller that will manage this bot.
     *
     * @return     Shared pointer to the newly created bot instance.
     */
    template <typename T, typename Controller>
    std::shared_ptr<T> create(Controller& bot_controller)
    {
        auto id     = this->_sequence++;
        auto bot    = std::make_shared<T>(bot_controller, id);
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
     *             then registers it with the appropriate worker thread. This method
     *             can only be called by bot_controller classes.
     *
     * @tparam     T          The bot type to create (must inherit from base_bot).
     * @tparam     Controller The bot_controller type that will manage this bot.
     * @param[in]  bot_controller The bot bot_controller that will manage this bot.
     * @param[in]  params     Initialization parameters for the bot.
     *
     * @return     Shared pointer to the newly created bot instance.
     */
    template <typename T, typename Controller>
    std::shared_ptr<T> create(Controller& bot_controller, const fb::stream& params)
    {
        auto id     = this->_sequence++;
        auto bot    = std::make_shared<T>(bot_controller, id, params);
        std::ignore = bot->thread()->dispatch([id, bot](auto& thread) -> async::task<void> {
            auto params = thread.template data<bot_thread_params>();
            params->bots.insert({id, bot});
            co_return;
        });
        return bot;
    }

public:
    /**
     * @brief      Gets the I/O context used by this container.
     *
     * @return     Reference to the boost::asio I/O context for network operations.
     */
    boost::asio::io_context& context() const;

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
};

} // namespace fb::bot

#endif