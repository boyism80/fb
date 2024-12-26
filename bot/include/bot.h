#ifndef __BOT_H__
#define __BOT_H__

#include <random>
#include <any>
#include <fb/socket.h>
#include <async/awaitable_get.h>
#include <boost/endian/conversion.hpp>
#include <fb/model/model.h>
#include <hook_params.h>

using namespace std::chrono_literals;
using namespace fb::model;

namespace fb::bot {

class bot_container;

class base_bot : public fb::socket<void*>
{
private:
    using handle_func    = std::function<async::task<void>(fb::protocol::base::header&)>;
    using deserilze_func = std::function<async::task<fb::protocol::base::header*>(fb::stream_reader<big_endian>&)>;
    using hook_func      = std::function<bool(const fb::protocol::base::header&)>;
    using hook_container = std::unordered_map<uint8_t, std::vector<hook_params>>;

protected:
    bot_container&                              _owner;
    fb::cryptor                                 _cryptor;
    std::unordered_map<uint8_t, handle_func>    _handler;
    std::unordered_map<uint8_t, deserilze_func> _deserializer;
    hook_container                              _hooks;

public:
    const uint32_t id;

protected:
    base_bot(bot_container& owner, uint32_t id);

public:
    virtual ~base_bot();

public:
    async::task<void> on_receive(fb::stream& stream);
    async::task<void> on_closed();

protected:
    virtual async::task<void> on_connected();
    virtual async::task<void> on_disconnected();
    virtual bool              on_encrypt(fb::stream& out);
    virtual bool              on_wrap(fb::stream& out);
    virtual bool              decrypt_policy(int cmd) const;

public:
    void                      connect(const boost::asio::ip::tcp::endpoint& endpoint);
    virtual async::task<void> on_timer(const fb::model::datetime& now)
    {
        co_return;
    }

    template <typename ResponseType>
    void bind(const std::function<async::task<void>(ResponseType&)>& fn)
    {
        this->_deserializer.insert({ResponseType::header, [](auto& reader) -> async::task<fb::protocol::base::header*> {
                                        auto protocol = new ResponseType();
                                        co_await protocol->deserialize(reader);
                                        co_return protocol;
                                    }});

        this->_handler.insert(
            {ResponseType::header, [this, fn](auto& header) -> async::task<void> {
                 if (this->_hooks.contains(ResponseType::header))
                 {
                     auto& matched_hooks = this->_hooks.at(ResponseType::header);
                     auto  i = std::find_if(matched_hooks.begin(), matched_hooks.end(), [&header](const auto& hook) {
                         return hook.condition(header);
                     });

                     if (i != matched_hooks.end())
                     {
                         auto callback = i->matched;
                         matched_hooks.erase(i);

                         callback(header);
                     }
                 }

                 auto protocol = static_cast<ResponseType&>(header);
                 co_await fn(protocol);
             }});
    }

    template <typename Class, typename ResponseType>
    void bind(async::task<void> (Class::*fn)(const ResponseType&))
    {
        this->bind<ResponseType>(std::bind(fn, static_cast<Class*>(this), std::placeholders::_1));
    }

    template <typename ResponseType>
    async::task<ResponseType> request(const fb::protocol::base::header&                    protocol,
                                      const std::function<bool(const ResponseType& resp)>& condition,
                                      bool                                                 encrypt = true,
                                      bool                                                 wrap    = true)
    {
        this->assert_thread();

        if (!this->_handler.contains(ResponseType::header))
        {
            this->bind<ResponseType>([](const ResponseType& resp) -> async::task<void> {
                co_return;
            });
        }

        auto promise = std::make_shared<async::task_completion_source<ResponseType>>();
        if (this->_hooks.contains(ResponseType::header) == false)
            this->_hooks.insert({ResponseType::header, {}});

        this->_hooks[ResponseType::header].push_back(hook_params{.condition =
                                                                     [promise, condition](const auto& header) {
                                                                         auto& protocol =
                                                                             static_cast<const ResponseType&>(header);
                                                                         return condition(protocol);
                                                                     },
                                                                 .matched =
                                                                     [promise](const auto& header) {
                                                                         auto& protocol =
                                                                             static_cast<const ResponseType&>(header);
                                                                         promise->set_value(protocol);
                                                                     }});
        this->send(protocol, encrypt, wrap);
        return promise->task();
    }

    template <typename ResponseType>
    async::task<ResponseType> request(const fb::protocol::base::header& protocol, bool encrypt = true, bool wrap = true)
    {
        co_return co_await this->request<ResponseType>(
            protocol,
            [](auto& resp) -> bool {
                return true;
            },
            encrypt,
            wrap);
    }

public:
    fb::thread* thread() const override final;
};

} // namespace fb::bot

#endif