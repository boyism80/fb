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
private:
    std::function<void(fb::base::socket<T>&)> _handle_received;
    std::function<void(fb::base::socket<T>&)> _handle_closed;

protected:
    std::array<char, 256>   _buffer;
    istream                 _instream;
    T*                      _data;
    uint32_t                _fd;

public:
    std::mutex              mutex;

protected:
    socket(boost::asio::io_context& context, const std::function<void(fb::base::socket<T>&)>& handle_receive, const std::function<void(fb::base::socket<T>&)>& handle_closed);
    ~socket() = default;

protected:
    virtual bool            on_encrypt(fb::ostream& out) { return true; }
    virtual bool            on_wrap(fb::ostream& out) { return true; }

public:
    void                    send(const ostream& stream, bool encrypt = true, bool wrap = true);
    void                    send(const fb::protocol::base::header& header, bool encrypt = true, bool wrap = true);

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
    socket(boost::asio::io_context& context, const std::function<void(fb::base::socket<T>&)>& handle_receive, const std::function<void(fb::base::socket<T>&)>& handle_closed);
    socket(boost::asio::io_context& context, const fb::cryptor& crt, const std::function<void(fb::base::socket<T>&)>& handle_receive, const std::function<void(fb::base::socket<T>&)>& handle_closed);
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

}

namespace fb { namespace internal {

template <typename T = void*>
class socket : public fb::base::socket<T>
{
public:
    template <typename R>
    class awaitable
    {
    private:
        socket<T>&                      _owner;
        const std::function<void()>     _on_suspend;

    public:
        R*                              result;
        std::optional<std::exception>   e;
        std::coroutine_handle<>         handler;

    public:
        awaitable(socket<T>& owner, uint8_t cmd, const std::function<void()>& on_suspend);
        ~awaitable();

        bool                        await_ready();
        void                        await_suspend(std::coroutine_handle<> h);
        R                           await_resume();
    };

private:
    std::mutex                      _awaiter_mutex;
    std::map<uint8_t, void*>        _coroutines;


public:
    socket(boost::asio::io_context& context, const std::function<void(fb::base::socket<T>&)>& handle_receive, const std::function<void(fb::base::socket<T>&)>& handle_closed);
    ~socket();

protected:
    bool                    on_wrap(fb::ostream& out);

public:
    void                    register_awaiter(uint8_t cmd, void* awaiter);

    template <typename R>
    void                    invoke_awaiter(uint8_t cmd, R& response);

public:
    template <typename R>
    auto                    request(const fb::protocol::base::header& header, bool encrypt = true, bool wrap = true);
};

} }

#include "socket.hpp"

#endif // !__SOCKET_H__
