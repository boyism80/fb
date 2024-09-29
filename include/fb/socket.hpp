#include "socket.h"
#include <fb/logger.h>

template<typename T>
fb::socket<T>::socket(boost::asio::io_context& context, const handler_event& handle_received, const handler_event& handle_closed) : 
    boost::asio::ip::tcp::socket(context),
    _handle_received(handle_received),
    _handle_closed(handle_closed)
{ }

template<typename T>
fb::socket<T>::socket(boost::asio::io_context& context, const fb::cryptor& crt, const handler_event& handle_received, const handler_event& handle_closed) : 
    fb::socket<T>::socket(context, handle_received, handle_closed),
    boost::asio::ip::tcp::socket(context),
    _crt(crt)
{ }

template<typename T>
fb::socket<T>::~socket()
{
    auto _1 = std::lock_guard(this->_boost_mutex);
    auto _2 = std::lock_guard(this->_instream_mutex);
    this->close();
}

template<typename T>
inline void fb::socket<T>::send(const fb::ostream& stream, bool encrypt, bool wrap, std::function<void(const boost::system::error_code&, size_t)> callback)
{
    auto clone = fb::ostream(stream);
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
inline void fb::socket<T>::send(const fb::ostream& stream, bool encrypt, bool wrap)
{
    static auto empty_fn = [](const boost::system::error_code ec, size_t size)
    {

    };
    this->send(stream, encrypt, wrap, empty_fn);
}

template<typename T>
inline void fb::socket<T>::send(const fb::protocol::base::header& response, bool encrypt, bool wrap)
{
    static auto empty_fn = [](const boost::system::error_code&, size_t) {};
    this->send(response, encrypt, wrap, empty_fn);
}

template<typename T>
inline void fb::socket<T>::send(const fb::protocol::base::header& response, bool encrypt, bool wrap, std::function<void(const boost::system::error_code&, size_t)> callback)
{
    fb::ostream             out_stream;
    response.serialize(out_stream);
    this->send(out_stream, encrypt, wrap, callback);
}

template <typename T>
void fb::socket<T>::recv()
{
    this->async_read_some
    (
        boost::asio::buffer(this->_buffer),
        [this](const boost::system::error_code& ec, size_t bytes_transferred)
        {
            try
            {
                if(ec)
                    throw boost::system::error_code(ec);
                
                this->in_stream<void>([this, bytes_transferred](auto& in_stream)
                {
                    this->_instream.insert(this->_instream.end(), this->_buffer.begin(), this->_buffer.begin() + bytes_transferred);
                });
                async::awaitable_get(this->_handle_received(*this));

                if (this->is_open() == false)
                    throw std::runtime_error("disconnected");

                this->recv();
            }
            catch (std::exception& e)
            {
                fb::logger::fatal(e.what());
                async::awaitable_get(this->_handle_closed(*this));
            }
            catch (boost::system::error_code& e)
            {
                auto ec = e.value();
                switch (ec)
                {
                case ERROR_FILE_NOT_FOUND:
                    async::awaitable_get(this->_handle_closed(*this));
                    break;

                case ERROR_OPERATION_ABORTED:
                    async::awaitable_get(this->_handle_closed(*this));
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
                async::awaitable_get(this->_handle_closed(*this));
            }
        }
    );
}

template <typename T>
void fb::socket<T>::data(T* value)
{
    this->_data = value;
}

template <typename T>
T* fb::socket<T>::data() const
{
    return _data;
}

template<typename T>
std::string fb::socket<T>::IP() const
{
    return this->remote_endpoint()
        .address()
        .to_string();
}

template<typename T>
uint32_t fb::socket<T>::fd()
{
    if(this->_fd != 0xFFFFFFFF)
        return this->_fd;

    this->_fd = (uint32_t)this->native_handle();
    return this->_fd;
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


// socket_container

template <typename T>
fb::socket_container<T>::~socket_container()
{
    auto _ = std::lock_guard(this->mutex);
}

template <typename T>
void fb::socket_container<T>::push(std::unique_ptr<fb::socket<T>>&& session)
{
    auto _ = std::lock_guard(this->mutex);

    auto fd = session->fd();
    std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::insert
    (
        std::pair<uint32_t, std::unique_ptr<fb::socket<T>>>
        (
            fd, 
            std::move(session)
        )
    );
}

template <typename T>
void fb::socket_container<T>::erase(fb::socket<T>& session)
{
    auto _ = std::lock_guard(this->mutex);

    std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::erase(session.fd());
}

template <typename T>
void fb::socket_container<T>::erase(uint32_t fd)
{
    auto _ = std::lock_guard(this->mutex);

    std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::erase(fd);
}

template <typename T>
bool fb::socket_container<T>::contains(uint32_t fd)
{
    auto _ = std::lock_guard(this->mutex);

    return std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::contains(fd);
}

template <typename T>
void fb::socket_container<T>::each(const std::function<void(fb::socket<T>&)> fn)
{
    auto _ = std::lock_guard(this->mutex);
    for(auto& [fd, socket] : *this)
    {
        fn(*socket);
    }
}

template <typename T>
fb::socket<T>* fb::socket_container<T>::find(const std::function<bool(fb::socket<T>&)> fn)
{
    auto _ = std::lock_guard(this->mutex);
    for(auto& [fd, socket] : *this)
    {
        if(fn(*socket))
            return socket.get();
    }

    return nullptr;
}

template <typename T>
bool fb::socket_container<T>::empty()
{
    auto _ = std::lock_guard(this->mutex);

    return std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::empty();
}

template <typename T>
void fb::socket_container<T>::close()
{
    auto _ = std::lock_guard(this->mutex);

    //auto empty = std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::empty();
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

    std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::clear();
}

template <typename T>
inline fb::socket<T>* fb::socket_container<T>::operator[](uint32_t fd)
{
    auto _ = std::lock_guard(this->mutex);

    const auto& found = std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::find(fd);
    if(found == this->cend())
        return nullptr;

    return found->second.get();
}