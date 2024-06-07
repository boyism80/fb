#include <fb/core/redis.h>

void fb::redis::try_lock(const std::string& key, fb::awaiter<int>& awaiter, const std::function<fb::task<int>(std::shared_ptr<fb::mst>&)>& fn, const std::string& uuid, std::shared_ptr<cpp_redis::client> conn, std::shared_ptr<cpp_redis::subscriber> subs, fb::thread* thread, std::shared_ptr<fb::mst>& trans)
{
    fb::logger::debug("lock id : %s", key.c_str());

    auto is_root = (trans == nullptr);
    auto current = std::make_shared<fb::mst>(key, trans);

    trans->add(current);
    //if (is_root)
    //{
    //    auto routes = current->routes();
    //    auto contains = std::all_of(routes.cbegin(), routes.cend(), [this](const fb::mst::node_route& route) mutable
    //        {
    //            auto keys = fb::mst::keys(route);
    //            return this->_routes.contains(keys);
    //        });

    //    if (!contains)
    //    {
    //        current_ptr = this->_root.add(*current_ptr);
    //        for (auto& route : routes)
    //        {
    //            this->_routes.insert(fb::mst::keys(route));
    //        }
    //    }
    //}

    try
    {
        current->root()->assert_dead_lock();
    }
    catch (std::exception& e)
    {
        awaiter.error = std::make_unique<std::runtime_error>(e.what());
        awaiter.handler.resume();
        return;
    }

    conn->evalsha(this->_scripts["lock"], 1, { key }, { std::to_string(this->_timeout) }, [this, conn, subs, key, uuid, thread, &awaiter, &fn, trans, current](cpp_redis::reply& v) mutable
    {
        auto success = v.as_integer() == 1;
        if (success == false)
            return;

        auto handler = [this, &fn, conn, subs, key, uuid, trans, current](auto& awaiter) mutable -> fb::task<void>
        {
            auto clone_key = std::string(key);
            auto clone_conn = conn;
            auto clone_subs = subs;
            auto clone_uuid = uuid;
            auto& subs_pool = this->subs;
            auto& conn_pool = this->conn;

            try
            {
                auto result = co_await fn(current);
                awaiter.result = &result;
            }
            catch (std::exception& e)
            {
                awaiter.error = std::make_unique<std::runtime_error>(e.what());
            }

            fb::logger::debug("delete lock key : %s", clone_key.c_str());
            clone_conn->del({ clone_key }, [&subs_pool, clone_subs, clone_key](auto& reply) mutable
            {
                    clone_subs->unsubscribe(clone_key);
                    clone_subs->commit();
                    subs_pool.release(clone_subs);
            });
            clone_conn->publish(clone_key, clone_uuid, [clone_key, &conn_pool, clone_conn, &awaiter](auto& reply) mutable
            {
                    conn_pool.release(clone_conn);
                    fb::logger::debug("awaiter address : 0x%X", &awaiter);
                awaiter.handler.resume();
            });
            clone_conn->commit();
        };
        
        if (thread != nullptr)
        {
            thread->dispatch([this, handler, &awaiter](uint8_t) mutable { handler(awaiter); });
        }
        else
        {
            handler(awaiter);
        }
    });
    conn->commit();
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
        subs->commit();
        this->try_lock(key, awaiter, fn, uuid, conn, subs, thread, trans);
    });
}

fb::awaiter<int> fb::redis::sync(const std::string& key, const std::function<fb::task<int>(std::shared_ptr<fb::mst>&)>& fn)
{
    return this->sync(key, fn, this->_root);
}