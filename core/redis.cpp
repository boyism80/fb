#include <fb/core/redis.h>

fb::task<void> fb::redis::handle_locked(fb::awaiter<int>& awaiter, const std::function<fb::task<int>(std::shared_ptr<fb::mst>&)>& fn, std::shared_ptr<fb::mst> current, const std::string key, const std::string uuid, std::shared_ptr<cpp_redis::client> conn, std::shared_ptr<cpp_redis::subscriber> subs)
{
    try
    {
        auto result = co_await fn(current);
        awaiter.result = &result;

        if (current->parent == nullptr)
            this->_root->add(current);
    }
    catch (std::exception& e)
    {
        awaiter.error = std::make_unique<std::runtime_error>(e.what());
    }

    conn->del({ key }, [this, subs, key](auto& reply) mutable
    {
        subs->unsubscribe(key);
        subs->commit();
        this->subs.release(subs);
    });
    conn->publish(key, uuid, [this, key, conn, &awaiter](auto& reply) mutable
    {
        this->conn.release(conn);
        awaiter.handler.resume();
    });
    conn->commit();
}

bool fb::redis::try_lock(const std::string& key, fb::awaiter<int>& awaiter, const std::function<fb::task<int>(std::shared_ptr<fb::mst>&)>& fn, const std::string& uuid, std::shared_ptr<cpp_redis::client> conn, std::shared_ptr<cpp_redis::subscriber> subs, fb::thread* thread, std::shared_ptr<fb::mst>& trans)
{
    auto current = std::make_shared<fb::mst>(key, trans);

    if(trans != nullptr)
        trans->add(current);

    try
    {
        auto root = current->root();
        root->assert_circulated_lock();
        this->_root->assert_dead_lock(root);
    }
    catch (std::exception& e)
    {
        awaiter.error = std::make_unique<std::runtime_error>(e.what());
        awaiter.handler.resume();
        return false;
    }

    conn->evalsha(this->_scripts["lock"], 1, { key }, { std::to_string(this->_timeout) }, [this, conn, subs, key, uuid, thread, &awaiter, &fn, trans, current](cpp_redis::reply& v) mutable
    {
        auto success = v.as_integer() == 1;
        if (success == false)
            return;

        if (thread != nullptr)
        {
            thread->dispatch([this, &awaiter, &fn, current, key, uuid, conn, subs](uint8_t) mutable 
            {
                this->handle_locked(awaiter, fn, current, key, uuid, conn, subs);
            });
        }
        else
        {
            this->handle_locked(awaiter, fn, current, key, uuid, conn, subs);
        }
    });
    conn->commit();
    return true;
}

fb::awaiter<int> fb::redis::sync(const std::string& key, const std::function<fb::task<int>(std::shared_ptr<fb::mst>&)>& fn, std::shared_ptr<fb::mst>& trans)
{
    return fb::awaiter<int>([this, key, &fn, trans](auto& awaiter) mutable
    {
        auto conn = this->conn.get();
        auto subs = this->subs.get();
        auto thread = this->_owner.current_thread();
        auto uuid = boost::uuids::to_string(boost::uuids::random_generator()());
        subs->subscribe(key, [this, key, uuid, conn, subs, &awaiter, &fn, thread, trans](const std::string& chan, const std::string& msg) mutable
        {
            if (chan != key)
                return;

            if (msg != uuid)
                this->try_lock(key, awaiter, fn, uuid, conn, subs, thread, trans);
        });

        if (this->try_lock(key, awaiter, fn, uuid, conn, subs, thread, trans))
            subs->commit();
    });
}

fb::awaiter<int> fb::redis::sync(const std::string& key, const std::function<fb::task<int>(std::shared_ptr<fb::mst>&)>& fn)
{
    auto empty_ptr = std::shared_ptr<fb::mst>(nullptr);
    return this->sync(key, fn, empty_ptr);
}