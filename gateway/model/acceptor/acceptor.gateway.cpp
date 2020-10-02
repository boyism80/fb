#include "model/acceptor/acceptor.gateway.h"

using namespace fb::gateway;

acceptor::acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay) : 
    fb::acceptor<session>(context, port, accept_delay)
{
    this->load_entries();

    // Register event handler
    this->bind<fb::protocol::request::gateway::assert_version>  (0x00, std::bind(&acceptor::handle_check_version,   this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::request::gateway::entry_list>      (0x57, std::bind(&acceptor::handle_entry_list,      this, std::placeholders::_1, std::placeholders::_2));
}

acceptor::~acceptor()
{}

bool fb::gateway::acceptor::load_entries()
{
    try
    {
        // Load gateway list
        auto entries = fb::config()["entries"];
        for(auto i = entries.begin(); i != entries.end(); i++)
        {
            this->_entries.push_back(entry
            (
                cp949(i.key().asCString()), 
                cp949((*i)["desc"].asCString()), 
                (*i)["ip"].asCString(), 
                (*i)["port"].asInt()
            ));
        }

        response::gateway::hosts(this->_entries).serialize(this->_entry_stream_cache);
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

fb::gateway::session* fb::gateway::acceptor::handle_alloc_session(fb::socket* socket)
{
    return new fb::gateway::session(socket);
}

bool acceptor::handle_connected(fb::gateway::session& session)
{
    static uint8_t data[] = {0xAA, 0x00, 0x13, 0x7E, 0x1B, 0x43, 0x4F, 0x4E, 0x4E, 0x45, 0x43, 0x54, 0x45, 0x44, 0x20, 0x53, 0x45, 0x52, 0x56, 0x45, 0x52, 0x0A};
    static ostream ostream(data, sizeof(data));

    session.send(ostream, false, false);
    std::cout << "connection request" << std::endl;
    return true;
}

bool acceptor::handle_disconnected(fb::gateway::session& session)
{
    std::cout << "disconnection request" << std::endl;
    return false;
}

bool acceptor::handle_check_version(fb::gateway::session& session, const fb::protocol::request::gateway::assert_version& request)
{
    try
    {
        this->_gateway_service.assert_client(request);

        auto crt = cryptor::generate();
        session.crt(crt);

        this->send(session, response::gateway::crt(crt, this->_entry_crc32_cache), false);
        return true;
    }
    catch(std::exception& e)
    {
        return false;
    }
}

bool acceptor::handle_entry_list(fb::gateway::session& session, const fb::protocol::request::gateway::entry_list& request)
{
    switch(request.action)
    {
    case 0x00:
    {
        const auto&         entry = this->_entries[request.index];
        this->transfer(session, entry.ip(), entry.port());
        return true;
    }

    case 0x01:
    {
        this->send_stream(session, this->_entry_stream_cache);
        return true;
    }

    default:
        return false;
    }
}