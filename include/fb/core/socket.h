#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <exception>
#include <map>
#include <deque>
#include <mutex>
#include <optional>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/system/error_code.hpp>
#include <fb/protocol/protocol.h>
#include <fb/core/cryptor.h>
#include <fb/core/stream.h>
#include <fb/core/coroutine.h>

namespace fb { namespace base {

template <typename T = void*>
class socket : public boost::asio::ip::tcp::socket
{
public:
    using handler_event = std::function<fb::task<void>(fb::base::socket<T>&)>;

private:
    handler_event                   _handle_received;
    handler_event                   _handle_closed;
    uint32_t                        _fd = 0xFFFFFFFF;
    istream                         _instream;
    std::recursive_mutex            _instream_mutex;
    std::vector<fb::task<void>>     _tasks;
    std::recursive_mutex            _tasks_mutex;

protected:
    std::array<char, 256>           _buffer;
    T*                              _data;
    std::recursive_mutex            _boost_mutex, _task_mutex;
    std::vector<fb::task<bool>>     _unfinished_tasks;
//
//public:
//    std::mutex              stream_mutex;

protected:
    socket(boost::asio::io_context& context, const handler_event& handle_receive, const handler_event& handle_closed);
    ~socket();

protected:
    virtual bool            on_encrypt(fb::ostream& out) { return true; }
    virtual bool            on_wrap(fb::ostream& out) { return true; }

public:
    void                    send(const ostream& stream, bool encrypt = true, bool wrap = true);
    void                    send(const ostream& stream, bool encrypt, bool wrap, std::function<void(const boost::system::error_code&, size_t)> callback);
    void                    send(const fb::protocol::base::header& header, bool encrypt = true, bool wrap = true);
    void                    send(const fb::protocol::base::header& header, bool encrypt, bool wrap, std::function<void(const boost::system::error_code&, size_t)> callback);

    void                    recv();
    void                    data(T* value);
    T*                      data() const;
    std::string             IP() const;
    uint32_t                fd();
    void                    push_task(fb::task<bool>&& task);
    bool                    flush_task();
    
    template<typename R = void>
    R in_stream(const std::function<R(fb::istream& in_stream)>& func)
    {
        auto _ = std::lock_guard(this->_instream_mutex);
        return func(this->_instream);
    }

    template<>
    void in_stream(const std::function<void(fb::istream& in_stream)>& func)
    {
        auto _ = std::lock_guard(this->_instream_mutex);
        func(this->_instream);
    }
};

template <template<class> class S, class T>
class acceptor;

template <template<class> class S, class T>
class socket_container : private std::map<uint32_t, std::unique_ptr<S<T>>>
{
private:
    std::mutex              mutex;

public:
    friend class acceptor<S, T>;

public:
    socket_container() = default;
    ~socket_container();

public:
    using std::map<uint32_t, std::unique_ptr<S<T>>>::begin;
    using std::map<uint32_t, std::unique_ptr<S<T>>>::end;
    using std::map<uint32_t, std::unique_ptr<S<T>>>::size;

private:
    void                    push(std::unique_ptr<S<T>>&& session);
    void                    erase(S<T>& session);
    void                    erase(uint32_t fd);
    bool                    empty();

public:
    bool                    contains(uint32_t fd);
    void                    each(const std::function<void(S<T>&)> fn);
    S<T>*                   find(const std::function<bool(S<T>&)> fn);
    void                    close();

public:
    S<T>*                   operator [] (uint32_t fd);
};

} }

namespace fb {

template <typename T>
class socket : public fb::base::socket<T>
{
private:
    fb::cryptor             _crt;

public:
    socket(boost::asio::io_context& context, const fb::base::socket<T>::handler_event& handle_receive, const fb::base::socket<T>::handler_event& handle_closed);
    socket(boost::asio::io_context& context, const fb::cryptor& crt, const fb::base::socket<T>::handler_event& handle_receive, const fb::base::socket<T>::handler_event& handle_closed);
    ~socket() = default;
    
protected:
    bool                    on_encrypt(fb::ostream& out);
    bool                    on_wrap(fb::ostream& out);

public:
    fb::cryptor&            crt();
    void                    crt(const fb::cryptor& crt);
    void                    crt(uint8_t enctype, const uint8_t* enckey);

public:
    operator                fb::cryptor& ();
};

template <typename T, typename C = uint8_t>
class awaitable_socket : public fb::base::socket<T>
{
private:
    std::mutex                      _awaiter_mutex;
    std::map<C, std::any>           _coroutines;

protected:
    awaitable_socket(boost::asio::io_context& context, const fb::base::socket<T>::handler_event& handle_receive, const fb::base::socket<T>::handler_event& handle_closed);
public:
    ~awaitable_socket();

private:
    template <typename R>
    void                            register_awaiter(C cmd, std::shared_ptr<fb::awaiter<R>>& awaiter);

public:
    template <typename R>
    void                            invoke_awaiter(C cmd, R& response);

public:
    template <typename R>
    fb::task<R, std::suspend_always>&   request(const fb::protocol::base::header& header, bool encrypt = true, bool wrap = true);

    template <typename R>
    fb::task<R, std::suspend_always>&   request(const fb::protocol::internal::header& header, bool encrypt = true, bool wrap = true);
};

}

namespace fb { namespace internal {

template <typename T = void*>
class socket : public awaitable_socket<T, std::string>
{
public:
    using handler_event = fb::base::socket<T>::handler_event;

public:
    socket(boost::asio::io_context& context, const handler_event& handle_receive, const handler_event& handle_closed);
    ~socket();

protected:
    bool                            on_wrap(fb::ostream& out);
};

} }

#include "socket.hpp"

#endif // !__SOCKET_H__
