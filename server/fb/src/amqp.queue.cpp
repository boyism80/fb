#include <fb/amqp/queue.h>

using namespace fb::amqp;

queue::queue(socket& owner, const amqp_bytes_t& name, fb::thread_container& threads) :
    _owner(owner),
    _raw_name(name),
    _threads(threads)
{ }

queue::~queue()
{
    if (this->_raw_name.bytes != nullptr)
        amqp_bytes_free(this->_raw_name);

    if (this->_raw_tag.bytes != nullptr)
        amqp_bytes_free(this->_raw_tag);
}

bool queue::bind(const std::string& exchange, const std::string& route)
{
    amqp_queue_bind(this->_owner,
                    1,
                    this->_raw_name,
                    amqp_cstring_bytes(exchange.c_str()),
                    amqp_cstring_bytes(route.c_str()),
                    amqp_empty_table);
    if (amqp_get_rpc_reply(this->_owner).reply_type != AMQP_RESPONSE_NORMAL)
        return false;

    auto name_bytes_c = static_cast<char*>(this->_raw_name.bytes);
    this->_name       = std::string(name_bytes_c, name_bytes_c + this->_raw_name.len);

    auto r = amqp_basic_consume(this->_owner, 1, this->_raw_name, amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
    if (amqp_get_rpc_reply(this->_owner).reply_type != AMQP_RESPONSE_NORMAL)
        return false;

    this->_raw_tag = amqp_bytes_malloc_dup(r->consumer_tag);
    if (this->_raw_tag.bytes == nullptr)
        throw std::runtime_error("Out of memory while copying consumer tag");

    auto tag_bytes_c = static_cast<char*>(this->_raw_tag.bytes);
    this->_tag       = std::string(tag_bytes_c, tag_bytes_c + this->_raw_tag.len);

    this->_route = route;
    return true;
}

const std::string& queue::name() const
{
    return this->_name;
}

const std::string& queue::route() const
{
    return this->_route;
}

const std::string& queue::consumer_tag() const
{
    return this->_tag;
}

async::task<void> queue::invoke(const std::vector<uint8_t>& message)
{
    auto stream = fb::stream(message.data(), message.size());
    auto reader = fb::stream_reader<>(stream);
    auto cmd    = reader.read<uint32_t>();
    auto size   = reader.read<uint32_t>();
    auto found  = this->_handler.find(cmd);
    if (found == this->_handler.end())
        co_return;

    co_await found->second(((const uint8_t*)stream.data()) + (sizeof(uint32_t) * 2));
}

void queue::invoke_async(const std::vector<uint8_t>& message)
{
    auto target_thread = this->_threads.least_loaded();
    if (target_thread == nullptr)
    {
        // Fallback to synchronous invoke if no thread available
        async::awaitable_then(this->invoke(message), [](async::awaitable_result<void> result) {
            // work done
            try
            {
                result();
            }
            catch (std::exception& e)
            {
                fb::logger::fatal("AMQP message processing error: {}", e.what());
            }
            catch (...)
            {
                fb::logger::fatal("AMQP message processing error: unknown error");
            }
        });
    }
    else
    {
        // Enqueue to the least loaded thread
        target_thread->enqueue(
            [message, this](auto& thread) -> async::task<void> {
                co_await this->invoke(message);
            },
            [](std::exception& e) {
                fb::logger::fatal("AMQP message processing error: {}", e.what());
            },
            []() {
                // work done
            });
    }
}

void queue::handler(uint32_t cmd, const handle_func& fn)
{
    this->_handler.insert({cmd, fn});
}