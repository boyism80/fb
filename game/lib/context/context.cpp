#include <context.h>
using namespace fb::game;
using namespace std::chrono_literals;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::context, "")
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::lua::luable, "fb.game.luable")
{"__gc",                fb::game::lua::luable::builtin_gc},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::model::spell, "fb.model.spell")
{"type",                fb::model::spell::builtin_type},
{"name",                fb::model::spell::builtin_name},
{"message",             fb::model::spell::builtin_message},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::map, "fb.game.map")
{"width",               fb::game::map::builtin_width},
{"height",              fb::game::map::builtin_height},
{"area",                fb::game::map::builtin_area},
{"objects",             fb::game::map::builtin_objects},
{"movable",             fb::game::map::builtin_movable},
{"door",                fb::game::map::builtin_door},
{"doors",               fb::game::map::builtin_doors},
{"contains",            fb::game::map::builtin_contains},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::model::map, "fb.model.map")
{"name",                fb::model::map::builtin_name},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::model::object, "fb.model.object")
{"name",                fb::model::object::builtin_name},
{"look",                fb::model::object::builtin_look},
{"color",               fb::model::object::builtin_color},
{"dialog",              fb::model::object::builtin_dialog},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::model::mob, "fb.model.mob")
{"speed",               fb::model::mob::builtin_speed},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::mob, "fb.game.mob")
{"__eq",                fb::game::object::builtin_eq},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::model::npc, "fb.model.npc")
{"input",               fb::model::npc::builtin_input},
{"menu",                fb::model::npc::builtin_menu},
{"item",                fb::model::npc::builtin_item},
{"slot",                fb::model::npc::builtin_slot},
{"sell",                fb::model::npc::builtin_sell},
{"buy",                 fb::model::npc::builtin_buy},
{"repair",              fb::model::npc::builtin_repair},
{"repair_all",          fb::model::npc::builtin_repair_all},
{"hold_money",          fb::model::npc::builtin_hold_money},
{"hold_item",           fb::model::npc::builtin_hold_item},
{"return_money",        fb::model::npc::builtin_return_money},
{"return_item",         fb::model::npc::builtin_return_item},
{"rename_weapon",       fb::model::npc::builtin_rename_weapon},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::npc, "fb.game.npc")
END_LUA_EXTENSION


IMPLEMENT_LUA_EXTENSION(fb::game::object, "fb.game.object")
{"model",               fb::game::object::builtin_model},
{"__eq",                fb::game::object::builtin_eq},
{"__tostring",          fb::game::object::builtin_tostring},
{"id",                  fb::game::object::builtin_id},
{"name",                fb::game::object::builtin_name},
{"sound",               fb::game::object::builtin_sound},
{"position",            fb::game::object::builtin_position},
{"direction",           fb::game::object::builtin_direction},
{"chat",                fb::game::object::builtin_chat},
{"message",             fb::game::object::builtin_message},
{"buff",                fb::game::object::builtin_buff},
{"isbuff",              fb::game::object::builtin_isbuff},
{"unbuff",              fb::game::object::builtin_unbuff},
{"effect",              fb::game::object::builtin_effect},
{"map",                 fb::game::object::builtin_map},
{"mkitem",              fb::game::object::builtin_mkitem},
{"showings",            fb::game::object::builtin_showings},
{"showns",              fb::game::object::builtin_showns},
{"front",               fb::game::object::builtin_front},
{"is",                  fb::game::object::builtin_is},
END_LUA_EXTENSION


IMPLEMENT_LUA_EXTENSION(fb::model::life, "fb.model.life")
{"hp",                  fb::model::life::builtin_hp},
{"mp",                  fb::model::life::builtin_mp},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::life, "fb.game.life")
{"__eq",                fb::game::object::builtin_eq},
{"hp",                  fb::game::life::builtin_hp},
{"mp",                  fb::game::life::builtin_mp},
{"hp_inc",              fb::game::life::builtin_hp_inc},
{"mp_inc",              fb::game::life::builtin_mp_inc},
{"hp_dec",              fb::game::life::builtin_hp_dec},
{"mp_dec",              fb::game::life::builtin_mp_dec},
{"base_hp",             fb::game::life::builtin_base_hp},
{"base_mp",             fb::game::life::builtin_base_mp},
{"action",              fb::game::life::builtin_action},
{"spell",               fb::game::life::builtin_spell},
{"damage",              fb::game::life::builtin_damage},
{"cast",                fb::game::life::builtin_cast},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::model::item, "fb.model.item")
{"make",                fb::model::item::builtin_make},
{"attr",                fb::model::item::builtin_attr},
{"capacity",            fb::model::item::builtin_capacity},
{"durability",          fb::model::item::builtin_durability},
{"price",               fb::model::item::builtin_price},
{"repair_price",        fb::model::item::builtin_repair_price},
{"rename_price",        fb::model::item::builtin_rename_price},
{"deposit_price",       fb::model::item::builtin_deposit_price},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::item, "fb.game.item")
{"model",               fb::game::item::builtin_model},
{"count",               fb::game::item::builtin_count},
{"durability",          fb::game::item::builtin_durability},
{"rename",              fb::game::item::builtin_rename},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::character, "fb.game.session")
{"__eq",                fb::game::object::builtin_eq},
{"look",                fb::game::character::builtin_look},
{"color",               fb::game::character::builtin_color},
{"money",               fb::game::character::builtin_money},
{"exp",                 fb::game::character::builtin_exp},
{"base_hp",             fb::game::character::builtin_base_hp},
{"base_mp",             fb::game::character::builtin_base_mp},
{"str",                 fb::game::character::builtin_strength},
{"dex",                 fb::game::character::builtin_dexterity},
{"int",                 fb::game::character::builtin_intelligence},
{"item",                fb::game::character::builtin_item},
{"items",               fb::game::character::builtin_items},
{"dropitem",            fb::game::character::builtin_item_drop},
{"mkitem",              fb::game::character::builtin_mkitem},
{"rmitem",              fb::game::character::builtin_rmitem},
{"state",               fb::game::character::builtin_state},
{"disguise",            fb::game::character::builtin_disguise},
{"class",               fb::game::character::builtin_class},
{"level",               fb::game::character::builtin_level},
{"assert",              fb::game::character::builtin_assert},
{"deposited_money",     fb::game::character::builtin_deposited_money},
{"deposited_item",      fb::game::character::builtin_deposited_item},
{"deposit_item",        fb::game::character::builtin_deposit_item},
{"withdraw_item",       fb::game::character::builtin_withdraw_item},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::door, "fb.game.door")
{"toggle",              fb::game::door::builtin_toggle},
{"lock",                fb::game::door::builtin_lock},
{"locked",              fb::game::door::builtin_locked},
{"opened",              fb::game::door::builtin_opened},
END_LUA_EXTENSION

context::context(boost::asio::io_context& context, uint16_t port) : // clang-format on
    fb::acceptor<character>(context, port),
    maps(*this, fb::config::get()["id"].asUInt()),
    _characters(*this),
    _groups(*this)
{
    this->bind_timer(
        [this]() -> async::task<void> {
            auto&  config   = fb::config::get();
            auto&& response = co_await this->post<internal_reqs::Ping, internal_resp::Pong>(
                "internal",
                "/in-game/ping",
                internal_reqs::Ping{this->id(),
                                    this->name(),
                                    this->service(),
                                    config["ip"].asString(),
                                    (uint16_t)config["port"].asUInt()});
        },
        1s);
}

context::~context()
{
    auto& threads = this->threads();
    for (int i = 0; i < threads.count(); i++)
    {
        auto thread = threads.at(i);
        auto params = thread->data<thread_params>();
        delete params;
    }
}

async::task<void> context::handle_start()
{
    co_await fb::acceptor<character>::handle_start();

    const auto& config = fb::config::get();

    lua::env<context>("context", this);
    lua::build<lua::luable>();
    lua::build<map, lua::luable>();
    lua::build<door, lua::luable>();
    lua::build<fb::model::spell, lua::luable>();
    lua::build<fb::model::map, lua::luable>();
    lua::build<fb::model::object, lua::luable>();
    lua::build<object, lua::luable>();
    lua::build<fb::model::life, fb::model::object>();
    lua::build<life, object>();
    lua::build<fb::model::mob, fb::model::life>();
    lua::build<mob, life>();
    lua::build<fb::model::npc, fb::model::object>();
    lua::build<npc, object>();
    lua::build<fb::model::item, fb::model::object>();
    lua::build<item, object>();
    lua::build<character, life>();

    lua::build("seed", builtin_seed);
    lua::build("sleep", builtin_sleep);
    lua::build("name2mob", builtin_name2mob);
    lua::build("name2item", builtin_name2item);
    lua::build("name2npc", builtin_name2npc);
    lua::build("name2map", builtin_name2map);
    lua::build("pursuit_sell", builtin_pursuit_sell);
    lua::build("pursuit_buy", builtin_pursuit_buy);
    lua::build("sell_price", builtin_sell_price);
    lua::build("buy_price", builtin_buy_price);
    lua::build("timer", builtin_timer);
    lua::build("weather", builtin_weather);
    lua::build("name_with", builtin_name_with);
    lua::build("assert_korean", builtin_assert_korean);
    lua::build("CP949", builtin_cp949);

    auto& threads = this->threads();
    for (int i = 0; i < threads.count(); i++)
    {
        auto thread = threads.at(i);
        thread->data(new thread_params{});

        co_await thread->dispatch([]() -> async::task<void> {
            auto& ist  = lua::container::ist();
            auto& main = ist.get();
            co_return;
        });
    }

    this->_amqp_thread = std::make_unique<std::thread>(&context::amqp_thread, this);

    this->bind(&context::handle_login);          // 게임서버 접속 핸들러
    this->bind(&context::handle_direction);      // 방향전환 핸들러
    this->bind(&context::handle_logout);         // 접속 종료
    this->bind(&context::handle_update_move);    // 이동과 맵 데이터 업데이트 핸들러
    this->bind(&context::handle_move);           // 이동 핸들러
    this->bind(&context::handle_attack);         // 공격 핸들러
    this->bind(&context::handle_pickup);         // 아이템 줍기 핸들러
    this->bind(&context::handle_emotion);        // 감정표현 핸들러
    this->bind(&context::handle_update_map);     // 맵 데이터 업데이트 핸들러
    this->bind(&context::handle_refresh);        // 새로고침 핸들러
    this->bind(&context::handle_active_item);    // 아이템 사용 핸들러
    this->bind(&context::handle_inactive_item);  // 아이템 장착 해제 핸들러
    this->bind(&context::handle_drop_item);      // 아이템 버리기 핸들러
    this->bind(&context::handle_drop_cash);      // 금전 버리기 핸들러
    this->bind(&context::handle_front_info);     // 앞방향 정보 핸들러
    this->bind(&context::handle_self_info);      // 나 자신의 정보 핸들러
    this->bind(&context::handle_option_changed); // 옵션 설정 핸들러
    this->bind(&context::handle_click_object);   // 오브젝트 클릭 핸들러
    this->bind(&context::handle_item_info);      // 인벤토리 우클릭 핸들러
    this->bind(&context::handle_itemmix);        // 아이템 조합 핸들러
    this->bind(&context::handle_trade);          // 교환 핸들러
    this->bind(&context::handle_group);          // 그룹 핸들러
    this->bind(&context::handle_user_list);      // 유저 리스트 핸들러
    this->bind(&context::handle_chat);           // 유저 채팅 핸들러
    this->bind(&context::handle_board);          // 게시판 섹션 리스트 핸들러
    this->bind(&context::handle_swap);           // 스펠 순서 변경
    this->bind(&context::handle_dialog);         // 다이얼로그
    this->bind(&context::handle_dialog, 0x39);   // 다이얼로그
    this->bind(&context::handle_throw_item);     // 아이템 던지기 핸들러
    this->bind(&context::handle_spell);          // 스펠 핸들러
    this->bind(&context::handle_door);           // 도어 핸들러
    this->bind(&context::handle_whisper);        // 귓속말 핸들러
    this->bind(&context::handle_world);          // 월드맵 핸들러

    this->timer(&context::handle_mob_action, 100ms);                                        // 몹 행동 타이머
    this->timer(&context::handle_mob_respawn, 60s);                                         // 몹 리젠 타이머
    this->timer(&context::handle_buff_timer, 1s);                                           // 버프 타이머
    this->timer(&context::handle_save_timer, std::chrono::seconds(config["save"].asInt())); // DB 저장 타이머
    this->timer(&context::handle_time, 1min);                                               // 세계 시간 타이머

    this->command("맵이동", &context::handle_command_map, true);
    this->command("사운드", &context::handle_command_sound, true);
    this->command("액션", &context::handle_command_action, true);
    this->command("날씨", &context::handle_command_weather, true);
    this->command("밝기", &context::handle_command_bright, true);
    this->command("타이머", &context::handle_command_timer, true);
    this->command("이펙트", &context::handle_command_effect, true);
    this->command("변신", &context::handle_command_disguise, true);
    this->command("변신해제", &context::handle_command_undisguise, true);
    this->command("마법배우기", &context::handle_command_spell, true);
    this->command("몬스터생성", &context::handle_command_mob, true);
    this->command("직업바꾸기", &context::handle_command_class, true);
    this->command("레벨바꾸기", &context::handle_command_level, true);
    this->command("아이템생성", &context::handle_command_item, true);
    this->command("월드맵", &context::handle_command_world, true);
    this->command("스크립트", &context::handle_command_script, true);
    this->command("머리바꾸기", &context::handle_command_hair, true);
    this->command("머리염색", &context::handle_command_hair_color, true);
    this->command("갑옷염색", &context::handle_command_armor_color, true);
    this->command("서버종료", &context::handle_command_exit, true);
    this->command("타일", &context::handle_command_tile, true);
    this->command("서버저장", &context::handle_command_save, true);
    this->command("맵오브젝트", &context::handle_command_mapobj, true);
    this->command("랜덤이동", &context::handle_command_randmap, true);
    this->command("엔피씨생성", &context::handle_command_npc, true);
    this->command("내구도", &context::handle_command_durability, true);
    this->command("동시성테스트", &context::handle_command_concurrency, true);
    this->command("sleep", &context::handle_command_sleep, true);
    this->command("맵타일", &context::handle_map_tile, true);
}

bool context::decrypt_policy(uint8_t cmd) const
{
    switch (cmd)
    {
    case 0x10:
        return false;

    default:
        return true;
    }
}

async::task<bool> context::handle_connected(fb::socket<character>& socket)
{
    fb::logger::info("{}님이 접속했습니다.", socket.IP());
    co_return true;
}

async::task<bool> context::handle_disconnected(fb::socket<character>& socket)
{
    auto& config  = fb::config::get();
    auto  session = socket.data();
    if (session == nullptr)
        co_return false;

    session->init(false);
    auto group = session->group();
    if (group != nullptr)
    {
        auto active_thread = this->threads().current();
        auto group_thread  = this->threads().modular(group->id());
        co_await group_thread->dispatch();
        group->leave_active_member(*session);
        co_await active_thread->dispatch();
    }

    this->_characters.lock<void>([name = session->name()](auto& characters) {
        if (characters.contains(name))
            characters.erase(name);
    });

    fb::logger::info("{}님이 접속을 종료했습니다.", session->name());

    co_await this->save(*session);
    std::ignore =
        co_await this->post<internal_reqs::Logout, internal_resp::Logout>("internal",
                                                                          "/in-game/logout",
                                                                          internal_reqs::Logout{session->name()});
    co_await session->destroy();
    socket.data(nullptr);
    co_return true;
}

async::task<void> context::handle_timer(uint64_t elapsed_milliseconds)
{
    for (auto& [key, value] : this->maps)
        co_await value.on_timer(elapsed_milliseconds);
}

std::string context::elapsed_message(const std::string& dt)
{
    auto elapsed = datetime() - datetime(dt);
    if (elapsed.total_milliseconds() > 1000 * 60)
    {
        auto sstream = std::stringstream();
        auto days    = elapsed.days();
        if (days > 0)
            sstream << days << "일 ";

        auto hours = elapsed.hours();
        if (hours > 0)
            sstream << hours << "시간 ";

        auto minutes = elapsed.minutes();
        if (minutes > 0)
            sstream << minutes << "분만에 바람으로...";

        auto msg = sstream.str();
        return msg;
    }
    else
    {
        return std::string();
    }
}

character* context::find(const std::string& name)
{
    return this->_characters.read<fb::game::character*>([this, &name](const auto& characters) -> fb::game::character* {
        auto i = characters.find(name);
        if (i == characters.end())
            return nullptr;

        return i->second;
    });
}

async::task<bool> context::init_ch(const internal::Character&           response,
                                   character&                           session,
                                   const std::optional<transfer_param>& transfer)
{
    auto map = response.map;
    session.id(response.id);
    session.name(response.name);
    session.pw(response.pw);
    session.updated_date(datetime(response.updated_date));
    session.admin(response.admin);
    co_await session.color(response.color);
    std::ignore = co_await session.direction(DIRECTION(response.direction));
    co_await session.look(response.look);
    co_await session.money(response.money);
    session.deposited_money(response.deposited_money);
    session.sex(SEX(response.sex));
    session.base_hp(response.base_hp);
    co_await session.hp(response.hp);
    session.base_mp(response.base_mp);
    co_await session.mp(response.mp);
    co_await session.experience(response.exp);
    co_await session.state(STATE(response.state));

    if (response.armor_color.has_value())
        co_await session.armor_color(response.armor_color.value());

    if (response.disguise.has_value())
        co_await session.disguise(response.disguise.value());
    else
        co_await session.undisguise();

    if (this->maps.contains(map) == false)
        co_return false;

    auto position_x = response.position.x;
    auto position_y = response.position.y;
    if (transfer != std::nullopt)
    {
        map        = transfer.value().map;
        position_x = uint32_t(transfer.value().position.x);
        position_y = uint32_t(transfer.value().position.y);
    }

    if (response.group.has_value())
    {
        // auto group = co_await this->_groups.lock<fb::locker<fb::game::group>*>(
        //     [this, &response, &session](auto& groups) -> async::task<fb::locker<fb::game::group>*> {
        //         if (groups.contains(response.group.value()))
        //             co_return groups[response.group.value()].get();

        //        auto&& group_resp = co_await this->get<internal_resp::GetGroup>(
        //            "internal",
        //            std::format("/in-game/group/{}", response.group.value()));

        //        switch (static_cast<ERROR_CODE>(group_resp.error))
        //        {
        //        case ERROR_CODE::NONE:
        //            break;

        //        default:
        //            throw std::runtime_error(std::format("cannot get group (error : {})", group_resp.error));
        //        }

        //        auto members = std::vector<std::string>();
        //        for (auto& x : group_resp.group.members)
        //            members.push_back(x);

        //        groups.insert({group_resp.group.id,
        //                       std::make_unique<fb::locker<fb::game::group>>(*this,
        //                                                                     group_resp.group.id,
        //                                                                     group_resp.group.master,
        //                                                                     members)});

        //        co_return groups[group_resp.group.id].get();
        //    });

        // group->template lock<void>([&session, group](auto& g) {
        //     g.enter(session);
        //     session.group(group);
        // });
    }

    co_return co_await session.map(&this->maps[map], point16_t(position_x, position_y));
}

async::task<void> context::init_option(const internal::Option& response, fb::game::character& session)
{
    co_await session.option(SETTING::WHISPER, response.whisper, false);
    co_await session.option(SETTING::GROUP, response.group, false);
    co_await session.option(SETTING::ROAR, response.roar, false);
    co_await session.option(SETTING::ROAR_WORLDS, response.roar_worlds, false);
    co_await session.option(SETTING::MAGIC_EFFECT, response.magic_effect, false);
    co_await session.option(SETTING::WEATHER_EFFECT, response.weather_effect, false);
    co_await session.option(SETTING::FIXED_MOVE, response.fixed_move, false);
    co_await session.option(SETTING::TRADE, response.trade, false);
    co_await session.option(SETTING::FAST_MOVE, response.fast_move, false);
    co_await session.option(SETTING::EFFECT_SOUND, response.effect_sound, false);
    co_await session.option(SETTING::PK_PROTECT, response.pk_protect, false);
}

async::task<void> context::init_items(const std::vector<internal::Item>& response, character& session)
{
    for (auto& x : response)
    {
        auto item = this->model.item[x.model].make(*this);
        item->count(x.count);

        if (x.durability.has_value())
            item->durability(x.durability.value());

        if (x.custom_name.has_value() && item->based<fb::model::item>().attr(ITEM_ATTRIBUTE::WEAPON))
            co_await static_cast<weapon*>(item)->custom_name(x.custom_name.value());

        if (x.deposited != -1)
            std::ignore = co_await session.deposit_item(*item);
        else if (x.parts == static_cast<uint32_t>(EQUIPMENT_PARTS::UNKNOWN))
            std::ignore = co_await session.items.add(*item, x.index);
        else
            session.items.wear((EQUIPMENT_PARTS)x.parts, static_cast<equipment*>(item));
    }
}

async::task<void> context::init_spells(const std::vector<internal::Spell>& response, character& session)
{
    for (auto& x : response)
    {
        if (this->model.spell.contains(x.model) == false)
            continue;

        auto& model = this->model.spell[x.model];
        std::ignore = co_await session.spells.add(model, x.slot);
    }
}

void context::assert_whisper(const internal_resp::Whisper& response) const
{
    switch (static_cast<ERROR_CODE>(response.error))
    {
    case ERROR_CODE::NONE:
        return;

    case ERROR_CODE::OFFLINE:
        throw std::runtime_error(std::format("{}님은 바람의나라에 없습니다.", response.to));

    case ERROR_CODE::DISABLED_WHISPER_TARGET:
        throw std::runtime_error(std::format("{}님은 귓속말 거부 상태입니다.", response.to));

    default:
        throw std::runtime_error(std::format("알 수 없는 에러가 발생했습니다. (에러코드 : {})", response.error));
    }
}

character* context::handle_accepted(fb::socket<character>& socket)
{
    return this->make<character>(socket);
}

async::task<void> context::send(object&                           object,
                                const fb::protocol::base::header& header,
                                context::scope                    scope,
                                bool                              exclude_self,
                                bool                              encrypt)
{
    switch (scope)
    {
    case context::scope::SELF:
        co_await object.send(header, encrypt);
        break;

    case context::scope::PIVOT:
    {
        auto nears = object.showings(OBJECT_TYPE::CHARACTER);
        if (!exclude_self)
            co_await object.send(header, encrypt);

        for (auto& x : nears)
            x->send(header, encrypt);
    }
    break;

        // case context::scope::GROUP:
        //{
        //     if (object.is(OBJECT_TYPE::CHARACTER) == false)
        //         return;

        //    auto& session = static_cast<character&>(object);
        //    auto  group   = session.group();
        //    if (group == nullptr)
        //        return;

        //    group->read<void>([&header, encrypt](const auto& g) {
        //        for (auto ch : g.active_members())
        //        {
        //            co_await ch->send(header, encrypt);
        //        }
        //    });
        //}
        // break;

    case context::scope::MAP:
    {
        auto map = object.map();
        if (map == nullptr)
            co_return;

        for (const auto& [seq, obj] : object.map()->objects)
        {
            if (exclude_self && obj == object)
                continue;

            co_await obj.send(header, encrypt);
        }
    }
    break;

    case context::scope::WORLD:
    {
        co_await this->send(header, encrypt);
    }
    break;
    }
}

async::task<void>
context::send(object& object, const protocol_generator& fn, context::scope scope, bool exclude_self, bool encrypt)
{
    switch (scope)
    {
    case context::scope::SELF:
        co_await object.send(*fn(object).get(), encrypt);
        break;

    case context::scope::PIVOT:
    {
        auto nears = object.showings(OBJECT_TYPE::CHARACTER);
        if (!exclude_self)
            co_await object.send(*fn(object).get(), encrypt);

        for (auto& x : nears)
            co_await x->send(*fn(*x).get(), encrypt);
    }
    break;

        // case context::scope::GROUP:
        //{
        //     if (object.is(OBJECT_TYPE::CHARACTER) == false)
        //         return;

        //    auto& session = static_cast<character&>(object);
        //    auto  group   = session.group();
        //    if (group == nullptr)
        //        return;

        //    group->lock<void>([&fn, encrypt](auto& g) {
        //        for (auto ch : g.active_members())
        //        {
        //            co_await ch->send(*fn(*ch).get(), encrypt);
        //        }
        //    });
        //}
        // break;

    case context::scope::MAP:
    {
        for (const auto& [seq, obj] : object.map()->objects)
        {
            if (exclude_self && obj == object)
                continue;

            co_await obj.send(*fn(obj).get(), encrypt);
        }
    }
    break;

    case context::scope::WORLD:
    {
        co_await this->sockets.each([&fn, encrypt](auto& socket) -> async::task<void> {
            co_await socket.send(*fn(*socket.data()).get(), encrypt);
        });
    }
    break;
    }
}

async::task<void> context::send(const fb::protocol::base::header& response, const map& map, bool encrypt)
{
    co_await this->sockets.each([&response, &map, encrypt](auto& socket) -> async::task<void> {
        auto session = socket.data();
        if (session->map() != &map)
            co_return;

        co_await socket.send(response, encrypt);
    });
}

async::task<void> context::send(const fb::protocol::base::header& response, bool encrypt)
{
    co_await this->sockets.each([&response, encrypt](auto& socket) -> async::task<void> {
        auto session = socket.data();
        co_await session->send(response, encrypt);
    });
}

async::task<void> context::save(character& session)
{
    auto items = std::vector<internal::Item>();
    for (auto i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = session.items[i];
        if (item == nullptr)
            continue;

        auto protocol  = item->to_protocol();
        protocol.index = i;
        items.push_back(protocol);
    }

    for (auto& [parts, equipment] : session.items.equipments())
    {
        if (equipment == nullptr)
            continue;

        items.push_back(equipment->to_protocol());
    }

    auto& deposited_items = session.deposited_items();
    for (int i = 0; i < deposited_items.size(); i++)
    {
        auto item          = deposited_items.at(i);
        auto protocol      = item->to_protocol();
        protocol.deposited = i;
        items.push_back(protocol);
    }

    auto spells = std::vector<internal::Spell>();
    for (uint8_t i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto spell = session.spells[i];
        if (spell == nullptr)
            continue;

        spells.push_back(internal::Spell{session.id(), i, spell->id});
    }

    auto&& response = co_await this->post<internal_reqs::Save, internal_resp::Save>(
        "internal",
        "/user/save",
        internal_reqs::Save{session.to_protocol(), items, spells});
}

uint32_t context::thread_id(const fb::socket<character>& socket) const
{
    auto character = socket.data();
    if (character == nullptr)
        throw std::runtime_error("character is empty");

    auto map = character->map();
    if (map == nullptr)
        return 0;

    return map->model.id;
}

fb::thread* context::thread(const map& map)
{
    auto count = this->threads().count();
    if (count == 0)
        return nullptr;

    return this->threads().at(map.model.id % count);
}

const fb::thread* context::current_thread() const
{
    return this->threads().current();
}

void context::amqp_thread()
{
    auto& config  = fb::config::get();
    auto  timeout = timeval{5, 0};
    while (this->running())
    {
        try
        {
            this->_amqp = std::make_unique<fb::amqp::socket>();
            this->_amqp->connect(config["amqp"]["ip"].asString(),
                                 config["amqp"]["port"].asUInt(),
                                 config["amqp"]["uid"].asString(),
                                 config["amqp"]["pwd"].asString(),
                                 "/");

            auto& queue1 = this->_amqp->declare_queue();
            queue1.bind("amq.direct", std::format("fb.game.{}", config["id"].asInt()));
            queue1.handler<internal_resp::Pong>([](auto& response) -> async::task<void> {
                co_return;
            });
            queue1.handler<internal_resp::KickOut>([this](auto& response) -> async::task<void> {
                auto socket = this->sockets.find([uid = response.uid](fb::socket<character>& socket) {
                    auto data = socket.data();
                    return data->id() == uid;
                });

                if (socket == nullptr)
                    co_return;

                socket->cancel();
            });

            queue1.handler<internal_resp::Whisper>([this](auto& response) -> async::task<void> {
                auto& config = fb::config::get();
                if (response.host == config["id"].asUInt())
                    co_return;

                auto error = std::string();
                try
                {
                    this->assert_whisper(response);
                    auto you = this->find(response.to);
                    if (you == nullptr)
                        throw std::runtime_error(std::format("cannot find whisper target user : {}", response.to));

                    co_await you->message(std::format("{}> {}", response.from, response.message), MESSAGE_TYPE::NOTIFY);
                }
                catch (std::exception& e)
                {
                    error = e.what();
                }

                if (error.empty() == false)
                {
                    auto me = this->find(response.from);
                    if (me != nullptr)
                        co_await me->message(error, MESSAGE_TYPE::NOTIFY);
                }
            });

            auto& queue2 = this->_amqp->declare_queue();
            queue2.bind("amq.direct", "fb.global");
            queue2.handler<internal_resp::Pong>([](auto& response) -> async::task<void> {
                co_return;
            });

            auto& queue3 = this->_amqp->declare_queue();
            queue3.bind("amq.direct", "fb.group");
            queue3.handler<internal_resp::EnterGroup>([this](internal_resp::EnterGroup& response) -> async::task<void> {
                if (response.host == fb::config::get()["id"].asUInt())
                    co_return;

                co_await this->on_enter_group(response);
            });

            queue3.handler<internal_resp::LeaveGroup>([this](internal_resp::LeaveGroup& response) -> async::task<void> {
                if (response.host == fb::config::get()["id"].asUInt())
                    co_return;

                co_await this->on_leave_group(response);
            });
        }
        catch (std::exception& e)
        {
            fb::logger::fatal(e.what());
            std::this_thread::sleep_for(1s);
            continue;
        }

        while (this->running())
        {
            try
            {
                if (this->_amqp->select(&timeout) == false)
                    continue;
            }
            catch (std::exception& e)
            {
                break;
            }
        }
    }
}

// TODO : 클릭도 인터페이스로
async::task<void> context::handle_click_mob(character& session, mob& mob)
{
    co_await this->send(session, fb_resp::session::message(mob.name(), MESSAGE_TYPE::STATE), scope::SELF);
}

void context::handle_click_npc(character& session, npc& npc)
{
    auto& model = npc.based<fb::model::npc>();
    if (model.script.empty())
        return;

    session.dialog.release();

    session.dialog.from(model.script.c_str())
        .func("on_interact")
        .pushobject(session)
        .pushobject(npc.based<fb::model::npc>())
        .resume(2);
}

async::task<bool> context::handle_login(fb::socket<character>& socket, const fb_reqs::login& request)
{
    auto session = socket.data();
    if (session->inited())
        co_return false;

    // Set crypt data
    socket.crt(request.enc_type, request.enc_key);
    session->init(true);

    // Where login from?
    auto from = request.from;

    session->name(request.name);
    fb::logger::info("{}님이 접속했습니다.", request.name);

    auto fd = socket.fd();

    try
    {
        auto  id       = request.id;
        auto  name     = std::string(request.name);
        auto  from     = request.from;
        auto  transfer = request.transfer;
        auto& config   = fb::config::get();
        auto  delay    = config["delay"].asInt();
        co_await this->sleep(std::chrono::seconds(delay));
        if (this->sockets.contains(fd) == false)
            co_return false;

        auto&& login_resp = co_await this->post<internal_reqs::Login, internal_resp::Login>(
            "internal",
            "/in-game/login",
            internal_reqs::Login{id, name, (uint8_t)config["id"].asUInt()});
        if (login_resp.error != (uint32_t)ERROR_CODE::NONE)
            co_return false;

        auto&& response = co_await this->get<internal_resp::Init>("internal", std::format("/user/init/{}", id));
        if (this->sockets.contains(fd) == false)
            co_return false;

        this->_characters.lock<void>([name, session](auto& characters) {
            if (characters.contains(name) == false)
                characters.insert({name, session});
        });

        if (co_await this->init_ch(response.character, *session, transfer) == false)
            co_return false;

        co_await this->init_option(response.option, *session);
        co_await this->send(*session, fb_resp::init(), scope::SELF);
        co_await this->send(*session, fb_resp::time(this->_time.hours()), scope::SELF);
        co_await this->send(*session, fb_resp::session::state(*session, STATE_LEVEL::LEVEL_MIN), scope::SELF);

        if (from == internal::services::LOGIN)
        {
            auto msg = this->elapsed_message(response.character.updated_date);
            if (msg.empty() == false)
                co_await session->message(msg, MESSAGE_TYPE::STATE);
        }

        co_await this->send(*session, fb_resp::session::state(*session, STATE_LEVEL::LEVEL_MAX), scope::SELF);
        co_await this->send(*session, fb_resp::session::option(*session), scope::SELF);

        co_await this->init_items(response.items, *session);
        co_await this->init_spells(response.spells, *session);
    }
    catch (std::exception& /*e*/)
    {
        if (this->sockets.contains(fd) == false)
            co_return false;

        socket.close();
    }

    co_return true;
}

async::task<bool> context::handle_direction(fb::socket<character>& socket, const fb_reqs::direction& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    if (co_await session->direction(request.value) == false)
        co_return false;

    co_return true;
}

async::task<bool> context::handle_logout(fb::socket<character>& socket, const fb_reqs::exit& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    const auto& config = fb::config::get();
    co_await this->transfer(socket,
                            config["login"]["ip"].asString(),
                            config["login"]["port"].asInt(),
                            internal::services::GAME);
    co_return true;
}

async::task<bool> context::handle_move(fb::socket<character>& socket, const fb_reqs::move& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto map = session->map();
    if (map == nullptr)
        co_return true;

    // TODO: 실제로 이동하지 않고 이동했을때의 위치를 구해서
    // 해당 위치에서 워프가 가능한지 확인하고
    // 워프가능하면 워프처리, 그렇지 않다면 해당 위치로 이동한다.
    auto forward = session->position_forward(request.direction);

    // 워프 위치라면 워프한다.
    const auto warp = map->warpable(forward);
    if (warp != nullptr)
    {
        if (session->condition(warp->condition) == false)
        {
            // 메시지 보냄
            co_await session->message("감히 접근할 수 없습니다.");
            co_await this->send(*session, fb_resp::session::position(*session), scope::SELF);
            co_return true;
        }

        switch (warp->dest.header)
        {
        case DSL::map:
        {
            auto  params = dsl::map(warp->dest.params);
            auto& map    = this->maps[params.id];
            std::ignore  = co_await session->map(&map, point16_t(params.x, params.y));
        }
        break;

        case DSL::world:
        {
            auto  params = dsl::world(warp->dest.params);
            auto& world  = this->model.world[params.id][params.index];
            co_await session->send(fb_resp::map::worlds(this->model, params.id, params.index));
        }
        break;

        default:
            throw std::runtime_error("invalid dsl header");
        }
    }
    else
    {
        std::ignore = co_await session->move(request.direction, request.position);
    }
    co_return true;
}

async::task<bool> context::handle_update_move(fb::socket<character>& socket, const fb_reqs::update_move& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto map = session->map();
    if (map == nullptr)
        co_return true;

    if (co_await this->handle_move(socket, request))
        co_await this->send(*session, fb_resp::map::update(*map, request.begin, request.size), scope::SELF);

    co_return true;
}

async::task<bool> context::handle_attack(fb::socket<character>& socket, const fb_reqs::attack& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    co_await session->attack();
    co_return true;
}

async::task<bool> context::handle_pickup(fb::socket<character>& socket, const fb_reqs::pick_up& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    co_await session->items.pickup(request.boost);
    co_return true;
}

async::task<bool> context::handle_emotion(fb::socket<character>& socket, const fb_reqs::emotion& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    co_await session->action(ACTION(static_cast<int>(ACTION::EMOTION) + request.value), DURATION::EMOTION);
    co_return true;
}

async::task<bool> context::handle_update_map(fb::socket<character>& socket, const fb_reqs::map::update& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto map = session->map();
    if (map == nullptr)
        co_return true;

    co_await this->send(*session, fb_resp::map::update(*map, request.position, request.size), scope::SELF);
    co_return true;
}

async::task<bool> context::handle_refresh(fb::socket<character>& socket, const fb_reqs::refresh& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    co_await this->send(*session, fb_resp::session::position(*session), scope::SELF);
    co_return true;
}

async::task<bool> context::handle_active_item(fb::socket<character>& socket, const fb_reqs::item::active& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    std::ignore = co_await session->items.active(request.index);
    co_return true;
}

async::task<bool> context::handle_inactive_item(fb::socket<character>& socket, const fb_reqs::item::inactive& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    std::ignore = co_await session->items.inactive(request.parts);
    co_return true;
}

async::task<bool> context::handle_drop_item(fb::socket<character>& socket, const fb_reqs::item::drop& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    std::ignore = co_await session->items.drop(request.index, request.all ? -1 : 1);
    co_return true;
}

async::task<bool> context::handle_drop_cash(fb::socket<character>& socket, const fb_reqs::item::drop_cash& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto map = session->map();
    if (map == nullptr)
        co_return false;

    auto chunk = std::min(session->money(), request.chunk);

    std::ignore = co_await session->money_drop(chunk);
    co_return true;
}

// TODO : on_notify를 이용
async::task<bool> context::handle_front_info(fb::socket<character>& socket, const fb_reqs::front_info& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto map = session->map();
    if (map == nullptr)
        co_return false;

    auto forwards = session->forwards();
    for (auto i = forwards.begin(); i != forwards.end(); i++)
    {
        auto object  = *i;
        auto message = object->is(OBJECT_TYPE::ITEM) ? static_cast<item*>(*i)->inven_name() : (*i)->name();

        co_await session->message(message, MESSAGE_TYPE::STATE);
    }

    co_return true;
}

async::task<bool> context::handle_self_info(fb::socket<character>& socket, const fb_reqs::self_info& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    co_await this->send(*session, fb_resp::session::internal_info(*session, this->model), scope::SELF);

    for (auto& [id, buff] : session->buffs)
        co_await this->send(*session, fb_resp::spell::buff(*buff), scope::SELF);
    co_return true;
}

async::task<bool> context::handle_option_changed(fb::socket<character>& socket, const fb_reqs::change_option& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto option = SETTING(request.option);
    switch (option)
    {
    case SETTING::EXTENSION:
        if (request.ride)
        {
            if (session->state() == STATE::RIDING)
                co_await session->unride();
            else
                co_await session->ride();
        }
        break;

    default:
        auto enabled = co_await session->option_toggle(option);
        if (option == SETTING::GROUP && !enabled)
        {
            auto&& response = co_await this->post<internal_reqs::LeaveGroup, internal_resp::LeaveGroup>(
                "internal",
                "/in-game/group/leave",
                internal_reqs::LeaveGroup{session->name()});

            co_await this->on_leave_group(response);
        }

        auto&& response = co_await this->post<internal_reqs::SetOption, internal_resp::SetOption>(
            "internal",
            "/user/option",
            internal_reqs::SetOption{session->id(), static_cast<uint8_t>(option), enabled});

        if (response.success == false)
            co_await session->message("설정을 변경하지 못했습니다.");
        break;
    }
    co_return true;
}

async::task<bool> context::handle_click_object(fb::socket<character>& socket, const fb_reqs::click& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    if (request.fd == 0xFFFFFFFF) // Press F1
        co_return true;

    if (request.fd == 0xFFFFFFFE) // Preff F2
        co_return true;

    if (request.fd == 1 && session->dialog.active())
    {
        session->dialog.pushnil().resume(1);
    }
    else
    {
        auto map = session->map();
        auto you = map->objects[request.fd];
        if (you == nullptr)
            co_return true;

        switch (you->what())
        {
        case OBJECT_TYPE::CHARACTER:
            co_await this->send(*session,
                                fb_resp::session::external_info(static_cast<character&>(*you), this->model),
                                scope::SELF);
            break;

        case OBJECT_TYPE::MOB:
            co_await this->handle_click_mob(*session, static_cast<mob&>(*you));
            break;

        case OBJECT_TYPE::NPC:
            this->handle_click_npc(*session, static_cast<npc&>(*you));
            break;
        }
    }

    co_return true;
}

// TODO : on_item_detail
async::task<bool> context::handle_item_info(fb::socket<character>& socket, const fb_reqs::item::info& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto item = session->items[request.slot];
    if (item == nullptr)
        co_return false;

    co_await this->send(*session, fb_resp::item::tip(request.position, item->tip_message()), scope::SELF);
    co_return true;
}

async::task<bool> context::handle_itemmix(fb::socket<character>& socket, const fb_reqs::item::mix& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    if (request.indices.size() > CONTAINER_CAPACITY - 1)
        co_return false;

    auto dsl = std::vector<fb::model::dsl::item>();
    for (auto index : request.indices)
    {
        auto item = session->items[index];
        if (item == nullptr)
            continue;

        auto& model = item->based<fb::model::item>();
        dsl.push_back(fb::model::dsl::item(model.id, item->count(), 0.0));
    }

    auto found = this->model.recipe.find(dsl);
    if (found == nullptr)
        throw std::runtime_error("no match exception");

    if (found->success.size() - found->source.size() > session->items.free_size())
        throw std::runtime_error("full inven exception");

    for (auto& x : found->source)
    {
        auto params        = fb::model::dsl::item(x.params);
        auto deleted_count = uint32_t(0);
        while (deleted_count <= params.count)
        {
            auto item  = session->items.find(this->model.item[params.id]);
            auto index = session->items.index(this->model.item[params.id]);
            if (item == nullptr)
                throw std::runtime_error("no match exception");

            auto count   = item->count();
            auto deleted = co_await session->items.remove(*item, count);
            if (deleted != nullptr)
                co_await deleted->destroy();

            deleted_count += count;
        }
    }

    auto  listener = session->get_listener<character>();
    auto  success  = (std::rand() % 100) < found->percent;
    auto& result   = success ? found->success : found->failed;
    for (auto& dsl : result)
    {
        auto  params = fb::model::dsl::item(dsl.params);
        auto& model  = this->model.item[params.id];
        auto  remain = params.count;
        while (remain > 0)
        {
            auto item  = this->make<fb::game::item>(this->model.item[params.id]);
            auto count = std::min<uint16_t>(model.capacity, remain);
            item->count(count);
            std::ignore  = co_await session->items.add(item);
            remain      -= count;
        }
    }

    if (listener != nullptr)
    {
        auto& message = success ? message::mix::SUCCESS : message::mix::FAILED;
        co_await listener->on_notify(*session, message);
    }

    co_return true;
}

async::task<bool> context::handle_trade(fb::socket<character>& socket, const fb_reqs::trade& request)
{
    auto me = socket.data();
    if (me->inited() == false)
        co_return true;

    auto map = me->map();
    if (map == nullptr)
        co_return true;

    auto you = static_cast<character*>(map->objects[request.fd]); // 파트너
    if (you == nullptr)
        co_return true;

    switch (static_cast<trade::state>(request.action))
    {
    case trade::state::REQUEST:
    {
        std::ignore = co_await me->trade.begin(*you);
        break;
    }

    case trade::state::UP_ITEM: // 아이템 올릴때
    {
        auto item = me->items[request.parameter.index - 1];
        if (item == nullptr)
            break;

        std::ignore = co_await me->trade.up(*item);
        break;
    }

    case trade::state::ITEM_COUNT: // 아이템 갯수까지 해서 올릴 때
    {
        std::ignore = co_await me->trade.count(request.parameter.count);
        break;
    }

    case trade::state::UP_MONEY: // 금전 올릴 때
    {
        // 클라이언트가 입력한 금전 양
        std::ignore = co_await me->trade.up(request.parameter.money);
        break;
    }

    case trade::state::CANCEL: // 취소한 경우
    {
        std::ignore = co_await me->trade.cancel();
        break;
    }

    case trade::state::LOCK:
    {
        std::ignore = co_await me->trade.lock();
        break;
    }
    }

    co_return true;
}

void context::assert_group(uint32_t error, const std::string& actor) const
{
    switch (static_cast<ERROR_CODE>(error))
    {
    case ERROR_CODE::NONE:
        return;

    case ERROR_CODE::CANNOT_GROUP_SELF:
        throw std::runtime_error("자기 자신과는 그룹할 수 없습니다.");

    case ERROR_CODE::GROUP_ALREADY_JOINED:
        throw std::runtime_error("이미 그룹에 참여중입니다.");

    case ERROR_CODE::OFFLINE:
        throw std::runtime_error(std::format("{}님은 바람의나라에 없습니다.", actor));

    case ERROR_CODE::GROUP_TARGET_ALREADY_JOINED:
        throw std::runtime_error(std::format("{}님은 이미 그룹에 참여중입니다.", actor));

    case ERROR_CODE::DISABLED_GROUP:
        throw std::runtime_error("그룹 참여 거부중입니다.");

    case ERROR_CODE::DISABLED_GROUP_TARGET:
        throw std::runtime_error(std::format("{}님은 그룹 참여 거부중입니다.", actor));

    case ERROR_CODE::NOT_GROUP_MASTER:
        throw std::runtime_error("당신은 그룹장이 아닙니다.");

    default:
        throw std::runtime_error(std::format("알 수 없는 에러가 발생했습니다. (에러코드 : {})", error));
    }
}

async::task<void> context::on_enter_group(internal_resp::EnterGroup response)
{
    this->assert_group(response.error, response.member);

    auto& threads       = this->threads();
    auto  active_thread = threads.current();

    // switch thread that matched with group id
    auto group_thread = threads.modular(response.group.id);
    co_await group_thread->dispatch();

    auto params = group_thread->data<thread_params>();
    if (params->groups.contains(response.group.id) == false)
    {
        params->groups.insert(
            {response.group.id,
             std::make_unique<group>(response.group.id, response.group.master, response.group.members)});
    }

    auto group = params->groups[response.group.id].get();
    this->_characters.lock<void>([this, &response, group](auto& characters) {
        auto members = std::vector<fb::game::character*>();

        if (characters.contains(response.group.master))
            members.push_back(characters[response.group.master]);

        for (auto& member : response.group.members)
        {
            if (characters.contains(member))
                members.push_back(characters[member]);
        }

        for (auto ch : members)
        {
            group->enter(*ch);
        }

        for (auto ch : members)
        {
            auto map    = ch->map();
            auto thread = this->thread(*map);

            // TODO: 좀 단순화해야함
            async::awaitable_then(thread->dispatch([this, ch, group, response, &characters]() -> async::task<void> {
                ch->group(group);

                switch (response.action)
                {
                case GroupAction::Enter:
                {
                    if (ch->name() != response.member)
                        co_await ch->message(std::format("{}님 그룹에 참여", response.member), MESSAGE_TYPE::STATE);
                    else
                        co_await ch->message("그룹에 참여했습니다.", MESSAGE_TYPE::STATE);
                }
                break;

                case GroupAction::Kick:
                {
                    co_await ch->message(std::format("{}님 그룹에서 탈퇴", response.member), MESSAGE_TYPE::STATE);

                    if (characters.contains(response.member))
                    {
                        characters[response.member]->group(nullptr);
                        co_await characters[response.member]->message("그룹에서 추방당했습니다.", MESSAGE_TYPE::STATE);
                    }
                }
                break;
                }

                co_return;
            }),
                                  [](auto result) {
                                  });
        }
    });

    co_await active_thread->dispatch();

    // auto group = this->_groups.lock<fb::locker<fb::game::group>*>([this, &response](auto& groups) {
    //     if (groups.contains(response.group.id))
    //         groups.erase(response.group.id);

    //    groups.insert({response.group.id,
    //                   std::make_unique<fb::locker<fb::game::group>>(*this,
    //                                                                 response.group.id,
    //                                                                 response.group.master,
    //                                                                 response.group.members)});
    //    return groups[response.group.id].get();
    //});

    // group->lock<void>([this, &response, group](auto& g) {
    //     this->_characters.lock<void>([this, &response, &g, group](auto& characters) {
    //         auto members = std::vector<fb::game::character*>();

    //        if (characters.contains(response.group.master))
    //            members.push_back(characters[response.group.master]);

    //        for (auto& member : response.group.members)
    //        {
    //            if (characters.contains(member))
    //                members.push_back(characters[member]);
    //        }

    //        for (auto ch : members)
    //        {
    //            g.enter(*ch);
    //            auto map    = ch->map();
    //            auto thread = this->thread(*map);

    //            thread->dispatch([this, ch, group]() -> async::task<void> {
    //                ch->group(group);
    //            });
    //        }

    //        switch (response.action)
    //        {
    //        case GroupAction::Enter:
    //        {
    //            for (auto ch : members)
    //            {
    //                if (ch->name() != response.member)
    //                    ch->message(std::format("{}님 그룹에 참여", response.member), MESSAGE_TYPE::STATE);
    //                else
    //                    ch->message("그룹에 참여했습니다.", MESSAGE_TYPE::STATE);
    //            }
    //        }
    //        break;

    //        case GroupAction::Kick:
    //        {
    //            for (auto ch : members)
    //            {
    //                ch->message(std::format("{}님 그룹에서 탈퇴", response.member), MESSAGE_TYPE::STATE);
    //            }

    //            if (characters.contains(response.member))
    //            {
    //                characters[response.member]->group(nullptr);
    //                characters[response.member]->message("그룹에서 추방당했습니다.", MESSAGE_TYPE::STATE);
    //            }
    //        }
    //        break;
    //        }
    //    });
    //});
}

async::task<void> context::on_leave_group(const internal_resp::LeaveGroup& response)
{
    this->assert_group(response.error, response.member);

    // auto group = this->_groups.lock<fb::locker<fb::game::group>*>([this, &response](auto& groups) {
    //     if (groups.contains(response.group.id))
    //         groups.erase(response.group.id);

    //    groups.insert({response.group.id,
    //                   std::make_unique<fb::locker<fb::game::group>>(*this,
    //                                                                 response.group.id,
    //                                                                 response.group.master,
    //                                                                 response.group.members)});
    //    return groups[response.group.id].get();
    //});

    // group->lock<void>([this, &response, group](auto& g) {
    //     this->_characters.lock<void>([this, &response, &g, group](auto& characters) {
    //         auto members = std::vector<fb::game::character*>();

    //        if (characters.contains(response.group.master))
    //            members.push_back(characters[response.group.master]);

    //        for (auto& uid : response.group.members)
    //        {
    //            if (characters.contains(uid))
    //                members.push_back(characters[uid]);
    //        }

    //        switch (response.action)
    //        {
    //        case GroupAction::Leave:
    //        {
    //            for (auto member : members)
    //            {
    //                co_await member->message(std::format("{}님 그룹에서 탈퇴", response.member), MESSAGE_TYPE::STATE);
    //            }

    //            if (characters.contains(response.member))
    //            {
    //                characters[response.member]->group(nullptr);
    //                co_await characters[response.member]->message("그룹 탈퇴", MESSAGE_TYPE::STATE);
    //            }
    //        }
    //        break;

    //        case GroupAction::BreakUp:
    //        {
    //            for (auto member : members)
    //            {
    //                member->group(nullptr);
    //                co_await member->message(std::format("그룹 해체", response.member), MESSAGE_TYPE::STATE);
    //            }

    //            this->_groups.lock<void>([this, &response](auto& groups) {
    //                switch (response.action)
    //                {
    //                case GroupAction::BreakUp:
    //                    groups.erase(response.group.id);
    //                    break;
    //                }
    //            });
    //        }
    //        break;
    //        }
    //    });
    //});

    co_return;
}

async::task<bool> context::handle_group(fb::socket<character>& socket, const fb_reqs::group& request)
{
    auto me = socket.data();
    if (me->inited() == false)
        co_return true;

    auto error = std::string();

    try
    {
        if (me->option(SETTING::GROUP) == false)
            throw std::runtime_error(message::group::DISABLED_MINE);

        auto&& response = co_await this->post<internal_reqs::EnterGroup, internal_resp::EnterGroup>(
            "internal",
            "/in-game/group/create",
            internal_reqs::EnterGroup{me->id(), request.name});

        co_await this->on_enter_group(response);
    }
    catch (std::exception& e)
    {
        error = e.what();
    }

    if (error.empty() == false)
        co_await this->send(*me, fb_resp::message(error, MESSAGE_TYPE::STATE), scope::SELF);

    co_return true;
}

async::task<bool> context::handle_user_list(fb::socket<character>& socket, const fb_reqs::user_list& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    co_await this->send(*session, fb_resp::user_list(*session, this->sockets), scope::SELF);
    co_return true;
}

async::task<bool> context::handle_chat(fb::socket<character>& socket, const fb_reqs::chat& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    if (session->admin() && co_await handle_command(*session, request.message))
        co_return true;

    co_await session->chat(request.message, request.shout);

    auto npcs = std::vector<npc*>();
    if (request.shout)
    {
        for (auto& [fd, obj] : session->map()->objects)
        {
            if (obj.is(OBJECT_TYPE::NPC))
                npcs.push_back(static_cast<npc*>(&obj));
        }
    }
    else
    {
        for (auto npc : session->showings(OBJECT_TYPE::NPC))
        {
            npcs.push_back(static_cast<fb::game::npc*>(npc));
        }
    }

    std::ignore = co_await session->inline_interaction(request.message, npcs);

    co_return true;
}

async::task<bool> context::handle_board(fb::socket<character>& socket, const fb_reqs::board::board& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto fd = session->fd();
    switch (request.action)
    {
    case BOARD_ACTION::SECTIONS:
    {
        co_await this->send(*session, fb_resp::board::sections(this->model), scope::SELF);
    }
    break;

    case BOARD_ACTION::ARTICLES:
    {
        auto error = std::string();
        try
        {
            if (this->model.board.contains(request.section) == false)
                throw std::runtime_error(message::board::SECTION_NOT_EXIST);

            auto section = &this->model.board[request.section];
            auto offset  = request.offset;

            auto&& response = co_await this->get<internal_resp::GetArticleList>(
                "internal",
                std::format("/board/{}&offset={}", section->id, offset));
            if (this->sockets.contains(fd) == false)
                co_return false;

            auto articles = std::list<board::article>();
            for (auto& summary : response.summary_list)
            {
                auto dt = datetime(summary.created_date);
                articles.push_back(board::article{summary.id,
                                                  section->id,
                                                  summary.user,
                                                  summary.user_name,
                                                  summary.title,
                                                  (uint8_t)dt.month(),
                                                  (uint8_t)dt.day()});
            }

            auto button_flags = BOARD_BUTTON_ENABLE::UP;
            if (session->condition(section->condition) == false)
                button_flags |= BOARD_BUTTON_ENABLE::WRITE;
            co_await this->send(*session, fb_resp::board::articles(*section, articles, button_flags), scope::SELF);
        }
        catch (std::exception& e)
        {
            error = e.what();
        }

        if (error.empty() == false)
            co_await this->send(*session, fb_resp::board::message(error, false, false), scope::SELF);
    }
    break;

    case BOARD_ACTION::ARTICLE:
    {
        auto error = std::string();
        try
        {
            if (this->model.board.contains(request.section) == false)
                throw std::runtime_error(message::board::SECTION_NOT_EXIST);

            auto   section  = &this->model.board[request.section]; // 코루틴땜시 포인터로
            auto&& response = co_await this->get<internal_resp::GetArticle>(
                "internal",
                std::format("/board/{}/{}", section->id, request.article));
            if (this->sockets.contains(fd) == false)
                co_return false;

            if (response.success == false)
            {
                co_await this->send(*session,
                                    fb_resp::board::message(message::board::ARTICLE_NOT_EXIST, false, false),
                                    scope::SELF);
                co_return true;
            }

            auto dt           = datetime(response.article.created_date);
            auto button_flags = BOARD_BUTTON_ENABLE::NONE;
            if (response.next)
                button_flags |= BOARD_BUTTON_ENABLE::NEXT;

            if (session->condition(section->condition) == false)
                button_flags |= BOARD_BUTTON_ENABLE::WRITE;

            auto article = board::article{response.article.id,
                                          section->id,
                                          response.article.user,
                                          response.article.user_name,
                                          response.article.title,
                                          (uint8_t)dt.month(),
                                          (uint8_t)dt.day(),
                                          response.article.contents};
            co_await this->send(*session, fb_resp::board::article(article, button_flags), scope::SELF);
        }
        catch (std::exception& e)
        {
            error = e.what();
        }

        if (error.empty() == false)
            co_await this->send(*session, fb_resp::board::message(error, false, false), scope::SELF);
    }
    break;

    case BOARD_ACTION::WRITE:
    {
        auto error = std::string();
        try
        {
            if (this->model.board.contains(request.section) == false)
                throw std::runtime_error(message::board::SECTION_NOT_EXIST);

            auto section = &this->model.board[request.section];
            if (session->condition(section->condition) == false)
                throw std::runtime_error(message::board::NOT_AUTH);

            if (request.title.length() > 64)
                throw std::runtime_error(message::board::TOO_LONG_TITLE);

            if (request.contents.length() > 256)
                throw std::runtime_error(message::board::TOO_LONG_CONTENTS);

            auto&& response = co_await this->post<internal_reqs::WriteArticle, internal_resp::WriteArticle>(
                "internal",
                "/board/write",
                internal_reqs::WriteArticle{section->id, session->id(), request.title, request.contents});

            if (this->sockets.contains(fd) == false)
                co_return false;

            if (response.success == false)
                throw std::runtime_error("게시글 작성 실패");

            co_await this->send(*session, fb_resp::board::message(message::board::WRITE, true, true), scope::SELF);
        }
        catch (std::exception& e)
        {
            error = e.what();
        }
        co_await this->send(*session, fb_resp::board::message(error, false, false), scope::SELF);
    }
    break;

    case BOARD_ACTION::DELETE:
    {
        auto error = std::string();
        try
        {
            if (this->model.board.contains(request.section) == false)
                throw std::runtime_error(message::board::SECTION_NOT_EXIST);

            auto section = &this->model.board[request.section];
            if (session->condition(section->condition) == false)
                throw std::runtime_error(message::board::NOT_AUTH);

            auto&& response = co_await this->post<internal_reqs::DeleteArticle, internal_resp::DeleteArticle>(
                "internal",
                "/board/delete",
                internal_reqs::DeleteArticle{request.article, session->id()});

            if (this->sockets.contains(fd) == false)
                co_return false;

            switch (response.result)
            {
            case -1: // article not found
                throw std::runtime_error(message::board::ARTICLE_NOT_EXIST);

            case -2: // article deleted
                throw std::runtime_error(message::board::ARTICLE_NOT_EXIST);

            case -3: // no authenticate
                throw std::runtime_error(message::board::NOT_AUTH);
            }

            co_await this->send(*session,
                                fb_resp::board::message(message::board::SUCCESS_DELETE, true, false),
                                scope::SELF);
        }
        catch (std::exception& e)
        {
            error = e.what();
        }

        if (error.empty() == false)
            co_await this->send(*session, fb_resp::board::message(error, false, false), scope::SELF);
    }
    break;

    default:
        co_return false;
    }

    co_return true;
}

async::task<bool> context::handle_swap(fb::socket<character>& socket, const fb_reqs::swap& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    switch (request.type)
    {
    case SWAP_TYPE::SPELL:
    {
        std::ignore = co_await session->spells.swap(request.src - 1, request.dst - 1);
        break;
    }

    case SWAP_TYPE::ITEM:
    {
        std::ignore = co_await session->items.swap(request.src - 1, request.dst - 1);
        break;
    }

    default:
        co_return false;
    }

    co_return true;
}

async::task<bool> context::handle_dialog(fb::socket<character>& socket, const fb_reqs::dialog& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    if (session->dialog.active() == false)
        co_return true;

    switch (request.interaction)
    {
    case dialog::interaction::NORMAL: // 일반 다이얼로그
    {
        session->dialog.pushinteger(request.action).resume(1);
        break;
    }

    case dialog::interaction::INPUT:
    {
        session->dialog.pushstring(request.message).resume(1);
        break;
    }

    case dialog::interaction::INPUT_EX:
    {
        if (request.action == 0x02) // OK button
            session->dialog.pushstring(request.message);
        else
            session->dialog.pushinteger(request.action);

        session->dialog.resume(1);
        break;
    }

    case dialog::interaction::MENU:
    {
        session->dialog.pushinteger(request.index).resume(1);
        break;
    }

    case dialog::interaction::SLOT:
    {
        session->dialog.pushinteger(request.index).resume(1);
        break;
    }

    case dialog::interaction::ITEM:
    {
        session->dialog.pushstring(request.name).resume(1);
        break;
    }

    default:
    {
        break;
    }
    }

    co_return true;
}

async::task<bool> context::handle_throw_item(fb::socket<character>& socket, const fb_reqs::item::throws& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    std::ignore = co_await session->items.throws(request.index);
    co_return true;
}

async::task<bool> context::handle_spell(fb::socket<character>& socket, const fb_reqs::spell::use& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    if (request.slot > CONTAINER_CAPACITY - 1)
        co_return false;

    auto model = session->spells[request.slot];
    if (model == nullptr)
        co_return false;

    request.parse(model->type);
    switch (model->type)
    {
    case SPELL_TYPE::INPUT:
        session->active(*model, request.message);
        break;

    case SPELL_TYPE::TARGET:
        session->active(*model, request.fd);
        break;

    case SPELL_TYPE::NORMAL:
        session->active(*model);
        break;
    }

    co_return true;
}

async::task<bool> context::handle_door(fb::socket<character>& socket, const fb_reqs::door& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto thread = lua::get();
    if (thread == nullptr)
        co_return true;

    thread->from("scripts/common/door.lua").func("on_door").pushobject(session).resume(1);
    co_return true;
}

async::task<bool> context::handle_whisper(fb::socket<character>& socket, const fb_reqs::whisper& request)
{
    auto me = socket.data();
    if (me->inited() == false)
        co_return true;

    auto  fd      = me->fd();
    auto& from    = me->name();
    auto  to      = std::string(request.name);
    auto  message = std::string(request.message);
    auto  error   = std::string();
    try
    {
        if (me->option(SETTING::WHISPER) == false)
            throw std::runtime_error("당신은 귓속말 거부 상태입니다.");

        co_await me->message(std::format("{}< {}", to, message), MESSAGE_TYPE::NOTIFY);
        auto you = this->find(to);
        if (you != nullptr)
        {
            auto response    = internal_resp::Whisper{};
            response.from    = from;
            response.to      = me->id();
            response.message = message;
            response.host    = fb::config::get()["id"].asUInt();
            if (you->option(SETTING::WHISPER))
                response.error = static_cast<uint32_t>(ERROR_CODE::NONE);
            else
                response.error = static_cast<uint32_t>(ERROR_CODE::DISABLED_WHISPER_TARGET);

            this->assert_whisper(response);
            co_await you->message(std::format("{}> {}", from, message), MESSAGE_TYPE::NOTIFY);
        }
        else
        {
            auto&& response = co_await this->post<internal_reqs::Whisper, internal_resp::Whisper>(
                "internal",
                "/in-game/whisper",
                internal_reqs::Whisper{from, to, message});
            if (this->sockets.contains(fd) == false)
                co_return false;

            this->assert_whisper(response);
        }
    }
    catch (std::exception& e)
    {
        if (this->sockets.contains(fd) == false)
            co_return false;

        error = e.what();
    }
    if (error.empty() == false)
        co_await me->message(error, MESSAGE_TYPE::NOTIFY);
    co_return true;
}

async::task<bool> context::handle_world(fb::socket<character>& socket, const fb_reqs::map::world& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto& world  = this->model.world[request.value];
    auto& before = world[request.before];
    auto& after  = world[request.after];

    if (session->map() == &this->maps[after.map])
    {
        co_await session->refresh_map();
    }
    else
    {
        std::ignore = co_await session->map(&this->maps[after.map], after.position);
        co_await this->save(*session);
    }
    co_return true;
}

async::task<void> context::handle_mob_action(const datetime& now, std::thread::id id)
{
    for (auto& [_, map] : this->maps)
    {
        if (map.active == false)
            continue;

        auto thread = this->thread(map);
        if (thread != nullptr && thread->id() != id)
            continue;

        const auto mobs = map.activateds(OBJECT_TYPE::MOB);

        for (auto x : mobs)
        {
            auto mob = static_cast<fb::game::mob*>(x);
            if (mob->alive() == false)
                continue;

            auto target = mob->target();
            if (target == nullptr || map.objects.contains(*target) == false || target->alive() == false)
                mob->target(nullptr);

            if (mob->action())
                continue;

            co_await mob->AI(now);
        }
    }
    co_return;
}

async::task<void> context::handle_mob_respawn(const datetime& now, std::thread::id id)
{
    for (auto& rezen : this->rezen)
    {
        rezen.spawn(id);
    }
    co_return;
}

async::task<void> context::handle_buff_timer(const datetime& now, std::thread::id id)
{
    for (auto& [_, map] : this->maps)
    {
        if (map.active == false)
            continue;

        auto thread = this->thread(map);
        if (thread != nullptr && thread->id() != id)
            continue;

        if (map.objects.size() == 0)
            continue;

        for (auto& [fd, obj] : map.objects)
        {
            if (obj.buffs.size() == 0)
                continue;

            auto ended_buffs = std::vector<buff*>();
            for (auto& [id, buff] : obj.buffs)
            {
                buff->time_dec(1);
                if (buff->time() <= 0ms)
                    ended_buffs.push_back(buff);
            }

            for (auto& buff : ended_buffs)
                std::ignore = co_await obj.buffs.remove(buff->model);
        }
    }

    co_return;
}

async::task<void> context::handle_save_timer(const datetime& now, std::thread::id id)
{
    for (auto& [_, map] : this->maps)
    {
        if (map.active == false)
            continue;

        auto thread = this->thread(map);
        if (thread != nullptr && thread->id() != id)
            continue;

        if (map.objects.size() == 0)
            continue;

        for (auto& [fd, obj] : map.objects)
        {
            if (obj.is(OBJECT_TYPE::CHARACTER) == false)
                continue;

            auto session = static_cast<character*>(&obj);
            co_await this->save(*session);
        }
    }
}

async::task<void> context::handle_time(const datetime& now, std::thread::id id)
{
    auto updated = datetime();
    if (this->_time.hours() != updated.hours())
        co_await this->send(fb_resp::time(updated.hours()));

    this->_time = updated;
}

async::task<bool> context::handle_command(character& session, const std::string& message)
{
    if (message.starts_with('/') == false)
        co_return false;

    std::vector<std::string> splitted;
    std::istringstream       sstream(message.substr(1));
    std::string              unit;
    while (std::getline(sstream, unit, ' '))
    {
        splitted.push_back(unit);
    }

    if (splitted.empty())
        co_return false;

    auto found = this->_commands.find(splitted[0]);
    if (found == this->_commands.end())
        co_return false;

    if (found->second.admin && session.admin() == false)
        co_return false;

    Json::Value parameters;
    for (auto i = splitted.begin() + 1; i != splitted.end(); i++)
    {
        auto digit = std::all_of((*i).begin(), (*i).end(), [](uint8_t c) {
            return std::isdigit(c);
        });
        if (digit)
            parameters.append(std::stoi(*i));
        else
            parameters.append(*i);
    }

    co_return co_await found->second.fn(session, parameters);
}
