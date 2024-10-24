#include <context.h>

fb::gateway::context::context(boost::asio::io_context& context, uint16_t port) : 
    fb::acceptor<fb::gateway::session>(context, port)
{
    static constexpr const char* message = "CONNECTED SERVER\n";
    this->_connection_cache.write_u8(0x7E)
                           .write_u8(0x1B)
                           .write((const void*)message, strlen(message));

    this->load_entries();

    // Register event handler
    this->bind<fb::protocol::gateway::request::assert_version>  (std::bind(&context::handle_check_version,   this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::gateway::request::entry_list>      (std::bind(&context::handle_entry_list,      this, std::placeholders::_1, std::placeholders::_2));
}

fb::gateway::context::~context()
{ }

bool fb::gateway::context::load_entries()
{
    try
    {
        // Load gateway list
        auto& entrypoints = fb::config::get()["entrypoints"];
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

fb::ostream fb::gateway::context::make_crt_stream(const fb::cryptor& crt)
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

bool fb::gateway::context::decrypt_policy(uint8_t cmd) const
{
    switch(cmd)
    {
    case 0x00:
        return false;

    default:
        return true;
    }
}

fb::gateway::session* fb::gateway::context::handle_accepted(fb::socket<fb::gateway::session>& socket)
{
    auto session = std::make_unique<fb::gateway::session>();
    auto ptr = session.get();
    this->_sessions.push_back(std::move(session));
    return ptr;
}

bool fb::gateway::context::handle_connected(fb::socket<fb::gateway::session>& socket)
{
    socket.send(this->_connection_cache, false);

    fb::logger::info("{}님이 접속했습니다.", socket.IP());
    return true;
}

async::task<bool> fb::gateway::context::handle_disconnected(fb::socket<fb::gateway::session>& socket)
{
    fb::logger::info("{}님의 연결이 끊어졌습니다.", socket.IP());
    co_return false;
}

async::task<bool> fb::gateway::context::handle_check_version(fb::socket<fb::gateway::session>& socket, const fb::protocol::gateway::request::assert_version& request)
{
    try
    {
        util::assert_client(request);

        auto crt = cryptor::generate();
        socket.crt(crt);

        this->send(socket, fb::protocol::gateway::response::crt(crt, this->_entry_crc32_cache), false);
        co_return true;
    }
    catch(std::exception& e)
    {
        co_return false;
    }
}

async::task<bool> fb::gateway::context::handle_entry_list(fb::socket<fb::gateway::session>& socket, const fb::protocol::gateway::request::entry_list& request)
{
    switch(request.action)
    {
    case 0x00:
    {
        const auto&         entry = this->_entrypoints[request.index];
        this->transfer(socket, entry.ip, entry.port, fb::protocol::internal::services::GATEWAY);
        co_return true;
    }

    case 0x01:
    {
        this->send_stream(socket, this->_entry_stream_cache);
        co_return true;
    }

    default:
        co_return false;
    }
}