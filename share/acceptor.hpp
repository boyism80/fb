#include "acceptor.h"

template <typename T>
fb::acceptor<T>::acceptor(boost::asio::io_context& context, uint16_t port) : 
	base_acceptor(context, port)
{
    this->accept();
}

template <typename T>
fb::acceptor<T>::~acceptor()
{
    for(auto x : this->sessions)
        delete x;
}

template <typename T>
void fb::acceptor<T>::accept()
{
	auto new_socket = new fb::socket(this);
	this->async_accept
	(
        *new_socket,
        boost::bind(&fb::acceptor<T>::handle_accept, 
        this, 
        new_socket,
        boost::asio::placeholders::error)
	);
}

template <typename T>
void fb::acceptor<T>::handle_accept(fb::socket* socket, const boost::system::error_code& error)
{
	try
    {
        if(error)
            throw std::runtime_error(error.message());

        auto session = this->handle_alloc_session(socket);
        this->_session_map.push(socket, session);
        this->sessions.push_back(session);

        socket->recv();
        this->accept();

        this->handle_connected(*session);
    }
    catch(std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

template<class T>
inline bool fb::acceptor<T>::call_handle(T& session, uint8_t cmd)
{
    auto i = this->_response_table.find(cmd);
    if(i == this->_response_table.end())
    	return true;

    return i->second(session);
}

template <typename T>
void fb::acceptor<T>::handle_parse(T& session)
{
    static uint8_t      base_size   = sizeof(uint8_t) + sizeof(uint16_t);
    auto&               istream     = session.in_stream();
    auto&               crt         = session.crt();

    while(true)
    {
        try
        {
            if(istream.readable_size() < base_size)
                break;

            // Read base head and check it is 0xAA
            uint8_t             head = istream.read_u8();
            if(head != 0xAA)
                throw std::exception();


            // Read data size and check it is greater than buffer size
            uint16_t            size = istream.read_u16(buffer::endian::BIG);
            if(size > istream.capacity())
                throw std::exception();


            // If data size is not enough to parse, do not anymore
            if(istream.readable_size() < size)
                break;


            // Erase head and size data
            istream.shift(base_size);

            // If command byte is not 0x10, this data must be decrypted by session's encrypt key
            uint8_t             cmd = istream.read_u8();
            if(cmd != 0x00 && cmd != 0x10)
                size = crt.decrypt(istream, istream.position(), size);

            // Call function that matched by command byte
            istream.reset();
            if(this->call_handle(session, cmd) == false)
                throw std::exception();

            // Set data pointer to process next packet bytes
            istream.shift(size);
        }
        catch(...)
        {
            break;
        }
    }

    istream.reset();
}

template <typename T>
void fb::acceptor<T>::handle_receive(fb::socket& socket)
{
    this->handle_parse(*this->_session_map[socket]);
}

template <typename T>
void fb::acceptor<T>::handle_disconnected(fb::socket& socket)
{
    auto session = this->_session_map[socket];
    if(session == nullptr)
        return;

    this->handle_disconnected(*session);
    this->_session_map.erase(socket);
    this->sessions.erase(session);

    delete session;
}

template <typename T>
void fb::acceptor<T>::register_fn(uint8_t cmd, std::function<bool(T&)> fn)
{
	this->_response_table.insert(std::make_pair(cmd, fn));
}

template<class T>
void fb::acceptor<T>::change_server(T& session, uint32_t ip, uint16_t port)
{
	fb::ostream                 buffer_stream;
	fb::cryptor&                crt = session.crt();

	buffer_stream.write_u8(0x03)
		.write_u32(ip)          // ip
		.write_u16(port)        // port
		.write_u8(0x0C)         // backward size
		.write_u8(crt.types())
		.write_u8(0x09)
		.write(crt.key(), 0x09)
		.write_u8(0x00);
	crt.wrap(buffer_stream);

	session.send(buffer_stream, false, false);
}

template <typename T>
void fb::acceptor<T>::send_stream(fb::base& base, const fb::ostream& stream, bool encrypt, bool wrap)
{
	if(stream.empty())
		return;

	base.send(stream, encrypt, wrap);
}