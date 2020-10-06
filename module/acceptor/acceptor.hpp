#include <module/acceptor/acceptor.h>

template <typename T>
fb::acceptor<T>::acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay) : 
    base_acceptor(context, port), _accept_delay(accept_delay)
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
                
                this->accept();

                std::async(std::launch::async, 
                    [&new_socket, &session, this]()
                    {
                        std::this_thread::sleep_for(std::chrono::seconds(this->_accept_delay));
                        this->handle_connected(*session);
                        new_socket->recv();
                    });
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
    auto i = this->_handler_dict.find(cmd);
    if(i == this->_handler_dict.end())
    {
        auto& c = fb::console::get();
        c.puts("정의되지 않은 요청입니다. [0x%2X]", cmd);
        return true; 
    }

    return i->second((void*)&session);
}

template <typename T>
void fb::acceptor<T>::handle_parse(T& session)
{
    static uint8_t      not_crt_cmd[] = {0x00, 0x10};
    static uint8_t      base_size   = sizeof(uint8_t) + sizeof(uint16_t);
    auto&               in_stream   = session.in_stream();
    auto&               crt         = session.crt();

    while(true)
    {
        try
        {
            if(in_stream.readable_size() < base_size)
                break;

            // Read base head and check it is 0xAA
            auto                head = in_stream.read_u8();
            if(head != 0xAA)
                throw std::exception();


            // Read data size and check it is greater than buffer size
            auto                size = in_stream.read_u16(buffer::endian::BIG);
            if(size > in_stream.capacity())
                throw std::exception();


            // If data size is not enough to parse, do not anymore
            if(in_stream.readable_size() < size)
                break;


            // If command byte is not 0x10, this data must be decrypted by session's encrypt key
            auto                cmd = in_stream.read_u8();
            if(std::all_of(not_crt_cmd, not_crt_cmd + sizeof(not_crt_cmd)/sizeof(uint8_t), [cmd] (uint8_t x) { return cmd != x; }))
                size = crt.decrypt(in_stream, in_stream.offset() - 1, size);

            // Call function that matched by command byte
            if(this->call_handle(session, cmd) == false)
                throw std::exception();

            // Set data pointer to process next packet bytes
            in_stream.reset();
            in_stream.shift(base_size + size);
            in_stream.flush();
        }
        catch(std::exception& e)
        {
            break;
        }
        catch(...)
        {
            break;
        }
    }

    in_stream.reset();
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
template <typename R>
void fb::acceptor<T>::bind(uint8_t cmd, std::function<bool(T&, R&)> fn)
{
    this->_handler_dict[cmd] = [this, fn](void* x)
    {
        auto& session = *static_cast<T*>(x);
        auto& in_stream = session.in_stream();
        R     request;
        request.deserialize(in_stream);
        return fn(session, request);
    };
}

template<class T>
void fb::acceptor<T>::transfer(T& session, uint32_t ip, uint16_t port)
{
    auto&                       crt = session.crt();
    fb::ostream                 data;
    data.write_u8(crt.type())
        .write_u8(cryptor::KEY_SIZE)
        .write(crt.key(), cryptor::KEY_SIZE);
    
    fb::ostream                 out_stream;
    fb::protocol::response::transfer(ip, port, data).serialize(out_stream);

    crt.wrap(out_stream);
    session.send(out_stream, false, false);
}

template<class T>
void fb::acceptor<T>::transfer(T& session, const std::string& ip, uint16_t port)
{
    this->transfer(session, inet_addr(ip.c_str()), port);
}

template<class T>
void fb::acceptor<T>::transfer(T& session, uint32_t ip, uint16_t port, const fb::ostream& parameter)
{
    auto&                       crt = session.crt();
    fb::ostream                 data;
    data.write_u8(crt.type())
        .write_u8(cryptor::KEY_SIZE)
        .write(crt.key(), cryptor::KEY_SIZE)
        .write(parameter.data(), parameter.size());

    fb::ostream                 out_stream;
    fb::protocol::response::transfer(ip, port, data).serialize(out_stream);

    crt.wrap(out_stream);
    session.send(out_stream, false, false);
}

template<class T>
void fb::acceptor<T>::transfer(T& session, const std::string& ip, uint16_t port, const fb::ostream& parameter)
{
    this->transfer(session, inet_addr(ip.c_str()), port, parameter);
}

template <typename T>
void fb::acceptor<T>::send_stream(fb::base& base, const fb::ostream& stream, bool encrypt, bool wrap, bool async)
{
    if(stream.empty())
        return;

    base.send(stream, encrypt, wrap, async);
}

template<class T>
void fb::acceptor<T>::send(fb::base& base, const fb::protocol::base::response& response, bool encrypt, bool wrap, bool async)
{
    fb::ostream                 out_stream;
    response.serialize(out_stream);
    if(out_stream.empty())
        return;

    base.send(out_stream, encrypt, wrap, async);
}
