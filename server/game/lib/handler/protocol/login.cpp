#include <fb/game/handler/protocol/login.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

login::login(fb::game::server& server) :
    fb::handler<fb::game::server, fb::protocol::game::request::login>(server)
{ }

async::task<bool> login::init_ch(const internal::Character&           response,
                                 character&                           ch,
                                 std::optional<uint32_t>              group,
                                 std::optional<uint32_t>              clan,
                                 const std::optional<transfer_param>& transfer)
{
    auto map  = response.map;
    auto weak = ch.weak_from_this_as<character>();
    ch.id(response.id);
    ch.name(response.name);
    ch.pw(response.pw);
    ch.birthday(response.birth);
    ch.updated_date(fb::model::datetime(response.updated_date));
    ch.role(static_cast<ROLE>(response.role));
    ch.cls(static_cast<CLASS>(response.class_type));
    ch.promotion(response.promotion);
    ch.color(response.color);
    ch.direction(DIRECTION(response.direction));
    ch.look(response.look);
    ch.money(response.money);
    ch.items.deposited(response.deposited_money);
    ch.sex(SEX(response.sex));
    ch.base_hp(response.base_hp);
    ch.hp(response.hp);
    ch.base_mp(response.base_mp);
    ch.mp(response.mp);
    ch.level(response.level);
    ch.exp(response.exp);
    ch.state(STATE(response.state));
    ch.title(response.title);

    if (response.armor_color.has_value())
        ch.armor_color(response.armor_color.value());

    if (response.disguise.has_value())
        ch.disguise(response.disguise.value());
    else
        ch.undisguise();

    for (auto& buff : response.buffs)
    {
        auto& model = this->server.model.spell[buff.model];
        ch.buffs.push_back(model, buff.time);
    }

    if (this->server.maps.contains(map) == false)
        co_return false;

    auto position_x = response.position.x;
    auto position_y = response.position.y;
    if (transfer != std::nullopt)
    {
        map        = transfer.value().map;
        position_x = uint32_t(transfer.value().position.x);
        position_y = uint32_t(transfer.value().position.y);
    }

    if (group.has_value())
    {
        this->server.upsert_group_then(group.value(), [&ch](auto& group) {
            group->enter(ch.weak_from_this_as<character>());
            ch.group_id(group->id());
        });
    }

    if (clan.has_value())
    {
        co_await this->server.upsert_clan_then(clan.value(), [weak](auto& clan) -> async::task<void> {
            auto ch = weak.lock();
            if (ch == nullptr)
                co_return;

            clan->attach_character(weak);
            ch->clan_id(clan->id());
            co_return;
        });
    }

    co_return co_await ch.map(this->server.maps[map], fb::model::point16_t(position_x, position_y));
}

void login::init_option(const internal::Option& response, fb::game::character& ch)
{
    ch.option(OPTION::WHISPER, response.whisper, false);
    ch.option(OPTION::GROUP, response.group, false);
    ch.option(OPTION::ROAR, response.roar, false);
    ch.option(OPTION::ROAR_WORLDS, response.roar_worlds, false);
    ch.option(OPTION::MAGIC_EFFECT, response.magic_effect, false);
    ch.option(OPTION::WEATHER_EFFECT, response.weather_effect, false);
    ch.option(OPTION::FIXED_MOVE, response.fixed_move, false);
    ch.option(OPTION::TRADE, response.trade, false);
    ch.option(OPTION::FAST_MOVE, response.fast_move, false);
    ch.option(OPTION::EFFECT_SOUND, response.effect_sound, false);
    ch.option(OPTION::PK_PROTECT, response.pk_protect, false);
}

void login::init_items(const std::vector<internal::Item>& response, character& ch)
{
    for (auto& x : response)
    {
        auto item = this->server.model.item[x.model].make(this->server);
        item->count(x.count);

        if (x.durability.has_value())
            item->durability(x.durability.value());

        if (x.stored != -1)
            ch.items.store(item);
        else if (x.parts == static_cast<uint32_t>(EQUIPMENT_PARTS::UNKNOWN))
            ch.items.add(item, x.index);
        else
            ch.items.wear((EQUIPMENT_PARTS)x.parts, std::static_pointer_cast<fb::game::equipment>(item));

        if (x.custom_name.has_value() && item->based<fb::model::item>().attr(ITEM_ATTRIBUTE::WEAPON))
            static_cast<weapon*>(item.get())->custom_name(x.custom_name.value());
    }
}

void login::init_spells(const std::vector<internal::Spell>& response, character& ch)
{
    for (auto& x : response)
    {
        if (this->server.model.spell.contains(x.model) == false)
            continue;

        auto& model = this->server.model.spell[x.model];
        auto  delay = fb::model::datetime(x.next) - fb::model::datetime();
        auto  sec   = delay.seconds();
        if (sec >= 0)
            sec += (delay.milliseconds() > 0 ? 1 : 0);
        else
            sec = 0;
        ch.spells.add(model, x.slot, sec);
    }
}

void login::init_quests(const std::vector<fb::protocol::internal::Quest>& response, fb::game::character& ch)
{
    for (auto& x : response)
    {
        ch.quests.add(x.qid, x.step, x.progress, x.completed, x.param);
    }
}

void login::init_achievements(const std::vector<fb::protocol::internal::Achievement>& response, fb::game::character& ch)
{
    for (auto& achievement : response)
    {
        if (this->server.model.achievement.contains(achievement.model) == false)
            continue;

        auto ptr = std::make_unique<fb::game::achievement>(this->server.model.achievement[achievement.model],
                                                           achievement.text,
                                                           achievement.icon,
                                                           achievement.color);
        ch.achievements.insert({achievement.model, std::move(ptr)});
    }
}

std::string login::elapsed_message(const std::string& dt)
{
    auto elapsed = fb::model::datetime() - fb::model::datetime(dt);
    if (elapsed.total_milliseconds() < 1000 * 60)
        return std::string();

    auto sstream = std::stringstream();
    auto days    = elapsed.days();
    if (days > 0)
        sstream << days << "일 ";

    auto hours = elapsed.hours();
    if (hours > 0)
        sstream << hours << "시간 ";

    auto minutes = elapsed.minutes();
    if (minutes > 0)
        sstream << minutes << "분";
    sstream << "만에 바람으로...";

    return sstream.str();
}

async::task<bool> login::handle(fb::socket<character>& session, fb::protocol::game::request::login& request)
{
    auto ch = session.data();
    if (ch == nullptr)
        co_return false;

    auto weak = ch->weak_from_this();
    session.encryption(request.enc_type, request.enc_key);

    ch->name(request.name);
    fb::logger::info("{} has connected.", request.name);

    auto delay = fb::config<uint32_t>("delay");
    co_await this->server.sleep(std::chrono::seconds(delay));

    auto&& login_resp = co_await this->server.http.post("internal",
                                                        "/in-game/login",
                                                        Login{request.id, request.name, fb::config<uint8_t>("id")});
    if (login_resp.error != (uint32_t)ERROR_CODE::NONE)
        co_return false;

    auto&& response =
        co_await this->server.http.get<internal_resp::Init>("internal", std::format("/user/init/{}", request.id));
    auto map = request.transfer.has_value() ? request.transfer->map : response.character.map;
    if (weak.expired())
        co_return false;

    ch->thread(this->server.maps[map]->thread());
    co_await this->server.threads.switching(weak);

    if (co_await this->init_ch(response.character, *ch, response.group, response.clan, request.transfer) == false)
        co_return false;
    co_await this->server.threads.switching(weak);

    ch->unread_mail(response.mail);

    this->init_items(response.items, *ch);
    this->init_spells(response.spells, *ch);
    this->init_achievements(response.achievements, *ch);
    this->init_quests(response.quests, *ch);
    this->init_option(response.option, *ch);
    ch->init();
    ch->update_time(this->server.time().hours());
    if (request.from == internal::Service::Login)
    {
        auto msg = this->elapsed_message(response.character.updated_date);
        if (msg.empty() == false)
            ch->message(msg, MESSAGE_TYPE::STATE);

        auto lua = fb::lua::new_context();
        if (lua != nullptr)
        {
#if defined DEBUG | defined _DEBUG
            lua->load("scripts/interaction.lua");
#endif
            lua->func("on_login");
            lua->pushobject(ch);
            std::ignore = lua->call(1);
        }
    }

    ch->update(STATE_LEVEL::LEVEL_MAX);
    ch->update_option();
    this->server.characters.insert(ch->shared_from_this_as<character>());
    co_return true;
}
