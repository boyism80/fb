#include "acceptor.h"

template <typename T>
fb::acceptor<T>::acceptor(boost::asio::io_context& context, uint16_t port) : 
    base_acceptor(context, port)
{
    this->accept();
}

template <typename T>
fb::acceptor<T>::~acceptor()
{
    for(auto x : this->sessions)
        delete x;
}

template <typename T>
void fb::acceptor<T>::accept()
{
    auto new_socket = new fb::socket(this);
    this->async_accept
    (
        *new_socket,
        [this, new_socket](boost::system::error_code error)
        {
            try
            {
                if(error)
                    throw std::runtime_error(error.message());

                auto session = this->handle_alloc_session(new_socket);
                this->_session_map.push(new_socket, session);
                this->sessions.push_back(session);

                new_socket->recv();
                this->accept();

                this->handle_connected(*session);
            }
            catch(std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }
        }
    );
}

template<class T>
inline bool fb::acceptor<T>::call_handle(T& session, uint8_t cmd)
{
    auto i = this->_response_table.find(cmd);
    if(i == this->_response_table.end())
        return true;

    return i->second(session);
}

template <typename T>
void fb::acceptor<T>::handle_parse(T& session)
{
    static uint8_t      not_crt_cmd[] = {0x00, 0x10};
    static uint8_t      base_size   = sizeof(uint8_t) + sizeof(uint16_t);
    auto&               istream     = session.in_stream();
    auto&               crt         = session.crt();

    while(true)
    {
        try
        {
            if(istream.readable_size() < base_size)
                break;

            // Read base head and check it is 0xAA
            uint8_t             head = istream.read_u8();
            if(head != 0xAA)
                throw std::exception();


            // Read data size and check it is greater than buffer size
            uint16_t            size = istream.read_u16(buffer::endian::BIG);
            if(size > istream.capacity())
                throw std::exception();


            // If data size is not enough to parse, do not anymore
            if(istream.readable_size() < size)
                break;


            // Erase head and size data
            istream.shift(base_size);

            // If command byte is not 0x10, this data must be decrypted by session's encrypt key
            uint8_t             cmd = istream.read_u8();
            if(std::all_of(not_crt_cmd, not_crt_cmd + sizeof(not_crt_cmd)/sizeof(uint8_t), [cmd] (uint8_t x) { return cmd != x; }))
                size = crt.decrypt(istream, istream.position(), size);

            // Call function that matched by command byte
            istream.reset();
            if(this->call_handle(session, cmd) == false)
                throw std::exception();

            // Set data pointer to process next packet bytes
            istream.shift(size);
        }
        catch(...)
        {
            break;
        }
    }

    istream.reset();
}

template <typename T>
fb::ostream fb::acceptor<T>::make_transfer_stream(const fb::cryptor& crt, uint32_t ip, uint16_t port, const fb::ostream* parameter)
{
    fb::ostream             ostream;
    auto                    backward_size = parameter != nullptr ? 0x0B + parameter->size() : 0x0B;
    ostream.write_u8(0x03)
        .write_u32(ip)                      // ip
        .write_u16(port)                    // port
        .write_u8(uint8_t(backward_size))   // backward size
        .write_u8(crt.types())
        .write_u8(cryptor::KEY_SIZE)
        .write(crt.key(), cryptor::KEY_SIZE);

    if(parameter != nullptr)
        ostream.write(*parameter);

    return ostream;
}

template <typename T>
void fb::acceptor<T>::handle_receive(fb::socket& socket)
{
    this->handle_parse(*this->_session_map[socket]);
}

template <typename T>
void fb::acceptor<T>::handle_disconnected(fb::socket& socket)
{
    auto session = this->_session_map[socket];
    if(session == nullptr)
        return;

    this->handle_disconnected(*session);
    this->_session_map.erase(socket);
    this->sessions.erase(session);

    delete session;
}

template <typename T>
void fb::acceptor<T>::register_fn(uint8_t cmd, std::function<bool(T&)> fn)
{
    this->_response_table.insert(std::make_pair(cmd, fn));
}

template<class T>
void fb::acceptor<T>::transfer(T& session, uint32_t ip, uint16_t port)
{
    auto&                       crt = session.crt();
    auto                        ostream = this->make_transfer_stream(crt, ip, port);
    crt.wrap(ostream);

    session.send(ostream, false, false);
}

template<class T>
void fb::acceptor<T>::transfer(T& session, uint32_t ip, uint16_t port, const fb::ostream& parameter)
{
    auto&                       crt = session.crt();
    auto                        ostream = this->make_transfer_stream(crt, ip, port, &parameter);
    crt.wrap(ostream);

    session.send(ostream, false, false);
}

template <typename T>
void fb::acceptor<T>::send_stream(fb::base& base, const fb::ostream& stream, bool encrypt, bool wrap)
{
    if(stream.empty())
        return;

    base.send(stream, encrypt, wrap);
}
