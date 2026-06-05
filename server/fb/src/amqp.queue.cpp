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

bool queue::bind(std::string_view exchange, std::string_view route)
{
    // Create amqp_bytes_t directly from string_view (no conversion needed for exchange)
    amqp_bytes_t exchange_bytes;
    exchange_bytes.len   = exchange.size();
    exchange_bytes.bytes = const_cast<void*>(static_cast<const void*>(exchange.data()));

    // route needs conversion only for member assignment at the end
    auto         route_str = std::string(route);
    amqp_bytes_t route_bytes;
    route_bytes.len   = route.size();
    route_bytes.bytes = const_cast<void*>(static_cast<const void*>(route.data()));

    amqp_queue_bind(this->_owner, 1, this->_raw_name, exchange_bytes, route_bytes, amqp_empty_table);
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

    // Only route needs conversion here because _route is std::string member
    this->_route = route_str;
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
    auto opcode = reader.read<uint32_t>();
    auto size   = reader.read<uint32_t>();
    auto found  = this->_handler.find(opcode);
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
        auto builder = target_thread->new_builder<void>();
        builder.func = [message, this](auto& thread) -> async::task<void> {
            co_await this->invoke(message);
        };
        builder.on_error = [](std::exception& e) {
            fb::logger::fatal("AMQP message processing error: {}", e.what());
        };
        builder.enqueue();
    }
}

void queue::handler(uint32_t opcode, handle_func&& fn)
{
    this->_handler.insert({opcode, std::move(fn)});
}