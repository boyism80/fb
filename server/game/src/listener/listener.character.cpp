#include <fb/game/server.h>
#include <fb/game/protocol/collection.h>
#include <fb/model/model.h>
#include <fb/protocol/client_version.h>
#include <macro.h>
#include <mutex>

using namespace fb::game;

namespace game_resp = fb::protocol::game::response;
namespace game_reqs = fb::protocol::game::request;

void listener_impl::on_message(character& me, std::string_view message, MESSAGE_TYPE type)
{
    me.send(game_resp::message(message, type));
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

    case OPTION::NEWS:
        sstream << "소식듣기    ";
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

    case OPTION::LOCK_WALK_SPEED:
        if (me.client_version == fb::protocol::CLIENT_VERSION::v651)
            sstream << "투구표시    ";
        else
            sstream << "이속고정    ";
        break;

    default:
        return;
    }

    sstream << ": " << (enabled ? "ON" : "OFF");
    me.message(sstream.str(), MESSAGE_TYPE::STATE);

    if (option == OPTION::VISIBLE_HELMET && me.client_version == fb::protocol::CLIENT_VERSION::v651)
    {
        me.update_external();
        if (me.items.helmet() != nullptr)
            me.refresh_group_portrait();
    }
}

void listener_impl::on_update_option(character& ch)
{
    fb::protocol::visit_client_version(ch.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        ch.send(game_resp::option<V>(ch));
    });
}

void listener_impl::on_update_map(character&                  ch,
                                  const map&                  map,
                                  const fb::model::point16_t& position,
                                  const fb::model::size8_t&   size,
                                  uint16_t                    crc)
{
    auto stream = this->server.maps.map_update_stream(ch, map, position, size, crc);
    if (stream.has_value())
        ch.send(*stream);
}

void listener_impl::on_update_buff(character& ch, const buffs& buffs)
{
    for (auto& [id, buff] : buffs)
    {
        ch.send(game_resp::spell_buff(*buff));
    }
}

void listener_impl::on_update_internal(character& ch)
{
    fb::protocol::visit_client_version(ch.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        ch.send(game_resp::internal_info<V>(ch));
    });
}

void listener_impl::on_level_up(character& me)
{
    using cv     = fb::protocol::CLIENT_VERSION;
    auto matches = [](object& o, cv ver) -> bool {
        if (o.is(OBJECT_TYPE::CHARACTER) == false)
            return false;
        return static_cast<character&>(o).client_version == ver;
    };

    this->server.send(me,
                      game_resp::effect_v550(me, 0x02),
                      scope::PIVOT,
                      {.with_me = matches(me, cv::v550), .condition = [matches](object& o) {
                           return matches(o, cv::v550);
                       }});
    this->server.send(me,
                      game_resp::effect_v565(me, 0x02),
                      scope::PIVOT,
                      {.with_me = matches(me, cv::v565), .condition = [matches](object& o) {
                           return matches(o, cv::v565);
                       }});
    this->server.send(me,
                      game_resp::effect_v651(me, 0x02),
                      scope::PIVOT,
                      {.with_me = matches(me, cv::v651), .condition = [matches](object& o) {
                           return matches(o, cv::v651);
                       }});
}

void listener_impl::on_update(character& me, UPDATE_STATE_LEVEL level)
{
    if (level == UPDATE_STATE_LEVEL::CROWD_CONTROL)
        me.send(game_resp::update_cc(me));
    else
    {
        fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
            me.send(game_resp::update_internal<V>(me, level));
        });
        if (ENUM_IN(level, UPDATE_STATE_LEVEL::HP_MP))
        {
            auto include_max = ENUM_IN(level, UPDATE_STATE_LEVEL::BASED);
            this->server.groups.update_hp(me, include_max);
        }
    }
}

async::task<void> listener_impl::on_transfer(character&                  me,
                                             map&                        map,
                                             const fb::model::point16_t& position,
                                             std::string_view            ip,
                                             uint16_t                    port,
                                             const transfer_option&      option)
{
    auto stream = fb::stream();
    auto writer = fb::stream_writer<big_endian>(stream);
    writer.write<uint32_t>(me.id);
    writer.write<std::string>(me.name());

    auto flags = game_reqs::TRANSFER_PARAM::MAP;
    if (option.match.has_value() && option.match->id.empty() == false)
        flags |= game_reqs::TRANSFER_PARAM::MATCH;
    if (me.client_version == fb::protocol::CLIENT_VERSION::v651 && me.ui_mode == fb::protocol::CLIENT_UI_MODE::NEW)
        flags |= game_reqs::TRANSFER_PARAM::UI_MODE;

    writer.write<uint8_t>(static_cast<uint8_t>(flags));
    if (ENUM_IN(flags, game_reqs::TRANSFER_PARAM::MAP))
    {
        writer.write<uint32_t>(me.world());
        writer.write<uint16_t>(map.model().id);
        writer.write<uint16_t>(position.x);
        writer.write<uint16_t>(position.y);
    }
    if (ENUM_IN(flags, game_reqs::TRANSFER_PARAM::MATCH))
    {
        writer.write<std::string>(option.match->id);
        writer.write<uint32_t>(option.match->type);
        writer.write<uint32_t>(option.match->team);
    }
    if (ENUM_IN(flags, game_reqs::TRANSFER_PARAM::UI_MODE))
        writer.write<uint8_t>(static_cast<uint8_t>(me.ui_mode));

    auto socket_ptr = me.socket_ptr();
    if (socket_ptr != nullptr)
        co_await this->server.transfer(*socket_ptr, ip, port, internal::Service::Game, stream);
}

void listener_impl::on_update_map(character& ch, const fb::game::map& map)
{
    fb::protocol::visit_client_version(ch.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        ch.send(game_resp::map_config<V>(map));
    });
}

void listener_impl::on_update_bgm(character& ch, uint16_t bgm, uint8_t volume)
{
    ch.send(game_resp::map_bgm(bgm, volume));
}

void listener_impl::on_stop_bgm(character& ch, uint16_t bgm_id)
{
    ch.send(game_resp::map_bgm_stop(bgm_id));
}

void listener_impl::on_update_time(character& ch, uint8_t hours, uint8_t minutes)
{
    ch.send(game_resp::time(hours, minutes));
}

void listener_impl::on_character_init(character& ch)
{
    ch.send(game_resp::init());
}

void listener_impl::on_update_position(character& ch)
{
    ch.send(game_resp::position(ch));
}

void listener_impl::on_move_confirm(character&                  ch,
                                    const fb::model::point16_t& before,
                                    const fb::model::point16_t& viewport,
                                    uint8_t                     walk_queue_slot)
{
    auto thread = ch.thread();
    if (thread == nullptr)
        return;

    fb::protocol::visit_client_version(ch.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        ch.send(game_resp::move_confirm<V>(ch.direction(), before, viewport, walk_queue_slot));
    });
}

void listener_impl::on_screen_refresh(character& ch)
{
    ch.update_id();
    ch.update_position();
    ch.update(UPDATE_STATE_LEVEL::ALL);

    auto map = ch.map();
    if (map == nullptr)
        return;

    for (auto& obj : ch.sight_in(OBJECT_TYPE::OBJECT))
    {
        if (obj->hidden(ch))
            continue;

        obj->show(ch);
    }
    ch.show(ch);
    ch.send(game_resp::direction(ch));
    ch.send(game_resp::screen_refresh_complete());
}

void listener_impl::on_browse_character(character& ch, const character& target)
{
    fb::protocol::visit_client_version(ch.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        ch.send(game_resp::external_info<V>(target, ch));
    });
}

void listener_impl::on_item_tooltip(character& ch, const item& item, uint16_t position)
{
    fb::protocol::visit_client_version(ch.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        ch.send(game_resp::item_tip<V>(position, item.tip_message()));
    });
}

async::task<void> listener_impl::on_show_user_list(character& ch)
{
    using user_data = game_resp::user_list::user_data;

    auto users  = std::make_shared<std::vector<user_data>>();
    auto mutex  = std::make_shared<std::mutex>();
    auto viewer = ch.shared_from_this_as<character>();

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

    ch.send(game_resp::user_list(std::move(*users)));
}

void listener_impl::on_show_bulletin(character& ch)
{
    ch.send(game_resp::bulletin_sections());
}

void listener_impl::on_show_bulletin(character&                          ch,
                                     const fb::model::bulletin&          section,
                                     const std::list<bulletin::article>& articles,
                                     BULLETIN_BUTTON_ENABLE              flag)
{
    ch.send(game_resp::bulletin_articles(section, articles, flag));
}

void listener_impl::on_show_bulletin(character& ch, const bulletin::article& article, BULLETIN_BUTTON_ENABLE flag)
{
    ch.send(game_resp::bulletin_article(article, flag));
}

void listener_impl::on_show_mail_box(character&                            ch,
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
    ch.send(game_resp::bulletin_mails(dto, flag));
}

void listener_impl::on_show_mail_box(character& ch, const mail_box::mail& mail, MAIL_BUTTON_ENABLE flag)
{
    auto dto = internal::Mail{mail.id,
                              mail.user,
                              mail.sender,
                              mail.title,
                              mail.contents,
                              mail.read,
                              mail.created_date,
                              std::nullopt};
    ch.send(game_resp::bulletin_mail(dto, flag));
}

void listener_impl::on_show_bulletin_message(character&            ch,
                                             std::string_view      message,
                                             bool                  success,
                                             BULLETIN_MESSAGE_TYPE action)
{
    ch.send(game_resp::bulletin_message(message, success, action));
}

void listener_impl::on_show_world_map(character& ch, uint32_t id, uint16_t index)
{
    auto use_offset_new =
        ch.client_version == fb::protocol::CLIENT_VERSION::v651 && ch.ui_mode == fb::protocol::CLIENT_UI_MODE::NEW;
    ch.send(game_resp::map_worlds(id, index, use_offset_new));
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
    this->server.send(ch, game_resp::bright(value), scope::PIVOT);
}

void listener_impl::on_update_id(character& ch)
{
    ch.send(game_resp::id(ch));
}

void listener_impl::on_ping(character& ch, uint32_t token)
{
    ch.send(fb::protocol::game::response::ping(token));
}

void listener_impl::on_save(character& ch)
{
    ch.send(game_resp::save());
}

void listener_impl::on_bulk_update(character& ch, const std::vector<object*>& objects)
{
    fb::protocol::visit_client_version(ch.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        ch.send(game_resp::update<V>(objects));
    });
}

void listener_impl::on_ad(character& ch, uint32_t width, uint32_t height, std::string_view url, uint8_t time)
{
    ch.send(game_resp::ad(width, height, std::string(url), time));
}

void listener_impl::on_web(character& ch, uint8_t type, std::string_view url, std::string_view message)
{
    fb::protocol::visit_client_version(ch.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        ch.send(game_resp::web<V>(type, std::string(url), std::string(message)));
    });
}

void listener_impl::on_ui(character& ch, uint8_t screen)
{
    ch.send(game_resp::ui_screen(static_cast<game_resp::UI_SCREEN>(screen)));
}

void listener_impl::on_item_throw_confirm(character& ch, uint8_t slot)
{
    ch.send(game_resp::item_throw_confirm(slot));
}

void listener_impl::on_freeze(character& ch, bool value)
{
    ch.send(game_resp::freeze(value));
}

void listener_impl::on_friends_sync(character& ch, uint8_t enabled)
{
    ch.send(game_resp::friends_sync(enabled));
}

void listener_impl::on_holyday_screen(character&                       ch,
                                      uint8_t                          screen,
                                      uint8_t                          hair,
                                      fb::model::enum_value::DIRECTION direction,
                                      const fb::model::point<uint8_t>& position)
{
    ch.send(game_resp::holyday_screen(screen, hair, direction, position));
}

void listener_impl::on_group_portrait(character& ch, std::vector<game_resp::group_portrait_entry> entries)
{
    if (ch.client_version != fb::protocol::CLIENT_VERSION::v651)
        return;

    if (entries.size() > 255)
        entries.resize(255);
    auto n = static_cast<uint8_t>(entries.size());
    ch.send(game_resp::group_portrait<fb::protocol::CLIENT_VERSION::v651>(2, n, std::move(entries)));
}

void listener_impl::on_group_portrait_hp(character& ch, std::string name, uint32_t cur_hp)
{
    if (ch.client_version != fb::protocol::CLIENT_VERSION::v651)
        return;

    ch.send(game_resp::group_portrait<fb::protocol::CLIENT_VERSION::v651>(std::move(name), cur_hp));
}

void listener_impl::on_collection_list(character& ch)
{
    if (ch.client_version != fb::protocol::CLIENT_VERSION::v651)
        return;

    auto entries = std::vector<game_resp::collection_entry>{};
    for (const auto& group : this->server.meta.groups())
        entries.push_back({group.id, ch.collections.unlocked_count(group.id)});

    ch.send(game_resp::collection<fb::protocol::CLIENT_VERSION::v651>(std::move(entries)));
}

void listener_impl::on_collection_flag(character& ch, uint8_t group_id, uint8_t slot, bool onoff)
{
    if (ch.client_version != fb::protocol::CLIENT_VERSION::v651)
        return;

    ch.send(game_resp::collection<fb::protocol::CLIENT_VERSION::v651>(group_id, slot, onoff));
}

void listener_impl::on_collection_dialog(character& ch, uint8_t group_id)
{
    if (ch.client_version != fb::protocol::CLIENT_VERSION::v651)
        return;

    auto bitmask = ch.collections.bitmask(group_id);
    ch.send(game_resp::collection<fb::protocol::CLIENT_VERSION::v651>(group_id, std::move(bitmask)));
}
