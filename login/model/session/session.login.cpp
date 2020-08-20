#include <model/session/session.login.h>

fb::login::session::session(fb::socket* socket) : 
    _socket(socket)
{}

fb::login::session::~session()
{}

void fb::login::session::send(const fb::ostream& stream, bool encrypt, bool wrap, bool async)
{
    this->_socket->send(stream, encrypt, wrap, async);
}

fb::cryptor& fb::login::session::crt()
{
    return this->_socket->crt();
}

void fb::login::session::crt(const fb::cryptor& crt)
{
    this->_socket->crt(crt);
}

void fb::login::session::crt(uint8_t enctype, const uint8_t* enckey)
{
    this->_socket->crt(enctype, enckey);
}

fb::istream& fb::login::session::in_stream()
{
    return this->_socket->in_stream();
}

fb::login::session::operator fb::socket & ()
{
    return *this->_socket;
}