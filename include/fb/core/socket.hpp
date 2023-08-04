#include "socket.h"

template<typename T>
fb::base::socket<T>::socket(boost::asio::io_context& context, const handler_event& handle_received, const handler_event& handle_closed) : 
    boost::asio::ip::tcp::socket(context),
    _handle_received(handle_received),
    _handle_closed(handle_closed)
{ }

template<typename T>
inline void fb::base::socket<T>::send(const fb::ostream& stream, bool encrypt, bool wrap)
{
    auto clone = stream;
    if(encrypt && this->on_encrypt(clone) == false)
        return;

    if(wrap && this->on_wrap(clone) == false)
        return;

    auto buffer = boost::asio::buffer(clone.data(), clone.size());
    boost::asio::async_write
    (
        *this, 
        buffer, 
        [this] (const boost::system::error_code error, size_t bytes_transferred)
        { }
    );
}

template<typename T>
inline void fb::base::socket<T>::send(const fb::protocol::base::header& response, bool encrypt, bool wrap)
{
    fb::ostream             out_stream;
    response.serialize(out_stream);
    this->send(out_stream, encrypt, wrap);
}

template <typename T>
void fb::base::socket<T>::recv()
{
    this->async_read_some
    (
        boost::asio::buffer(this->_buffer),
        [this](const boost::system::error_code& error, size_t bytes_transferred)
        {
            // TODO: 이 콜백함수가 호출되는게 메인스레드인지 확인
            // io_context를 여러 스레드에서 run했을 때는 어떻게 되는지도 확인
            try
            {
                if(error)
                    throw std::exception();
                
                auto gd = std::lock_guard<std::mutex>(mutex);

                this->_instream.insert(this->_instream.end(), this->_buffer.begin(), this->_buffer.begin() + bytes_transferred);
                this->_handle_received(*this);
                this->recv();
            }
            catch(std::exception&)
            {
                this->_handle_closed(*this);
            }
        }
    );
}

template <typename T>
fb::istream& fb::base::socket<T>::in_stream()
{
    return this->_instream;
}

template <typename T>
void fb::base::socket<T>::data(T* value)
{
    this->_data = value;
}

template <typename T>
T* fb::base::socket<T>::data()
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
    uint32_t fd = this->native_handle();
    if(fd != 0xFFFFFFFF)
        this->_fd = fd;
    
    return this->_fd;
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

template <typename T>
template <typename R>
fb::awaitable_socket<T>::awaitable<R>::awaitable(awaitable_socket<T>& owner, uint8_t cmd, const std::function<void()>& on_suspend) : _owner(owner), _on_suspend(on_suspend)
{ }

template <typename T>
template <typename R>
fb::awaitable_socket<T>::awaitable<R>::~awaitable()
{ }

template <typename T>
template <typename R>
bool fb::awaitable_socket<T>::awaitable<R>::await_ready()
{
    return false;
}

template <typename T>
template <typename R>
void fb::awaitable_socket<T>::awaitable<R>::await_suspend(std::coroutine_handle<> h)
{
    this->handler = h;
    auto id = R().__id;
    this->_owner.register_awaiter(id, this);
    this->_on_suspend();
}

template <typename T>
template <typename R>
R fb::awaitable_socket<T>::awaitable<R>::await_resume()
{
    if (this->e.has_value())
        throw this->e.value();

    return *this->result;
}

template <typename T>
fb::awaitable_socket<T>::awaitable_socket(boost::asio::io_context& context, const fb::base::socket<T>::handler_event& handle_received, const fb::base::socket<T>::handler_event& handle_closed) : 
    fb::socket<T>(context, handle_received, handle_closed)
{ }

template <typename T>
fb::awaitable_socket<T>::~awaitable_socket()
{ }


template <typename T>
template <typename R>
void fb::awaitable_socket<T>::register_awaiter(uint8_t cmd, awaitable<R>* awaiter)
{
    auto mg = std::lock_guard<std::mutex>(this->_awaiter_mutex);
    auto i = this->_coroutines.find(cmd);
    if(i != this->_coroutines.end())
    {
        auto awaiter = static_cast<awaitable_socket<T>::awaitable<R>*>(i->second);
        awaiter->e = std::exception();
    }
    this->_coroutines[cmd] = static_cast<void*>(awaiter);
}


template <typename T>
template <typename R>
void fb::awaitable_socket<T>::invoke_awaiter(uint8_t cmd, R& response)
{
    awaitable_socket<T>::awaitable<R>* awaiter = nullptr;
    
    {
        auto mg = std::lock_guard<std::mutex>(this->_awaiter_mutex);
        if (this->_coroutines.contains(cmd) == false)
            return;

        awaiter = static_cast<awaitable_socket<T>::awaitable<R>*>(this->_coroutines[cmd]);
        this->_coroutines.erase(cmd);
    }

    awaiter->result = &response;
    awaiter->handler.resume();
    
}

template <typename T>
template <typename R>
auto fb::awaitable_socket<T>::request(const fb::protocol::base::header& header, bool encrypt, bool wrap)
{
    return awaitable_socket<T>::awaitable<R>(*this, header.__id, 
        [=, &header, this]
        { 
            this->send(header, encrypt, wrap); 
        });
}



// fb::internal::socket
template <typename T>
fb::internal::socket<T>::socket(boost::asio::io_context& context, const fb::base::socket<T>::handler_event& handle_received, const fb::base::socket<T>::handler_event& handle_closed) : 
    fb::awaitable_socket<T>(context, handle_received, handle_closed)
{ }

template <typename T>
fb::internal::socket<T>::~socket()
{ }

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
void fb::base::socket_container<S, T>::push(std::unique_ptr<S<T>>&& session)
{
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
    std::map<uint32_t, std::unique_ptr<S<T>>>::erase(session.fd());
}

template <template<class> class S, class T>
void fb::base::socket_container<S, T>::erase(uint32_t fd)
{
    std::map<uint32_t, std::unique_ptr<S<T>>>::erase(fd);
}

template <template<class> class S, class T>
inline S<T>* fb::base::socket_container<S, T>::operator[](uint32_t fd) const
{
    const auto& found = std::map<uint32_t, std::unique_ptr<S<T>>>::find(fd);
    if(found == this->cend())
        return nullptr;

    return found->second.get();
}