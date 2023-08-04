#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <exception>
#include <map>
#include <deque>
#include <mutex>
#include <optional>
#include <coroutine>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <fb/protocol/protocol.h>
#include <fb/core/cryptor.h>
#include <fb/core/stream.h>

namespace fb { namespace base {

template <typename T = void*>
class socket : public boost::asio::ip::tcp::socket
{
public:
    using handler_event = std::function<void(fb::base::socket<T>&)>;

private:
    handler_event           _handle_received;
    handler_event           _handle_closed;

protected:
    std::array<char, 256>   _buffer;
    istream                 _instream;
    T*                      _data;
    uint32_t                _fd;

public:
    std::mutex              mutex;

protected:
    socket(boost::asio::io_context& context, const handler_event& handle_receive, const handler_event& handle_closed);
    ~socket() = default;

protected:
    virtual bool            on_encrypt(fb::ostream& out) { return true; }
    virtual bool            on_wrap(fb::ostream& out) { return true; }

public:
    void                    send(const ostream& stream, bool encrypt = true, bool wrap = true);
    void                    send(const ostream& stream, bool encrypt, bool wrap, std::function<void(const boost::system::error_code, size_t)> callback);
    void                    send(const fb::protocol::base::header& header, bool encrypt = true, bool wrap = true);
    void                    send(const fb::protocol::base::header& header, bool encrypt, bool wrap, std::function<void(const boost::system::error_code, size_t)> callback);

    void                    recv();
    fb::istream&            in_stream();
    void                    data(T* value);
    T*                      data();
    std::string             IP() const;
    uint32_t                fd();
};

template <template<class> class S, class T>
class acceptor;

template <template<class> class S, class T>
class socket_container : private std::map<uint32_t, std::unique_ptr<S<T>>>
{
public:
    friend class acceptor<S, T>;

public:
    using std::map<uint32_t, std::unique_ptr<S<T>>>::begin;
    using std::map<uint32_t, std::unique_ptr<S<T>>>::end;
    using std::map<uint32_t, std::unique_ptr<S<T>>>::size;
    using std::map<uint32_t, std::unique_ptr<S<T>>>::empty;

private:
    void                    push(std::unique_ptr<S<T>>&& session);
    void                    erase(S<T>& session);
    void                    erase(uint32_t fd);

public:
    S<T>*                   operator [] (uint32_t fd) const;
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

template <typename T>
class awaitable_socket : public fb::base::socket<T>
{
public:
    template <typename R>
    class awaitable
    {
        using suspend_handler = std::function<void(awaitable&)>;

    private:
        awaitable_socket<T>&            _owner;
        const suspend_handler           _on_suspend;

    public:
        R*                              result;
        std::optional<std::exception>   e;
        std::coroutine_handle<>         handler;

    public:
        awaitable(awaitable_socket<T>& owner, uint8_t cmd, const suspend_handler& on_suspend);
        ~awaitable();

        bool                        await_ready();
        void                        await_suspend(std::coroutine_handle<> h);
        R                           await_resume();
    };

private:
    std::mutex                      _awaiter_mutex;
    std::map<uint8_t, void*>        _coroutines;

protected:
    awaitable_socket(boost::asio::io_context& context, const fb::base::socket<T>::handler_event& handle_receive, const fb::base::socket<T>::handler_event& handle_closed);
public:
    ~awaitable_socket();

private:
    template <typename R>
    void                            register_awaiter(uint8_t cmd, awaitable<R>* awaiter);

public:
    template <typename R>
    void                            invoke_awaiter(uint8_t cmd, R& response);

public:
    template <typename R>
    auto                            request(const fb::protocol::base::header& header, bool encrypt = true, bool wrap = true);
};

struct task
{
    struct promise_type
    {
        auto get_return_object() { return task{}; }
        auto initial_suspend() { return std::suspend_never{}; }
        auto final_suspend() noexcept { return std::suspend_never{}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

}

namespace fb { namespace internal {

template <typename T = void*>
class socket : public awaitable_socket<T>
{
public:
    socket(boost::asio::io_context& context, const fb::base::socket<T>::handler_event& handle_receive, const fb::base::socket<T>::handler_event& handle_closed);
    ~socket();

protected:
    bool                            on_wrap(fb::ostream& out);
};

} }

#include "socket.hpp"

#endif // !__SOCKET_H__
