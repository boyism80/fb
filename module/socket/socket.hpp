#include "socket.h"

template<typename T>
fb::base::socket<T>::socket(boost::asio::io_context& context, std::function<void(fb::base::socket<T>&)> handle_received, std::function<void(fb::base::socket<T>&)> handle_closed) : 
    boost::asio::ip::tcp::socket(context),
    _data(nullptr),
    _handle_received(handle_received),
    _handle_closed(handle_closed)
{
}

template <typename T>
fb::base::socket<T>::~socket()
{
}

template<typename T>
inline void fb::base::socket<T>::send(const fb::ostream& stream, bool encrypt, bool wrap)
{
    auto clone = stream;
    if(encrypt)
        this->on_encrypt(clone);

    if(wrap)
        this->on_wrap(clone);

    auto buffer = boost::asio::buffer(clone.data(), clone.size());
    boost::asio::async_write
    (
        *this, 
        buffer, 
        [this] (const boost::system::error_code error, size_t bytes_transferred)
        {}
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
            try
            {
                if(error)
                    throw std::exception();
                
                this->_instream.insert(this->_instream.end(), this->_buffer.begin(), this->_buffer.begin() + bytes_transferred);
                this->_handle_received(*this);
                this->recv();
            }
            catch(std::exception& e)
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
    return this->_data;
}

template<typename T>
std::string fb::base::socket<T>::IP() const
{
    return this->remote_endpoint()
        .address()
        .to_string();
}





// fb::socket

template<typename T>
fb::socket<T>::socket(boost::asio::io_context& context, std::function<void(fb::base::socket<T>&)> handle_received, std::function<void(fb::base::socket<T>&)> handle_closed) : 
    fb::base::socket<T>(context, handle_received, handle_closed)
{
}

template<typename T>
fb::socket<T>::socket(boost::asio::io_context& context, const fb::cryptor& crt, std::function<void(fb::base::socket<T>&)> handle_received, std::function<void(fb::base::socket<T>&)> handle_closed) : 
    fb::socket<T>(context, handle_received, handle_closed)
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



// fb::internal::socket
template <typename T>
fb::internal::socket<T>::socket(boost::asio::io_context& context, std::function<void(fb::base::socket<T>&)> handle_received, std::function<void(fb::base::socket<T>&)> handle_closed) : 
    fb::base::socket<T>(context, handle_received, handle_closed)
{}

template <typename T>
fb::internal::socket<T>::~socket()
{}

template <typename T>
void fb::internal::socket<T>::on_wrap(fb::ostream& out)
{
    auto                size = out.size();
    const uint8_t       header[] = {uint8_t(size >> 8 & 0xFF), uint8_t(size & 0xFF)};
    out.insert(out.begin(), header, header + sizeof(header));
}



// socket_container

template <template<class> class S, class T>
void fb::base::socket_container<S, T>::push(S<T>& session)
{
    this->push_back(&session);
}

template <template<class> class S, class T>
void fb::base::socket_container<S, T>::erase(S<T>& session)
{
    std::vector<S<T>*>::erase(std::find(this->begin(), this->end(), &session));
}

template <template<class> class S, class T>
void fb::base::socket_container<S, T>::erase(uint32_t fd)
{
    this->erase(this->operator[](fd));
}

template <template<class> class S, class T>
inline S<T>* fb::base::socket_container<S, T>::operator[](uint32_t fd)
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