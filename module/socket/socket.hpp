#include "socket.h"

template <typename T>
fb::socket<T>::socket(fb::acceptor<T>& owner) : 
    boost::asio::ip::tcp::socket(owner.get_executor()),
    _owner(owner),
    _data(nullptr)
{
}

template <typename T>
fb::socket<T>::socket(fb::acceptor<T>& owner, const fb::cryptor& crt) : 
    fb::socket<T>(owner)
{
    this->_crt = crt;
}

template <typename T>
fb::socket<T>::~socket()
{
}

template <typename T>
void fb::socket<T>::send(const ostream& stream, bool encrypt, bool wrap, bool async)
{
    if(stream.size() == 0)
        return;

    auto clone = ostream(stream);
    if(encrypt)
        this->_crt.encrypt(clone);

    if(wrap)
        this->_crt.wrap(clone);

    auto buffer = boost::asio::buffer(clone.data(), clone.size());
    if(async)
    {
        boost::asio::async_write
        (
            *this, 
            buffer, 
            [this] (const boost::system::error_code error, size_t bytes_transferred)
            {
                this->_owner.handle_send(*this);
            }
        );
    }
    else
    {
        boost::asio::write(*this, buffer);
        this->_owner.handle_send(*this);
    }
}

template<typename T>
void fb::socket<T>::send(const fb::protocol::base::response& response, bool encrypt, bool wrap, bool async)
{
    fb::ostream             out_stream;
    response.serialize(out_stream);
    this->send(out_stream, encrypt, wrap, async);
}

template <typename T>
fb::cryptor& fb::socket<T>::crt()
{
    return this->_crt;
}

template <typename T>
void fb::socket<T>::crt(const fb::cryptor& crt)
{
    this->_crt = cryptor(crt);
}

template <typename T>
void fb::socket<T>::crt(uint8_t enctype, const uint8_t* enckey)
{
    this->_crt = cryptor(enctype, enckey);
}

template <typename T>
void fb::socket<T>::recv()
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
                this->_owner.handle_receive(*this);
            }
            catch(std::exception& e)
            {
                this->_owner.handle_closed(*this);
            }
        }
    );
}

template <typename T>
fb::acceptor<T>& fb::socket<T>::owner() const
{
    return this->_owner;
}

template <typename T>
fb::istream& fb::socket<T>::in_stream()
{
    return this->_instream;
}

template <typename T>
void fb::socket<T>::data(T* value)
{
    this->_data = value;
}

template <typename T>
T* fb::socket<T>::data()
{
    return this->_data;
}

template<typename T>
std::string fb::socket<T>::IP() const
{
    return this->remote_endpoint()
        .address()
        .to_string();
}

template <typename T>
fb::socket<T>::operator fb::cryptor& ()
{
    return this->_crt;
}






// socket_container
template <typename T>
void fb::socket_container<T>::push(fb::socket<T>& session)
{
    this->push_back(&session);
}

template <typename T>
void fb::socket_container<T>::erase(fb::socket<T>& session)
{
    std::vector<fb::socket<T>*>::erase(std::find(this->begin(), this->end(), &session));
}

template <typename T>
void fb::socket_container<T>::erase(uint32_t fd)
{
    this->erase(this->operator[](fd));
}

template <typename T>
inline fb::socket<T>* fb::socket_container<T>::operator[](uint32_t fd)
{
    auto i = std::find_if
    (
        this->begin(), this->end(), 
        [fd] (fb::socket<T>* x) 
        { 
            uint32_t x_fd = x->native_handle();
            return x_fd == fd; 
        }
    );
    return i == this->end() ? nullptr : *i;
}