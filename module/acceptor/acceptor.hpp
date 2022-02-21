#include "acceptor.h"

template <template<class> class S, class T>
fb::base::acceptor<S, T>::acceptor(boost::asio::io_context& context, uint16_t port, std::chrono::seconds delay, uint8_t num_threads) : 
    boost::asio::ip::tcp::acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    _context(context),
    _delay(delay),
    _threads(context, num_threads == 0xFF ? std::thread::hardware_concurrency() : num_threads),
    _exit(false)
{
    this->accept();
}

template <template<class> class S, class T>
fb::base::acceptor<S, T>::~acceptor()
{ }

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::handle_work(S<T>* socket, uint8_t id)
{
    if(this->handle_thread_index(*socket) != id)
        return;

    // 소켓 담당 스레드가 변경되는지를 감시하는 람다함수
    //  id                      현재 스레드의 id
    //  handle_thread_index     소켓이 처리되어야 할 스레드 id
    auto fn = [this, id] (S<T>& socket) 
    {
        return this->handle_thread_index(socket) == id; 
    };

    auto switched = false;

    if(id != 0xFF)
    {
        auto gd = std::lock_guard<std::mutex>(socket->mutex);
        switched = this->handle_parse(*socket, fn);
    }
    else
    {
        switched = this->handle_parse(*socket, fn);
    }

    if(switched == false)
        return;

    this->dispatch(socket, std::bind(&fb::base::acceptor<S, T>::handle_work, this, socket, std::placeholders::_1));
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::shutdown()
{
    // exiting
    fb::async::exit();
    this->threads().exit();

    // do
    this->handle_exit();

    // exited
    this->_context.stop();
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::accept()
{
    auto socket = std::make_unique<S<T>>(this->_context, std::bind(&fb::base::acceptor<S, T>::handle_receive, this, std::placeholders::_1), std::bind(&fb::base::acceptor<S, T>::handle_closed, this, std::placeholders::_1));
    auto ptr = socket.get();
    this->async_accept
    (
        *ptr,
        [this, socket = std::move(socket), ptr] (boost::system::error_code error) mutable
        {
            try
            {
                if(error)
                    throw std::runtime_error(error.message());

                if(this->_exit)
                    throw std::runtime_error("cannot accept socket. acceptor is cleaning now.");

                ptr->data(this->handle_accepted(*ptr));

                this->sockets.push(std::move(socket));
                this->handle_connected(*ptr);

                this->_threads.dispatch
                (
                    [ptr] () { ptr->recv(); },
                    this->_delay,
                    true
                );

                this->accept();
            }
            catch(std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }
        }
    );
}

template <template<class> class S, class T>
fb::threads& fb::base::acceptor<S, T>::threads()
{
    return this->_threads;
}

template <template<class> class S, class T>
const fb::threads& fb::base::acceptor<S, T>::threads() const
{
    return this->_threads;
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::handle_receive(fb::base::socket<T>& socket)
{
    auto                        casted = static_cast<S<T>*>(&socket);
    auto                        id     = this->handle_thread_index(*casted);

    if(id == 0xFF)
    {
        this->handle_work(casted, id);
    }
    else
    {
        this->dispatch(casted, std::bind(&fb::base::acceptor<S,T>::handle_work, this, casted, std::placeholders::_1));
    }
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::handle_closed(fb::base::socket<T>& socket)
{
    auto                        casted = static_cast<S<T>*>(&socket);
    auto                        id     = this->handle_thread_index(*casted);
    auto                        fn     = [this, casted] (uint8_t id)
    {
        this->handle_disconnected(*casted);

        boost::asio::dispatch
        (
            this->get_executor(),
            [this, casted] () 
            {
                this->sockets.erase(*casted);

                if(this->_exit && this->sockets.empty())
                    this->shutdown();
            }
        );
    };

    if(id == 0xFF)
    {
        fn(id);
    }
    else
    {
        this->_threads[id]->dispatch(fn, true);
    }
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::transfer(S<T>& socket, uint32_t ip, uint16_t port, fb::protocol::internal::services from)
{
    auto&                       crt = socket.crt();
    fb::ostream                 data;
    data.write_u8(crt.type())
        .write_u8(cryptor::KEY_SIZE)
        .write(crt.key(), cryptor::KEY_SIZE)
        .write_u8(from);

    fb::ostream                 out_stream;
    fb::protocol::response::transfer(ip, port, data).serialize(out_stream);

    crt.wrap(out_stream);
    socket.send(out_stream, false, false);
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::transfer(S<T>& socket, const std::string& ip, uint16_t port, fb::protocol::internal::services from)
{
    this->transfer(socket, inet_addr(ip.c_str()), port, from);
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::transfer(S<T>& socket, uint32_t ip, uint16_t port, fb::protocol::internal::services from, const fb::ostream& parameter)
{
    auto&                       crt = socket.crt();
    fb::ostream                 data;
    data.write_u8(crt.type())
        .write_u8(cryptor::KEY_SIZE)
        .write(crt.key(), cryptor::KEY_SIZE)
        .write_u8(from)
        .write(parameter.data(), parameter.size());

    fb::ostream                 out_stream;
    fb::protocol::response::transfer(ip, port, data).serialize(out_stream);

    crt.wrap(out_stream);
    socket.send(out_stream, false, false);
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::transfer(S<T>& socket, const std::string& ip, uint16_t port, fb::protocol::internal::services from, const fb::ostream& parameter)
{
    this->transfer(socket, inet_addr(ip.c_str()), port, from, parameter);
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

template <template<class> class S, class T>
bool fb::base::acceptor<S, T>::precedence(S<T>* socket, fb::queue_callback&& fn)
{
    auto id = this->handle_thread_index(*socket);
    auto thread = this->_threads[id];

    if(thread == nullptr)
        fn(id);
    else
        this->_threads[id]->dispatch(std::move(fn), true);
    return true;
}

template <template<class> class S, class T>
bool fb::base::acceptor<S, T>::dispatch(S<T>* socket, fb::queue_callback&& fn)
{
    auto id = this->handle_thread_index(*socket);
    auto thread = this->_threads[id];
    
    if(thread == nullptr)
        fn(id);
    else
        this->_threads[id]->dispatch(std::move(fn));
    return true;
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::exit()
{
    this->_exit = true;

    for(auto& pair : this->sockets)
        pair.second.get()->close();

    this->shutdown();
}

template <template<class> class S, class T>
inline fb::base::acceptor<S, T>::operator boost::asio::io_context& () const
{
    return this->_context;
}




template <typename T>
fb::acceptor<T>::acceptor(boost::asio::io_context& context, uint16_t port, std::chrono::seconds delay, const INTERNAL_CONNECTION& internal_connection, uint8_t num_threads) : 
    fb::base::acceptor<fb::socket, T>(context, port, delay, num_threads),
    _internal_connection(internal_connection)
{
    this->connect_internal();
}

template <typename T>
fb::acceptor<T>::~acceptor()
{
    this->exit();
}

template <typename T>
bool fb::acceptor<T>::call(fb::socket<T>& socket, uint8_t cmd)
{
    auto i = this->_public_handler_dict.find(cmd);
    if(i == this->_public_handler_dict.end())
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
    this->_internal.reset(new fb::internal::socket<>
        (
            this->_context,
            [&] (fb::base::socket<>& socket)
            {
                static constexpr uint8_t base_size = sizeof(uint16_t);
                auto& in_stream = socket.in_stream();

                while(true)
                {
                    try
                    {
                        if(in_stream.readable_size() < base_size)
                            throw std::exception();

                        auto size = in_stream.read_u16();
                        if(size > in_stream.capacity())
                            throw std::exception();

                        if(in_stream.readable_size() < size)
                            throw std::exception();

                        auto cmd = in_stream.read_8();
                        auto found = this->_private_handler_dict.find(cmd);
                        if(found == this->_private_handler_dict.end())
                            throw std::exception();

                        found->second(static_cast<fb::internal::socket<>&>(socket));

                        in_stream.reset();
                        in_stream.shift(base_size + size);
                        in_stream.flush();
                    }
                    catch(...)
                    {
                        break;
                    }
                }

                in_stream.reset();
                return false;
            },

            [&] (fb::base::socket<>& socket)
            {
                this->_internal_connection.handle_disconnected();
                std::this_thread::sleep_for(1000ms);
                this->connect_internal();
            }
        )
    );
    
    auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(this->_internal_connection.ip), this->_internal_connection.port);
    this->_internal->async_connect
    (
        endpoint,
        [&] (const auto& error)
        {
            if(error)
            {
                this->_internal_connection.handle_connected(*this->_internal, false);
                std::this_thread::sleep_for(1000ms);
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
bool fb::acceptor<T>::handle_parse(fb::socket<T>& socket, const std::function<bool(fb::socket<T>&)>& fn)
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

            // 콜백 조건이 만족하지 못하는 경우 즉시 종료
            if(fn(socket) == false)
                return true;
        }
        catch(std::exception&)
        {
            in_stream.clear();
            break;
        }
        catch(...)
        {
            in_stream.clear();
            break;
        }
    }

    in_stream.reset();
    return false;
}

template <typename T>
template <typename R>
void fb::acceptor<T>::bind(uint8_t cmd, const std::function<bool(fb::socket<T>&, R&)>& fn)
{
    this->_public_handler_dict[cmd] = [this, fn](fb::socket<T>& socket)
    {
        auto& in_stream = socket.in_stream();
        R     header;
        header.deserialize(in_stream);
        return fn(socket, header);
    };
}

template <typename T>
template <typename R>
void fb::acceptor<T>::bind(const std::function<bool(fb::internal::socket<>&, R&)>& fn)
{
    this->_private_handler_dict[R::id] = [this, fn](fb::internal::socket<>& socket)
    {
        auto& in_stream = socket.in_stream();
        R     header;
        header.deserialize(in_stream);
        return fn(socket, header);
    };
}