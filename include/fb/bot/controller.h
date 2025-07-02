#ifndef __BOT_CONTROLLER_H__
#define __BOT_CONTROLLER_H__

#include <fb/bot/bot.h>
#include <fb/bot/container.h>
#include <fb/locker.h>
#include <fb/protocol/header.h>

namespace fb::bot {

class base_bot_controller
{
public:
    bot_container& container;

public:
    base_bot_controller(bot_container& container) :
        container(container)
    { }

public:
    virtual async::task<void> on_receive(fb::socket<>& socket, fb::stream& stream) = 0;
    virtual async::task<void> on_closed(fb::socket<>& socket)                      = 0;

    /**
     * @brief      Determines whether a command should be decrypted.
     *
     *             Specifies which protocol commands require decryption based on
     *             the bot_controller's security policy. Override this method to
     *             customize decryption behavior for specific bot types.
     *
     * @param[in]  cmd  The protocol command identifier.
     *
     * @return     True if the command should be decrypted, false otherwise.
     */
    virtual bool decrypt_policy(int cmd) const = 0;

    /**
     * @brief      Handles bot connection events.
     *
     *             Pure virtual function called when a bot successfully connects
     *             to the server. Must be implemented by concrete bot_controller classes.
     *
     * @param      bot  The bot that connected.
     *
     * @return     An async task that completes when connection handling is finished.
     */
    virtual async::task<void> on_bot_connected(base_bot& bot) = 0;

    /**
     * @brief      Handles bot disconnection events.
     *
     *             Pure virtual function called when a bot disconnects from
     *             the server. Must be implemented by concrete bot_controller classes.
     *
     * @param      bot  The bot that disconnected.
     *
     * @return     An async task that completes when disconnection handling is finished.
     */
    virtual async::task<void> on_bot_disconnected(base_bot& bot) = 0;

    /**
     * @brief      Ensures a handler is registered for the specified command.
     *
     *             Pure virtual function that ensures a deserializer and handler
     *             are registered for the given command, enabling hook processing.
     *
     * @param[in]  cmd  The protocol command identifier.
     */
    virtual void ensure_handler_registered(uint8_t cmd) = 0;
};

template <typename BotType>
class bot_controller : public base_bot_controller
{
public:
    using handle_func = std::function<async::task<void>(BotType&, fb::protocol::header&)>;
    using deserilze_func =
        std::function<async::task<std::shared_ptr<fb::protocol::header>>(fb::stream_reader<big_endian>&)>;

private:
    std::unordered_map<uint8_t, handle_func>    _handler;
    std::unordered_map<uint8_t, deserilze_func> _deserializer;
    std::shared_mutex                           _handler_mutex; ///< Mutex for thread-safe handler operations

protected:
    /**
     * @brief      Thread-safe container for managing active bot instances.
     *
     *             Uses fb::locker to provide thread-safe access to the bot collection.
     *             Key is bot ID, value is shared pointer to the bot instance.
     */
    fb::locker<std::unordered_map<uint32_t, std::shared_ptr<BotType>>> _bots;

protected:
    bot_controller(bot_container& container) :
        base_bot_controller(container)
    { }

protected:
    /**
     * @brief      Default decryption policy implementation.
     *
     *             By default, all commands are decrypted. Override this method
     *             in specific bot_controller implementations to customize behavior.
     *
     * @param[in]  cmd  The protocol command identifier.
     *
     * @return     True by default (decrypt all commands).
     */
    virtual bool decrypt_policy(int cmd) const override
    {
        return true;
    }

    template <typename Class>
    void bind_timer(async::task<void> (Class::*fn)(), std::chrono::steady_clock::duration interval)
    {
        this->container.bind_timer(std::bind(fn, static_cast<Class*>(this)), interval);
    }

    template <typename Class>
    void bind_thread_timer(async::task<void> (Class::*fn)(const fb::model::datetime&, std::thread::id),
                           std::chrono::steady_clock::duration interval)
    {
        this->container.bind_thread_timer(
            std::bind(fn, static_cast<Class*>(this), std::placeholders::_1, std::placeholders::_2),
            interval);
    }

    /**
     * @brief      Handles bot connection events (base implementation).
     *
     *             Casts the base bot to the specific bot type and delegates
     *             to the typed connection handler.
     *
     * @param      bot  The bot that connected.
     *
     * @return     An async task that completes when connection handling is finished.
     */
    virtual async::task<void> on_bot_connected(base_bot& bot) override
    {
        static_assert(std::is_base_of_v<base_bot, BotType>, "BotType must inherit from base_bot");
        auto& typed_bot = static_cast<BotType&>(bot);
        co_await this->on_bot_connected(typed_bot);
    }

    /**
     * @brief      Handles bot disconnection events (base implementation).
     *
     *             Casts the base bot to the specific bot type and delegates
     *             to the typed disconnection handler.
     *
     * @param      bot  The bot that disconnected.
     *
     * @return     An async task that completes when disconnection handling is finished.
     */
    virtual async::task<void> on_bot_disconnected(base_bot& bot) override
    {
        static_assert(std::is_base_of_v<base_bot, BotType>, "BotType must inherit from base_bot");
        auto& typed_bot = static_cast<BotType&>(bot);
        co_await this->on_bot_disconnected(typed_bot);
    }

    /**
     * @brief      Ensures a handler is registered for the specified command.
     *
     *             Registers a default deserializer and handler for the given command
     *             to enable hook processing even when no specific handler exists.
     *
     * @param[in]  cmd  The protocol command identifier.
     */
    virtual void ensure_handler_registered(uint8_t cmd) override
    {
        // First check with shared lock (read access)
        {
            auto shared_lock = std::shared_lock<std::shared_mutex>(this->_handler_mutex);
            if (this->_deserializer.contains(cmd))
                return; // Already registered
        }

        // Need to register handler, acquire exclusive lock
        auto unique_lock = std::unique_lock<std::shared_mutex>(this->_handler_mutex);

        // Double-check in case another thread registered it while we were waiting
        if (this->_deserializer.contains(cmd))
            return;

        // Register default deserializer and empty handler for hook processing
        this->_deserializer[cmd] =
            [](fb::stream_reader<big_endian>& reader) -> async::task<std::shared_ptr<fb::protocol::header>> {
            // Return nullptr for default deserialization - hook processing uses cmd parameter separately
            co_return nullptr;
        };

        this->_handler[cmd] = [](BotType& bot, const fb::protocol::header& protocol) -> async::task<void> {
            // Empty handler - hooks will still be processed
            co_return;
        };
    }

public:
    /**
     * @brief      Ensures a handler is registered for the specified protocol type.
     *
     *             Uses the existing bind mechanism to register an empty handler
     *             for the given protocol type, enabling hook processing.
     *
     * @tparam     ResponseType  The protocol response type to ensure handler for.
     */
    template <typename ResponseType>
    void ensure_handler_registered()
    {
        // First check with shared lock (read access)
        {
            auto shared_lock = std::shared_lock<std::shared_mutex>(this->_handler_mutex);
            if (this->_handler.contains(ResponseType::header))
                return; // Already registered
        }

        // Use existing bind mechanism with empty handler
        this->bind<ResponseType>([](BotType& bot, const ResponseType& protocol) -> async::task<void> {
            // Empty handler - hooks will still be processed
            co_return;
        });
    }

    /**
     * @brief      Creates a new bot instance without parameters.
     *
     * @return     Shared pointer to the newly created bot instance.
     */
    std::shared_ptr<BotType> create()
    {
        // Cast to derived bot_controller type for bot creation
        auto& derived_bot_controller = static_cast<typename BotType::bot_controller_type&>(*this);
        auto  bot                    = this->container.create<BotType>(derived_bot_controller);

        // Add bot to our managed collection
        this->_bots.write([&](auto& bots) {
            bots[bot->id] = bot;
        });

        return bot;
    }

    /**
     * @brief      Creates a new bot instance with initialization parameters.
     *
     * @param[in]  params  Transfer parameters containing crypto information.
     *
     * @return     Shared pointer to the newly created bot instance.
     */
    std::shared_ptr<BotType> create(const fb::stream& params)
    {
        // Cast to derived bot_controller type for bot creation
        auto& derived_bot_controller = static_cast<typename BotType::bot_controller_type&>(*this);
        auto  bot                    = this->container.create<BotType>(derived_bot_controller, params);

        // Add bot to our managed collection
        this->_bots.write([&](auto& bots) {
            bots[bot->id] = bot;
        });

        return bot;
    }

    /**
     * @brief      Checks if a bot with the specified ID is managed by this bot_controller.
     *
     *             Provides thread-safe read access to the bot collection to check
     *             if a bot with the given ID exists.
     *
     * @param[in]  id  The unique identifier of the bot.
     *
     * @return     True if the bot is managed, false otherwise.
     */
    bool contains(uint32_t id) const
    {
        return this->_bots.template read<bool>([&](const auto& bots) {
            return bots.contains(id);
        });
    }

    async::task<void> on_receive(fb::socket<>& socket, fb::stream& stream)
    {
        static_assert(std::is_base_of_v<base_bot, BotType>, "BotType must inherit from base_bot");
        static constexpr uint8_t base_size = sizeof(uint8_t) + sizeof(uint16_t);

        auto& bot    = static_cast<BotType&>(static_cast<base_bot&>(socket));
        auto  reader = fb::stream_reader<big_endian>(stream);
        while (true)
        {
            try
            {
                if (reader.readable_size() < base_size)
                    co_return;

                auto head = reader.read<uint8_t>();
                if (head != 0xAA)
                    throw std::runtime_error("magic code mismatch");

                auto size = reader.read<uint16_t>();
                if (size > reader.readable_size())
                {
                    reader.seek(0);
                    co_return;
                }

                auto cmd = reader.read<uint8_t>();
                if (this->decrypt_policy(cmd))
                {
                    auto& crypto = bot.crt();
                    size         = crypto.decrypt(stream, reader.seek() - 1, size);
                }

                reader.flush();

                // Thread-safe handler lookup
                std::shared_ptr<fb::protocol::header> protocol;
                handle_func                           handler;
                deserilze_func                        deserializer;

                {
                    auto shared_lock = std::shared_lock<std::shared_mutex>(this->_handler_mutex);
                    // Deserializer should always exist now due to ensure_handler_registered
                    if (this->_deserializer.contains(cmd))
                    {
                        deserializer = this->_deserializer.at(cmd);

                        if (this->_handler.contains(cmd))
                            handler = this->_handler.at(cmd);
                    }
                    else
                    {
                        // Fallback: create default deserializer if somehow missing
                        deserializer = [](fb::stream_reader<big_endian>& reader)
                            -> async::task<std::shared_ptr<fb::protocol::header>> {
                            // Return nullptr for default deserialization - hook processing uses cmd parameter
                            // separately
                            co_return nullptr;
                        };
                    }
                }

                // Process packet with deserializer (always exists now)
                protocol = co_await deserializer(reader);
                if (protocol != nullptr)
                {
                    auto weak = bot.template weak_from_this_as<BotType>();

                    this->container.threads.enqueue(weak, [=](auto& thread) -> async::task<void> {
                        auto shared = weak.lock();
                        if (shared == nullptr)
                            co_return;

                        [[maybe_unused]] volatile auto holder = protocol;
                        shared->process_hooks(cmd, *protocol.get());
                        co_await handler(*shared, *protocol.get());
                    });
                }

                reader.seek(size - sizeof(uint8_t));
                reader.flush(); // remove packet body
            }
            catch (std::exception& e)
            {
                fb::logger::fatal("bot_controller::on_receive: {}", e.what());
                reader.clear();
                break;
            }
            catch (...)
            {
                reader.clear();
                break;
            }
        }
    }

    async::task<void> on_closed(fb::socket<>& socket)
    {
        auto& bot    = static_cast<BotType&>(static_cast<base_bot&>(socket));
        auto  thread = bot.thread();
        co_await thread->switching();

        // Notify bot_controller of bot disconnection
        co_await this->on_bot_disconnected(bot);

        // Remove bot from our managed collection
        this->_bots.write([&](auto& bots) {
            bots.erase(bot.id);
        });

        // Remove bot from thread-local bot collection
        auto params = thread->template data<bot_thread_params>();
        params->bots.erase(bot.id);
    }

    /**
     * @brief      Binds a response handler for a specific protocol type.
     *
     *             Registers a handler function that will be called whenever
     *             a protocol message of the specified type is received.
     *             Also sets up deserialization for the protocol type.
     *
     * @param[in]  fn   The handler function to bind.
     *
     * @tparam     ResponseType  The protocol response type to handle.
     */
    template <typename ResponseType>
    void bind(const std::function<async::task<void>(BotType&, ResponseType&)>& fn)
    {
        static_assert(std::is_base_of_v<fb::protocol::header, ResponseType>,
                      "ResponseType must inherit from fb::protocol::header");
        static_assert(std::is_same_v<decltype(ResponseType::header), const uint8_t>,
                      "ResponseType must have 'static constexpr uint8_t header' member");

        auto unique_lock = std::unique_lock<std::shared_mutex>(this->_handler_mutex);

        this->_deserializer.insert(
            {ResponseType::header, [](auto& reader) -> async::task<std::shared_ptr<fb::protocol::header>> {
                 auto protocol = std::make_shared<ResponseType>();
                 co_await protocol->deserialize(reader);
                 co_return protocol;
             }});

        this->_handler.insert({ResponseType::header, [this, fn](auto& bot, auto& header) -> async::task<void> {
                                   bot.process_hooks(ResponseType::header, header);

                                   auto protocol = static_cast<ResponseType&>(header);
                                   co_await fn(bot, protocol);
                               }});
    }

    /**
     * @brief      Binds a member function as a response handler.
     *
     *             Convenience method for binding class member functions
     *             as protocol response handlers.
     *
     * @param[in]  fn   The member function to bind.
     *
     * @tparam     Class         The class type containing the member function.
     * @tparam     ResponseType  The protocol response type to handle.
     */
    template <typename Class, typename ResponseType>
    void bind(async::task<void> (Class::*fn)(BotType&, const ResponseType&))
    {
        static_assert(std::is_base_of_v<fb::protocol::header, ResponseType>,
                      "ResponseType must inherit from fb::protocol::header");
        static_assert(std::is_same_v<decltype(ResponseType::header), const uint8_t>,
                      "ResponseType must have 'static constexpr uint8_t header' member");

        this->bind<ResponseType>(
            std::bind(fn, static_cast<Class*>(this), std::placeholders::_1, std::placeholders::_2));
    }

    virtual async::task<void> on_bot_connected(BotType& bot)
    {
        co_return;
    }

    virtual async::task<void> on_bot_disconnected(BotType& bot)
    {
        co_return;
    }

    /**
     * @brief      Gets the number of active bots managed by this bot_controller.
     *
     * @return     The count of active bot instances.
     */
    size_t bot_count() const
    {
        return this->_bots.read([](const auto& bots) {
            return bots.size();
        });
    }

    /**
     * @brief      Performs a read operation on the bot collection.
     *
     *             Provides thread-safe read access to the bot collection.
     *             Multiple threads can read concurrently.
     *
     * @param[in]  fn    Function to execute with read access to the bot collection.
     *
     * @tparam     Func  The function type (lambda or function object).
     *
     * @return     The value returned by the function.
     */
    template <typename Func>
    auto read_bots(Func&& fn) const
        -> decltype(fn(std::declval<const std::unordered_map<uint32_t, std::shared_ptr<BotType>>&>()))
    {
        return this->_bots.read(std::forward<Func>(fn));
    }

    /**
     * @brief      Performs a write operation on the bot collection.
     *
     *             Provides thread-safe write access to the bot collection.
     *             Only one thread can write at a time.
     *
     * @param[in]  fn    Function to execute with write access to the bot collection.
     *
     * @tparam     Func  The function type (lambda or function object).
     *
     * @return     The value returned by the function.
     */
    template <typename Func>
    auto write_bots(Func&& fn) -> decltype(fn(std::declval<std::unordered_map<uint32_t, std::shared_ptr<BotType>>&>()))
    {
        return this->_bots.write(std::forward<Func>(fn));
    }

    /**
     * @brief      Performs a write operation on the bot collection (void return).
     *
     *             Provides thread-safe write access to the bot collection.
     *             Only one thread can write at a time.
     *
     * @param[in]  fn    Function to execute with write access to the bot collection.
     */
    void write_bots(const std::function<void(std::unordered_map<uint32_t, std::shared_ptr<BotType>>&)>& fn)
    {
        this->_bots.write(fn);
    }
};

// ============================================================================
// Template method implementations for bot<ControllerType>
// ============================================================================
// Note: These implementations are placed here after all class definitions
//       to resolve circular dependencies between bot.h and controller.h

template <typename BotType>
bot<BotType>::bot(bot_controller<BotType>& bot_controller, uint32_t id) :
    base_bot(bot_controller.container,
             bot_controller,
             std::bind(&base_bot_controller::on_receive, &bot_controller, std::placeholders::_1, std::placeholders::_2),
             std::bind(&base_bot_controller::on_closed, &bot_controller, std::placeholders::_1),
             id),
    _controller(bot_controller)
{ }

template <typename BotType>
template <typename ResponseType>
async::task<ResponseType> bot<BotType>::request(const fb::protocol::header&                          protocol,
                                                const std::function<bool(const ResponseType& resp)>& condition,
                                                const fb::model::timespan&                           timeout,
                                                bool                                                 encrypt,
                                                bool                                                 wrap)
{
    this->assert_thread();

    // Ensure deserializer is registered for hook processing
    this->_controller.template ensure_handler_registered<ResponseType>();

    // Create request context for RAII management
    auto self_ptr = std::static_pointer_cast<BotType>(this->shared_from_this());
    auto context =
        std::make_shared<typename BotType::template request_context<ResponseType>>(self_ptr, ResponseType::header);

    // Set up timeout timer if specified
    if (timeout > 0s)
    {
        auto thread    = this->thread();
        context->timer = thread->settimer(
            [context](auto& datetime, auto thread_id) -> async::task<void> {
                context->complete_timeout();
                co_return;
            },
            timeout,
            fb::timer::repeat_type::once);
    }

    // Ensure hook container exists
    if (this->_hooks.contains(ResponseType::header) == false)
        this->_hooks.insert({ResponseType::header, {}});

    // Register hook with context pointer for cleanup
    this->_hooks[ResponseType::header].push_back(hook_params{.condition =
                                                                 [context, condition](const auto& header) {
                                                                     auto& protocol =
                                                                         static_cast<const ResponseType&>(header);
                                                                     return condition(protocol);
                                                                 },
                                                             .matched =
                                                                 [context](const auto& header) {
                                                                     auto& protocol =
                                                                         static_cast<const ResponseType&>(header);
                                                                     context->complete_success(protocol);
                                                                 },
                                                             .context_ptr = context.get()});

    this->send(protocol, encrypt, wrap);
    return context->task();
}

template <typename BotType>
template <typename ResponseType>
async::task<ResponseType>
bot<BotType>::request(const fb::protocol::header& protocol, const fb::model::timespan& timeout, bool encrypt, bool wrap)
{
    co_return co_await this->request<ResponseType>(
        protocol,
        [](auto& resp) -> bool {
            return true;
        },
        timeout,
        encrypt,
        wrap);
}

} // namespace fb::bot

#endif // __BOT_CONTROLLER_H__