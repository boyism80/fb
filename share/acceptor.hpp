#include "acceptor.h"
#include <sysinfoapi.h>
using namespace fb;

template <typename T>
fb::base_acceptor<T>::base_acceptor(uint16_t port) : socket(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)),
_running(false),
_execute_thread(NULL)
{
    std::srand(static_cast<uint32_t>(std::time(0)));

    SOCKADDR_IN         address = { 0, };
    address.sin_addr.s_addr     = INADDR_ANY;
    address.sin_family          = AF_INET;
    address.sin_port            = htons(port);
    if (::bind(*this, (struct sockaddr*) & address, sizeof(address)) == SOCKET_ERROR)
        throw "cannot bind socket";

    if (::listen(*this, SOMAXCONN) == SOCKET_ERROR)
        throw "cannot listen socket";

    this->_sockets.root(*this, this);
}

template <typename T>
fb::base_acceptor<T>::~base_acceptor()
{
    this->exit();
    this->close();

    for(auto timer : this->_timers)
        delete timer;
}

template <typename T>
bool fb::base_acceptor<T>::do_session()
{
    struct timeval timeout;
    timeout.tv_sec  = 0;
    timeout.tv_usec = 500000;

    fd_set& reads = this->_sockets;
    fd_set  copy  = reads;

    if (::select(0, &copy, NULL, NULL, &timeout) == SOCKET_ERROR)
        return false;

    for (auto i = 0; i < reads.fd_count; i++)
    {
        auto                fd = reads.fd_array[i];
        if (FD_ISSET(fd, &copy) == false)
            continue;

        if(fd == *this)
        {
            this->handle_accept();
        }
        else
        {
            auto session = this->_sockets[fd];
            this->handle_receive(*session);
        }
    }

    for(auto i = 0; i < reads.fd_count; i++)
    {
        this->_sockets[reads.fd_array[i]]->in_stream().flush();
        this->_sockets[reads.fd_array[i]]->send();
    }
    return true;
}

template <typename T>
bool fb::base_acceptor<T>::disconnect(SOCKET fd)
{
    auto session = this->session(fd);

    this->handle_disconnected((T&)*this->_sockets[fd]);
    this->_session_table.erase(fd);
    this->_sessions.erase(std::find(this->_sessions.begin(), this->_sessions.end(), session));
    this->_sockets.remove(fd);
    return true;
}

template <typename T>
bool fb::base_acceptor<T>::execute(bool async)
{
    if (this->_running)
        return false;

    if(async)
    {
        if(this->_execute_thread != NULL)
            return false;

        this->_execute_thread = new std::thread(&base_acceptor<T>::execute, this, false);
        return true;
    }
    else
    {
        this->_running = true;
        this->_sockets.clear();
        while (this->_running)
        {
            this->do_session();

            uint64_t now = ::GetTickCount64();
            for(auto timer : this->_timers)
            {
                if(timer->callable(now) == false)
                    continue;

                (this->*(timer->callback()))(now);
                timer->reset(now);
            }
        }

        return true;
    }
}

template<class T>
inline void fb::base_acceptor<T>::exit()
{
    this->_running = false;
    if(this->_execute_thread != NULL)
    {
        this->_execute_thread->join();
        delete this->_execute_thread;
        this->_execute_thread = NULL;
    }
}

template<class T>
inline T* fb::base_acceptor<T>::session(SOCKET fd)
{
    auto                    found = this->_session_table.find(fd);
    if(found == this->_session_table.end())
        return nullptr;

    return found->second;
}

template<class T>
inline std::vector<T*>& fb::base_acceptor<T>::sessions()
{
    return this->_sessions;
}

template<class T>
template<class fn>
void fb::base_acceptor<T>::register_timer(uint32_t interval, fn callback)
{
    this->_timers.push_back(new fb_timer<TimerCallbackFunc>(interval, (TimerCallbackFunc)callback));
}

template <typename T>
bool fb::base_acceptor<T>::handle_accept()
{
    SOCKADDR_IN             address = {0,};
    int                     address_size = sizeof(address);
    auto                    fd = ::accept(*this, (struct sockaddr*) & address, &address_size);
    auto                    session = this->handle_allocate_session(fd);
    if(session == nullptr)
        return false;

    this->_sockets.add(fd, &static_cast<socket&>(*session));
    this->_sessions.push_back(session);
    this->_session_table.insert(std::make_pair(fd, session));

    return this->handle_connected(*session);
}

template<typename T>
inline void fb::base_acceptor<T>::handle_receive(socket& session)
{
    if(session.recv() == false || this->handle_parse(session) == false)
        this->disconnect(session);
}

template<class T>
inline T* fb::base_acceptor<T>::operator[](uint32_t fd) const
{
    return this->session(fd);
}


template<typename fn>
inline fb::fb_timer<fn>::fb_timer(uint64_t interval, fn callback) : _begin(::GetTickCount()), _interval(interval), _callback(callback)
{
}

template<typename fn>
inline fb::fb_timer<fn>::fb_timer(const fb_timer& timer) : fb_timer(timer._interval, timer._callback)
{
}

template<typename fn>
inline fb::fb_timer<fn>::~fb_timer()
{
}

template<typename fn>
inline void fb::fb_timer<fn>::reset(uint64_t now)
{
    this->_begin = now + this->_interval;
}

template<typename fn>
inline bool fb::fb_timer<fn>::callable(uint64_t now) const
{
    return this->_begin + this->_interval < now;
}

template<typename fn>
inline fn fb::fb_timer<fn>::callback()
{
    return this->_callback;
}

template <class T>
fb::fb_acceptor<T>::fb_acceptor(uint16_t port) : base_acceptor<T>(port)
{ }

template <class T>
fb::fb_acceptor<T>::~fb_acceptor()
{ }

template <class T>
template <class fn>
bool fb::fb_acceptor<T>::register_handle(uint8_t cmd, fn handle_login)
{
    auto i = this->_response_table.find(cmd);
    if(i != this->_response_table.end())
        return false;

    this->_response_table.insert(std::make_pair(cmd, (ResponseFunc)handle_login));
    return true;
}

template<class T>
bool fb::fb_acceptor<T>::change_server(T& session, uint32_t ip, uint16_t port)
{
    fb::ostream&                ostream = session.out_stream();
    fb::ostream                 buffer_stream;
    auto&                       crtsocket = static_cast<fb::crtsocket&>(session);
    auto&                       crt = static_cast<fb::cryptor&>(crtsocket);

    buffer_stream.write_u8(0x03)
        .write_u32(ip)          // ip
        .write_u16(port)        // port
        .write_u8(0x0C)         // backward size
        .write_u8(crt.types())
        .write_u8(0x09)
        .write(crt.key(), 0x09)
        .write_u8(0x00);
    crt.wrap(buffer_stream);
    ostream.write(buffer_stream);
    return true;
}

template<class T>
inline bool fb::fb_acceptor<T>::call_handle(T& session, uint8_t cmd)
{
    auto i = this->_response_table.find(cmd);
    if(i == this->_response_table.end())
        return true;

    return (this->*i->second)(session);
}

template<class T>
inline bool fb::fb_acceptor<T>::handle_parse(socket& socket)
{
    static uint8_t      base_size   = sizeof(uint8_t) + sizeof(uint16_t);
    auto&               istream     = socket.in_stream();
    auto                success     = true;

    auto&               crtsocket   = static_cast<fb::crtsocket&>(socket);
    auto&               crt         = static_cast<fb::cryptor&>(crtsocket);

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
            if(this->call_handle(*this->session(socket), cmd) == false)
                throw std::exception();

            // Set data pointer to process next packet bytes
            istream.shift(size);
        }
        catch(...)
        {
            success = false;
            break;
        }
    }

    istream.reset();
    return success;
}

template<class T>
inline bool fb::fb_acceptor<T>::send_stream(fb::base& session, const fb::ostream& stream, bool encrypt, bool wrap)
{
    if(stream.empty())
        return true;

    return session.send(stream, encrypt, wrap);
}
