template <typename T>
fb::acceptor<T>::acceptor(boost::asio::io_context& context, uint16_t port) :
    icontext(context, port),
    _context(context),
    _threads(context),
    _redis(*this, fb::config::get()["redis"]["default"]["ip"].asString(), fb::config::get()["redis"]["default"]["port"].asUInt()),
    _mutex(*this)
{
    this->accept();

    this->bind_timer(
        [this]() -> async::task<void> {
            auto&  config   = fb::config::get();
            auto&& response = co_await this->post<fb::protocol::internal::request::Ping, fb::protocol::internal::response::Pong>(
                "internal",
                "/in-game/ping",
                fb::protocol::internal::request::Ping{this->id(), this->name(), this->service(), config["ip"].asString(), (uint16_t)config["port"].asUInt()});
        },
        1s);
}

template <typename T>
fb::acceptor<T>::~acceptor()
{
    this->exit();
}

template <typename T>
async::task<void> fb::acceptor<T>::handle_work(fb::socket<T>* socket)
{
    if (this->_running == false)
        co_return;

    auto switched = co_await this->handle_parse(*socket);
    if (switched == false)
        co_return;

    co_await this->dispatch(socket, std::bind(&fb::acceptor<T>::handle_work, this, socket));
}

template <typename T>
void fb::acceptor<T>::handle_background()
{
    while (this->_running)
    {
        background_func func;
        {
            auto _ = std::lock_guard(this->_background_queue_mutex);
            if (this->_background_queue.size() > 0)
            {
                func = this->_background_queue.front();
                this->_background_queue.pop();
            }
        }

        if (func)
            async::awaitable_get(func());
        else
            std::this_thread::sleep_for(100ms);
    }
}

template <typename T>
void fb::acceptor<T>::accept()
{
    auto socket = std::make_unique<fb::socket<T>>(
        this->_context, std::bind(&fb::acceptor<T>::handle_receive, this, std::placeholders::_1), std::bind(&fb::acceptor<T>::handle_closed, this, std::placeholders::_1));
    auto ptr = socket.get();
    this->async_accept(*ptr, [this, socket = std::move(socket), ptr](boost::system::error_code error) mutable {
        try
        {
            if (error)
                throw std::runtime_error(error.message());

            if (this->_running == false)
                throw std::runtime_error("cannot accept socket. acceptor is cleaning now.");

            ptr->data(this->handle_accepted(*ptr));

            this->sockets.push(std::move(socket));
            this->handle_connected(*ptr);

            boost::asio::co_spawn(*this, ptr->recv(), boost::asio::detached);
            this->accept();
        }
        catch (std::exception& /*e*/)
        {
            // std::cout << e.what() << std::endl;
        }
    });
}

template <typename T>
fb::threads& fb::acceptor<T>::threads()
{
    return this->_threads;
}

template <typename T>
const fb::threads& fb::acceptor<T>::threads() const
{
    return this->_threads;
}

template <typename T>
uint8_t fb::acceptor<T>::handle_thread_index(fb::socket<T>& socket) const
{
    auto count = this->_threads.count();
    if (count == 0)
        return 0xFF;

    auto fd = uint32_t(socket.fd());
    return uint8_t(fd % count);
}

template <typename T>
async::task<void> fb::acceptor<T>::handle_receive(fb::socket<T>& socket)
{
    if (this->_running == false)
        throw std::runtime_error("acceptor closed");

    auto casted = static_cast<fb::socket<T>*>(&socket);
    auto id     = this->handle_thread_index(*casted);

    if (id == 0xFF)
    {
        co_await this->handle_work(casted);
    }
    else
    {
        co_await this->dispatch(casted, std::bind(&fb::acceptor<T>::handle_work, this, casted));
    }
}

template <typename T>
async::task<void> fb::acceptor<T>::handle_closed(fb::socket<T>& socket)
{
    if (socket.data() == nullptr)
        co_return;

    auto casted = static_cast<fb::socket<T>*>(&socket);
    auto id     = this->handle_thread_index(*casted);
    if (id != 0xFF)
        co_await this->_threads[id]->dispatch();

    co_await this->handle_disconnected(*casted);
    this->sockets.erase(*casted);
}

template <typename T>
void fb::acceptor<T>::transfer(fb::socket<T>& socket, uint32_t ip, uint16_t port, fb::protocol::internal::services from)
{
    auto&       crt = socket.crt();
    fb::ostream data;
    data.write_u8(crt.type()).write_u8(cryptor::KEY_SIZE).write(crt.key(), cryptor::KEY_SIZE).write_u8(from);

    fb::ostream out_stream;
    fb::protocol::response::transfer(ip, port, data).serialize(out_stream);

    crt.wrap(out_stream);
    socket.send(out_stream, false, false);
}

template <typename T>
void fb::acceptor<T>::transfer(fb::socket<T>& socket, const std::string& ip, uint16_t port, fb::protocol::internal::services from)
{
    this->transfer(socket, inet_addr(ip.c_str()), port, from);
}

template <typename T>
void fb::acceptor<T>::transfer(fb::socket<T>& socket, uint32_t ip, uint16_t port, fb::protocol::internal::services from, const fb::ostream& parameter)
{
    auto&       crt = socket.crt();
    fb::ostream data;
    data.write_u8(crt.type()).write_u8(cryptor::KEY_SIZE).write(crt.key(), cryptor::KEY_SIZE).write_u8(from).write(parameter.data(), parameter.size());

    fb::ostream out_stream;
    fb::protocol::response::transfer(ip, port, data).serialize(out_stream);

    crt.wrap(out_stream);
    socket.send(out_stream, false, false);
}

template <typename T>
void fb::acceptor<T>::transfer(fb::socket<T>& socket, const std::string& ip, uint16_t port, fb::protocol::internal::services from, const fb::ostream& parameter)
{
    this->transfer(socket, inet_addr(ip.c_str()), port, from, parameter);
}

template <typename T>
void fb::acceptor<T>::send_stream(fb::socket<T>& socket, const fb::ostream& stream, bool encrypt, bool wrap)
{
    if (stream.empty())
        return;

    socket.send(stream, encrypt, wrap);
}

template <typename T>
void fb::acceptor<T>::send(fb::socket<T>& socket, const fb::protocol::base::header& response, bool encrypt, bool wrap)
{
    fb::ostream out_stream;
    response.serialize(out_stream);
    if (out_stream.empty())
        return;

    socket.send(out_stream, encrypt, wrap);
}

// template <typename T>
// template <typename Response>
// async::task<Response> fb::acceptor<T>::get(const std::string& host, const std::string& path, httplib::Headers headers)
// {
//     auto thread = this->current_thread();
//     if (thread != nullptr)
//         co_await thread->dispatch();
// }

template <typename T>
template <typename R>
async::task<R> fb::acceptor<T>::background(const std::function<async::task<R>()>& func)
{
    auto _       = std::lock_guard(this->_background_queue_mutex);
    auto promise = std::make_shared<async::task_completion_source<R>>();
    auto thread  = this->current_thread();
    this->_background_queue.push([this, promise, thread, func]() -> async::task<void> {
        try
        {
            if constexpr (std::is_void_v<R>)
            {
                co_await func();
                if (thread != nullptr)
                    co_await thread->dispatch();
                promise->set_value();
            }
            else
            {
                R result = co_await func();
                if (thread != nullptr)
                    co_await thread->dispatch();
                promise->set_value(std::move(result));
            }
        }
        catch (std::exception& e)
        {
            promise->set_exception(std::make_exception_ptr(e));
        }
    });

    return promise->task();
}

template <typename T>
async::task<httplib::Result> fb::acceptor<T>::get_internal(const std::string& host, const std::string& path, httplib::Headers headers)
{
    headers.insert({"Content-Type", "application/octet-stream"});

    co_return co_await this->background<httplib::Result>([=, this]() -> async::task<httplib::Result> {
        auto client = httplib::Client(host);
        co_return client.Get(UTF8(path, PLATFORM::Windows), headers);
    });
}

template <typename T>
async::task<httplib::Result> fb::acceptor<T>::post_internal(const std::string& host, const std::string& path, httplib::Headers headers, const void* bytes, size_t size)
{
    auto promise = std::make_shared<async::task_completion_source<httplib::Result>>();
    auto buffer  = std::vector<uint8_t>(size);
    std::memcpy(buffer.data(), bytes, size);
    co_return co_await this->background<httplib::Result>([=, this]() -> async::task<httplib::Result> {
        auto client = httplib::Client(host);
        co_return client.Post(UTF8(path, PLATFORM::Windows), headers, (const char*)buffer.data(), buffer.size(), "application/octet-stream");
    });
}

template <typename T>
template <typename Response>
async::task<Response> fb::acceptor<T>::get_internal(const std::string& host, const std::string& path /* , httplib::Headers headers,  */)
{
    httplib::Headers headers;
    auto&&           res = co_await this->get_internal(host, path, headers);
    if (!res)
        throw std::runtime_error("http error");

    if (res->status != 200)
        throw std::runtime_error("http error");

    auto ptr           = (const uint8_t*)res->body.c_str();
    auto size          = std::stoi(res->get_header_value("Content-Length"));
    auto in_stream     = fb::istream(ptr, size);

    auto protocol_type = in_stream.read_u32();
    auto protocol_size = in_stream.read_u32();
    co_return Response::Deserialize(ptr + sizeof(uint32_t) + sizeof(uint32_t));
}

template <typename T>
template <typename Request, typename Response>
async::task<Response> fb::acceptor<T>::post_internal(const std::string& host, const std::string& path, /* httplib::Headers headers,  */ const Request& body)
{
    httplib::Headers headers;
    auto             serialized = body.Serialize();

    auto             out_stream = fb::ostream();
    out_stream.write_u32(Request::FlatBufferProtocolType);
    out_stream.write_u32(serialized.size());
    out_stream.write((const void*)serialized.data(), serialized.size());

    auto&& res = co_await this->post_internal(host, path, headers, out_stream.data(), out_stream.size());
    if (!res)
        throw std::runtime_error("http error");

    if (res->status != 200)
        throw std::runtime_error("http error");

    auto ptr           = (const uint8_t*)res->body.c_str();
    auto size          = std::stoi(res->get_header_value("Content-Length"));
    auto in_stream     = fb::istream(ptr, size);

    auto protocol_type = in_stream.read_u32();
    auto protocol_size = in_stream.read_u32();
    co_return Response::Deserialize(ptr + sizeof(uint32_t) + sizeof(uint32_t));
}

template <typename T>
template <typename Response>
async::task<Response> fb::acceptor<T>::get(const std::string& route, const std::string& path)
{
    auto& config = fb::config::get();
    auto  host   = std::format("http://{}:{}", config[route]["ip"].asCString(), config[route]["port"].asUInt());
    co_return co_await this->get_internal<Response>(host, path);
}

template <typename T>
template <typename Request, typename Response>
async::task<Response> fb::acceptor<T>::post(const std::string& route, const std::string& path, const Request& body)
{
    auto& config = fb::config::get();
    auto  host   = std::format("http://{}:{}", config[route]["ip"].asCString(), config[route]["port"].asUInt());
    co_return co_await this->post_internal<Request, Response>(host, path, body);
}

template <typename T>
fb::thread* fb::acceptor<T>::current_thread()
{
    return this->_threads.current();
}

template <typename T>
async::task<void> fb::acceptor<T>::dispatch(fb::socket<T>* socket, const std::function<async::task<void>(void)>& fn, uint32_t priority)
{
    auto id     = this->handle_thread_index(*socket);
    auto thread = this->_threads[id];

    if (thread == nullptr)
        throw std::runtime_error("thread does not exists");

    co_await this->_threads[id]->dispatch(fn, 0s, priority);
}

template <typename T>
async::task<void> fb::acceptor<T>::dispatch(fb::socket<T>* socket, uint32_t priority)
{
    auto id     = this->handle_thread_index(*socket);
    auto thread = this->_threads[id];

    if (thread == nullptr)
        throw std::runtime_error("dispatch thread cannot be nullptr");

    co_await this->_threads[id]->dispatch(priority);
}

template <typename T>
void fb::acceptor<T>::run()
{
    auto& config   = fb::config::get();
    auto  threads  = std::vector<std::thread>();

    this->_running = true;
    for (int i = 0; i < config["thread"]["io"].asUInt(); i++)
    {
        threads.push_back(std::thread([this]() {
            this->_context.run();
        }));
    }

    for (int i = 0; i < config["thread"]["background"].asUInt(); i++)
    {
        threads.push_back(std::thread([this]() {
            this->handle_background();
        }));
    }

    async::awaitable_get(this->handle_start());
    for (auto& thread : threads)
    {
        thread.join();
    }
}

template <typename T>
bool fb::acceptor<T>::running() const
{
    return this->_running;
}

template <typename T>
async::task<void> fb::acceptor<T>::sleep(const fb::model::timespan& duration)
{
    auto thread = this->_threads.current();
    if (thread == nullptr)
        throw std::runtime_error("thread not exists");

    co_await thread->sleep(duration);
}

template <typename T>
void fb::acceptor<T>::exit()
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

template <typename T>
inline fb::acceptor<T>::operator boost::asio::io_context& () const
{
    return this->_context;
}

template <typename T>
bool fb::acceptor<T>::decrypt_policy(uint8_t cmd) const
{
    return true;
}

template <typename T>
async::task<bool> fb::acceptor<T>::handle_parse(fb::socket<T>& socket)
{
    static constexpr uint8_t base_size = sizeof(uint8_t) + sizeof(uint16_t);

    co_return co_await socket.template in_stream<async::task<bool>>([this, &socket](auto& in_stream) -> async::task<bool> {
        auto& crt = socket.crt();
        while (true)
        {
            try
            {
                if (in_stream.readable_size() < base_size)
                    break;

                // Read base head and check it is 0xAA
                auto head = in_stream.read_u8();
                if (head != 0xAA)
                    throw std::exception();

                // Read data size and check it is greater than buffer size
                auto size = in_stream.read_u16(buffer::endian::BIG);
                if (size > in_stream.capacity())
                    throw std::exception();

                // If data size is not enough to parse, do not anymore
                if (in_stream.readable_size() < size)
                    break;

                auto cmd = in_stream.read_u8();
                if (this->decrypt_policy(cmd))
                    size = crt.decrypt(in_stream, in_stream.offset() - 1, size);

                // Call function that matched by command byte
                if (this->_handler.contains(cmd) == false)
                {
                    fb::logger::warn("정의되지 않은 요청입니다. [{:#x}]", cmd);
                    in_stream.reset();
                    in_stream.shift(base_size + size);
                    in_stream.flush();
                    continue;
                }

                auto before = this->handle_thread_index(socket);
                auto result = co_await this->_handler[cmd](socket, [&in_stream, size] {
                    in_stream.reset();
                    in_stream.shift(base_size + size);
                    in_stream.flush();
                });
                auto after  = this->handle_thread_index(socket);

                // 콜백 조건이 만족하지 못하는 경우 즉시 종료
                if (before != after)
                    co_return true;
            }
            catch (std::exception& e)
            {
                fb::logger::fatal(e.what());
                in_stream.clear();
                break;
            }
            catch (...)
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
void fb::acceptor<T>::bind(int cmd, const std::function<async::task<bool>(fb::socket<T>&, const R&)>& fn)
{
    this->_handler.insert({cmd, [this, fn](fb::socket<T>& socket, const std::function<void()>& callback) {
                               return socket.template in_stream<async::task<bool>>([this, &fn, &socket, &callback](auto& in_stream) {
                                   R header;
                                   header.deserialize(in_stream);
                                   callback();
                                   return fn(socket, header);
                               });
                           }});
}

template <typename T>
template <typename R>
void fb::acceptor<T>::bind(const std::function<async::task<bool>(fb::socket<T>&, const R&)>& fn)
{
    auto id = R().__id;
    this->bind(id, fn);
}

template <typename T>
void fb::acceptor<T>::bind_timer(const std::function<async::task<void>()>& fn, const std::chrono::steady_clock::duration& duration)
{
    auto timer        = std::make_shared<boost::asio::deadline_timer>(this->_context, boost::posix_time::seconds(1));
    auto callback_ptr = std::make_shared<std::function<void(const boost::system::error_code&)>>();
    auto callback     = [=](const boost::system::error_code&) {
        async::awaitable_then(fn(), [timer, callback_ptr, duration](async::awaitable_result<void> result) {
            timer->expires_at(timer->expires_at() + boost::posix_time::milliseconds(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()));
            timer->async_wait(*callback_ptr.get());
        });
    };
    *callback_ptr = callback;
    timer->async_wait(*callback_ptr.get());
}