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

template <typename T>
class acceptor;

template <typename T>
class socket : public boost::asio::ip::tcp::socket
{
private:
    fb::acceptor<T>&        _owner;
    fb::cryptor             _crt;

    std::array<char, 256>   _buffer;
    istream                 _instream;
    T*                      _data;

public:
    socket(fb::acceptor<T>& owner);
    socket(fb::acceptor<T>& owner, const fb::cryptor& crt);
    ~socket();

public:
    void                    send(const ostream& stream, bool encrypt = true, bool wrap = true, bool async = true);
    void                    send(const fb::protocol::base::response& response, bool encrypt = true, bool wrap = true, bool async = true);
    fb::cryptor&            crt();
    void                    crt(const fb::cryptor& crt);
    void                    crt(uint8_t enctype, const uint8_t* enckey);
    void                    recv();
    fb::acceptor<T>&        owner() const;
    fb::istream&            in_stream();
    void                    data(T* value);
    T*                      data();
    std::string             IP() const;

public:
    operator                fb::cryptor& ();
};

template <typename T>
class socket_container : private std::vector<fb::socket<T>*>
{
public:
    template <typename> friend class acceptor;

public:
    using std::vector<fb::socket<T>*>::begin;
    using std::vector<fb::socket<T>*>::end;
    using std::vector<fb::socket<T>*>::size;

private:
    void                    push(fb::socket<T>& session);
    void                    erase(fb::socket<T>& session);
    void                    erase(uint32_t fd);

public:
    fb::socket<T>*          operator [] (uint32_t fd);
};

#include "socket.hpp"

}

#endif // !__SOCKET_H__
