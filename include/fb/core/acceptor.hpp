#include "acceptor.h"

template <template<class> class S, class T>
fb::base::acceptor<S, T>::acceptor(boost::asio::io_context& context, uint16_t port, uint8_t num_threads) : 
    icontext(context, port),
    _context(context),
    _threads(context, num_threads),
    _redis(*this, fb::config::get()["redis"]["default"]["ip"].asString(), fb::config::get()["redis"]["default"]["port"].asUInt()),
    _mutex(*this)
{
    this->accept();
}

template <template<class> class S, class T>
fb::base::acceptor<S, T>::~acceptor()
{
    this->exit();
}

template <template<class> class S, class T>
fb::task<void> fb::base::acceptor<S, T>::handle_work(S<T>* socket)
{
    if (this->_running == false)
        co_return;

    auto switched = co_await this->handle_parse(*socket);
    if (switched == false)
        co_return;

    co_await this->dispatch(socket, std::bind(&fb::base::acceptor<S, T>::handle_work, this, socket));
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

                if(this->_running == false)
                    throw std::runtime_error("cannot accept socket. acceptor is cleaning now.");

                ptr->data(this->handle_accepted(*ptr));

                this->sockets.push(std::move(socket));
                this->handle_connected(*ptr);
                ptr->recv();
                this->accept();
            }
            catch(std::exception& /*e*/)
            {
                // std::cout << e.what() << std::endl;
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
uint8_t fb::base::acceptor<S, T>::handle_thread_index(S<T>& socket) const
{
    auto count = this->_threads.count();
    if(count == 0)
        return 0xFF;

    auto fd = uint32_t(socket.fd());
    return uint8_t(fd % count);
}

template <template<class> class S, class T>
fb::task<void> fb::base::acceptor<S, T>::handle_receive(fb::base::socket<T>& socket)
{
    if (this->_running == false)
        throw std::runtime_error("acceptor closed");

    auto                        casted = static_cast<S<T>*>(&socket);
    auto                        id     = this->handle_thread_index(*casted);

    if(id == 0xFF)
    {
        co_await this->handle_work(casted);
    }
    else
    {
        co_await this->dispatch(casted, std::bind(&fb::base::acceptor<S, T>::handle_work, this, casted));
    }
}

template <template<class> class S, class T>
fb::task<void> fb::base::acceptor<S, T>::handle_closed(fb::base::socket<T>& socket)
{
    if(socket.data() == nullptr)
        co_return;

    auto                        casted = static_cast<S<T>*>(&socket);
    auto                        id     = this->handle_thread_index(*casted);
    auto                        fn     = [this, casted] () -> fb::task<void>
    {
        this->handle_disconnected(*casted);
        this->sockets.erase(*casted);
        co_return;
    };

    if(id == 0xFF)
    {
        co_await fn();
    }
    else
    {
        co_await this->_threads[id]->dispatch(fn);
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
template <typename R>
fb::task<R> fb::base::acceptor<S, T>::request(const fb::protocol::internal::header& header, bool encrypt, bool wrap)
{
    auto thread = this->current_thread();
    auto error = boost::system::error_code();
    try
    {
        auto& response = co_await this->_internal->request<R>(header, encrypt, wrap);
        if (this->_running == false)
            throw std::runtime_error("server exited");

        if (thread != nullptr)
            co_await thread->dispatch();

        co_return response;
    }
    catch (const boost::system::error_code& ec)
    {
        error = ec;
    }

    if (thread != nullptr)
        co_await thread->dispatch();
    
    throw boost::system::error_code(error);
}

template <template<class> class S, class T>
fb::thread* fb::base::acceptor<S, T>::current_thread()
{
    return this->_threads.current();
}

template <template<class> class S, class T>
fb::awaiter<void> fb::base::acceptor<S, T>::dispatch(S<T>* socket, const std::function<fb::task<void>(void)>& fn, uint32_t priority)
{
    auto id = this->handle_thread_index(*socket);
    auto thread = this->_threads[id];
    
    if(thread == nullptr)
        throw std::runtime_error("thread does not exists");

    return this->_threads[id]->dispatch(fn, 0s, priority);
}

template <template<class> class S, class T>
fb::awaiter<void> fb::base::acceptor<S, T>::dispatch(S<T>* socket, uint32_t priority)
{
    auto id = this->handle_thread_index(*socket);
    auto thread = this->_threads[id];
    
    if(thread == nullptr)
        throw std::runtime_error("dispatch thread cannot be nullptr");

    return this->_threads[id]->dispatch(priority);
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::run(int thread_size)
{
    if(this->_boost_threads != nullptr)
        return;

    this->_running = true;
    this->_boost_threads = std::make_unique<boost::asio::thread_pool>(thread_size);
    for(int i = 0; i < thread_size; i++)
    {
        post(*this->_boost_threads, [this]
        {
            this->_context.run();
        });
    }

    this->handle_start().wait();
}

template <template<class> class S, class T>
bool fb::base::acceptor<S, T>::running() const
{
    return this->_running;
}

template <template<class> class S, class T>
fb::task<void> fb::base::acceptor<S, T>::sleep(const std::chrono::steady_clock::duration& duration)
{
    auto thread = this->_threads.current();
    if (thread == nullptr)
        throw std::runtime_error("thread not exists");

    co_await thread->sleep(duration);
}

template <template<class> class S, class T>
void fb::base::acceptor<S, T>::exit()
{
    auto _ = std::lock_guard(this->_mutex_exit);

    if (this->_running == false)
        return;

    this->_threads.exit();

    this->handle_exit();
    this->cancel();
    this->sockets.close();
    this->_running = false;
    // if (this->_internal != nullptr)
    //     this->_internal->close();
    // this->_threads.exit();
}

template <template<class> class S, class T>
inline fb::base::acceptor<S, T>::operator boost::asio::io_context& () const
{
    return this->_context;
}




template <typename T>
fb::acceptor<T>::acceptor(boost::asio::io_context& context, uint16_t port, uint8_t num_threads) : 
    fb::base::acceptor<fb::socket, T>(context, port, num_threads)
{
    
}

template <typename T>
fb::acceptor<T>::~acceptor()
{ }

template <typename T>
fb::task<bool> fb::acceptor<T>::call(fb::socket<T>& socket, uint8_t cmd)
{
    auto i = this->_external_handler.find(cmd);
    if(i == this->_external_handler.end())
    {
        auto& c = fb::console::get();
        fb::logger::warn("정의되지 않은 요청입니다. [0x%2X]", cmd);
        return this->default_handler();
    }

    return i->second(socket);
}

template <typename T>
fb::task<void> fb::acceptor<T>::handle_internal_receive(fb::base::socket<>& socket)
{
    static constexpr uint8_t base_size = sizeof(uint16_t);
    co_return co_await socket.in_stream<fb::task<void>>([this, &socket](auto& in_stream) -> fb::task<void>
    {
        while(true)
        {
            try
            {
                socket.flush_task();

                if(in_stream.readable_size() < base_size)
                    throw std::exception();

                auto size = in_stream.read_u16();
                if(size > in_stream.capacity())
                    throw std::exception();

                if(in_stream.readable_size() < size)
                    throw std::exception();

                auto cmd = in_stream.read_8();
                if (this->_internal_handler.contains(cmd))
                {
                    auto task = this->_internal_handler[cmd](static_cast<fb::internal::socket<>&>(socket));
                    if (!task.done())
                        socket.push_task(std::move(task));
                }

                in_stream.reset();
                in_stream.shift(base_size + size);
                in_stream.flush();
            }
            catch(std::exception& /*e*/)
            {
                break;
            }
            catch(...)
            {
                break;
            }
        }

        in_stream.reset();
        co_return;
    });
}

template <typename T>
fb::task<void> fb::acceptor<T>::on_internal_connected()
{
    co_await this->handle_internal_connected();
}

template <typename T>
fb::task<void> fb::acceptor<T>::handle_internal_connected()
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream sstream;
    sstream << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    fb::logger::info("Success connect to internal server. (%s)", sstream.str().c_str());
    co_return;
}

template <typename T>
fb::task<void> fb::acceptor<T>::handle_internal_denied(const std::string& error)
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream sstream;
    sstream << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    fb::logger::warn("Failed connect to internal server. (%s)", sstream.str().c_str());
    co_return;
}

template <typename T>
fb::task<void> fb::acceptor<T>::on_internal_disconnected(fb::base::socket<>& socket)
{
    this->_internal->close();
    co_await this->handle_internal_disconnected(socket);
    
    if (this->running())
        co_await this->connect_internal();
}

template <typename T>
fb::task<void> fb::acceptor<T>::handle_internal_disconnected(fb::base::socket<>& socket)
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream sstream;
    sstream << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    fb::logger::warn("Disconnected from internal server. (%s)", sstream.str().c_str());
    co_return;
}

template <typename T>
fb::awaiter<void> fb::acceptor<T>::co_connect_internal(const std::string& ip, uint16_t port)
{
    auto await_callback = [this, ip, port](auto& awaiter)
    {
        auto handle_receive = std::bind(&fb::acceptor<T>::handle_internal_receive, this, std::placeholders::_1);
        auto handle_disconnected = std::bind(&fb::acceptor<T>::on_internal_disconnected, this, std::placeholders::_1);
        auto socket = new fb::internal::socket<>(this->_context, handle_receive, handle_disconnected);
        this->_internal.reset(socket);

        auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), port);
        this->_internal->async_connect
        (
            endpoint,
            [this, &awaiter] (const auto& error)
            {
                if (error)
                    awaiter.error(boost::system::error_code(error));
                else
                    this->_internal->recv();

                awaiter.resume();
            }
        );
    };

    return fb::awaiter<void>(await_callback);
}

template <typename T>
fb::task<bool> fb::acceptor<T>::default_handler()
{
    co_return true;
}

template <typename T>
fb::task<void> fb::acceptor<T>::connect_internal()
{
    auto&           config = fb::config::get();
    auto            ip     = config["internal"]["ip"].asString();
    auto            port   = (uint16_t)config["internal"]["port"].asInt();
    auto            error  = std::string();
    while(this->running())
    {
        try
        {
            co_await this->co_connect_internal(ip, port);
            //co_await this->on_internal_connected();
            puts("internal conn success");
            co_return;
        }
        catch (boost::system::error_code& e)
        {
            if (!this->running())
                co_return;

            error = e.message();
        }
        catch(std::exception& e)
        {
            if (!this->running())
                co_return;

            error = e.what();
        }
        catch (...)
        {
            if (!this->running())
                co_return;

            error = "unhandled exception";
        }

        co_await this->handle_internal_denied(error);
        std::this_thread::sleep_for(1000ms);
    }
}

template <typename T>
bool fb::acceptor<T>::decrypt_policy(uint8_t cmd) const
{
    return true;
}

template <typename T>
fb::task<void> fb::acceptor<T>::handle_start()
{
    co_await this->connect_internal();
}

template <typename T>
fb::task<bool> fb::acceptor<T>::handle_parse(fb::socket<T>& socket)
{
    static constexpr uint8_t    base_size     = sizeof(uint8_t) + sizeof(uint16_t);

    co_return co_await socket.in_stream<fb::task<bool>>([this, &socket](auto& in_stream) -> fb::task<bool>
    {
        auto& crt = socket.crt();
        while(true)
        {
            try
            {
                socket.flush_task();

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


                auto                cmd = in_stream.read_u8();
                if (this->decrypt_policy(cmd))
                    size = crt.decrypt(in_stream, in_stream.offset() - 1, size);

                // Call function that matched by command byte
                auto before = this->handle_thread_index(socket);
                auto result = co_await this->call(socket, cmd);
                auto after = this->handle_thread_index(socket);

                // Set data pointer to process next packet bytes
                in_stream.reset();
                in_stream.shift(base_size + size);
                in_stream.flush();

                // 콜백 조건이 만족하지 못하는 경우 즉시 종료
                co_return before != after;
            }
            catch(std::exception& e)
            {
                fb::logger::fatal(e.what());
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
        co_return false;
    });
}

template <typename T>
template <typename R>
void fb::acceptor<T>::bind(int cmd, const std::function<fb::task<bool>(fb::socket<T>&, const R&)>& fn)
{
    this->_external_handler.insert({cmd, [this, fn](fb::socket<T>& socket)
    {
        return socket.in_stream<fb::task<bool>>([this, &fn, &socket] (auto& in_stream)
        {
            R     header;
            header.deserialize(in_stream);
            return fn(socket, header);
        });
    }});
}

template <typename T>
template <typename R>
void fb::acceptor<T>::bind(const std::function<fb::task<bool>(fb::socket<T>&, const R&)>& fn)
{
    auto id = R().__id;
    this->bind(id, fn);
}

template <typename T>
template <typename R>
void fb::acceptor<T>::bind(const std::function<fb::task<bool>(fb::internal::socket<>&, const R&)>& fn)
{
    auto id = R().__id;
    this->_internal_handler.insert({id, [this, fn](fb::internal::socket<>& socket)
    {
        return socket.in_stream<fb::task<bool>>([this, &fn, &socket](auto& in_stream)
        {
            R     header;
            header.deserialize(in_stream);
            socket.invoke_awaiter(header.trans, header);
            return fn(socket, header);
        });
    }});
}

template <typename T>
template <typename R>
void fb::acceptor<T>::bind()
{
    this->bind<R>([this] (fb::internal::socket<>&, const R&) -> fb::task<bool>
    {
        co_return true;
    });
}