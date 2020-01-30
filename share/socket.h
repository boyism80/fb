#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <winsock2.h>
#include <exception>
#include <map>
#include "stream.h"

namespace fb {

class socket
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
