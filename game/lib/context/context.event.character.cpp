#include <fb/game/context.h>

using namespace fb::game;

void context::on_message(character& me, const std::string& message, MESSAGE_TYPE type)
{
    this->send(me, fb_resp::message(message, type), scope::SELF);
}

void context::on_option(character& me, SETTING option, bool enabled)
{
    std::stringstream sstream;

    switch (option)
    {
    case SETTING::WHISPER:
        sstream << "귓속말듣기  ";
        break;

    case SETTING::GROUP:
    {
        sstream << "그룹허가    ";
        break;
    }

    case SETTING::ROAR:
        sstream << "외치기듣기  ";
        break;

    case SETTING::ROAR_WORLDS:
        sstream << "세계후      ";
        break;

    case SETTING::MAGIC_EFFECT:
        sstream << "마법이펙트  ";
        break;

    case SETTING::WEATHER_EFFECT:
        sstream << "날씨변화    ";
        break;

    case SETTING::FIXED_MOVE:
        sstream << "고정이동    ";
        break;

    case SETTING::TRADE:
        sstream << "교환가능    ";
        break;

    case SETTING::FAST_MOVE:
        sstream << "빠른이동    ";
        break;

    case SETTING::EFFECT_SOUND:
        sstream << "소리듣기    ";
        break;

    case SETTING::PK_PROTECT:
        sstream << "PK보호      ";
        break;

    default:
        return;
    }

    sstream << ": " << (enabled ? "ON" : "OFF");
    this->send(me, fb_resp::message(sstream.str(), MESSAGE_TYPE::STATE), scope::SELF);
    this->send(me, fb_resp::option(me), scope::SELF);
}

void context::on_level_up(character& me)
{
    this->send(me, fb_resp::effect(me, 0x02), scope::PIVOT);
}

void context::on_action(character& me, ACTION action, DURATION duration, uint8_t sound)
{
    this->send(me, fb_resp::action(me, action, duration), scope::PIVOT);
}

void context::on_update(character& me, STATE_LEVEL level)
{
    this->send(me, fb_resp::update_internal(me, level), scope::SELF);
}

async::task<bool> context::on_transfer(character& me, map& map, const point16_t& position)
{
    auto& socket = static_cast<fb::socket<character>&>(me);
    auto  fd     = static_cast<uint32_t>(socket.native_handle());
    auto  error  = std::string();
    try
    {
        auto&& response =
            co_await this->post<fb::protocol::internal::request::Transfer, fb::protocol::internal::response::Transfer>(
                "internal",
                "/in-game/transfer",
                fb::protocol::internal::request::Transfer{fb::protocol::internal::Service::Game,
                                                          map.model.host,
                                                          me.name(),
                                                          false});

        switch (static_cast<ERROR_CODE>(response.error))
        {
        case ERROR_CODE::NONE:
            break;

        case ERROR_CODE::SERVER_NOT_READY:
            throw std::runtime_error("비바람이 휘몰아치고 있습니다.");

        default:
            throw std::runtime_error(std::format("알 수 없는 에러가 발생했습니다. (에러코드 : {})", response.error));
        }

        auto ch     = socket.data();
        std::ignore = co_await ch->map(nullptr);

        this->save(*ch);
        auto stream = fb::stream();
        auto writer = fb::stream_writer<big_endian>(stream);
        writer.write<uint32_t>(me.id());
        writer.write<std::string>(ch->name());
        writer.write<uint8_t>(1);
        writer.write<uint16_t>(map.model.id);
        writer.write<uint16_t>(position.x);
        writer.write<uint16_t>(position.y);
        std::ignore = this->transfer(socket, response.ip, response.port, fb::protocol::internal::Service::Game, stream);
        co_return true;
    }
    catch (std::exception& e)
    {
        error = e.what();
    }
    catch (boost::system::error_code& /*e*/)
    {
        error = "비바람이 휘몰아치고 있습니다.";
    }

    auto ch = this->_sockets.template lock<character*>([fd](auto& container) -> character* {
        if (container.contains(fd))
            return container.at(fd)->data();

        return nullptr;
    });
    if (ch != nullptr)
    {
        ch->refresh_map();
        this->on_message(*ch, error, MESSAGE_TYPE::STATE);
    }
    co_return false;
}

void context::on_map_changed(object& me, map* before, map* after)
{
    if (after == nullptr)
        return;

    if (me.is(OBJECT_TYPE::CHARACTER) == false)
        return;

    auto& ch = static_cast<character&>(me);
    this->send(ch, fb_resp::id(ch), scope::SELF);
    this->send(ch, fb_resp::map_config(*after), scope::SELF);
    this->send(ch, fb_resp::map_bgm(*after), scope::SELF);
    this->send(ch, fb_resp::position(ch), scope::SELF);
    this->send(ch, fb_resp::update_external(ch, ch, false), scope::SELF);
    this->send(ch, fb_resp::direction(ch), scope::SELF);

    if (before == nullptr)
        this->save(ch);
}