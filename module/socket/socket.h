#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <winsock2.h>
#include <exception>
#include <map>
#include <deque>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <module/stream/stream.h>
#include <module/crypto/cryptor.h>
#include <protocol/protocol.h>

namespace fb {

class base_acceptor;

class base
{
private:
    uint32_t                _id;

protected:
    base() : _id(0xFFFFFFFF) {}
    base(uint32_t id) : _id(id) {}
    virtual ~base() {}

public:
    uint32_t                id() const { return this->_id; }
    void                    id(uint32_t value) { this->_id = value; }

    virtual void            send(const fb::ostream& stream, bool encrypt = true, bool wrap = true, bool async = true) { }
    void                    send(const fb::protocol::base::response& response, bool encrypt = true, bool wrap = true, bool async = true);
};

class socket : public boost::asio::ip::tcp::socket, public virtual base
{
private:
    base_acceptor*          _owner;
    fb::cryptor             _crt;

    std::array<char, 256>   _buffer;
    istream                 _instream;

public:
    socket(base_acceptor* owner);
    socket(base_acceptor* owner, const fb::cryptor& crt);
    ~socket();

public:
    using base::send;
    void                    send(const ostream& stream, bool encrypt = true, bool wrap = true, bool async = true);
    fb::cryptor&            crt();
    void                    crt(const fb::cryptor& crt);
    void                    crt(uint8_t enctype, const uint8_t* enckey);
    void                    recv();
    base_acceptor*          owner() const;
    fb::istream&            in_stream();

public:
    operator                fb::cryptor& ();
};


template <typename T>
class session_map : private std::map<const fb::socket*, T*>
{
public:
    using std::map<const fb::socket*, T*>::begin;
    using std::map<const fb::socket*, T*>::end;
    using std::map<const fb::socket*, T*>::size;
    using std::map<const fb::socket*, T*>::erase;

public:
    session_map();
    ~session_map();

public:
    T*                      find(const fb::socket* key);
    T*                      find(const fb::socket& key);
    T*                      push(const fb::socket* key, T* val);
    void                    erase(const fb::socket& key);

public:
    T*                      operator [] (const fb::socket* key);
    T*                      operator [] (const fb::socket& key);
};


template <typename T>
class session_container : private std::vector<T*>
{
public:
    template <typename> friend class acceptor;

public:
    using std::vector<T*>::begin;
    using std::vector<T*>::end;
    using std::vector<T*>::size;

private:
    void                    push(T* session);
    void                    erase(T* session);
    void                    erase(uint32_t fd);

public:
    T*                      operator [] (uint32_t fd);
};

#include "socket.hpp"

}

#endif // !__SOCKET_H__
