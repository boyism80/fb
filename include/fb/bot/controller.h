#ifndef __BOT_CONTROLLER_H__
#define __BOT_CONTROLLER_H__

#include <fb/bot/bot.h>
#include <fb/bot/container.h>
#include <fb/synchronized.h>
#include <fb/protocol/header.h>
#include <fb/logger.h>
#include <format>

namespace fb::bot {

class base_bot_controller : public std::enable_shared_from_this<base_bot_controller>
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
    virtual async::task<void> on_bot_connected(base_bot& bot)                      = 0;
    virtual async::task<void> on_bot_disconnected(base_bot& bot)                   = 0;
    virtual void              ensure_handler_registered(uint8_t opcode)            = 0;
};

template <typename BotType>
class bot_controller : public base_bot_controller
{
public:
    using handle_func        = std::function<async::task<void>(BotType&, fb::protocol::header&)>;
    using deserilze_func     = std::function<std::shared_ptr<fb::protocol::header>(fb::stream_reader<big_endian>&)>;
    using response_cloner_fn = std::function<std::shared_ptr<fb::protocol::header>(const fb::protocol::header&)>;

private:
    std::unordered_map<uint8_t, handle_func>    _handler;
    std::unordered_map<uint8_t, deserilze_func> _deserializer;
    std::unordered_map<uint8_t, bool>           _decrypt;
    mutable std::shared_mutex                   _handler_mutex;

    inline static std::unordered_map<uint8_t, response_cloner_fn>     _response_cloners;
    inline static std::unordered_map<std::string, response_cloner_fn> _typed_response_cloners;
    inline static std::shared_mutex                                   _cloner_mutex;

    template <typename ResponseType> static void register_opcode_response_cloner()
    {
        auto unique_lock = std::unique_lock<std::shared_mutex>(_cloner_mutex);
        if (_response_cloners.contains(ResponseType::opcode))
            return;

        _response_cloners[ResponseType::opcode] =
            [](const fb::protocol::header& header) -> std::shared_ptr<fb::protocol::header> {
            return std::make_shared<ResponseType>(static_cast<const ResponseType&>(header));
        };
    }

public:
    static void register_response_cloner(std::string_view type_key, response_cloner_fn fn)
    {
        auto unique_lock                               = std::unique_lock<std::shared_mutex>(_cloner_mutex);
        _typed_response_cloners[std::string(type_key)] = std::move(fn);
    }

    static std::shared_ptr<fb::protocol::header> clone_response(uint8_t opcode, const fb::protocol::header& header)
    {
        auto shared_lock = std::shared_lock<std::shared_mutex>(_cloner_mutex);
        auto it          = _response_cloners.find(opcode);
        if (it == _response_cloners.end())
            throw std::runtime_error(std::format("response opcode {:#04x} is not registered", opcode));

        return it->second(header);
    }

    static std::shared_ptr<fb::protocol::header> clone_response_by_type(std::string_view            type_key,
                                                                        const fb::protocol::header& header)
    {
        auto shared_lock = std::shared_lock<std::shared_mutex>(_cloner_mutex);
        auto it          = _typed_response_cloners.find(std::string(type_key));
        if (it == _typed_response_cloners.end())
            throw std::runtime_error(std::format("response type {} is not registered", type_key));

        return it->second(header);
    }

protected:
    fb::synchronized<std::unordered_map<uint32_t, std::shared_ptr<BotType>>> _bots;

protected:
    bot_controller(bot_container& container) :
        base_bot_controller(container)
    { }

protected:
    // Unknown opcodes default to decrypting (same as server registry).
    bool should_decrypt(uint8_t opcode) const
    {
        auto shared_lock = std::shared_lock<std::shared_mutex>(this->_handler_mutex);
        auto it          = this->_decrypt.find(opcode);
        if (it == this->_decrypt.end())
            return true;
        return it->second;
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

    virtual async::task<void> on_bot_connected(base_bot& bot) override
    {
        static_assert(std::is_base_of_v<base_bot, BotType>, "BotType must inherit from base_bot");
        auto& typed_bot = static_cast<BotType&>(bot);
        co_await this->on_bot_connected(typed_bot);
    }

    virtual async::task<void> on_bot_disconnected(base_bot& bot) override
    {
        static_assert(std::is_base_of_v<base_bot, BotType>, "BotType must inherit from base_bot");
        auto& typed_bot = static_cast<BotType&>(bot);
        co_await this->on_bot_disconnected(typed_bot);
    }

    virtual void ensure_handler_registered(uint8_t opcode) override
    {
        {
            auto shared_lock = std::shared_lock<std::shared_mutex>(this->_handler_mutex);
            if (this->_deserializer.contains(opcode))
                return;
        }

        auto unique_lock = std::unique_lock<std::shared_mutex>(this->_handler_mutex);

        if (this->_deserializer.contains(opcode))
            return;

        this->_deserializer[opcode] =
            [](fb::stream_reader<big_endian>& reader) -> std::shared_ptr<fb::protocol::header> {
            return nullptr;
        };

        this->_handler[opcode] = [](BotType& bot, const fb::protocol::header& protocol) -> async::task<void> {
            co_return;
        };
    }

public:
    template <typename T>
    std::weak_ptr<T> weak_from_this_as()
    {
        return std::static_pointer_cast<T>(this->shared_from_this());
    }

    template <typename ResponseType> void ensure_handler_registered()
    {
        {
            auto shared_lock = std::shared_lock<std::shared_mutex>(this->_handler_mutex);
            if (this->_handler.contains(ResponseType::opcode))
                return;
        }

        this->bind<ResponseType>([](BotType& bot, ResponseType& protocol) -> async::task<void> {
            co_return;
        });
    }

    std::shared_ptr<BotType> create()
    {
        auto& derived_bot_controller = static_cast<typename BotType::bot_controller_type&>(*this);
        auto  bot                    = this->container.create<BotType>(derived_bot_controller);

        {
            auto guard             = this->_bots.enter_write();
            guard.value()[bot->id] = bot;
        }

        return bot;
    }

    std::shared_ptr<BotType> create(const fb::stream& params)
    {
        auto& derived_bot_controller = static_cast<typename BotType::bot_controller_type&>(*this);
        auto  bot                    = this->container.create<BotType>(derived_bot_controller, params);

        {
            auto guard             = this->_bots.enter_write();
            guard.value()[bot->id] = bot;
        }

        return bot;
    }

    bool contains(uint32_t id) const
    {
        auto guard = this->_bots.enter_read();
        return guard.value().contains(id);
    }

    async::task<void> on_receive(fb::socket<>& socket, fb::stream& stream)
    {
        static_assert(std::is_base_of_v<base_bot, BotType>, "BotType must inherit from base_bot");
        static constexpr uint8_t base_size = sizeof(uint8_t) + sizeof(uint16_t);

        auto& bot    = static_cast<BotType&>(static_cast<base_bot&>(socket));
        auto  reader = fb::stream_reader<big_endian>(stream);
        while (true)
        {
            auto processed_cmd = std::optional<uint8_t>{};
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

                auto opcode   = reader.read<uint8_t>();
                processed_cmd = opcode;
                if (this->should_decrypt(opcode))
                {
                    auto& encryption = bot.encryption();
                    size             = encryption.decrypt(stream, reader.seek() - 1, size);
                }

                reader.flush();

                std::shared_ptr<fb::protocol::header> protocol;
                handle_func                           handler;
                deserilze_func                        deserializer;

                {
                    auto shared_lock = std::shared_lock<std::shared_mutex>(this->_handler_mutex);
                    if (this->_deserializer.contains(opcode))
                    {
                        deserializer = this->_deserializer.at(opcode);

                        if (this->_handler.contains(opcode))
                            handler = this->_handler.at(opcode);
                    }
                    else
                    {
                        deserializer =
                            [](fb::stream_reader<big_endian>& reader) -> std::shared_ptr<fb::protocol::header> {
                            return nullptr;
                        };
                    }
                }

                protocol = deserializer(reader);
                if (protocol != nullptr)
                {
                    auto weak = bot.template weak_from_this_as<BotType>();

                    auto builder = this->container.threads.new_builder(weak);
                    builder.func = [=, this](auto& thread) -> async::task<void> {
                        auto shared = weak.lock();
                        if (shared == nullptr)
                            co_return;

                        co_await handler(*shared, *protocol.get());
                    };
                    builder.enqueue();
                }

                reader.seek(size - sizeof(uint8_t));
                reader.flush();
            }
            catch (std::exception& e)
            {
                if (processed_cmd.has_value())
                    fb::logger::fatal("bot_controller::on_receive: opcode={:#x} error={}\n{}",
                                      processed_cmd.value(),
                                      e.what(),
                                      boost::stacktrace::to_string(boost::stacktrace::stacktrace()));
                else
                    fb::logger::fatal("bot_controller::on_receive: error={}\n{}",
                                      e.what(),
                                      boost::stacktrace::to_string(boost::stacktrace::stacktrace()));
                reader.clear();
                break;
            }
            catch (...)
            {
                if (processed_cmd.has_value())
                    fb::logger::fatal("bot_controller::on_receive: opcode={:#x} error=unknown\n{}",
                                      processed_cmd.value(),
                                      boost::stacktrace::to_string(boost::stacktrace::stacktrace()));
                else
                    fb::logger::fatal("bot_controller::on_receive: error=unknown\n{}",
                                      boost::stacktrace::to_string(boost::stacktrace::stacktrace()));

                reader.clear();
                break;
            }
        }
    }

    async::task<void> on_closed(fb::socket<>& socket)
    {
        auto& raw_bot = static_cast<BotType&>(static_cast<base_bot&>(socket));
        auto  bot_id  = raw_bot.id;
        auto  thread  = raw_bot.thread();

        // Keep the bot alive while close handling (and the awaiting recv) finish.
        // Erasing the last map entry without this destroys the object mid-co_await.
        std::shared_ptr<BotType> bot_ptr;
        {
            auto guard = this->_bots.enter_read();
            auto it    = guard.value().find(bot_id);
            if (it != guard.value().end())
                bot_ptr = it->second;
        }

        co_await thread->switching();

        if (bot_ptr != nullptr)
            co_await this->on_bot_disconnected(*bot_ptr);

        {
            auto guard = this->_bots.enter_write();
            guard.value().erase(bot_id);
        }

        auto params = thread->template data<bot_thread_params>();
        if (params != nullptr)
            params->bots.erase(bot_id);
    }

    template <typename ResponseType> void bind(std::function<async::task<void>(BotType&, ResponseType&)>&& fn)
    {
        static_assert(std::is_base_of_v<fb::protocol::header, ResponseType>,
                      "ResponseType must inherit from fb::protocol::header");
        static_assert(std::is_same_v<decltype(ResponseType::opcode), const uint8_t>,
                      "ResponseType must have 'static constexpr uint8_t header' member");

        register_opcode_response_cloner<ResponseType>();

        auto unique_lock = std::unique_lock<std::shared_mutex>(this->_handler_mutex);

        this->_deserializer.insert({ResponseType::opcode, [](auto& reader) -> std::shared_ptr<fb::protocol::header> {
                                        auto protocol = std::make_shared<ResponseType>();
                                        protocol->deserialize(reader);
                                        return protocol;
                                    }});

        this->_decrypt[ResponseType::opcode] = fb::protocol::protocol_decrypt<ResponseType>();

        this->_handler.insert(
            {ResponseType::opcode, [this, fn = std::move(fn)](auto& bot, auto& header) -> async::task<void> {
                 auto          protocol   = static_cast<ResponseType&>(header);
                 volatile auto controller = this;

                 co_await fn(bot, protocol);

                 co_await controller->on_integration_hook_execution(ResponseType::opcode, bot, header);

                 bot.process_hooks(ResponseType::opcode, header);
             }});
    }

    template <typename Class, typename ResponseType> void bind(async::task<void> (Class::*fn)(BotType&,
                                                                                              const ResponseType&))
    {
        static_assert(std::is_base_of_v<fb::protocol::header, ResponseType>,
                      "ResponseType must inherit from fb::protocol::header");
        static_assert(std::is_same_v<decltype(ResponseType::opcode), const uint8_t>,
                      "ResponseType must have 'static constexpr uint8_t header' member");

        this->bind<ResponseType>(std::function<async::task<void>(BotType&, ResponseType&)>(
            std::bind(fn, static_cast<Class*>(this), std::placeholders::_1, std::placeholders::_2)));
    }

    virtual async::task<void> on_bot_connected(BotType& bot)
    {
        co_return;
    }

    virtual async::task<void> on_bot_disconnected(BotType& bot)
    {
        co_return;
    }

    size_t bot_count() const
    {
        auto guard = this->_bots.enter_read();
        return guard.value().size();
    }

    template <typename Func> auto read_bots(Func&& fn) const
        -> decltype(fn(std::declval<const std::unordered_map<uint32_t, std::shared_ptr<BotType>>&>()))
    {
        auto guard = this->_bots.enter_read();
        return fn(guard.value());
    }

    template <typename Func> auto write_bots(Func&& fn)
        -> decltype(fn(std::declval<std::unordered_map<uint32_t, std::shared_ptr<BotType>>&>()))
    {
        auto guard = this->_bots.enter_write();
        return fn(guard.value());
    }

    void write_bots(const std::function<void(std::unordered_map<uint32_t, std::shared_ptr<BotType>>&)>& fn)
    {
        auto guard = this->_bots.enter_write();
        fn(guard.value());
    }

    virtual async::task<void> on_integration_hook_execution(uint8_t                     opcode,
                                                            BotType&                    bot,
                                                            const fb::protocol::header& header)
    {
        co_return;
    }
};

template <typename BotType>
bot<BotType>::bot(bot_controller<BotType>& bot_controller, uint32_t id) :
    base_bot(bot_controller.container,
             bot_controller,
             std::bind(&base_bot_controller::on_receive, &bot_controller, std::placeholders::_1, std::placeholders::_2),
             std::bind(&base_bot_controller::on_closed, &bot_controller, std::placeholders::_1),
             id),
    controller(bot_controller)
{ }

template <typename BotType>
async::task<std::shared_ptr<fb::protocol::header>>
bot<BotType>::request_by_opcode(std::shared_ptr<BotType>                                     target,
                                uint8_t                                                      response_opcode,
                                const fb::protocol::header*                                  protocol,
                                const std::function<bool(const fb::protocol::header& resp)>& condition,
                                const fb::model::timespan&                                   timeout,
                                bool                                                         encrypt,
                                bool                                                         wrap,
                                std::function<std::shared_ptr<fb::protocol::header>(const fb::protocol::header&)> clone)
{
    static_cast<base_bot_controller&>(target->controller).ensure_handler_registered(response_opcode);

    auto self_ptr = std::static_pointer_cast<BotType>(target->shared_from_this());
    auto context  = std::make_shared<request_erased_context>(self_ptr, response_opcode);

    fb::logger::debug("bot request start: bot_id={} response_opcode=0x{:02X} timeout_ms={} wait_only={}",
                      target->id,
                      response_opcode,
                      timeout.total_milliseconds(),
                      protocol == nullptr);

    if (timeout > 0s)
    {
        auto thread  = target->thread();
        auto builder = thread->template new_builder<void>();
        builder.func = [context, timeout](auto& thread) -> async::task<void> {
            context->timer = thread.settimer(
                [context](auto& datetime, auto thread_id) -> async::task<void> {
                    context->complete_timeout();
                    co_return;
                },
                timeout,
                fb::timer::repeat_type::once);
            co_return;
        };
        builder.enqueue();
    }

    if (target->_hooks.contains(response_opcode) == false)
        target->_hooks.insert({response_opcode, {}});

    target->_hooks[response_opcode].push_back(
        hook_params{.condition =
                        [condition](const auto& opcode) {
                            return condition(opcode);
                        },
                    .matched =
                        [context, response_opcode, clone = std::move(clone)](const auto& opcode) {
                            std::shared_ptr<fb::protocol::header> response;
                            if (clone)
                            {
                                response = clone(opcode);
                            }
                            else
                            {
                                response = bot_controller<BotType>::clone_response(response_opcode, opcode);
                            }
                            context->complete_success(std::move(response));
                        },
                    .context_ptr = context.get()});

    if (protocol != nullptr)
    {
        async::awaitable_then(this->send(*protocol, encrypt, wrap), [](auto result) {
            try
            {
                result();
            }
            catch (std::exception& e)
            {
                fb::logger::fatal(e.what());
            }
        });
    }
    co_return co_await context->task();
}

template <typename BotType>
async::task<std::shared_ptr<fb::protocol::header>>
bot<BotType>::request_by_opcode(std::shared_ptr<BotType>                                     target,
                                uint8_t                                                      response_opcode,
                                const fb::protocol::header&                                  protocol,
                                const std::function<bool(const fb::protocol::header& resp)>& condition,
                                const fb::model::timespan&                                   timeout,
                                bool                                                         encrypt,
                                bool                                                         wrap,
                                std::function<std::shared_ptr<fb::protocol::header>(const fb::protocol::header&)> clone)
{
    co_return co_await this
        ->request_by_opcode(target, response_opcode, &protocol, condition, timeout, encrypt, wrap, std::move(clone));
}

template <typename BotType>
async::task<std::shared_ptr<fb::protocol::header>>
bot<BotType>::request_by_opcode(uint8_t                                                      response_opcode,
                                const fb::protocol::header*                                  protocol,
                                const std::function<bool(const fb::protocol::header& resp)>& condition,
                                const fb::model::timespan&                                   timeout,
                                bool                                                         encrypt,
                                bool                                                         wrap,
                                std::function<std::shared_ptr<fb::protocol::header>(const fb::protocol::header&)> clone)
{
    co_return co_await this->request_by_opcode(this->shared_from_this_as<BotType>(),
                                               response_opcode,
                                               protocol,
                                               condition,
                                               timeout,
                                               encrypt,
                                               wrap,
                                               std::move(clone));
}

template <typename BotType>
async::task<std::shared_ptr<fb::protocol::header>>
bot<BotType>::request_by_opcode(uint8_t                                                      response_opcode,
                                const fb::protocol::header&                                  protocol,
                                const std::function<bool(const fb::protocol::header& resp)>& condition,
                                const fb::model::timespan&                                   timeout,
                                bool                                                         encrypt,
                                bool                                                         wrap,
                                std::function<std::shared_ptr<fb::protocol::header>(const fb::protocol::header&)> clone)
{
    co_return co_await this->request_by_opcode(this->shared_from_this_as<BotType>(),
                                               response_opcode,
                                               &protocol,
                                               condition,
                                               timeout,
                                               encrypt,
                                               wrap,
                                               std::move(clone));
}

template <typename BotType>
template <typename ResponseType>
async::task<ResponseType> bot<BotType>::request(std::shared_ptr<BotType>                             target,
                                                const fb::protocol::header&                          protocol,
                                                const std::function<bool(const ResponseType& resp)>& condition,
                                                const fb::model::timespan&                           timeout,
                                                bool                                                 encrypt,
                                                bool                                                 wrap)
{
    target->controller.template ensure_handler_registered<ResponseType>();

    auto response = co_await this->request_by_opcode(
        target,
        ResponseType::opcode,
        protocol,
        [&condition](const fb::protocol::header& resp) {
            return condition(static_cast<const ResponseType&>(resp));
        },
        timeout,
        encrypt,
        wrap,
        [](const fb::protocol::header& header) -> std::shared_ptr<fb::protocol::header> {
            return std::make_shared<ResponseType>(static_cast<const ResponseType&>(header));
        });

    co_return *static_cast<ResponseType*>(response.get());
}

template <typename BotType>
template <typename ResponseType>
async::task<ResponseType> bot<BotType>::request(const fb::protocol::header&                          protocol,
                                                const std::function<bool(const ResponseType& resp)>& condition,
                                                const fb::model::timespan&                           timeout,
                                                bool                                                 encrypt,
                                                bool                                                 wrap)
{
    co_return co_await this
        ->request<ResponseType>(this->shared_from_this_as<BotType>(), protocol, condition, timeout, encrypt, wrap);
}

template <typename BotType>
template <typename ResponseType>
async::task<ResponseType> bot<BotType>::request(std::shared_ptr<BotType>    target,
                                                const fb::protocol::header& protocol,
                                                const fb::model::timespan&  timeout,
                                                bool                        encrypt,
                                                bool                        wrap)
{
    co_return co_await this->request<ResponseType>(
        target,
        protocol,
        [](auto& resp) -> bool {
            return true;
        },
        timeout,
        encrypt,
        wrap);
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