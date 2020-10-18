#include "acceptor.h"

template <template<class> class S, class T>
fb::base::acceptor<S, T>::acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay) : 
    boost::asio::ip::tcp::acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    _context(context),
    _accept_delay(accept_delay),
    _internal(nullptr)
{
    this->accept();
}

template <template<class> class S, class T>
fb::base::acceptor<S, T>::~acceptor()
{
    for(auto x : this->sockets)
    {
        if(x->data() != nullptr)
            delete x->data();
        delete x;
    }

    if(this->_internal == nullptr)
        delete this->_internal;
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::accept()
{
    auto socket = new S<T>(this->_context, std::bind(&fb::base::acceptor<S, T>::handle_receive, this, std::placeholders::_1), std::bind(&fb::base::acceptor<S, T>::handle_closed, this, std::placeholders::_1));
    this->async_accept
    (
        *socket,
        [this, socket](boost::system::error_code error)
        {
            try
            {
                if(error)
                    throw std::runtime_error(error.message());

                socket->data(this->handle_accepted(*socket));
                this->sockets.push(*socket);

                this->accept();
                this->handle_connected(*socket);
                auto timer = new boost::asio::steady_timer(this->_context, std::chrono::seconds(this->_accept_delay));
                timer->async_wait(boost::bind(&acceptor<S, T>::handle_accept_timer, this, boost::asio::placeholders::error, socket, timer));
            }
            catch(std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }
        }
    );
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::handle_accept_timer(const boost::system::error_code& e, S<T>* socket, boost::asio::steady_timer* timer)
{
    socket->recv();
    delete timer;
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::handle_receive(fb::base::socket<T>& socket)
{
    auto& casted = static_cast<S<T>&>(socket);
    this->handle_parse(casted);
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::handle_closed(fb::base::socket<T>& socket)
{
    auto& casted = static_cast<S<T>&>(socket);
    this->handle_disconnected(casted);
    this->sockets.erase(casted);

    delete &casted;
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::transfer(S<T>& socket, uint32_t ip, uint16_t port)
{
    auto&                       crt = socket.crt();
    fb::ostream                 data;
    data.write_u8(crt.type())
        .write_u8(cryptor::KEY_SIZE)
        .write(crt.key(), cryptor::KEY_SIZE);

    fb::ostream                 out_stream;
    fb::protocol::response::transfer(ip, port, data).serialize(out_stream);

    crt.wrap(out_stream);
    socket.send(out_stream, false, false);
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::transfer(S<T>& socket, const std::string& ip, uint16_t port)
{
    this->transfer(socket, inet_addr(ip.c_str()), port);
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::transfer(S<T>& socket, uint32_t ip, uint16_t port, const fb::ostream& parameter)
{
    auto&                       crt = socket.crt();
    fb::ostream                 data;
    data.write_u8(crt.type())
        .write_u8(cryptor::KEY_SIZE)
        .write(crt.key(), cryptor::KEY_SIZE)
        .write(parameter.data(), parameter.size());

    fb::ostream                 out_stream;
    fb::protocol::response::transfer(ip, port, data).serialize(out_stream);

    crt.wrap(out_stream);
    socket.send(out_stream, false, false);
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::transfer(S<T>& socket, const std::string& ip, uint16_t port, const fb::ostream& parameter)
{
    this->transfer(socket, inet_addr(ip.c_str()), port, parameter);
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::send_stream(S<T>& socket, const fb::ostream& stream, bool encrypt, bool wrap)
{
    if(stream.empty())
        return;

    socket.send(stream, encrypt, wrap);
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::send(S<T>& socket, const fb::protocol::base::header& response, bool encrypt, bool wrap)
{
    fb::ostream                 out_stream;
    response.serialize(out_stream);
    if(out_stream.empty())
        return;

    socket.send(out_stream, encrypt, wrap);
}




template <typename T>
fb::acceptor<T>::acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay, const INTERNAL_CONNECTION& internal_connection) : 
    fb::base::acceptor<fb::socket, T>(context, port, accept_delay),
    _internal_connection(internal_connection)
{
    this->connect_internal();
}

template <typename T>
fb::acceptor<T>::~acceptor()
{}

template <typename T>
bool fb::acceptor<T>::call(fb::socket<T>& socket, uint8_t cmd)
{
    auto i = this->_handler_dict.find(cmd);
    if(i == this->_handler_dict.end())
    {
        auto& c = fb::console::get();
        c.puts("정의되지 않은 요청입니다. [0x%2X]", cmd);
        return true; 
    }

    return i->second(socket);
}

template <typename T>
void fb::acceptor<T>::connect_internal()
{
    if(this->_internal != nullptr)
        delete this->_internal;

    this->_internal = new fb::internal::socket<>
        (
            this->_context,
            [&] (fb::base::socket<>& socket)
            {
                puts("internal received");
            },

            [&] (fb::base::socket<>& socket)
            {
                this->_internal_connection.handle_disconnected();
                this->connect_internal();
            }
        );
    
    auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(this->_internal_connection.ip), this->_internal_connection.port);
    this->_internal->async_connect
    (
        endpoint,
        [&] (const boost::system::error_code& error)
        {
            if(error)
            {
                this->_internal_connection.handle_connected(*this->_internal, false);
                this->connect_internal();
            }
            else
            {
                this->_internal_connection.handle_connected(*this->_internal, true);
                this->_internal->recv();
            }
        }
    );
}

template <typename T>
void fb::acceptor<T>::handle_parse(fb::socket<T>& socket)
{
    static constexpr uint8_t    not_crt_cmd[] = {0x00, 0x10};
    static constexpr uint8_t    base_size     = sizeof(uint8_t) + sizeof(uint16_t);
    auto&                       in_stream     = socket.in_stream();
    auto&                       crt           = socket.crt();

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


            // If command byte is not 0x10, this data must be decrypted by socket's encrypt key
            auto                cmd = in_stream.read_u8();
            if(std::all_of(not_crt_cmd, not_crt_cmd + sizeof(not_crt_cmd)/sizeof(uint8_t), [cmd] (uint8_t x) { return cmd != x; }))
                size = crt.decrypt(in_stream, in_stream.offset() - 1, size);

            // Call function that matched by command byte
            if(this->call(socket, cmd) == false)
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
template <typename R>
void fb::acceptor<T>::bind(uint8_t cmd, std::function<bool(fb::socket<T>&, R&)> fn)
{
    this->_handler_dict[cmd] = [this, fn](fb::socket<T>& socket)
    {
        auto& in_stream = socket.in_stream();
        R     header;
        header.deserialize(in_stream);
        return fn(socket, header);
    };
}