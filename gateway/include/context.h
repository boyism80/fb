#ifndef __FB_GATEWAY_H__
#define __FB_GATEWAY_H__

#include <fb/acceptor.h>
#include <fb/config.h>
#include <session.h>
#include <util.h>
#include <zlib.h>
#include <memory>
#include <fb/protocol/gateway.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/encoding.h>

using namespace fb::protocol::internal;

namespace fb { namespace gateway {

using namespace fb::protocol::gateway;

/**
 * @brief      This class describes a context.
 */
class context : public fb::acceptor<fb::gateway::session>
{
public:
    using unique_session = std::unique_ptr<fb::gateway::session>;

private:
    std::vector<entry>          _entrypoints;
    fb::ostream                 _entry_stream_cache;
    uint32_t                    _entry_crc32_cache = 0;
    fb::ostream                 _connection_cache;
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
    bool load_entries();

private:
    /**
     * @brief      Makes a crt stream.
     *
     * @param[in]  crt   The crt
     *
     * @return     { description_of_the_return_value }
     */
    fb::ostream make_crt_stream(const fb::cryptor& crt);

    // override

protected:
    /**
     * @brief      { function_description }
     *
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool decrypt_policy(uint8_t) const final;
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
    async::task<bool> handle_connected(fb::socket<fb::gateway::session>& session) final;
    /**
     * @brief      { function_description }
     *
     * @param      session  The session
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> handle_disconnected(fb::socket<fb::gateway::session>& session) final;

    // for heart-beat

protected:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    Service service() const final
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
    async::task<bool> handle_check_version(fb::socket<fb::gateway::session>& session,
                                           const fb::protocol::gateway::request::assert_version&);
    /**
     * @brief      { function_description }
     *
     * @param      session    The session
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> handle_entry_list(fb::socket<fb::gateway::session>& session,
                                        const fb::protocol::gateway::request::entry_list&);
};

}} // namespace fb::gateway

#endif // !__FB_GATEWAY_H__
