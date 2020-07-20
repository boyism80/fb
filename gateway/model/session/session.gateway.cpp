#include <model/session/session.gateway.h>

fb::gateway::session::session(fb::socket* socket) : 
    _socket(socket)
{}

fb::gateway::session::~session()
{}

void fb::gateway::session::send(const fb::ostream& stream, bool encrypt, bool wrap)
{
    this->_socket->send(stream, encrypt, wrap);
}

fb::cryptor& fb::gateway::session::crt()
{
    return this->_socket->crt();
}

void fb::gateway::session::crt(const fb::cryptor& crt)
{
    this->_socket->crt(crt);
}

void fb::gateway::session::crt(uint8_t enctype, const uint8_t* enckey)
{
    this->_socket->crt(enctype, enckey);
}

fb::istream& fb::gateway::session::in_stream()
{
    return this->_socket->in_stream();
}

fb::gateway::session::operator fb::socket & ()
{
    return *this->_socket;
}