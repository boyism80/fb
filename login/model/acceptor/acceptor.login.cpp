#include "model/acceptor/acceptor.login.h"
using namespace fb::login;

acceptor::acceptor(boost::asio::io_context& context, uint16_t port) : 
    fb::acceptor<session>(context, port)
{
    this->load_gateways();
    this->load_agreement();

    // Register event handler
    this->register_fn(0x00, std::bind(&acceptor::handle_check_version, this, std::placeholders::_1));
    this->register_fn(0x02, std::bind(&acceptor::handle_create_account, this, std::placeholders::_1));
    this->register_fn(0x03, std::bind(&acceptor::handle_login, this, std::placeholders::_1));
    this->register_fn(0x04, std::bind(&acceptor::handle_account_complete, this, std::placeholders::_1));
    this->register_fn(0x10, std::bind(&acceptor::handle_agreement, this, std::placeholders::_1));
    this->register_fn(0x26, std::bind(&acceptor::handle_change_password, this, std::placeholders::_1));
    this->register_fn(0x57, std::bind(&acceptor::handle_gateway_list, this, std::placeholders::_1));
}

acceptor::~acceptor()
{
    Json::StyledWriter writer;
    std::ofstream ofstream;
}

bool fb::login::acceptor::load_gateways()
{
    try
    {
        // Load gateway list
        for(auto x : fb::config()["game"]["gateway"])
        {
            this->_gateways.push_back(new gateway
            (
                x["name"].asCString(), 
                x["desc"].asCString(), 
                x["ip"].asCString(), 
                x["port"].asInt()
            ));
        }

        this->_gateway_stream_cache = this->_gateways.make_stream();
        this->_gateway_crc32_cache = this->_gateway_stream_cache.crc();
        return true;
    }
    catch(...)
    {
        return false;
    }
}

bool fb::login::acceptor::load_agreement()
{
    try
    {
        auto agreement = fb::config()["login"]["agreement"].asString();
        auto compressed = fb::buffer((uint8_t*)agreement.data(), agreement.size()).compress();

        this->_agreement_stream_cache
            .write_u8(0x60)
            .write_u8(0x01)
            .write_u16((uint16_t)compressed.size())
            .write(compressed.data(), (uint16_t)compressed.size());

        return true;
    }
    catch(...)
    {
        return false;
    }
}

fb::ostream fb::login::acceptor::make_crt_stream(const fb::cryptor& crt)
{
    fb::ostream             ostream;
    ostream.write_u8(0x00)      // cmd : 0x00
        .write_u8(0x00)
        .write_u32(this->_gateway_crc32_cache)
        .write_u8(crt.type())
        .write_u8(0x09)
        .write(crt.key(), 0x09)
        .write_u8(0x00);

    return ostream;
}

const fb::ostream fb::login::acceptor::make_message_stream(int type, const char* msg) const
{
    ostream                 ostream;
    auto                    size = uint32_t(strlen(msg) + 1);

    ostream.write_u8(0x02)
        .write_u8(type)
        .write_u8((uint8_t)size)
        .write((const void*)msg, size);

    return ostream;
}

fb::login::session* acceptor::handle_alloc_session(fb::socket* socket)
{
    return new login::session(socket);
}

bool acceptor::handle_connected(fb::login::session& session)
{
    static uint8_t data[] = {0xAA, 0x00, 0x13, 0x7E, 0x1B, 0x43, 0x4F, 0x4E, 0x4E, 0x45, 0x43, 0x54, 0x45, 0x44, 0x20, 0x53, 0x45, 0x52, 0x56, 0x45, 0x52, 0x0A};
    static ostream ostream(data, sizeof(data));
    
    session.send(ostream, false, false);
    std::cout << "connection request" << std::endl;
    return true;
}

bool acceptor::handle_disconnected(fb::login::session& session)
{
    std::cout << "disconnection request" << std::endl;
    return false;
}

bool acceptor::handle_check_version(fb::login::session& session)
{
    try
    {
        this->_auth_service.assert_client(session);

        auto crt = cryptor::generate();
        session.crt(crt);
        
        this->send_stream(session, this->make_crt_stream(crt), false);
        return true;
    }
    catch(std::exception& e)
    {
        return false;
    }
}

bool acceptor::handle_gateway_list(fb::login::session& session)
{
    auto&                   istream = session.in_stream();
    auto                    cmd     = istream.read_u8();
    auto                    request = istream.read_u8();

    switch(request)
    {
    case 0x00:
    {
        auto                index = istream.read_u8();
        const auto&         gateway = this->_gateways[index];
        this->transfer(session, gateway->ip(), gateway->port());
        return true;
    }

    case 0x01:
    {
        this->send_stream(session, this->_gateway_stream_cache);
        return true;
    }

    default:
        return false;
    }
}

bool acceptor::handle_agreement(fb::login::session& session)
{
    auto&                   istream  = session.in_stream();
    auto                    cmd      = istream.read_u8();

    try
    {
        auto                enc_type = istream.read_u8();
        auto                enc_key_size = istream.read_u8();
        uint8_t             enc_key[0x09];
        istream.read(enc_key, enc_key_size);
        if(cryptor::validate(enc_type, enc_key, enc_key_size) == false)
            throw std::exception();

        session.crt(enc_type, enc_key);
        this->send_stream(session, this->_agreement_stream_cache);
    }
    catch(std::exception& e)
    {
        return false;
    }
}

bool acceptor::handle_create_account(fb::login::session& session)
{
    auto&                   istream = session.in_stream();
    auto                    cmd     = istream.read_u8();
    auto                    id = istream.readstr_u8();
    auto                    pw = istream.readstr_u8();
    try
    {
        this->_auth_service.create_account(id, pw);
        this->send_stream(session, this->make_message_stream(0x00, ""));

        // 일단 아이디 선점해야함
        session.created_id = id;
        return true;
    }
    catch(login_exception& e)
    {
        this->send_stream(session, this->make_message_stream(e.exc_type(), e.what()));
        return true;
    }
    catch(std::exception& e)
    {
        return false;
    }
}

bool acceptor::handle_account_complete(fb::login::session& session)
{
    auto&                   istream  = session.in_stream();
    auto                    cmd      = istream.read_u8();
    auto                    hair     = istream.read_u8();
    auto                    sex      = istream.read_u8();
    auto                    nation   = istream.read_u8();
    auto                    creature = istream.read_u8();

    try
    {
        if(session.created_id.empty())
            throw std::exception();

        this->_auth_service.init_account(session.created_id, hair, sex, nation, creature);
        this->send_stream(session, this->make_message_stream(0x00, message::SUCCESS_REGISTER_ACCOUNT));
        session.created_id.clear();
        return true;
    }
    catch(login_exception& e)
    {
        return true;
    }
    catch(std::exception& e)
    {
        return false;
    }
}

bool acceptor::handle_login(fb::login::session& session)
{
    auto&                   istream = session.in_stream();
    auto                    cmd     = istream.read_u8();
    auto                    id    = istream.readstr_u8();
    auto                    pw      = istream.readstr_u8();

    try
    {
#if !DEBUG & !_DEBUG
        this->_auth_service.login(id, pw);
#endif
        this->send_stream(session, this->make_message_stream(0x00, ""));

        fb::ostream         parameter;
        parameter.write(id);
        this->transfer(session, inet_addr("192.168.0.100"), 10021, parameter);
        return true;
    }
    catch(login_exception& e)
    {
        this->send_stream(session, this->make_message_stream(e.exc_type(), e.what()));
        return true;
    }
    catch(std::exception& e)
    {
        return false;
    }
}

bool acceptor::handle_change_password(fb::login::session& session)
{
    auto&                   istream  = session.in_stream();
    auto                    cmd      = istream.read_u8();
    auto                    name     = istream.readstr_u8();
    auto                    pw       = istream.readstr_u8();
    auto                    new_pw   = istream.readstr_u8();
    auto                    birthday = istream.read_u32(buffer::endian::BIG);

    try
    {
        this->_auth_service.change_pw(name, pw, new_pw, birthday);
        this->send_stream(session, this->make_message_stream(0x00, message::SUCCESS_CHANGE_PASSWORD));
        return true;
    }
    catch(login_exception& e)
    {
        this->send_stream(session, this->make_message_stream(e.exc_type(), e.what()));
        return true;
    }
    catch(std::exception& e)
    {
        return false;
    }
}
