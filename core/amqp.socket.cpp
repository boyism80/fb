#include <fb/amqp.h>

using namespace fb::amqp;

socket::socket()
{ }

socket::~socket()
{
    amqp_channel_close(this->_conn, 1, AMQP_REPLY_SUCCESS);
    amqp_connection_close(this->_conn, AMQP_REPLY_SUCCESS);
    amqp_destroy_connection(this->_conn);
}

bool socket::connect(const std::string& hostname, uint16_t port, const std::string& id, const std::string& pw, const std::string& vhost)
{
    this->_conn = amqp_new_connection();
    this->_socket = amqp_tcp_socket_new(this->_conn);
    if (!this->_socket)
        return false;

    auto status = amqp_socket_open(this->_socket, hostname.c_str(), port);
    if (status)
        return false;

    if (amqp_login(this->_conn, vhost.c_str(), 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, id.c_str(), pw.c_str()).reply_type != AMQP_RESPONSE_NORMAL)
        return false;

    amqp_channel_open(this->_conn, 1);
    if (amqp_get_rpc_reply(this->_conn).reply_type != AMQP_RESPONSE_NORMAL)
        return false;

    return true;
}

queue& socket::declare_queue()
{
    auto r = amqp_queue_declare(this->_conn, 1, amqp_empty_bytes, 0, 0, 0, 1, amqp_empty_table);
    if (amqp_get_rpc_reply(this->_conn).reply_type != AMQP_RESPONSE_NORMAL)
        throw std::runtime_error("Declaring queue");

    auto name = amqp_bytes_malloc_dup(r->queue);
    if (name.bytes == nullptr)
        throw std::runtime_error("Out of memory while copying queue name");

    auto ptr = new queue(*this, name);
    _queues.push_back(std::unique_ptr<queue>(ptr));

    return *ptr;
}

socket::operator amqp_connection_state_t()
{
    return this->_conn;
}

bool socket::select(const timeval* timeout)
{
    amqp_maybe_release_buffers(this->_conn);

    amqp_envelope_t envelope;
    auto ret = amqp_consume_message(this->_conn, &envelope, timeout, 0);
    if (ret.reply_type == AMQP_RESPONSE_NORMAL)
    {
        auto consumer_tag = std::string((const char*)envelope.consumer_tag.bytes, (const char*)envelope.consumer_tag.bytes + envelope.consumer_tag.len);
        for (auto& queue : this->_queues)
        {
            if (queue->consumer_tag() == consumer_tag)
            {
                auto message = std::vector<uint8_t>((uint8_t*)envelope.message.body.bytes, (uint8_t*)envelope.message.body.bytes + envelope.message.body.len);
                queue->invoke(message);
                break;
            }
        }

        
        amqp_destroy_envelope(&envelope);
        return true;
    }

    if (ret.reply_type != AMQP_RESPONSE_LIBRARY_EXCEPTION)
        return false;
    
    if (ret.library_error != AMQP_STATUS_UNEXPECTED_STATE)
        return false;
    
    amqp_frame_t frame;
    if (amqp_simple_wait_frame(this->_conn, &frame) != AMQP_STATUS_OK)
        throw std::runtime_error("amqp_simple_wait_frame failed");

    if (AMQP_FRAME_METHOD != frame.frame_type)
        return false;

    switch (frame.payload.method.id)
    {
    case AMQP_BASIC_ACK_METHOD:
        return false;

    case AMQP_BASIC_RETURN_METHOD:
    {
        amqp_message_t message;
        ret = amqp_read_message(this->_conn, frame.channel, &message, 0);
        if (AMQP_RESPONSE_NORMAL != ret.reply_type)
            throw std::runtime_error("AMQP_BASIC_RETURN_METHOD");

        amqp_destroy_message(&message);
        return false;
    }
    break;

    case AMQP_CHANNEL_CLOSE_METHOD:
        throw std::runtime_error("AMQP_CHANNEL_CLOSE_METHOD");

    case AMQP_CONNECTION_CLOSE_METHOD:
        throw std::runtime_error("AMQP_CONNECTION_CLOSE_METHOD");

    default:
        throw std::runtime_error("An unexpected method was received");
    }
}