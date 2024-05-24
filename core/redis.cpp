#include <fb/core/redis.h>

void fb::redis::try_lock(const std::string& key, fb::awaitable<int>& awaitable, const std::function<int()>& fn, const std::string& uuid, std::shared_ptr<cpp_redis::client> conn, std::shared_ptr<cpp_redis::subscriber> subs, fb::thread* thread)
{
    auto script =
        "local success = redis.call('setnx', KEYS[1], 'lock')\n"
        "if success == 1 then\n"
        "    -- redis.call('expire', KEYS[1], 5)\n"
        "end\n"
        "return success\n";


    conn->eval(script, 1, { key }, {}, [this, conn, subs, key, uuid, thread, &awaitable, &fn](cpp_redis::reply& v) mutable
        {
            auto success = v.as_integer();
            if (success == 1)
            {
                fb::logger::debug("LOCK");
                auto handler = [this, &fn, conn, subs, key, uuid, &awaitable](uint8_t)
                {
                    auto result = fn();
                    awaitable.result = &result;
                    awaitable.handler.resume();

                    conn->del({ key }, [this, conn, subs, key](auto& reply)  mutable
                    {
                        auto deleted = reply.as_integer();
                        if (deleted != 1)
                        {
                            fb::logger::debug("ERROR");
                        }
                        else
                        {
                            fb::logger::debug("UNLOCK");
                            subs->unsubscribe(key);
                            subs->commit();
                            this->_pool.subs.release(subs);
                        }
                    });
                    conn->publish(key, uuid, [this, conn](auto& reply)  mutable
                    {
                        this->_pool.conn.release(conn);
                    });
                    conn->commit();
                };

                if (thread != nullptr)
                    thread->dispatch(handler);
                else
                    handler(-1);
            }
            else if(success == -100)
            {
                //fb::logger::debug("리소스 점유에 실패하여 대기");
            }
            else
            {
                fb::logger::debug("RESULT CODE : %d", success);
            }
        });
    conn->commit();
}

fb::awaitable<int> fb::redis::sync(const std::string& key, const std::function<int()>& fn)
{
    auto conn = this->_pool.conn.get();
    auto subs = this->_pool.subs.get();
    return fb::awaitable<int>([this, key, &fn, conn, subs](auto& awaitable) mutable
    {
        auto thread = this->_owner.current_thread();
        auto uuid = boost::uuids::to_string(boost::uuids::random_generator()());
        subs->subscribe(key, [&, this, key, uuid, conn, subs, thread](const std::string& chan, const std::string& msg) mutable
        {
            if (chan != key)
                return;

            if (msg != uuid)
                this->try_lock(key, awaitable, fn, uuid, conn, subs, thread);
        });
        subs->commit();
        this->try_lock(key, awaitable, fn, uuid, conn, subs, thread);
    });
}