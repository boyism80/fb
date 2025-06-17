#include <fb/game/context.h>

using namespace fb::game;

void listener_impl::on_message(character& me, const std::string& message, MESSAGE_TYPE type)
{
    me.send(fb_resp::message(message, type));
}

void listener_impl::on_option_changed(character& me, OPTION option, bool enabled)
{
    std::stringstream sstream;

    switch (option)
    {
    case OPTION::WHISPER:
        sstream << "귓속말듣기  ";
        break;

    case OPTION::GROUP:
    {
        sstream << "그룹허가    ";
        break;
    }

    case OPTION::ROAR:
        sstream << "외치기듣기  ";
        break;

    case OPTION::ROAR_WORLDS:
        sstream << "세계후      ";
        break;

    case OPTION::MAGIC_EFFECT:
        sstream << "마법이펙트  ";
        break;

    case OPTION::WEATHER_EFFECT:
        sstream << "날씨변화    ";
        break;

    case OPTION::FIXED_MOVE:
        sstream << "고정이동    ";
        break;

    case OPTION::TRADE:
        sstream << "교환가능    ";
        break;

    case OPTION::FAST_MOVE:
        sstream << "빠른이동    ";
        break;

    case OPTION::EFFECT_SOUND:
        sstream << "소리듣기    ";
        break;

    case OPTION::PK_PROTECT:
        sstream << "PK보호      ";
        break;

    default:
        return;
    }

    sstream << ": " << (enabled ? "ON" : "OFF");
    me.message(sstream.str(), MESSAGE_TYPE::STATE);
}

void listener_impl::on_update_option(character& ch)
{
    ch.send(fb_resp::option(ch));
}

void listener_impl::on_update_map(character&                  ch,
                                  const map&                  map,
                                  const fb::model::point16_t& begin,
                                  const fb::model::size8_t&   size)
{
    ch.send(fb_resp::map_update(map, begin, size));
}

void listener_impl::on_update_buff(character& ch, const buffs& buffs)
{
    for (auto& [id, buff] : buffs)
        ch.send(fb_resp::spell_buff(*buff));
}

void listener_impl::on_update_internal(character& ch)
{
    ch.send(fb_resp::internal_info(ch, this->context.model));
}

void listener_impl::on_level_up(character& me)
{
    this->context.send(me, fb_resp::effect(me, 0x02), scope::PIVOT);
}

void listener_impl::on_update(character& me, STATE_LEVEL level)
{
    me.send(fb_resp::update_internal(me, level));
}

async::task<bool> listener_impl::on_transfer(character& me, map& map, const fb::model::point16_t& position)
{
    if (me.map() == nullptr)
        co_return false;

    auto& socket = static_cast<fb::socket<character>&>(me);
    auto  fd     = static_cast<uint32_t>(socket.native_handle());
    auto  error  = std::string();
    auto  p      = fb::model::point16_t{position};
    try
    {
        auto&& response =
            co_await this->context
                .post<fb::protocol::internal::request::Transfer, fb::protocol::internal::response::Transfer>(
                    "internal",
                    "/in-game/transfer",
                    fb::protocol::internal::request::Transfer{fb::protocol::internal::Service::Game,
                                                              map.model.host,
                                                              me.name(),
                                                              false});
        co_await this->context.switch_thread(me);
        switch (static_cast<ERROR_CODE>(response.error))
        {
        case ERROR_CODE::NONE:
            break;

        case ERROR_CODE::SERVER_NOT_READY:
            throw std::runtime_error(_TEXT(MESSAGE_NOT_READY_GAME_SERVER));

        default:
            throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), response.error));
        }

        std::ignore = co_await me.map(nullptr);
        std::ignore = this->context.save(me);
        auto stream = fb::stream();
        auto writer = fb::stream_writer<big_endian>(stream);
        writer.write<uint32_t>(me.id());
        writer.write<std::string>(me.name());
        writer.write<uint8_t>(1);
        writer.write<uint16_t>(map.model.id);
        writer.write<uint16_t>(p.x);
        writer.write<uint16_t>(p.y);
        std::ignore =
            this->context.transfer(socket, response.ip, response.port, fb::protocol::internal::Service::Game, stream);
        co_return true;
    }
    catch (std::exception& e)
    {
        if (this->context.alive(me))
        {
            me.update_map();
            me.update_external(false);
            this->on_message(me, e.what(), MESSAGE_TYPE::STATE);
        }
        co_return false;
    }
    catch (boost::system::error_code& /*e*/)
    {
        if (this->context.alive(me))
        {
            me.update_map();
            me.update_external(false);
            this->on_message(me, _TEXT(MESSAGE_NOT_READY_GAME_SERVER), MESSAGE_TYPE::STATE);
        }
        co_return false;
    }
}

void listener_impl::on_update_map(character& ch, const fb::game::map& map)
{
    ch.send(fb_resp::map_config(map));
}

void listener_impl::on_update_bgm(character& ch, uint16_t bgm, uint8_t volume)
{
    ch.send(fb_resp::map_bgm(bgm, volume));
}

void listener_impl::on_update_time(character& ch, uint16_t hours)
{
    ch.send(fb_resp::time(hours));
}

void listener_impl::on_character_init(character& ch)
{
    ch.send(fb_resp::init());
}

void listener_impl::on_update_position(character& ch)
{
    ch.send(fb_resp::position(ch));
}

void listener_impl::on_browse_character(character& ch, const character& target)
{
    ch.send(fb_resp::external_info(target, this->context.model));
}

void listener_impl::on_item_tooltip(character& ch, const item& item, uint16_t position)
{
    ch.send(fb_resp::item_tip(position, item.tip_message()));
}

void listener_impl::on_show_user_list(character& ch)
{
    this->context.access_sockets([this, &ch](const auto& sockets) {
        auto users = std::vector<character*>{};
        for (auto& [_, socket] : sockets)
        {
            users.push_back(socket->data());
        };

        ch.send(fb_resp::user_list(ch, std::move(users)));
    });
}

void listener_impl::on_show_bulletin(character& ch)
{
    ch.send(fb_resp::bulletin_sections(this->context.model));
}

void listener_impl::on_show_bulletin(character&                          ch,
                                     const fb::model::bulletin&          section,
                                     const std::list<bulletin::article>& articles,
                                     BULLETIN_BUTTON_ENABLE              flag)
{
    ch.send(fb_resp::bulletin_articles(section, articles, flag));
}

void listener_impl::on_show_bulletin(character& ch, const bulletin::article& article, BULLETIN_BUTTON_ENABLE flag)
{
    ch.send(fb_resp::bulletin_article(article, flag));
}

void listener_impl::on_show_mail_box(character& ch, const std::vector<MailSummary>& mails, MAIL_BUTTON_ENABLE flag)
{
    ch.send(fb_resp::bulletin_mails(mails, flag));
}

void listener_impl::on_show_mail_box(character& ch, const Mail& mail, MAIL_BUTTON_ENABLE flag)
{
    ch.send(fb_resp::bulletin_mail(mail, flag));
}

void listener_impl::on_show_bulletin_message(character& ch, const std::string& message, bool success, bool mail)
{
    ch.send(fb_resp::bulletin_message(message, success, mail));
}

void listener_impl::on_show_world_map(character& ch, uint32_t id, uint16_t index)
{
    ch.send(fb_resp::map_worlds(this->context.model, id, index));
}

void listener_impl::on_timer(character& ch, uint32_t time, TIMER_TYPE type)
{
    ch.send(fb::protocol::game::response::timer(time, type));
}

void listener_impl::on_weather(character& ch, WEATHER_TYPE weather)
{
    ch.send(fb::protocol::game::response::weather(weather));
}

void listener_impl::on_bright(character& ch, uint8_t value)
{
    this->context.send(ch, fb_resp::bright(value), scope::PIVOT);
}

void listener_impl::on_update_id(character& ch)
{
    ch.send(fb_resp::id(ch));
}