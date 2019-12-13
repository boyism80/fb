#include "acceptor.h"
using namespace fb;

base_session::base_session(SOCKET socket) : socket(socket)
{
}

base_session::~base_session()
{
}


fb_session::fb_session(SOCKET socket) : base_session(socket)
{
}

fb_session::~fb_session()
{
}

fb::cryptor& fb_session::crt()
{
	return this->_cryptor;
}

void fb_session::crt(const cryptor& cryptor)
{
	this->_cryptor = cryptor;
}

void fb_session::crt(uint8_t types, uint8_t* key)
{
	this->_cryptor = cryptor(types, key);
}
