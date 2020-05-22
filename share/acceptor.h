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

template <typename T>
class session_container : private std::vector<T*>
{
public:
    template <typename> friend class base_acceptor;

public:
    using std::vector<T*>::begin;
    using std::vector<T*>::end;
    using std::vector<T*>::size;

public:
    T*                      find(const std::string& name) const;
};


template <class T>
class base_acceptor : private fb::socket
{
public:
    typedef void (base_acceptor<T>::*TimerCallbackFunc)(uint64_t now);
    typedef std::vector<fb_timer<TimerCallbackFunc>*> timer_list;

private:
    bool                    _running;
    fb::socket_map          _sockets;
    std::map<SOCKET, T*>    _session_table;
    std::thread*            _execute_thread;
    timer_list              _timers;

public:
    session_container<T>    sessions;
    

protected:
    base_acceptor(uint16_t port);
    virtual ~base_acceptor();

private:
    bool                    do_session();
    bool                    disconnect(SOCKET fd);

public:
    bool                    execute(bool async = false);
    void                    exit();
    T*                      session(SOCKET fd);

    template <typename fn>
    void                    register_timer(uint32_t interval, fn callback);

public:
    bool                    handle_accept();
    void                    handle_receive(socket& base_session);

public:
    virtual bool            handle_connected(T& base_session) { return true; }
    virtual bool            handle_disconnected(T& base_session) { return true; }
    virtual bool            handle_parse(socket& base_session) = 0;
    virtual T*              handle_allocate_session(SOCKET socket) = 0;

public:
    T*                      operator [] (uint32_t fd) const;
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
    virtual bool            handle_parse(socket& base_session);

public:
    bool                    send_stream(fb::base& session, const fb::ostream& stream, bool encrypt = true, bool wrap = true);
};

#include "acceptor.hpp"

}

#endif // !__FB_ACCEPTOR_H__