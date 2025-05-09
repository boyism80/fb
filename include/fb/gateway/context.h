#ifndef __FB_GATEWAY_H__
#define __FB_GATEWAY_H__

#include <fb/acceptor.h>
#include <fb/gateway/session.h>
#include <fb/gateway/util.h>

using namespace fb::protocol::internal;
namespace internal_resp = fb::protocol::internal::response;

namespace fb::gateway {

using namespace fb::protocol::gateway;

/**
 * @brief      This class describes a context.
 */
class context : public fb::acceptor<fb::gateway::session>
{
public:
    using unique_session = std::unique_ptr<fb::gateway::session>;

private:
    std::vector<endpoint>       _entrypoints;
    fb::stream                  _endpoint_bytes;
    uint32_t                    _endpoint_crc = 0;
    fb::stream                  _connection_cache;
    std::vector<unique_session> _sessions;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  port     The port
     */
    context(boost::asio::io_context& context, uint16_t port);
    /**
     * @brief      Destroys the object.
     */
    ~context();

private:
    /**
     * @brief      Loads entries.
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> load_entries();

private:
    /**
     * @brief      Makes a crt stream.
     *
     * @param[in]  crt   The crt
     *
     * @return     { description_of_the_return_value }
     */
    fb::stream make_crt_stream(const fb::crypto& crt);

    // override

protected:
    /**
     * @brief      { function_description }
     *
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool decrypt_policy(uint8_t) const override final;

    /**
     * @brief      { function_description }
     *
     * @param      amqp  The amqp
     */
    void handle_declare_amqp_queue(fb::amqp::socket& amqp) override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_start() final;

    /**
     * @brief      { function_description }
     *
     * @param      socket  The socket
     *
     * @return     { description_of_the_return_value }
     */
    fb::gateway::session* handle_accepted(fb::socket<fb::gateway::session>& socket) final;
    /**
     * @brief      { function_description }
     *
     * @param      session  The session
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_connected(fb::socket<fb::gateway::session>& session) final;
    /**
     * @brief      { function_description }
     *
     * @param      session  The session
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_disconnected(fb::socket<fb::gateway::session>& session) final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_shutdown(const internal_resp::Shutdown& response);

    // for heart-beat

protected:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    Service service() const override final
    {
        return Service::Gateway;
    }

public:
    /**
     * @brief      { function_description }
     *
     * @param      session    The session
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_check_version(fb::socket<fb::gateway::session>& session,
                                                         const fb::protocol::gateway::request::version&);
    /**
     * @brief      { function_description }
     *
     * @param      session    The session
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_entry_list(fb::socket<fb::gateway::session>& session,
                                                      const fb::protocol::gateway::request::endpoint&);
};

} // namespace fb::gateway

#endif // !__FB_GATEWAY_H__
