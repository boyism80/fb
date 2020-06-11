#include "fb_login.h"
using namespace fb::login;

session::session(fb::socket* socket) : 
    _socket(socket)
{}

session::~session()
{}

void fb::login::session::send(const fb::ostream& stream, bool encrypt, bool wrap)
{
    this->_socket->send(stream, encrypt, wrap);
}

fb::cryptor& fb::login::session::crt()
{
    return this->_socket->crt();
}

void fb::login::session::crt(const fb::cryptor& crt)
{
    this->_socket->crt(crt);
}

void fb::login::session::crt(uint8_t enctype, const uint8_t* enckey)
{
    this->_socket->crt(enctype, enckey);
}

fb::istream& fb::login::session::in_stream()
{
    return this->_socket->in_stream();
}

fb::login::session::operator fb::socket & ()
{
    return *this->_socket;
}

acceptor::acceptor(boost::asio::io_context& context, uint16_t port) : 
    fb::acceptor<session>(context, port), _gateway_crc(0)
{
    const auto& config = fb::config();

    // Parse gateway 
    for(auto x : config["game"]["gateway"])
        this->_gateway_list.push_back(std::make_unique<gateway_form>(x["name"].asCString(), x["desc"].asCString(), x["ip"].asCString(), x["port"].asInt()));
    this->_gateway_data = this->make_gateway_stream(&this->_gateway_crc);


    // Parse agreement
    for(auto x : config["login"]["account option"]["forbidden"])
        this->_forbidden_names.push_back(x.asString());
    this->_agreement_data = this->make_agreement_stream();


    // Register event handler
    this->register_fn(0x00, std::bind(&acceptor::handle_check_version, this, std::placeholders::_1));
    this->register_fn(0x02, std::bind(&acceptor::handle_check_id, this, std::placeholders::_1));
    this->register_fn(0x03, std::bind(&acceptor::handle_login, this, std::placeholders::_1));
    this->register_fn(0x04, std::bind(&acceptor::handle_create_account, this, std::placeholders::_1));
    this->register_fn(0x10, std::bind(&acceptor::handle_agreement, this, std::placeholders::_1));
    this->register_fn(0x26, std::bind(&acceptor::handle_change_password, this, std::placeholders::_1));
    this->register_fn(0x57, std::bind(&acceptor::handle_gateway_list, this, std::placeholders::_1));
}

acceptor::~acceptor()
{
    Json::StyledWriter writer;
    std::ofstream ofstream;
}

fb::login::session* acceptor::handle_alloc_session(fb::socket* socket)
{
    return new login::session(socket);
}

uint32_t acceptor::compress(const uint8_t* source, uint32_t size, uint8_t* dest) const
{
    static uint8_t              buffer[65536];
    uint32_t                    buf_size = sizeof(buffer);

    compress2(buffer, (uLongf*)&buf_size, source, size, Z_BEST_COMPRESSION);
    memcpy(dest, buffer, buf_size);
    dest[buf_size]          = 0x00;
    return buf_size;
}

fb::ostream acceptor::make_gateway_stream(uint32_t* crc) const
{
    // 서버정보를 바이너리 형식으로 변환
    ostream                     formats;
    char                        buffer[256];
    formats.write_u8((uint8_t)this->_gateway_list.size());
    for(uint32_t i = 0; i < this->_gateway_list.size(); i++)
    {
        uint32_t                len = sprintf(buffer, "%s;%s", this->_gateway_list[i]->name.c_str(), this->_gateway_list[i]->desc.c_str()) + 1;

        formats.write_u8(i)
               .write_u32(this->_gateway_list[i]->ip)
               .write_u16(this->_gateway_list[i]->port)
               .write(buffer, len);
    }

    // 바이너리 형식의 crc 계산
    if(crc != nullptr)
        *crc = crc32(0, formats.data(), formats.size());

    // 바이너리 데이터 압축
    uint8_t                     compressed_buffer[1024];
    uint32_t                    compressed_size = this->compress(formats.data(), formats.size(), compressed_buffer);

    // 패킷 형식으로 저장
    fb::ostream                 ostream;
    ostream.write_u8(0x56)
           .write_u16(compressed_size)
           .write(compressed_buffer, compressed_size + 1);

    return ostream;
}

fb::ostream acceptor::make_agreement_stream() const
{
    uint8_t                 buffer[4096];
    const auto              agreement_str = fb::config()["login"]["agreement"].asString();
    uint32_t                agreement_compressed_size = this->compress((const uint8_t*)agreement_str.c_str(), agreement_str.length() + 2, buffer);
    

    fb::ostream             ostream;
    ostream.write_u8(0x60)
           .write_u8(0x01)
           .write_u16(agreement_compressed_size)
           .write(buffer, agreement_compressed_size);

    return ostream;
}

const fb::ostream acceptor::make_message_stream(int type, const char* msg) const
{
    ostream                 ostream;
    uint32_t                message_size = strlen(msg) + 1;

    ostream.write_u8(0x02)
           .write_u8(type)
           .write_u8(message_size)
           .write((const void*)msg, message_size);

    return ostream;
}

bool fb::login::acceptor::is_hangul(const char* str)
{
    auto                    size = strlen(str);
    if(size % 2 != 0)
        return false;

    for(uint32_t i = 0; i < size; i += 2)
    {
        if(strncmp(str + i, "가", sizeof(char) * 2) == -1)
            return false;

        if(strncmp(str + i, "힣", sizeof(char) * 2) == 1)
            return false;
    }

    return true;
}

bool fb::login::acceptor::is_forbidden(const char* str)
{
    return std::any_of
    (
        this->_forbidden_names.begin(), 
        this->_forbidden_names.end(), 
        [str](std::string& x)
        {
            return x == str;
        }
    );
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
    auto&                   istream = session.in_stream();
    auto                    cmd     = istream.read_u8();
    auto                    client_version  = istream.read_u16();
    if(client_version != 0x0226)
        return false;

    auto                    national_key    = istream.read_u8();
    if(national_key != 0xD7)
        return false;

    auto                    enc_type        = rand() % 0x09;
    uint8_t                 enc_key[0x09] = {0,};

    for(int i = 0; i < 0x09; i++)
        enc_key[i] = rand() % 255 + 1;


    fb::ostream             ostream;
    ostream.write_u8(0x00)      // cmd : 0x00
           .write_u8(0x00)
           .write_u32(this->_gateway_crc)
           .write_u8(enc_type)
           .write_u8(0x09)
           .write(enc_key, 0x09)
           .write_u8(0x00);
    this->send_stream(session, ostream, false);
    session.crt(enc_type, enc_key);
    return true;
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
        this->transfer(session, this->_gateway_list[index]->ip, this->_gateway_list[index]->port);
    }

    case 0x01:
    {
        this->send_stream(session, this->_gateway_data);
    }

    default:
        return false;
    }
    return true;
}

bool acceptor::handle_agreement(fb::login::session& session)
{
    auto&                   istream  = session.in_stream();
    auto                    cmd      = istream.read_u8();

    // Read encrypt type
    auto                    enc_type = istream.read_u8();
    if(enc_type > 0x09)
        return false;

    // Read encrypt key size
    auto                    enc_key_size = istream.read_u8();
    if(enc_key_size != 0x09)
        return false;

    // Read encrypt key
    uint8_t                 enc_key[0x09];
    istream.read(enc_key, enc_key_size);
    for(int i = 0; i < 0x09; i++)
        if(enc_key[i] == 0) return false;

    // Set encrypt type and key
    session.crt(enc_type, enc_key);


    // Send agreement data
    this->send_stream(session, this->_agreement_data);
    return true;
}

bool acceptor::handle_check_id(fb::login::session& session)
{
    auto&                   istream = session.in_stream();

    try
    {
        auto                cmd = istream.read_u8(); // 0x02


        // Read character's name
        auto                name = istream.readstr_u8();
        if(name.length() < MIN_NAME_SIZE || name.length() > MAX_NAME_SIZE)
            throw id_exception(message::INVALID_NAME);

        // Name must be full-hangul characters
        if(fb::config()["login"]["account option"]["allow other language"].asBool() == false && this->is_hangul(name.c_str()) == false)
            throw id_exception(message::INVALID_NAME);

        // Name cannot contains subcharacters in forbidden list
        if(this->is_forbidden(name.c_str()))
            throw id_exception(message::INVALID_NAME);

        
        // Read character's password
        auto                pw = istream.readstr_u8();
        if(pw.length() < MIN_PASSWORD_SIZE || pw.length() > MAX_PASSWORD_SIZE)
            throw pw_exception(message::PASSWORD_SIZE);

        this->send_stream(session, this->make_message_stream(0x00, ""));
    }
    catch(login_exception& e)
    {
        this->send_stream(session, this->make_message_stream(e.exc_type(), e.what()));
    }

    return true;
}

bool acceptor::handle_create_account(fb::login::session& session)
{
    auto&                   istream  = session.in_stream();

    // 각각의 요소들 유효성 체크해야함
    auto                    cmd      = istream.read_u8();
    auto                    hair     = istream.read_u8();
    auto                    sex      = istream.read_u8();
    auto                    nation   = istream.read_u8();
    auto                    creature = istream.read_u8();

    this->send_stream(session, this->make_message_stream(0x00, message::SUCCESS_REGISTER_ACCOUNT));
    return true;
}

bool acceptor::handle_login(fb::login::session& session)
{
    auto&                   istream = session.in_stream();
    auto                    cmd     = istream.read_u8();

    auto                    name    = istream.readstr_u8();
    auto                    pw      = istream.readstr_u8();

    this->send_stream(session, this->make_message_stream(0x00, ""));
    this->transfer(session, inet_addr("192.168.0.100"), 10021);
    return true;
}

bool acceptor::handle_change_password(fb::login::session& session)
{
    auto&                   istream = session.in_stream();
    auto                    cmd     = istream.read_u8();


    try
    {
        auto                name = istream.readstr_u8();
        if(name.length() < MIN_NAME_SIZE || name.length() > MAX_NAME_SIZE)
            throw id_exception(message::INVALID_NAME);

        // Name must be full-hangul characters
        if(fb::config()["login"]["account option"]["allow other language"].asBool() == false && this->is_hangul(name.c_str()) == false)
            throw id_exception(message::INVALID_NAME);

        // Name cannot contains subcharacters in forbidden list
        if(this->is_forbidden(name.c_str()))
            throw id_exception(message::INVALID_NAME);


        // TODO : 존재하는 아이디인지 검사



        auto                pw = istream.readstr_u8();
        if(pw.length() < MIN_PASSWORD_SIZE || pw.length() > MAX_PASSWORD_SIZE)
            throw pw_exception(message::PASSWORD_SIZE);

        // TODO : 올바른 비밀번호인지 체크



        auto                new_pw = istream.readstr_u8();
        if(new_pw.length() < MIN_PASSWORD_SIZE || new_pw.length() > MAX_PASSWORD_SIZE)
            throw newpw_exception(message::PASSWORD_SIZE);
        // TODO : 너무 쉬운 비밀번호인지 체크

        if(pw == new_pw)
            throw newpw_exception(message::NEW_PW_EQUALIZATION);

        auto                birthday = istream.read_u32(buffer::endian::BIG);
        if(birthday < 100000 || birthday >= 1000000)
            throw btd_exception();
        // TODO : 올바른 생년월일인지 체크

        this->send_stream(session, this->make_message_stream(0x00, message::SUCCESS_CHANGE_PASSWORD));
    }
    catch(login_exception& e)
    {
        this->send_stream(session, this->make_message_stream(e.exc_type(), e.what()));
    }
    catch(...)
    {
        return false;
    }
    return true;
}
