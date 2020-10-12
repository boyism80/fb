#include "socket.h"

template<typename T>
fb::base_socket<T>::base_socket(boost::asio::io_context& context, std::function<void(fb::base_socket<T>&)> handle_receive, std::function<void(fb::base_socket<T>&)> handle_closed) : 
    boost::asio::ip::tcp::socket(context),
    _data(nullptr),
    _handle_receive(handle_receive),
    _handle_closed(handle_closed)
{
}

template <typename T>
fb::base_socket<T>::~base_socket()
{
}

template<typename T>
inline void fb::base_socket<T>::send(const ostream& stream)
{
    auto buffer = boost::asio::buffer(stream.data(), stream.size());
    boost::asio::async_write
    (
        *this, 
        buffer, 
        [this] (const boost::system::error_code error, size_t bytes_transferred)
        {}
    );
}

template<typename T>
inline void fb::base_socket<T>::send(const fb::protocol::base::response& response)
{
    fb::ostream             out_stream;
    response.serialize(out_stream);
    this->send(out_stream);
}

template <typename T>
void fb::base_socket<T>::recv()
{
    this->async_read_some
    (
        boost::asio::buffer(this->_buffer),
        [this](const boost::system::error_code& error, size_t bytes_transferred)
        {
            try
            {
                if(error)
                {
                    if(error == boost::asio::error::eof)
                        throw std::exception();
                    else 
                        throw std::exception();
                }

                this->_instream.insert(this->_instream.end(), this->_buffer.begin(), this->_buffer.begin() + bytes_transferred);
                this->recv();
                this->_handle_receive(*this);
            }
            catch(std::exception& e)
            {
                this->_handle_closed(*this);
            }
        }
    );
}

template <typename T>
fb::istream& fb::base_socket<T>::in_stream()
{
    return this->_instream;
}

template <typename T>
void fb::base_socket<T>::data(T* value)
{
    this->_data = value;
}

template <typename T>
T* fb::base_socket<T>::data()
{
    return this->_data;
}

template<typename T>
std::string fb::base_socket<T>::IP() const
{
    return this->remote_endpoint()
        .address()
        .to_string();
}





// fb::socket

template<typename T>
fb::socket<T>::socket(boost::asio::io_context& context, std::function<void(fb::base_socket<T>&)> handle_receive, std::function<void(fb::base_socket<T>&)> handle_closed) : 
    fb::base_socket<T>(context, handle_receive, handle_closed)
{
}

template<typename T>
fb::socket<T>::socket(boost::asio::io_context& context, const fb::cryptor& crt, std::function<void(fb::base_socket<T>&)> handle_receive, std::function<void(fb::base_socket<T>&)> handle_closed) : 
    fb::socket<T>(context, handle_receive, handle_closed)
{
    this->_crt = crt;
}

template<typename T>
fb::socket<T>::~socket()
{
}

template<typename T>
inline void fb::socket<T>::on_encrypt(fb::ostream& out)
{
    this->_crt.encrypt(out);
}

template<typename T>
inline void fb::socket<T>::on_wrap(fb::ostream& out)
{
    this->_crt.wrap(out);
}

template <typename T>
void fb::socket<T>::send(const fb::ostream& stream, bool encrypt, bool wrap)
{
    if(stream.size() == 0)
        return;

    auto clone = fb::ostream(stream);
    if(encrypt)
        this->on_encrypt(clone);

    if(wrap)
        this->on_wrap(clone);

    fb::base_socket<T>::send(clone);
}

template<typename T>
void fb::socket<T>::send(const fb::protocol::base::response& response, bool encrypt, bool wrap)
{
    fb::ostream             out_stream;
    response.serialize(out_stream);
    this->send(out_stream, encrypt, wrap);
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

template <template<class> class S, class T>
void fb::socket_container<S, T>::push(S<T>& session)
{
    this->push_back(&session);
}

template <template<class> class S, class T>
void fb::socket_container<S, T>::erase(S<T>& session)
{
    std::vector<S<T>*>::erase(std::find(this->begin(), this->end(), &session));
}

template <template<class> class S, class T>
void fb::socket_container<S, T>::erase(uint32_t fd)
{
    this->erase(this->operator[](fd));
}

template <template<class> class S, class T>
inline S<T>* fb::socket_container<S, T>::operator[](uint32_t fd)
{
    auto i = std::find_if
    (
        this->begin(), this->end(), 
        [fd] (S<T>* x) 
        { 
            uint32_t x_fd = x->native_handle();
            return x_fd == fd; 
        }
    );
    return i == this->end() ? nullptr : *i;
}