#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <winsock2.h>
#include <exception>
#include <map>
#include "stream.h"
#include "cryptor.h"

namespace fb {

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

    virtual bool            send(const fb::ostream& stream, bool encrypt, bool wrap = true) { return true; }
};

class socket : public base
{
private:
    SOCKET                  _fd;
    istream                 _istream;
    ostream                 _ostream;

protected:
    socket(SOCKET socket);
public:
    virtual ~socket();

protected:
    bool                    valid() const;

public:
    bool                    send();
    bool                    recv();
    void                    close();

public:
    istream&                in_stream();
    ostream&                out_stream();

public:
    operator                SOCKET () const;
};

class crtsocket : public socket
{
private:
    fb::cryptor             _crt;

public:
    crtsocket(SOCKET fd);
    crtsocket(SOCKET fd, const fb::cryptor& crt);
    virtual ~crtsocket();

public:
    bool                    send(const fb::ostream& stream, bool encrypt = true, bool wrap = true);
    void                    crt(const fb::cryptor& crt);
    void                    crt(uint8_t enctype, const uint8_t* enckey);

    // operator
public:
    operator                fb::cryptor& ();
};


class socket_map : private std::map<SOCKET, socket*>
{
private:
    fd_set                  _fd_set;
    socket*                 _root;

public:
    using std::map<SOCKET, socket*>::find;
    using std::map<SOCKET, socket*>::begin;
    using std::map<SOCKET, socket*>::end;

public:
    socket_map();
    ~socket_map();

public:
    bool                    add(SOCKET fd, socket* socket);
    bool                    root(SOCKET fd, socket* socket);
    bool                    remove(SOCKET fd);
    void                    clear();
    socket*                 get(SOCKET fd) const;
    bool                    contains(SOCKET fd) const;

public:
    socket*                 operator [] (SOCKET fd);
    operator                fd_set& ();
};

}

#endif // !__SOCKET_H__
