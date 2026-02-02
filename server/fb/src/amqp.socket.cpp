#include <fb/amqp.h>
#include <fb/logger.h>

using namespace fb::amqp;

socket::socket()
{ }

socket::~socket()
{
    if (this->_conn != nullptr)
    {
        amqp_channel_close(this->_conn, 1, AMQP_REPLY_SUCCESS);
        amqp_connection_close(this->_conn, AMQP_REPLY_SUCCESS);
        amqp_destroy_connection(this->_conn);
    }
}

bool socket::connect(std::string_view hostname,
                     uint16_t           port,
                     std::string_view id,
                     std::string_view pw,
                     std::string_view vhost)
{
    // Clean up any existing connection before creating a new one
    if (this->_conn != nullptr)
    {
        amqp_destroy_connection(this->_conn);
        this->_conn   = nullptr;
        this->_socket = nullptr;
    }

    auto hostname_str = std::string(hostname);
    auto id_str = std::string(id);
    auto pw_str = std::string(pw);
    auto vhost_str = std::string(vhost);

    this->_conn   = amqp_new_connection();
    this->_socket = amqp_tcp_socket_new(this->_conn);
    if (!this->_socket)
    {
        amqp_destroy_connection(this->_conn);
        this->_conn = nullptr;
        return false;
    }

    auto status = amqp_socket_open(this->_socket, hostname_str.c_str(), port);
    if (status)
    {
        amqp_destroy_connection(this->_conn);
        this->_conn   = nullptr;
        this->_socket = nullptr;
        return false;
    }

    if (amqp_login(this->_conn, vhost_str.c_str(), 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, id_str.c_str(), pw_str.c_str())
            .reply_type != AMQP_RESPONSE_NORMAL)
    {
        amqp_connection_close(this->_conn, AMQP_REPLY_SUCCESS);
        amqp_destroy_connection(this->_conn);
        this->_conn   = nullptr;
        this->_socket = nullptr;
        return false;
    }

    amqp_channel_open(this->_conn, 1);
    if (amqp_get_rpc_reply(this->_conn).reply_type != AMQP_RESPONSE_NORMAL)
    {
        amqp_connection_close(this->_conn, AMQP_REPLY_SUCCESS);
        amqp_destroy_connection(this->_conn);
        this->_conn   = nullptr;
        this->_socket = nullptr;
        return false;
    }

    return true;
}

queue& socket::declare_queue(bool durable, bool exclusive, bool auto_delete, bool quorum, fb::thread_container& threads)
{
    amqp_table_t arguments = amqp_empty_table;

    // Set quorum queue type if requested
    // Use static storage for table entry to ensure it remains valid during amqp_queue_declare call
    static amqp_table_entry_t quorum_entry;
    if (quorum)
    {
        quorum_entry.key               = amqp_cstring_bytes("x-queue-type");
        quorum_entry.value.kind        = AMQP_FIELD_KIND_UTF8;
        quorum_entry.value.value.bytes = amqp_cstring_bytes("quorum");

        arguments.num_entries = 1;
        arguments.entries     = &quorum_entry;
    }

    // Use empty bytes for auto-generated queue name
    auto r     = amqp_queue_declare(this->_conn,
                                1,
                                amqp_empty_bytes,
                                0,
                                durable ? 1 : 0,
                                exclusive ? 1 : 0,
                                auto_delete ? 1 : 0,
                                arguments);
    auto reply = amqp_get_rpc_reply(this->_conn);
    if (reply.reply_type != AMQP_RESPONSE_NORMAL)
    {
        std::string error_detail;
        if (reply.reply_type == AMQP_RESPONSE_SERVER_EXCEPTION)
        {
            error_detail = "Queue declaration failed (PRECONDITION_FAILED).";
        }
        else if (reply.reply_type == AMQP_RESPONSE_LIBRARY_EXCEPTION)
        {
            error_detail = "Library exception occurred during queue declaration.";
        }
        else
        {
            error_detail = "Unexpected reply type during queue declaration.";
        }

        fb::logger::warn("Failed to declare auto-generated queue: {} (reply_type: {})",
                         error_detail,
                         static_cast<int>(reply.reply_type));
        throw std::runtime_error("Declaring auto-generated queue: " + error_detail);
    }

    auto name = amqp_bytes_malloc_dup(r->queue);
    if (name.bytes == nullptr)
        throw std::runtime_error("Out of memory while copying queue name");

    // queue constructor is private (friend class socket), so cannot use std::make_unique
    auto  ptr = std::unique_ptr<queue>(new queue(*this, name, threads));
    auto& ref = *ptr;
    this->_queues.push_back(std::move(ptr));

    return ref;
}

bool socket::publish(std::string_view             exchange,
                     std::string_view             routing_key,
                     const std::vector<uint8_t>&    message,
                     const amqp_basic_properties_t* properties)
{
    if (this->_conn == nullptr)
        return false;

    auto exchange_str = std::string(exchange);
    auto routing_key_str = std::string(routing_key);
    amqp_bytes_t exchange_bytes    = amqp_cstring_bytes(exchange_str.c_str());
    amqp_bytes_t routing_key_bytes = amqp_cstring_bytes(routing_key_str.c_str());
    amqp_bytes_t message_bytes;
    message_bytes.len   = message.size();
    message_bytes.bytes = const_cast<void*>(static_cast<const void*>(message.data()));

    // Use default properties if not provided
    amqp_basic_properties_t default_props;
    if (properties == nullptr)
    {
        default_props._flags        = AMQP_BASIC_DELIVERY_MODE_FLAG;
        default_props.delivery_mode = 2; // Persistent message
        default_props.content_type  = amqp_cstring_bytes("application/json");
        properties                  = &default_props;
    }

    int result = amqp_basic_publish(this->_conn,
                                    1, // channel
                                    exchange_bytes,
                                    routing_key_bytes,
                                    0, // mandatory
                                    0, // immediate
                                    properties,
                                    message_bytes);

    if (result != 0)
        return false;

    // Check for publish errors
    amqp_rpc_reply_t reply = amqp_get_rpc_reply(this->_conn);
    return reply.reply_type == AMQP_RESPONSE_NORMAL;
}

socket::operator amqp_connection_state_t ()
{
    return this->_conn;
}

bool socket::select(const timeval* timeout)
{
    amqp_maybe_release_buffers(this->_conn);

    amqp_envelope_t envelope;
    auto            ret = amqp_consume_message(this->_conn, &envelope, timeout, 0);
    if (ret.reply_type == AMQP_RESPONSE_NORMAL)
    {
        auto consumer_tag_bytes_c = static_cast<char*>(envelope.consumer_tag.bytes);
        auto consumer_tag         = std::string(consumer_tag_bytes_c, consumer_tag_bytes_c + envelope.consumer_tag.len);
        for (auto& queue : this->_queues)
        {
            if (queue->consumer_tag() == consumer_tag)
            {
                auto message_bytes_c = static_cast<uint8_t*>(envelope.message.body.bytes);
                auto message = std::vector<uint8_t>(message_bytes_c, message_bytes_c + envelope.message.body.len);
                queue->invoke_async(message);
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