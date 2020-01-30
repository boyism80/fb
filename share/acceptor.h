#ifndef __FB_ACCEPTOR_H__
#define __FB_ACCEPTOR_H__

#include <thread>
#include <map>
#include <ctime>
#include "zlib/zlib.h"
#include "stream.h"
#include "socket.h"
#include "cryptor.h"

namespace fb {

template <typename fn>
class fb_timer
{
private:
    uint64_t                _begin;
    uint64_t                _interval;
    fn                      _callback;

public:
    fb_timer(uint64_t interval, fn callback);
    fb_timer(const fb_timer& timer);
    ~fb_timer();

public:
    void                    reset(uint64_t now);
    bool                    callable(uint64_t now) const;
    fn                      callback();
};

class base_session : public socket
{
public:
    base_session(SOCKET socket);
    virtual ~base_session();
};

template <class T>
class base_acceptor : private fb::socket
{
public:
    typedef void (base_acceptor<T>::*TimerCallbackFunc)(uint64_t now);
    typedef std::vector<fb_timer<TimerCallbackFunc>*> timer_list;

    typedef std::vector<T*> session_list;

private:
    bool                    _running;
    fb::socket_map          _sockets;
    session_list            _sessions;
    std::thread*            _execute_thread;
    timer_list              _timers;
    

protected:
    base_acceptor(uint16_t port);
    virtual ~base_acceptor();

private:
    bool                    do_session();
    bool                    disconnect(SOCKET fd);

public:
    bool                    execute(bool async = false);
    void                    exit();
    T*                      session(uint32_t fd);
    std::vector<T*>&        sessions();

    template <typename fn>
    void                    register_timer(uint32_t interval, fn callback);

public:
    bool                    handle_accept();
    void                    handle_receive(T& base_session);

public:
    virtual bool            handle_connected(T& base_session) { return true; }
    virtual bool            handle_disconnected(T& base_session) { return true; }
    virtual bool            handle_parse(T& base_session) = 0;

public:
    T*                      operator [] (uint32_t fd) const;
};


class fb_session : public base_session
{
private:
    cryptor                 _cryptor;

public:
    fb_session(SOCKET socket);
    virtual ~fb_session();

public:
    cryptor&                crt();
    void                    crt(const cryptor& cryptor);
    void                    crt(uint8_t types, uint8_t* key);
};


template <class T>
class fb_acceptor : public base_acceptor<T>
{
public:
    typedef bool (fb_acceptor<T>::*ResponseFunc)(T& base_session);
    typedef std::map<uint8_t, ResponseFunc> response_table;

private:
    response_table          _response_table;
    

protected:
    fb_acceptor(uint16_t port);
    virtual ~fb_acceptor();

public:
    template <class fn>
    bool                    register_handle(uint8_t cmd, fn handle_login);
    bool                    change_server(T& base_session, uint32_t ip, uint16_t port);
    bool                    call_handle(T& session, uint8_t cmd);
    virtual bool            handle_parse(T& base_session);

public:
    bool                    send_stream(fb_session& session, const fb::ostream& stream, bool encrypt = true);
};

#include "acceptor.hpp"

}

#endif // !__FB_ACCEPTOR_H__