#include "socket.h"
#include <fb/core/logger.h>

template<typename T>
fb::base::socket<T>::socket(boost::asio::io_context& context, const handler_event& handle_received, const handler_event& handle_closed) : 
    boost::asio::ip::tcp::socket(context),
    _handle_received(handle_received),
    _handle_closed(handle_closed)
{ }

template<typename T>
fb::base::socket<T>::~socket()
{
    auto _1 = std::lock_guard(this->_boost_mutex);
    auto _2 = std::lock_guard(this->_instream_mutex);
}

template<typename T>
inline void fb::base::socket<T>::send(const fb::ostream& stream, bool encrypt, bool wrap, std::function<void(const boost::system::error_code&, size_t)> callback)
{
    auto clone = stream;
    if (encrypt && this->on_encrypt(clone) == false)
        return;

    if (wrap && this->on_wrap(clone) == false)
        return;

    auto buffer = boost::asio::buffer(clone.data(), clone.size());
    {
        auto _ = std::lock_guard(this->_boost_mutex);
        boost::asio::async_write(*this, buffer, callback);
    }
}

template<typename T>
inline void fb::base::socket<T>::send(const fb::ostream& stream, bool encrypt, bool wrap)
{
    static auto empty_fn = [](const boost::system::error_code ec, size_t size)
    {

    };
    this->send(stream, encrypt, wrap, empty_fn);
}

template<typename T>
inline void fb::base::socket<T>::send(const fb::protocol::base::header& response, bool encrypt, bool wrap)
{
    static auto empty_fn = [](const boost::system::error_code&, size_t) {};
    this->send(response, encrypt, wrap, empty_fn);
}

template<typename T>
inline void fb::base::socket<T>::send(const fb::protocol::base::header& response, bool encrypt, bool wrap, std::function<void(const boost::system::error_code&, size_t)> callback)
{
    fb::ostream             out_stream;
    response.serialize(out_stream);
    this->send(out_stream, encrypt, wrap, callback);
}

template <typename T>
void fb::base::socket<T>::recv()
{
    auto _ = std::lock_guard(this->_boost_mutex);
    {
        auto _ = std::lock_guard(this->_tasks_mutex);
        for(int i = this->_tasks.size() - 1; i >= 0; i--)
        {
            auto& task = this->_tasks[i];
            if(task.done())
                this->_tasks.erase(this->_tasks.begin() + i);
        }
    }

    this->async_read_some
    (
        boost::asio::buffer(this->_buffer),
        [this](const boost::system::error_code& ec, size_t bytes_transferred)
        {
            try
            {
                if(ec)
                    throw boost::system::error_code(ec);
                
                // auto gd = std::lock_guard<std::mutex>(this->stream_mutex);

                this->in_stream<void>([this, bytes_transferred](auto& in_stream)
                {
                    auto _ = std::lock_guard(this->_tasks_mutex);
                    this->_instream.insert(this->_instream.end(), this->_buffer.begin(), this->_buffer.begin() + bytes_transferred);
                    this->_tasks.push_back(this->_handle_received(*this));
                });

                if (this->is_open() == false)
                    throw std::runtime_error("disconnected");

                this->recv();
            }
            catch (std::exception& e)
            {
                fb::logger::fatal(e.what());
                this->_handle_closed(*this);
            }
            catch (boost::system::error_code& e)
            {
                auto ec = e.value();
                switch (ec)
                {
                case ERROR_FILE_NOT_FOUND:
                    this->_handle_closed(*this);
                    break;

                case ERROR_OPERATION_ABORTED:
                    this->_handle_closed(*this);
                    break;

                case WSAECONNRESET:
                    fb::logger::info("SYSTEM SHUTDOWN ALERT?");
                    break;

                default:
                    break;
                }
            }
            catch(...)
            {
                this->_handle_closed(*this);
            }
        }
    );
}

template <typename T>
void fb::base::socket<T>::data(T* value)
{
    this->_data = value;
}

template <typename T>
T* fb::base::socket<T>::data() const
{
    return _data;
}

template<typename T>
std::string fb::base::socket<T>::IP() const
{
    return this->remote_endpoint()
        .address()
        .to_string();
}

template<typename T>
uint32_t fb::base::socket<T>::fd()
{
    if(this->_fd != 0xFFFFFFFF)
        return this->_fd;

    this->_fd = (uint32_t)this->native_handle();
    return this->_fd;
}

template <typename T>
void fb::base::socket<T>::push_task(fb::task<bool>&& task)
{
    auto _ = std::lock_guard(this->_task_mutex);
    this->_unfinished_tasks.push_back(std::move(task));
}

template <typename T>
bool fb::base::socket<T>::flush_task()
{
    auto _ = std::lock_guard(this->_task_mutex);
    auto size = this->_unfinished_tasks.size();
    if(size == 0)
        return true;

    auto result = false;
    for(int i = size - 1; i >= 0; i--)
    {
        auto& task = this->_unfinished_tasks[i];
        if(!task.done())
            continue;

        result |= task.value();
        this->_unfinished_tasks.erase(this->_unfinished_tasks.begin() + i);
    }

    return result;
}





// fb::socket

template<typename T>
fb::socket<T>::socket(boost::asio::io_context& context, const fb::base::socket<T>::handler_event& handle_received, const fb::base::socket<T>::handler_event& handle_closed) : 
    fb::base::socket<T>(context, handle_received, handle_closed)
{ }

template<typename T>
fb::socket<T>::socket(boost::asio::io_context& context, const fb::cryptor& crt, const fb::base::socket<T>::handler_event& handle_received, const fb::base::socket<T>::handler_event& handle_closed) : 
    fb::socket<T>(context, handle_received, handle_closed)
{
    this->_crt = crt;
}

template<typename T>
inline bool fb::socket<T>::on_encrypt(fb::ostream& out)
{
    return this->_crt.encrypt(out);
}

template<typename T>
inline bool fb::socket<T>::on_wrap(fb::ostream& out)
{
    return this->_crt.wrap(out);
}

template <typename T>
fb::cryptor& fb::socket<T>::crt()
{
    return this->_crt;
}

template <typename T>
void fb::socket<T>::crt(const fb::cryptor& crt)
{
    this->_crt = fb::cryptor(crt);
}

template <typename T>
void fb::socket<T>::crt(uint8_t enctype, const uint8_t* enckey)
{
    this->_crt = fb::cryptor(enctype, enckey);
}

template <typename T>
fb::socket<T>::operator fb::cryptor& ()
{
    return this->_crt;
}

template <typename T, typename C>
fb::awaitable_socket<T,C>::awaitable_socket(boost::asio::io_context& context, const fb::base::socket<T>::handler_event& handle_received, const fb::base::socket<T>::handler_event& handle_closed) : 
    fb::base::socket<T>(context, handle_received, handle_closed)
{ }

template <typename T, typename C>
fb::awaitable_socket<T,C>::~awaitable_socket()
{
    auto _ = std::lock_guard(this->_awaiter_mutex);
}


template <typename T, typename C>
template <typename R>
void fb::awaitable_socket<T,C>::register_awaiter(C cmd, std::shared_ptr<fb::awaiter<R>>& awaiter)
{
    auto _ = std::lock_guard(this->_awaiter_mutex);
    auto i = this->_coroutines.find(cmd);
    if(i != this->_coroutines.end())
    {
        auto& exists = any_cast<std::shared_ptr<R>>(i->second);
        exists->set_error(boost::system::error_code());
        this->_coroutines.erase(cmd);
    }
    
    this->_coroutines.insert({cmd, awaiter});
}


template <typename T, typename C>
template <typename R>
void fb::awaitable_socket<T,C>::invoke_awaiter(C cmd, R& response)
{
    {
        auto _ = std::lock_guard(this->_awaiter_mutex);
        if (this->_coroutines.contains(cmd) == false)
            return;

        auto& exists = any_cast<std::shared_ptr<fb::awaiter<R>>&>(this->_coroutines[cmd]);
        this->_coroutines.erase(cmd);
        exists->set_result(response);
    }
}

template <typename T, typename C>
template <typename R>
fb::task<R, std::suspend_always>& fb::awaitable_socket<T, C>::request(const fb::protocol::base::header& header, bool encrypt, bool wrap)
{
    auto cmd = R().__id;
    auto awaiter = std::make_shared<fb::awaiter<R>>();
    this->send(header, encrypt, wrap, [this, awaiter] (const boost::system::error_code& ec, auto transfer_size)
    {
        if(!ec)
            return;

        auto sstream = std::stringstream();
        sstream << "boost socket error(" << ec.value() << ")";
        awaiter->set_error(std::runtime_error(sstream.str()));
    });

    return awaiter->task;
}

template <typename T, typename C>
template <typename R>
fb::task<R, std::suspend_always>& fb::awaitable_socket<T, C>::request(const fb::protocol::internal::header& header, bool encrypt, bool wrap)
{
    auto cmd = R().__id;
    auto awaiter = std::make_shared<fb::awaiter<R>>();
    this->send(header, encrypt, wrap, [this, awaiter] (const boost::system::error_code& ec, auto transfer_size)
    {
        if(!ec)
            return;

        auto sstream = std::stringstream();
        sstream << "boost socket error (" << ec.value() << ")";
        awaiter->set_error(std::runtime_error(sstream.str()));
    });

    return awaiter->task;
}


// fb::internal::socket
template <typename T>
fb::internal::socket<T>::socket(boost::asio::io_context& context, const handler_event& handle_received, const handler_event& handle_closed) :
    fb::awaitable_socket<T,std::string>(context, handle_received, handle_closed)
{ 
    
}

template <typename T>
fb::internal::socket<T>::~socket()
{
    
}

template <typename T>
bool fb::internal::socket<T>::on_wrap(fb::ostream& out)
{
    auto                size = out.size();
    const uint8_t       header[] = {uint8_t(size >> 8 & 0xFF), uint8_t(size & 0xFF)};
    out.insert(out.begin(), header, header + sizeof(header));
    return true;
}



// socket_container

template <template<class> class S, class T>
fb::base::socket_container<S, T>::~socket_container()
{
    auto _ = std::lock_guard(this->mutex);
}

template <template<class> class S, class T>
void fb::base::socket_container<S, T>::push(std::unique_ptr<S<T>>&& session)
{
    auto _ = std::lock_guard(this->mutex);

    auto fd = session->fd();
    std::map<uint32_t, std::unique_ptr<S<T>>>::insert
    (
        std::pair<uint32_t, std::unique_ptr<S<T>>>
        (
            fd, 
            std::move(session)
        )
    );
}

template <template<class> class S, class T>
void fb::base::socket_container<S, T>::erase(S<T>& session)
{
    auto _ = std::lock_guard(this->mutex);

    std::map<uint32_t, std::unique_ptr<S<T>>>::erase(session.fd());
}

template <template<class> class S, class T>
void fb::base::socket_container<S, T>::erase(uint32_t fd)
{
    auto _ = std::lock_guard(this->mutex);

    std::map<uint32_t, std::unique_ptr<S<T>>>::erase(fd);
}

template <template<class> class S, class T>
bool fb::base::socket_container<S, T>::contains(uint32_t fd)
{
    auto _ = std::lock_guard(this->mutex);

    return std::map<uint32_t, std::unique_ptr<S<T>>>::contains(fd);
}

template <template<class> class S, class T>
void fb::base::socket_container<S, T>::each(const std::function<void(S<T>&)> fn)
{
    auto _ = std::lock_guard(this->mutex);
    for(auto& [fd, socket] : *this)
    {
        fn(*socket);
    }
}

template <template<class> class S, class T>
S<T>* fb::base::socket_container<S, T>::find(const std::function<bool(S<T>&)> fn)
{
    auto _ = std::lock_guard(this->mutex);
    for(auto& [fd, socket] : *this)
    {
        if(fn(*socket))
            return socket.get();
    }

    return nullptr;
}

template <template<class> class S, class T>
bool fb::base::socket_container<S, T>::empty()
{
    auto _ = std::lock_guard(this->mutex);

    return std::map<uint32_t, std::unique_ptr<S<T>>>::empty();
}

template <template<class> class S, class T>
void fb::base::socket_container<S, T>::close()
{
    auto _ = std::lock_guard(this->mutex);

    //auto empty = std::map<uint32_t, std::unique_ptr<S<T>>>::empty();
    //if(empty)
    //    return;

    //for (auto& [k, v] : *this)
    //{
    //    try
    //    {
    //        v->close();
    //    }
    //    catch(std::exception& e)
    //    {
    //        std::cout << e.what() << std::endl;
    //    }
    //}

    std::map<uint32_t, std::unique_ptr<S<T>>>::clear();
}

template <template<class> class S, class T>
inline S<T>* fb::base::socket_container<S, T>::operator[](uint32_t fd)
{
    auto _ = std::lock_guard(this->mutex);

    const auto& found = std::map<uint32_t, std::unique_ptr<S<T>>>::find(fd);
    if(found == this->cend())
        return nullptr;

    return found->second.get();
}