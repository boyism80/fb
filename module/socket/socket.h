#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <exception>
#include <map>
#include <deque>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include "module/stream/stream.h"
#include "module/cryptor/cryptor.h"
#include "protocol/protocol.h"

namespace fb {

template <template<class> class S, class T>
class acceptor;

template <typename T>
class base_socket : public boost::asio::ip::tcp::socket
{
private:
    std::function<void(fb::base_socket<T>&)> _handle_receive;
    std::function<void(fb::base_socket<T>&)> _handle_closed;

protected:
    std::array<char, 256>   _buffer;
    istream                 _instream;
    T*                      _data;

public:
    base_socket(boost::asio::io_context& context, std::function<void(fb::base_socket<T>&)> handle_receive, std::function<void(fb::base_socket<T>&)> handle_closed);
    ~base_socket();

public:
    void                    send(const ostream& stream);
    void                    send(const fb::protocol::base::response& response);

    void                    recv();
    fb::istream&            in_stream();
    void                    data(T* value);
    T*                      data();
    std::string             IP() const;
};

template <typename T>
class socket : public fb::base_socket<T>
{
private:
    fb::cryptor             _crt;

public:
    socket(boost::asio::io_context& context, std::function<void(fb::base_socket<T>&)> handle_receive, std::function<void(fb::base_socket<T>&)> handle_closed);
    socket(boost::asio::io_context& context, const fb::cryptor& crt, std::function<void(fb::base_socket<T>&)> handle_receive, std::function<void(fb::base_socket<T>&)> handle_closed);
    ~socket();
    
private:
    virtual void            on_encrypt(fb::ostream& out);
    virtual void            on_wrap(fb::ostream& out);

public:
    fb::cryptor&            crt();
    void                    crt(const fb::cryptor& crt);
    void                    crt(uint8_t enctype, const uint8_t* enckey);

    void                    send(const ostream& stream, bool encrypt = true, bool wrap = true);
    void                    send(const fb::protocol::base::response& response, bool encrypt = true, bool wrap = true);

public:
    operator                fb::cryptor& ();
};



template <template<class> class S, class T>
class socket_container : private std::vector<S<T>*>
{
public:
    friend class acceptor<S, T>;

public:
    using std::vector<S<T>*>::begin;
    using std::vector<S<T>*>::end;
    using std::vector<S<T>*>::size;

private:
    void                    push(S<T>& session);
    void                    erase(S<T>& session);
    void                    erase(uint32_t fd);

public:
    S<T>*                   operator [] (uint32_t fd);
};

#include "socket.hpp"

}

#endif // !__SOCKET_H__
