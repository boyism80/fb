#include <fb/game/server.h>
#include <fb/model/model.h>
#include <mutex>
#include <tuple>

using namespace fb::game;

namespace game_resp = fb::protocol::game::response;

async::task<void> listener_impl::on_message(character& me, std::string_view message, MESSAGE_TYPE type)
{
    std::ignore = co_await me.send(game_resp::message(message, type));
    co_return;
}

async::task<void> listener_impl::on_option_changed(character& me, OPTION option, bool enabled)
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
        co_return;
    }

    sstream << ": " << (enabled ? "ON" : "OFF");
    co_await me.message(sstream.str(), MESSAGE_TYPE::STATE);
    co_return;
}

async::task<void> listener_impl::on_update_option(character& ch)
{
    std::ignore = co_await ch.send(game_resp::option(ch));
    co_return;
}

async::task<void> listener_impl::on_update_map(character&                  ch,
                                               const map&                  map,
                                               const fb::model::point16_t& position,
                                               const fb::model::size8_t&   size,
                                               uint16_t                    crc)
{
    auto stream = this->server.maps.map_update_stream(ch, map, position, size, crc);
    if (stream.has_value())
        std::ignore = co_await ch.send(*stream);
    co_return;
}

async::task<void> listener_impl::on_update_buff(character& ch, const buffs& buffs)
{
    for (auto& [id, buff] : buffs)
    {
        std::ignore = co_await ch.send(game_resp::spell_buff(*buff));
    }
    co_return;
}

async::task<void> listener_impl::on_update_internal(character& ch)
{
    std::ignore = co_await ch.send(game_resp::internal_info(ch));
    co_return;
}

async::task<void> listener_impl::on_level_up(character& me)
{
    try
    {
        co_await this->server.send(me, game_resp::effect(me, 0x02), scope::PIVOT);
    }
    catch (const std::exception& e)
    {
        fb::logger::fatal("on_level_up send failed for {}: {}", me.name(), e.what());
    }
    co_return;
}

async::task<void> listener_impl::on_update(character& me, UPDATE_STATE_LEVEL level)
{
    if (level == UPDATE_STATE_LEVEL::CROWD_CONTROL)
        std::ignore = co_await me.send(game_resp::update_cc(me));
    else
        std::ignore = co_await me.send(game_resp::update_internal(me, level));
    co_return;
}

void listener_impl::on_transfer(character&                  me,
                                map&                        map,
                                const fb::model::point16_t& position,
                                std::string_view            ip,
                                uint16_t                    port)
{
    auto stream = fb::stream();
    auto writer = fb::stream_writer<big_endian>(stream);
    writer.write<uint32_t>(me.id);
    writer.write<std::string>(me.name());
    writer.write<uint8_t>(1);
    writer.write<uint16_t>(map.model.id);
    writer.write<uint16_t>(position.x);
    writer.write<uint16_t>(position.y);

    auto socket_ptr = me.socket_ptr();
    if (socket_ptr != nullptr)
        std::ignore = this->server.transfer(*socket_ptr, ip, port, internal::Service::Game, stream);
}

async::task<void> listener_impl::on_update_map(character& ch, const fb::game::map& map)
{
    std::ignore = co_await ch.send(game_resp::map_config(map));
    co_return;
}

async::task<void> listener_impl::on_update_bgm(character& ch, uint16_t bgm, uint8_t volume)
{
    std::ignore = co_await ch.send(game_resp::map_bgm(bgm, volume));
    co_return;
}

async::task<void> listener_impl::on_update_time(character& ch, uint16_t hours)
{
    std::ignore = co_await ch.send(game_resp::time(hours));
    co_return;
}

async::task<void> listener_impl::on_character_init(character& ch)
{
    std::ignore = co_await ch.send(game_resp::init());
    co_return;
}

async::task<void> listener_impl::on_update_position(character& ch)
{
    std::ignore = co_await ch.send(game_resp::position(ch));
    co_return;
}

async::task<void> listener_impl::on_screen_refresh(character& ch)
{
    co_await ch.update_id();
    co_await ch.update_position();
    co_await ch.update(UPDATE_STATE_LEVEL::ALL);

    auto map = ch.map();
    if (map == nullptr)
        co_return;

    for (auto& obj : ch.sight_in(OBJECT_TYPE::OBJECT))
    {
        if (obj->hidden(ch))
            continue;

        try
        {
            co_await obj->update_external(ch, true);
        }
        catch (const std::exception& e)
        {
            fb::logger::fatal("on_screen_refresh update_external failed (oid={}): {}", obj->oid(), e.what());
        }
    }

    try
    {
        co_await ch.update_external(ch, true);
    }
    catch (const std::exception& e)
    {
        fb::logger::fatal("on_screen_refresh self update_external failed for {}: {}", ch.name(), e.what());
    }

    std::ignore = co_await ch.send(game_resp::direction(ch));
    std::ignore = co_await ch.send(game_resp::screen_refresh_complete());
    co_return;
}

async::task<void> listener_impl::on_browse_character(character& ch, const character& target)
{
    std::ignore = co_await ch.send(game_resp::external_info(target, ch));
    co_return;
}

async::task<void> listener_impl::on_item_tooltip(character& ch, const item& item, uint16_t position)
{
    std::ignore = co_await ch.send(game_resp::item_tip(position, item.tip_message()));
    co_return;
}

async::task<void> listener_impl::on_show_user_list(character& ch)
{
    using user_data = game_resp::user_list::user_data;

    auto users  = std::make_shared<std::vector<user_data>>();
    auto mutex  = std::make_shared<std::mutex>();
    auto viewer = ch.shared_from_this_as<character>();

    users->reserve(this->server.characters.size());
    co_await this->server.characters.foreach_async([users, mutex, viewer](auto& other) -> async::task<void> {
        if (other->hidden(*viewer))
            co_return;

        auto entry      = user_data{};
        entry.nation    = static_cast<uint8_t>(other->nation());
        entry.cls       = static_cast<uint8_t>(other->cls());
        entry.promotion = other->promotion();
        entry.level     = other->level();
        entry.color     = (other.get() == viewer.get()) ? static_cast<uint8_t>(0x88) : static_cast<uint8_t>(0x0F);
        entry.name      = other->name();

        {
            auto lock = std::lock_guard(*mutex);
            users->push_back(std::move(entry));
        }
        co_return;
    });

    std::ignore = co_await ch.send(game_resp::user_list(std::move(*users)));
}

async::task<void> listener_impl::on_show_bulletin(character& ch)
{
    std::ignore = co_await ch.send(game_resp::bulletin_sections());
    co_return;
}

async::task<void> listener_impl::on_show_bulletin(character&                          ch,
                                                  const fb::model::bulletin&          section,
                                                  const std::list<bulletin::article>& articles,
                                                  BULLETIN_BUTTON_ENABLE              flag)
{
    std::ignore = co_await ch.send(game_resp::bulletin_articles(section, articles, flag));
    co_return;
}

async::task<void> listener_impl::on_show_bulletin(character&               ch,
                                                  const bulletin::article& article,
                                                  BULLETIN_BUTTON_ENABLE   flag)
{
    std::ignore = co_await ch.send(game_resp::bulletin_article(article, flag));
    co_return;
}

async::task<void> listener_impl::on_show_mail_box(character&                            ch,
                                                  const std::vector<mail_box::summary>& mails,
                                                  MAIL_BUTTON_ENABLE                    flag)
{
    auto dto = std::vector<internal::MailSummary>();
    for (auto& summary : mails)
    {
        dto.push_back(internal::MailSummary{summary.id,
                                            summary.user,
                                            summary.sender,
                                            summary.read,
                                            summary.title,
                                            summary.created_date});
    }
    std::ignore = co_await ch.send(game_resp::bulletin_mails(dto, flag));
    co_return;
}

async::task<void> listener_impl::on_show_mail_box(character& ch, const mail_box::mail& mail, MAIL_BUTTON_ENABLE flag)
{
    auto dto = internal::Mail{mail.id, mail.user, mail.sender, mail.title, mail.contents, mail.read, mail.created_date};
    std::ignore = co_await ch.send(game_resp::bulletin_mail(dto, flag));
    co_return;
}

async::task<void> listener_impl::on_show_bulletin_message(character&            ch,
                                                          std::string_view      message,
                                                          bool                  success,
                                                          BULLETIN_MESSAGE_TYPE action)
{
    std::ignore = co_await ch.send(game_resp::bulletin_message(message, success, action));
    co_return;
}

async::task<void> listener_impl::on_show_world_map(character& ch, uint32_t id, uint16_t index)
{
    std::ignore = co_await ch.send(game_resp::map_worlds(id, index));
    co_return;
}

async::task<void> listener_impl::on_timer(character& ch, uint32_t time, TIMER_TYPE type)
{
    std::ignore = co_await ch.send(fb::protocol::game::response::timer(time, type));
    co_return;
}

async::task<void> listener_impl::on_weather(character& ch, WEATHER_TYPE weather)
{
    std::ignore = co_await ch.send(fb::protocol::game::response::weather(weather));
    co_return;
}

async::task<void> listener_impl::on_bright(character& ch, uint8_t value)
{
    try
    {
        co_await this->server.send(ch, game_resp::bright(value), scope::PIVOT);
    }
    catch (const std::exception& e)
    {
        fb::logger::fatal("on_bright send failed for {}: {}", ch.name(), e.what());
    }
    co_return;
}

async::task<void> listener_impl::on_update_id(character& ch)
{
    std::ignore = co_await ch.send(game_resp::id(ch));
    co_return;
}

async::task<void> listener_impl::on_ping(character& ch, uint32_t token)
{
    std::ignore = co_await ch.send(fb::protocol::game::response::ping(token));
    co_return;
}

async::task<void> listener_impl::on_save(character& ch)
{
    std::ignore = co_await ch.send(game_resp::save());
    co_return;
}

async::task<void> listener_impl::on_bulk_update(character& ch, const std::vector<object*>& objects)
{
    std::ignore = co_await ch.send(game_resp::update(objects));
    co_return;
}

async::task<void>
listener_impl::on_ad(character& ch, uint32_t width, uint32_t height, std::string_view url, uint8_t time)
{
    std::ignore = co_await ch.send(game_resp::ad(width, height, std::string(url), time));
    co_return;
}

async::task<void> listener_impl::on_web(character& ch, uint8_t type, std::string_view url, std::string_view message)
{
    std::ignore = co_await ch.send(game_resp::web(type, std::string(url), std::string(message)));
    co_return;
}

async::task<void> listener_impl::on_ui(character& ch, uint8_t screen)
{
    std::ignore = co_await ch.send(game_resp::ui_screen(static_cast<game_resp::UI_SCREEN>(screen)));
    co_return;
}

async::task<void> listener_impl::on_item_throw_confirm(character& ch, uint8_t slot)
{
    std::ignore = co_await ch.send(game_resp::item_throw_confirm(slot));
    co_return;
}

async::task<void> listener_impl::on_freeze(character& ch, bool value)
{
    std::ignore = co_await ch.send(game_resp::freeze(value));
    co_return;
}

async::task<void> listener_impl::on_friends_sync(character& ch, uint8_t enabled)
{
    std::ignore = co_await ch.send(game_resp::friends_sync(enabled));
    co_return;
}

async::task<void> listener_impl::on_holyday_screen(character&                       ch,
                                                   uint8_t                          screen,
                                                   uint8_t                          hair,
                                                   fb::model::enum_value::DIRECTION direction,
                                                   const fb::model::point<uint8_t>& position)
{
    std::ignore = co_await ch.send(game_resp::holyday_screen(screen, hair, direction, position));
    co_return;
}
