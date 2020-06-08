#include "socket.h"
#include "acceptor.h"

fb::socket::socket(fb::base_acceptor* owner) : 
    boost::asio::ip::tcp::socket(owner->get_executor()),
    _owner(owner)
{
}

fb::socket::socket(base_acceptor* owner, const fb::cryptor& crt) : 
    boost::asio::ip::tcp::socket(owner->get_executor()),
    _crt(crt)
{
}

fb::socket::~socket()
{
}

void fb::socket::handle_send(const boost::system::error_code& error, size_t bytes_transferred)
{
    if(this->_deque.empty())
        return;

    delete[] this->_deque.front();
    this->_deque.pop_front();
    this->send();
}

void fb::socket::handle_receive(const boost::system::error_code& error, size_t bytes_transferred)
{
    try
    {
        if(error)
        {
            if(error == boost::asio::error::eof)
                throw std::runtime_error("disconnected from client.");
            else 
                throw std::runtime_error(error.message());

            _owner->handle_disconnect(*this);
        }

        this->_instream.insert(this->_instream.end(), this->_buffer.begin(), this->_buffer.begin() + bytes_transferred);
        this->recv();
        this->_owner->handle_receive(*this);
    }
    catch(std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

void fb::socket::send()
{
    if(this->_deque.empty())
        return;

    auto stream = this->_deque.front();
    boost::asio::async_write(*this, boost::asio::buffer(stream->data(), stream->size()),
        boost::bind(&fb::socket::handle_send, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred)
    );
}

void fb::socket::send(const ostream& stream, bool encrypt, bool wrap)
{
    auto clone = new ostream(stream);

    if(encrypt)
        this->_crt.encrypt(*clone);

    if(wrap)
        this->_crt.wrap(*clone);


    this->_deque.push_back(clone);
    this->send();
    this->_owner->handle_send(*this);
}

fb::cryptor& fb::socket::crt()
{
    return this->_crt;
}

void fb::socket::crt(const fb::cryptor& crt)
{
    this->_crt = cryptor(crt);
}

void fb::socket::crt(uint8_t enctype, const uint8_t* enckey)
{
    this->_crt = cryptor(enctype, enckey);
}

void fb::socket::recv()
{
    this->async_read_some
    (
        boost::asio::buffer(this->_buffer),
        boost::bind
        (
            &fb::socket::handle_receive, 
            this, 
            boost::asio::placeholders::error, 
            boost::asio::placeholders::bytes_transferred
        )
    );
}

fb::base_acceptor* fb::socket::owner() const
{
    return this->_owner;
}

fb::istream& fb::socket::in_stream()
{
    return this->_instream;
}

fb::socket::operator fb::cryptor& ()
{
    return this->_crt;
}