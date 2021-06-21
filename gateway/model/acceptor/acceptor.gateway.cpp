#include "model/acceptor/acceptor.gateway.h"

fb::gateway::acceptor::acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay, const INTERNAL_CONNECTION& internal_connection) : 
    fb::acceptor<fb::gateway::session>(context, port, accept_delay, internal_connection, fb::config::get()["thread"].isNull() ? 0xFF : fb::config::get()["thread"].asInt())
{
    static const char* message = "CONNECTED SERVER\n";
    this->_connection_cache.write_u8(0x7E)
                           .write_u8(0x1B)
                           .write((const void*)message, strlen(message));

    this->load_entries();

    // Register event handler
    this->bind<fb::protocol::gateway::request::assert_version>  (0x00, std::bind(&acceptor::handle_check_version,   this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::gateway::request::entry_list>      (0x57, std::bind(&acceptor::handle_entry_list,      this, std::placeholders::_1, std::placeholders::_2));
}

fb::gateway::acceptor::~acceptor()
{}

bool fb::gateway::acceptor::load_entries()
{
    try
    {
        // Load gateway list
        auto entrypoints = fb::config::get()["entrypoints"];
        for(auto i = entrypoints.begin(); i != entrypoints.end(); i++)
        {
            this->_entrypoints.push_back(entry
            (
                cp949((*i)["name"].asCString()), 
                cp949((*i)["desc"].asCString()), 
                (*i)["ip"].asCString(), 
                (*i)["port"].asInt()
            ));
        }

        fb::protocol::gateway::response::hosts(this->_entrypoints).serialize(this->_entry_stream_cache);
        this->_entry_crc32_cache = this->_entry_stream_cache.crc();
        return true;
    }
    catch(...)
    {
        return false;
    }
}

fb::ostream fb::gateway::acceptor::make_crt_stream(const fb::cryptor& crt)
{
    fb::ostream             ostream;
    ostream.write_u8(0x00)      // cmd : 0x00
        .write_u8(0x00)
        .write_u32(this->_entry_crc32_cache)
        .write_u8(crt.type())
        .write_u8(0x09)
        .write(crt.key(), 0x09)
        .write_u8(0x00);

    return ostream;
}

fb::gateway::session* fb::gateway::acceptor::handle_accepted(fb::socket<fb::gateway::session>& socket)
{
    return new fb::gateway::session();
}

bool fb::gateway::acceptor::handle_connected(fb::socket<fb::gateway::session>& socket)
{
    socket.send(this->_connection_cache, false);

    auto& c = fb::console::get();
    c.puts("%s님이 접속했습니다.", socket.IP().c_str());
    return true;
}

bool fb::gateway::acceptor::handle_disconnected(fb::socket<fb::gateway::session>& socket)
{
    auto& c = fb::console::get();
    c.puts("%s님의 연결이 끊어졌습니다.", socket.IP().c_str());
    return false;
}

bool fb::gateway::acceptor::handle_check_version(fb::socket<fb::gateway::session>& socket, const fb::protocol::gateway::request::assert_version& request)
{
    try
    {
        this->_gateway_service.assert_client(request);

        auto crt = cryptor::generate();
        socket.crt(crt);

        this->send(socket, fb::protocol::gateway::response::crt(crt, this->_entry_crc32_cache), false);
        return true;
    }
    catch(std::exception& e)
    {
        return false;
    }
}

bool fb::gateway::acceptor::handle_entry_list(fb::socket<fb::gateway::session>& socket, const fb::protocol::gateway::request::entry_list& request)
{
    switch(request.action)
    {
    case 0x00:
    {
        const auto&         entry = this->_entrypoints[request.index];
        this->transfer(socket, entry.ip(), entry.port(), fb::protocol::internal::services::SERVICE_GATEWAY);
        return true;
    }

    case 0x01:
    {
        this->send_stream(socket, this->_entry_stream_cache);
        return true;
    }

    default:
        return false;
    }
}