#include <fb/login/handler/timer/name_keep_alive.h>

#include <fb/config.h>
#include <fb/logger.h>
#include <fb/protocol/flatbuffer/protocol.h>

#include <cstdint>
#include <exception>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace internal_reqs = fb::protocol::internal::request;

namespace fb::login::handler::timer {

name_keep_alive::name_keep_alive(fb::login::server& server) :
    fb::handler::timer<fb::login::server>(server)
{ }

async::task<void> name_keep_alive::handle()
{
    auto sockets = std::unordered_map<fb::thread*, std::vector<std::shared_ptr<fb::socket<fb::login::session>>>>();
    this->server.access_sockets([&sockets](const auto& container) {
        for (auto& [fd, socket] : container)
        {
            auto thread = socket->thread();
            if (thread != nullptr)
                sockets[thread].push_back(socket);
        }
    });

    // pending_uid is written by protocol handlers on the session thread.
    auto uids = std::vector<uint32_t>();
    for (auto& [thread, list] : sockets)
    {
        co_await thread->switching();
        for (auto& socket : list)
        {
            auto data = socket->data_ptr();
            if (data != nullptr && data->pending_uid.has_value())
                uids.push_back(data->pending_uid.value());
        }
    }

    auto world = fb::config<uint32_t>("world");
    for (auto uid : uids)
    {
        try
        {
            auto&& resp = co_await this->server.http.post("internal",
                                                          "/account/name-keepalive",
                                                          internal_reqs::ReserveName{world, std::string{}, uid});
            if (resp.success == false)
                fb::logger::warn("Name reservation {} is no longer held", uid);
        }
        catch (const std::exception& e)
        {
            fb::logger::warn("Failed to refresh name reservation: {}", e.what());
        }
    }
}

} // namespace fb::login::handler::timer
