#include <fb/login/handler/protocol/create_account.h>

using namespace fb::login::handler::protocol;

create_account::create_account(fb::login::server& server) :
    fb::handler::protocol<fb::login::server, fb::protocol::login::request::create>(server)
{ }

async::task<bool> create_account::handle(fb::socket<fb::login::session>& session, fb::protocol::login::request::create& request)
{
    auto fd   = session.fd();
    auto weak = session.weak_from_this_as<fb::socket<fb::login::session>>();

    try
    {
        auto name = std::string(request.id);
        auto pw   = std::string(request.pw);

        this->server.assert_account(name, pw);
        auto&& response1 = co_await this->server.http.post("internal", "/account/reserve", ReserveName{name});
        co_await this->server.threads.switching(weak);

        if (response1.success == false)
            throw id_exception("이미 존재하는 이름입니다.");

        auto        uid    = response1.uid;
        static auto device = std::random_device{};
        static auto gen    = std::mt19937{device()};
        static auto dist   = std::uniform_int_distribution<uint32_t>{0, 0xFFFFFFFF};

        auto   i         = dist(gen) % config<>("init:position").size();
        auto   init_x    = static_cast<uint16_t>(config<>("init:position")[i]["x"].asUInt());
        auto   init_y    = static_cast<uint16_t>(config<>("init:position")[i]["y"].asUInt());
        auto   admin     = fb::config<bool>("admin_mode");
        auto&& response2 = co_await this->server.http.post("internal",
                                                           "/account/init",
                                                           InitCharacter{
                                                               uid,
                                                               name,
                                                               pw,
                                                               fb::config<uint32_t>("init:hp:base") + dist(gen) % fb::config<uint32_t>("init:hp:range"), // hp
                                                               fb::config<uint32_t>("init:mp:base") + dist(gen) % fb::config<uint32_t>("init:mp:range"), // mp
                                                               fb::config<uint16_t>("init:map"),                                                         // map
                                                               init_x,                                                                                   // position_x
                                                               init_y,                                                                                   // position_y
                                                               static_cast<uint8_t>(admin ? ROLE::ADMIN : ROLE::USER),                                   // admin
                                                           });

        co_await this->server.threads.switching(weak);

        if (response2.success == false)
            throw id_exception("이미 존재하는 이름입니다.");

        this->server.send(session, response::message("", 0x00));
        auto session_data  = session.data();
        session_data->pk   = uid;
        session_data->name = name;
        co_return true;
    }
    catch (login_exception& e)
    {
        if (weak.expired() == false)
            session.send(response::message(e.what(), e.type()));
    }
    catch (std::exception& e)
    {
        if (weak.expired() == false)
            session.send(response::message(e.what(), 0x0E));
    }

    co_return true;
}
