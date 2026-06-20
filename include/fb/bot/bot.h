#ifndef __BOT_H__
#define __BOT_H__

#include <random>
#include <any>
#include <memory>
#include <fb/socket.h>
#include <async/awaitable_get.h>
#include <boost/endian/conversion.hpp>
#include <fb/model/model.h>
#include <fb/bot/hook_params.h>

namespace fb::bot {

using namespace std::chrono_literals;
using namespace fb::model;

// Forward declarations
class bot_container;
class base_bot_controller;
template <typename BotType> class bot_controller;

class base_bot : public fb::socket<void*>
{
private:
    using hook_func      = std::function<bool(const fb::protocol::header&)>;
    using hook_container = std::unordered_map<uint8_t, std::vector<hook_params>>;

protected:
    hook_container       _hooks;
    fb::async_executor&  _executor;
    base_bot_controller& _bot_controller;

public:
    const uint32_t id;

protected:
    base_bot(fb::async_executor&                                                 executor,
             base_bot_controller&                                                bot_controller,
             std::function<async::task<void>(fb::socket<void*>&, fb::stream&)>&& on_receive,
             std::function<async::task<void>(fb::socket<void*>&)>&&              on_closed,
             uint32_t                                                            id);

public:
    virtual ~base_bot();

protected:
    virtual bool on_encrypt(fb::stream& out);
    virtual bool on_wrap(fb::stream& out);

public:
    void connect(const boost::asio::ip::tcp::endpoint& endpoint);
    bool process_hooks(uint8_t opcode, fb::protocol::header& header);
    bool remove_hook_by_context(uint8_t opcode, const void* context_ptr);

public:
    virtual fb::thread* thread() const;
};

template <typename BotType>
class bot : public base_bot
{
public:
    template <typename ResponseType> struct request_context
    {
        using promise_type = async::task_completion_source<ResponseType>;

        std::shared_ptr<promise_type> promise;
        std::shared_ptr<fb::timer>    timer;
        std::weak_ptr<BotType>        bot_weak;
        uint8_t                       hook_cmd;
        std::atomic<bool>             completed{false};
        const void*                   context_ptr;

        request_context(std::shared_ptr<BotType> bot, uint8_t opcode) :
            promise(std::make_shared<promise_type>()),
            bot_weak(bot),
            hook_cmd(opcode),
            context_ptr(this)
        { }

        void complete_success(const ResponseType& response)
        {
            if (completed.exchange(true))
                return;

            if (timer)
                timer->cancel();
            promise->set_value(response);
        }

        void complete_timeout()
        {
            if (completed.exchange(true))
                return;

            if (auto bot = bot_weak.lock())
            {
                bot->remove_hook_by_context(hook_cmd, context_ptr);
            }
            promise->set_exception(std::make_exception_ptr(std::runtime_error("request timeout")));
        }

        async::task<ResponseType> task()
        {
            return promise->task();
        }
    };

    struct request_erased_context
    {
        using promise_type = async::task_completion_source<std::shared_ptr<fb::protocol::header>>;

        std::shared_ptr<promise_type> promise;
        std::shared_ptr<fb::timer>    timer;
        std::weak_ptr<BotType>        bot_weak;
        uint8_t                       hook_cmd;
        std::atomic<bool>             completed{false};
        const void*                   context_ptr;

        request_erased_context(std::shared_ptr<BotType> bot, uint8_t opcode) :
            promise(std::make_shared<promise_type>()),
            bot_weak(bot),
            hook_cmd(opcode),
            context_ptr(this)
        { }

        void complete_success(std::shared_ptr<fb::protocol::header> response)
        {
            if (completed.exchange(true))
                return;

            if (timer)
                timer->cancel();
            promise->set_value(std::move(response));
        }

        void complete_timeout()
        {
            if (completed.exchange(true))
                return;

            if (auto bot = bot_weak.lock())
            {
                bot->remove_hook_by_context(hook_cmd, context_ptr);
            }
            promise->set_exception(std::make_exception_ptr(std::runtime_error("request timeout")));
        }

        async::task<std::shared_ptr<fb::protocol::header>> task()
        {
            return promise->task();
        }
    };

public:
    bot_controller<BotType>& controller;

protected:
    bot(bot_controller<BotType>& controller, uint32_t id);

public:
    async::task<std::shared_ptr<fb::protocol::header>> request_by_opcode(
        std::shared_ptr<BotType>                                                          target,
        uint8_t                                                                           response_opcode,
        const fb::protocol::header&                                                       protocol,
        const std::function<bool(const fb::protocol::header& resp)>&                      condition,
        const fb::model::timespan&                                                        timeout = 0s,
        bool                                                                              encrypt = true,
        bool                                                                              wrap    = true,
        std::function<std::shared_ptr<fb::protocol::header>(const fb::protocol::header&)> clone   = nullptr);

    async::task<std::shared_ptr<fb::protocol::header>> request_by_opcode(
        uint8_t                                                                           response_opcode,
        const fb::protocol::header&                                                       protocol,
        const std::function<bool(const fb::protocol::header& resp)>&                      condition,
        const fb::model::timespan&                                                        timeout = 0s,
        bool                                                                              encrypt = true,
        bool                                                                              wrap    = true,
        std::function<std::shared_ptr<fb::protocol::header>(const fb::protocol::header&)> clone   = nullptr);

    template <typename ResponseType> async::task<ResponseType>
    request(std::shared_ptr<BotType>                             target,
            const fb::protocol::header&                          protocol,
            const std::function<bool(const ResponseType& resp)>& condition,
            const fb::model::timespan&                           timeout = 0s,
            bool                                                 encrypt = true,
            bool                                                 wrap    = true);

    template <typename ResponseType>
    async::task<ResponseType> request(const fb::protocol::header&                          protocol,
                                      const std::function<bool(const ResponseType& resp)>& condition,
                                      const fb::model::timespan&                           timeout = 0s,
                                      bool                                                 encrypt = true,
                                      bool                                                 wrap    = true);

    template <typename ResponseType> async::task<ResponseType> request(std::shared_ptr<BotType>    target,
                                                                       const fb::protocol::header& protocol,
                                                                       const fb::model::timespan&  timeout = 0s,
                                                                       bool                        encrypt = true,
                                                                       bool                        wrap    = true);

    template <typename ResponseType>
    async::task<ResponseType> request(const fb::protocol::header& protocol,
                                      const fb::model::timespan&  timeout = 0s,
                                      bool                        encrypt = true,
                                      bool                        wrap    = true);
};

} // namespace fb::bot

#endif