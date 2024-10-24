#include <fb/amqp.h>

using namespace fb::amqp;

queue::queue(socket& owner, const amqp_bytes_t& name) :
    _owner(owner),
    _raw_name(name)
{ }

queue::~queue()
{
    if (this->_raw_name.bytes != nullptr)
        amqp_bytes_free(this->_raw_name);

    if (this->_raw_tag.bytes != nullptr)
        amqp_bytes_free(this->_raw_tag);
}

bool queue::bind(const std::string& exchange, const std::string& binding_key)
{
    amqp_queue_bind(this->_owner, 1, this->_raw_name, amqp_cstring_bytes(exchange.c_str()), amqp_cstring_bytes(binding_key.c_str()), amqp_empty_table);
    if (amqp_get_rpc_reply(this->_owner).reply_type != AMQP_RESPONSE_NORMAL)
        return false;
    this->_name = std::string((const char*)this->_raw_name.bytes, (const char*)this->_raw_name.bytes + this->_raw_name.len);

    auto r      = amqp_basic_consume(this->_owner, 1, this->_raw_name, amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
    if (amqp_get_rpc_reply(this->_owner).reply_type != AMQP_RESPONSE_NORMAL)
        return false;

    this->_raw_tag = amqp_bytes_malloc_dup(r->consumer_tag);
    if (this->_raw_tag.bytes == nullptr)
        throw std::runtime_error("Out of memory while copying consumer tag");
    this->_tag = std::string((const char*)this->_raw_tag.bytes, (const char*)this->_raw_tag.bytes + this->_raw_tag.len);

    return true;
}

const std::string& queue::name() const
{
    return this->_name;
}

const std::string& queue::consumer_tag() const
{
    return this->_tag;
}

async::task<void> queue::invoke(const std::vector<uint8_t>& message)
{
    auto in_stream = fb::istream(message.data(), message.size());
    auto cmd       = in_stream.read_u32();
    auto size      = in_stream.read_u32();
    auto found     = this->_handler.find(cmd);
    if (found == this->_handler.end())
        co_return;

    co_await found->second(((const uint8_t*)in_stream.data()) + (sizeof(uint32_t) * 2));
}