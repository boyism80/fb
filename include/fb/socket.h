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
#include <fb/cryptor.h>
#include <fb/stream.h>
#include <async/task.h>
#include <async/task_completion_source.h>
#include <async/awaitable_get.h>

namespace fb {

template <typename T = void*>
class socket : public boost::asio::ip::tcp::socket
{
public:
    using handler_event = std::function<async::task<void>(fb::socket<T>&)>;

private:
    fb::cryptor                     _crt;
    handler_event                   _handle_received;
    handler_event                   _handle_closed;
    uint32_t                        _fd = 0xFFFFFFFF;
    istream                         _instream;
    std::recursive_mutex            _instream_mutex;

protected:
    std::array<char, 256>           _buffer;
    T*                              _data;
    std::recursive_mutex            _boost_mutex, _task_mutex;
    std::vector<async::task<bool>>  _unfinished_tasks;
//
//public:
//    std::mutex              stream_mutex;

public:
    socket(boost::asio::io_context& context, const handler_event& handle_receive, const handler_event& handle_closed);
    socket(boost::asio::io_context& context, const fb::cryptor& crt, const handler_event& handle_receive, const handler_event& handle_closed);
    ~socket();

protected:
    virtual bool                    on_encrypt(fb::ostream& out);
    virtual bool                    on_wrap(fb::ostream& out);

public:
    void                            send(const ostream& stream, bool encrypt = true, bool wrap = true);
    void                            send(const ostream& stream, bool encrypt, bool wrap, std::function<void(const boost::system::error_code&, size_t)> callback);
    void                            send(const fb::protocol::base::header& header, bool encrypt = true, bool wrap = true);
    void                            send(const fb::protocol::base::header& header, bool encrypt, bool wrap, std::function<void(const boost::system::error_code&, size_t)> callback);

    void                            recv();
    void                            data(T* value);
    T*                              data() const;
    std::string                     IP() const;
    uint32_t                        fd();

public:
    fb::cryptor&                    crt();
    void                            crt(const fb::cryptor& crt);
    void                            crt(uint8_t enctype, const uint8_t* enckey);

public:
    operator                        fb::cryptor& ();
    
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

template <typename T>
class acceptor;

template <typename T>
class socket_container : private std::map<uint32_t, std::unique_ptr<fb::socket<T>>>
{
private:
    std::mutex              mutex;

public:
    friend class acceptor<T>;

public:
    socket_container() = default;
    ~socket_container();

public:
    using std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::begin;
    using std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::end;
    using std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::size;

private:
    void                    push(std::unique_ptr<fb::socket<T>>&& session);
    void                    erase(fb::socket<T>& session);
    void                    erase(uint32_t fd);
    bool                    empty();

public:
    bool                    contains(uint32_t fd);
    void                    each(const std::function<void(fb::socket<T>&)> fn);
    fb::socket<T>*          find(const std::function<bool(fb::socket<T>&)> fn);
    void                    close();

public:
    fb::socket<T>*          operator [] (uint32_t fd);
};

}

#include "socket.hpp"

#endif // !__SOCKET_H__
