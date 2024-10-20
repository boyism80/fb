#include <context.h>
using namespace fb::game;
using namespace std::chrono_literals;

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
{"group",               fb::game::character::builtin_group},
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
{"update",              fb::game::door::builtin_update},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::group, "fb.game.group")
{"members",             fb::game::group::builtin_members},
{"leader",              fb::game::group::builtin_leader},
END_LUA_EXTENSION

fb::game::context::context(boost::asio::io_context& context, uint16_t port) : 
    fb::acceptor<fb::game::character>(context, port),
    maps(*this, fb::config::get()["id"].asUInt())
{ }

fb::game::context::~context()
{}

async::task<void> fb::game::context::handle_start()
{
    co_await fb::acceptor<fb::game::character>::handle_start();

    const auto& config = fb::config::get();

    lua::env<fb::game::context>("context", this);
    lua::bind_class<lua::luable>();
    lua::bind_class<fb::game::map,     lua::luable>();
    lua::bind_class<fb::game::door,    lua::luable>();
    lua::bind_class<fb::game::group,   lua::luable>();
    lua::bind_class<fb::model::spell,  lua::luable>();
    lua::bind_class<fb::model::map,    lua::luable>();
    lua::bind_class<fb::model::object, lua::luable>();          lua::bind_class<fb::game::object, lua::luable>();
    lua::bind_class<fb::model::life,   fb::model::object>();    lua::bind_class<fb::game::life,   fb::game::object>();
    lua::bind_class<fb::model::mob,    fb::model::life>();      lua::bind_class<fb::game::mob,    fb::game::life>();
    lua::bind_class<fb::model::npc,    fb::model::object>();    lua::bind_class<fb::game::npc,    fb::game::object>();
    lua::bind_class<fb::model::item,   fb::model::object>();    lua::bind_class<fb::game::item,   fb::game::object>();
    lua::bind_class<fb::game::character, fb::game::life>();

    lua::bind_function("seed",              builtin_seed);
    lua::bind_function("sleep",             builtin_sleep);
    lua::bind_function("name2mob",          builtin_name2mob);
    lua::bind_function("name2item",         builtin_name2item);
    lua::bind_function("name2npc",          builtin_name2npc);
    lua::bind_function("name2map",          builtin_name2map);
    lua::bind_function("pursuit_sell",      builtin_pursuit_sell);
    lua::bind_function("pursuit_buy",       builtin_pursuit_buy);
    lua::bind_function("sell_price",        builtin_sell_price);
    lua::bind_function("buy_price",         builtin_buy_price);
    lua::bind_function("timer",             builtin_timer);
    lua::bind_function("weather",           builtin_weather);
    lua::bind_function("name_with",         builtin_name_with);
    lua::bind_function("assert_korean",     builtin_assert_korean);
    lua::bind_function("CP949",             builtin_cp949);

    auto& threads = this->threads();
    for(int i = 0; i < threads.count(); i++)
    {
        auto thread = threads.at(i);
        co_await thread->dispatch([] () -> async::task<void>
        {
            auto& ist = fb::game::lua::container::ist();
            auto& main = ist.get();
            co_return;
        });
    }

    this->_amqp_thread = std::make_unique<std::thread>(&context::amqp_thread, this);

    this->bind<fb::protocol::game::request::login>            (std::bind(&context::handle_login,           this, std::placeholders::_1, std::placeholders::_2));   // 게임서버 접속 핸들러
    this->bind<fb::protocol::game::request::direction>        (std::bind(&context::handle_direction,       this, std::placeholders::_1, std::placeholders::_2));   // 방향전환 핸들러
    this->bind<fb::protocol::game::request::exit>             (std::bind(&context::handle_logout,          this, std::placeholders::_1, std::placeholders::_2));   // 접속 종료
    this->bind<fb::protocol::game::request::update_move>      (std::bind(&context::handle_update_move,     this, std::placeholders::_1, std::placeholders::_2));   // 이동과 맵 데이터 업데이트 핸들러
    this->bind<fb::protocol::game::request::move>             (std::bind(&context::handle_move,            this, std::placeholders::_1, std::placeholders::_2));   // 이동 핸들러
    this->bind<fb::protocol::game::request::attack>           (std::bind(&context::handle_attack,          this, std::placeholders::_1, std::placeholders::_2));   // 공격 핸들러
    this->bind<fb::protocol::game::request::pick_up>          (std::bind(&context::handle_pickup,          this, std::placeholders::_1, std::placeholders::_2));   // 아이템 줍기 핸들러
    this->bind<fb::protocol::game::request::emotion>          (std::bind(&context::handle_emotion,         this, std::placeholders::_1, std::placeholders::_2));   // 감정표현 핸들러
    this->bind<fb::protocol::game::request::map::update>      (std::bind(&context::handle_update_map,      this, std::placeholders::_1, std::placeholders::_2));   // 맵 데이터 업데이트 핸들러
    this->bind<fb::protocol::game::request::refresh>          (std::bind(&context::handle_refresh,         this, std::placeholders::_1, std::placeholders::_2));   // 새로고침 핸들러
    this->bind<fb::protocol::game::request::item::active>     (std::bind(&context::handle_active_item,     this, std::placeholders::_1, std::placeholders::_2));   // 아이템 사용 핸들러
    this->bind<fb::protocol::game::request::item::inactive>   (std::bind(&context::handle_inactive_item,   this, std::placeholders::_1, std::placeholders::_2));   // 아이템 장착 해제 핸들러
    this->bind<fb::protocol::game::request::item::drop>       (std::bind(&context::handle_drop_item,       this, std::placeholders::_1, std::placeholders::_2));   // 아이템 버리기 핸들러
    this->bind<fb::protocol::game::request::item::drop_cash>  (std::bind(&context::handle_drop_cash,       this, std::placeholders::_1, std::placeholders::_2));   // 금전 버리기 핸들러
    this->bind<fb::protocol::game::request::front_info>       (std::bind(&context::handle_front_info,      this, std::placeholders::_1, std::placeholders::_2));   // 앞방향 정보 핸들러
    this->bind<fb::protocol::game::request::self_info>        (std::bind(&context::handle_self_info,       this, std::placeholders::_1, std::placeholders::_2));   // 나 자신의 정보 핸들러
    this->bind<fb::protocol::game::request::change_option>    (std::bind(&context::handle_option_changed,  this, std::placeholders::_1, std::placeholders::_2));   // 옵션 설정 핸들러
    this->bind<fb::protocol::game::request::click>            (std::bind(&context::handle_click_object,    this, std::placeholders::_1, std::placeholders::_2));   // 오브젝트 클릭 핸들러
    this->bind<fb::protocol::game::request::item::info>       (std::bind(&context::handle_item_info,       this, std::placeholders::_1, std::placeholders::_2));   // 인벤토리 우클릭 핸들러
    this->bind<fb::protocol::game::request::item::mix>        (std::bind(&context::handle_itemmix,         this, std::placeholders::_1, std::placeholders::_2));   // 아이템 조합 핸들러
    this->bind<fb::protocol::game::request::trade>            (std::bind(&context::handle_trade,           this, std::placeholders::_1, std::placeholders::_2));   // 교환 핸들러
    this->bind<fb::protocol::game::request::group>            (std::bind(&context::handle_group,           this, std::placeholders::_1, std::placeholders::_2));   // 그룹 핸들러
    this->bind<fb::protocol::game::request::user_list>        (std::bind(&context::handle_user_list,       this, std::placeholders::_1, std::placeholders::_2));   // 유저 리스트 핸들러
    this->bind<fb::protocol::game::request::chat>             (std::bind(&context::handle_chat,            this, std::placeholders::_1, std::placeholders::_2));   // 유저 채팅 핸들러
    this->bind<fb::protocol::game::request::board::board>     (std::bind(&context::handle_board,           this, std::placeholders::_1, std::placeholders::_2));   // 게시판 섹션 리스트 핸들러
    this->bind<fb::protocol::game::request::swap>             (std::bind(&context::handle_swap,            this, std::placeholders::_1, std::placeholders::_2));   // 스펠 순서 변경
    this->bind<fb::protocol::game::request::dialog1>          (std::bind(&context::handle_dialog,          this, std::placeholders::_1, std::placeholders::_2));   // 다이얼로그
    this->bind<fb::protocol::game::request::dialog2>          (std::bind(&context::handle_dialog,          this, std::placeholders::_1, std::placeholders::_2));   // 다이얼로그 (TODO: 호출되는지 확인)
    this->bind<fb::protocol::game::request::item::throws>     (std::bind(&context::handle_throw_item,      this, std::placeholders::_1, std::placeholders::_2));   // 아이템 던지기 핸들러
    this->bind<fb::protocol::game::request::spell::use>       (std::bind(&context::handle_spell,           this, std::placeholders::_1, std::placeholders::_2));   // 스펠 핸들러
    this->bind<fb::protocol::game::request::door>             (std::bind(&context::handle_door,            this, std::placeholders::_1, std::placeholders::_2));   // 도어 핸들러
    this->bind<fb::protocol::game::request::whisper>          (std::bind(&context::handle_whisper,         this, std::placeholders::_1, std::placeholders::_2));   // 귓속말 핸들러
    this->bind<fb::protocol::game::request::map::world>       (std::bind(&context::handle_world,           this, std::placeholders::_1, std::placeholders::_2));   // 월드맵 핸들러

    this->bind_timer(std::bind(&context::handle_mob_action,   this, std::placeholders::_1, std::placeholders::_2), 100ms);                                         // 몹 행동 타이머
    this->bind_timer(std::bind(&context::handle_mob_respawn,  this, std::placeholders::_1, std::placeholders::_2), 60s);                                           // 몹 리젠 타이머
    this->bind_timer(std::bind(&context::handle_buff_timer,   this, std::placeholders::_1, std::placeholders::_2), 1s);                                            // 버프 타이머
    this->bind_timer(std::bind(&context::handle_save_timer,   this, std::placeholders::_1, std::placeholders::_2), std::chrono::seconds(config["save"].asInt()));  // DB 저장 타이머
    this->bind_timer(std::bind(&context::handle_time,         this, std::placeholders::_1, std::placeholders::_2), 1min);                                          // 세계 시간 타이머

    this->bind_command("맵이동", command 
        { 
            .fn = std::bind(&context::handle_command_map, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("사운드", command
        {
            .fn = std::bind(&context::handle_command_sound, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("액션", command
        {
            .fn = std::bind(&context::handle_command_action, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("날씨", command
        {
            .fn = std::bind(&context::handle_command_weather, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("밝기", command
        {
            .fn = std::bind(&context::handle_command_bright, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("타이머", command
        {
            .fn = std::bind(&context::handle_command_timer, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("이펙트", command
        {
            .fn = std::bind(&context::handle_command_effect, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("변신", command
        {
            .fn = std::bind(&context::handle_command_disguise, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("변신해제", command
        {
            .fn = std::bind(&context::handle_command_undisguise, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("마법배우기", command
        {
            .fn = std::bind(&context::handle_command_spell, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("몬스터생성", command
        {
            .fn = std::bind(&context::handle_command_mob, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("직업바꾸기", command
        {
            .fn = std::bind(&context::handle_command_class, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("레벨바꾸기", command
        {
            .fn = std::bind(&context::handle_command_level, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("아이템생성", command
        {
            .fn = std::bind(&context::handle_command_item, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("월드맵", command
        {
            .fn = std::bind(&context::handle_command_world, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("스크립트", command
        {
            .fn = std::bind(&context::handle_command_script, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("머리바꾸기", command
        {
            .fn = std::bind(&context::handle_command_hair, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("머리염색", command
        {
            .fn = std::bind(&context::handle_command_hair_color, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("갑옷염색", command
        {
            .fn = std::bind(&context::handle_command_armor_color, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("서버종료", command
        {
            .fn = std::bind(&context::handle_command_exit, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("타일", command
        {
            .fn = std::bind(&context::handle_command_tile, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
    this->bind_command("서버저장", command
        {
            .fn = std::bind(&context::handle_command_save, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });

    this->bind_command("맵오브젝트", command
        {
            .fn = std::bind(&context::handle_command_mapobj, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });

    this->bind_command("랜덤이동", command
        {
            .fn = std::bind(&context::handle_command_randmap, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });

    this->bind_command("엔피씨생성", command
        {
            .fn = std::bind(&context::handle_command_npc, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });

    this->bind_command("내구도", command
        {
            .fn = std::bind(&context::handle_command_durability, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });

    this->bind_command("동시성테스트", command
        {
            .fn = std::bind(&context::handle_command_concurrency, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });

    this->bind_command("sleep", command
        {
            .fn = std::bind(&context::handle_command_sleep, this, std::placeholders::_1, std::placeholders::_2),
            .admin = true
        });
}

bool fb::game::context::decrypt_policy(uint8_t cmd) const
{
    switch(cmd)
    {
    case 0x10:
        return false;

    default:
        return true;
    }
}

bool fb::game::context::handle_connected(fb::socket<fb::game::character>& socket)
{
    fb::logger::info("{}님이 접속했습니다.", socket.IP());
    return true;
}

async::task<bool> fb::game::context::handle_disconnected(fb::socket<fb::game::character>& socket)
{
    auto& config = fb::config::get();
    auto session = socket.data();
    session->init(false);

    fb::logger::info("{}님이 접속을 종료했습니다.", session->name());

    co_await this->save(*session);
    co_await this->post<fb::protocol::internal::request::Logout, fb::protocol::internal::response::Logout>("internal", "/in-game/logout",
        fb::protocol::internal::request::Logout
        {
            UTF8(session->name(), PLATFORM::Windows)
        });
    co_await session->destroy();
    socket.data(nullptr);
    co_return true;
}

void fb::game::context::handle_timer(uint64_t elapsed_milliseconds)
{
    for(auto& [key, value] : this->maps)
        value.on_timer(elapsed_milliseconds);
}

std::string fb::game::context::elapsed_message(const std::string& dt)
{
    auto elapsed = datetime() - datetime(dt);
    if(elapsed.total_milliseconds() > 1000 * 60)
    {
        auto sstream = std::stringstream();
        auto days = elapsed.days();
        if(days > 0)
            sstream << days << "일 ";

        auto hours = elapsed.hours();
        if(hours > 0)
            sstream << hours << "시간 ";

        auto minutes = elapsed.minutes();
        if(minutes > 0)
            sstream << minutes << "분만에 바람으로...";

        auto msg = sstream.str();
        return msg;
    }
    else
    {
        return std::string();
    }
}

fb::game::character* fb::game::context::find(const std::string& name)
{
    auto socket = this->sockets.find([&name] (auto& socket)
    {
        return socket.data()->name() == name;
    });

    if(socket == nullptr)
        return nullptr;

    return socket->data();
}

void fb::game::context::bind_timer(const std::function<void(const datetime&, std::thread::id)>& fn, const std::chrono::steady_clock::duration& duration)
{
    auto& threads = this->threads();
    threads.settimer(fn, duration);
}

void fb::game::context::bind_command(const std::string& cmd, const fb::game::context::command& param)
{
    this->_commands.insert({cmd, param});
}

bool fb::game::context::init_ch(const fb::protocol::db::Character& response, fb::game::character& session, const std::optional<transfer_param>& transfer)
{
    auto map = response.map;
    session.id(response.id);
    session.name(CP949(response.name, PLATFORM::Windows));
    session.last_login(datetime(response.last_login));
    session.admin(response.admin);
    session.color(response.color);
    session.direction(DIRECTION(response.direction));
    session.look(response.look);
    session.money(response.money);
    session.deposited_money(response.deposited_money);
    session.sex(SEX(response.sex));
    session.base_hp(response.base_hp);
    session.hp(response.hp);
    session.base_mp(response.base_mp);
    session.mp(response.mp);
    session.experience(response.exp);
    session.state(STATE(response.state));

    if (response.armor_color.has_value())
        session.armor_color(response.armor_color.value());

    if (response.disguise.has_value())
        session.disguise(response.disguise.value());
    else
        session.undisguise();

    if (this->maps.contains(map) == false)
        return false;

    auto position_x = response.position.x;
    auto position_y = response.position.y;
    if (transfer != std::nullopt)
    {
        map = transfer.value().map;
        position_x = uint32_t(transfer.value().position.x);
        position_y = uint32_t(transfer.value().position.y);
    }
    session.map(&this->maps[map], point16_t(position_x, position_y));
    return true;
}

void fb::game::context::init_items(const std::vector<fb::protocol::db::Item>& response, fb::game::character& session)
{
    for (auto& x : response)
    {
        auto item = this->model.item[x.model].make(*this);

        item->count(x.count);
        if (x.durability.has_value())
            item->durability(x.durability.value());

        if (x.custom_name.has_value() && item->based<fb::model::item>().attr(ITEM_ATTRIBUTE::WEAPON))
            static_cast<fb::game::weapon*>(item)->custom_name(x.custom_name.value());

        if(x.deposited != -1)
            session.deposit_item(*item);
        else if(x.parts == static_cast<uint32_t>(EQUIPMENT_PARTS::UNKNOWN))
            session.items.add(*item, x.index);
        else
            session.items.wear((EQUIPMENT_PARTS)x.parts, static_cast<fb::game::equipment*>(item));
    }
}

void fb::game::context::init_spells(const std::vector<fb::protocol::db::Spell>& response, fb::game::character& session)
{
    for (auto& x : response)
    {
        if (this->model.spell.contains(x.model) == false)
            continue;

        auto& model = this->model.spell[x.model];
        session.spells.add(model, x.slot);
    }
}

fb::game::character* fb::game::context::handle_accepted(fb::socket<fb::game::character>& socket)
{
    return this->make<fb::game::character>(socket);
}

void fb::game::context::send(object& object, const fb::protocol::base::header& header, context::scope scope, bool exclude_self, bool encrypt)
{
    switch(scope)
    {
    case context::scope::SELF:
        object.send(header, encrypt);
        break;

    case context::scope::PIVOT:
    {
        auto nears = object.showings(OBJECT_TYPE::CHARACTER);
        if(!exclude_self)
            object.send(header, encrypt);

        for(auto& x : nears)
            x->send(header, encrypt);
    } break;

    case context::scope::GROUP:
    {
        if(object.is(OBJECT_TYPE::CHARACTER) == false)
            return;

        auto& session = static_cast<fb::game::character&>(object);
        auto group = session.group();
        if(group == nullptr)
            return;

        for(const auto session : group->members())
            session->send(header, encrypt);
    } break;

    case context::scope::MAP:
    {
        auto map = object.map();
        if (map == nullptr)
            return;

        for(const auto& [seq, obj] : object.map()->objects)
        {
            if(exclude_self && obj == object)
                continue;

            obj.send(header, encrypt);
        }
    } break;

    case context::scope::WORLD:
    {
        this->send(header, encrypt);
    } break;

    }
}

void fb::game::context::send(fb::game::object& object, const std::function<std::unique_ptr<fb::protocol::base::header>(const fb::game::object&)>& fn, context::scope scope, bool exclude_self, bool encrypt)
{
    switch(scope)
    {
    case context::scope::SELF:
        object.send(*fn(object).get(), encrypt);
        break;

    case context::scope::PIVOT:
    {
        auto nears = object.showings(OBJECT_TYPE::CHARACTER);
        if(!exclude_self)
            object.send(*fn(object).get(), encrypt);

        for(auto& x : nears)
            x->send(*fn(*x).get(), encrypt);
    } break;

    case context::scope::GROUP:
    {
        if(object.is(OBJECT_TYPE::CHARACTER) == false)
            return;

        auto& session = static_cast<fb::game::character&>(object);
        auto group = session.group();
        if(group == nullptr)
            return;

        for(const auto session : group->members())
            session->send(*fn(*session).get(), encrypt);
    } break;

    case context::scope::MAP:
    {
        for(const auto& [seq, obj] : object.map()->objects)
        {
            if(exclude_self && obj == object)
                continue;

            obj.send(*fn(obj).get(), encrypt);
        }
    } break;

    case context::scope::WORLD:
    {
        this->sockets.each([&fn, encrypt] (auto& socket)
        {
            socket.send(*fn(*socket.data()).get(), encrypt);
        });
    } break;

    }
}

void fb::game::context::send(const fb::protocol::base::header& response, const fb::game::map& map, bool encrypt)
{
    this->sockets.each([&response, &map, encrypt] (auto& socket)
    {
        auto session = socket.data();
        if(session->map() != &map)
            return;

        socket.send(response, encrypt);
    });
}

void fb::game::context::send(const fb::protocol::base::header& response, bool encrypt)
{
    this->sockets.each([&response, encrypt](auto& socket)
    {
        auto session = socket.data();
        session->send(response, encrypt);
    });
}

async::task<void> fb::game::context::save(fb::game::character& session)
{
    auto items = std::vector<fb::protocol::db::Item>();
    for (auto i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = session.items[i];
        if (item == nullptr)
            continue;

        auto protocol = item->to_protocol();
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
        auto item = deposited_items.at(i);
        auto protocol = item->to_protocol();
        protocol.deposited = i;
        items.push_back(protocol);
    }

    auto spells = std::vector<fb::protocol::db::Spell>();
    for (uint8_t i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto spell = session.spells[i];
        if (spell == nullptr)
            continue;

        spells.push_back(fb::protocol::db::Spell
            {
                session.id(),
                i,
                spell->id
            });
    }

    auto&& response = co_await this->post<fb::protocol::db::request::Save, fb::protocol::db::response::Save>("db", "/user/save", fb::protocol::db::request::Save
    {
        session.to_protocol(),
        items,
        spells
    });
}

uint8_t fb::game::context::handle_thread_index(fb::socket<fb::game::character>& socket) const
{
    return this->thread_index(*socket.data());
}

fb::thread* fb::game::context::thread(const fb::game::map* map) const
{
    auto index = this->thread_index(map);
    return this->threads().at(index);
}

uint8_t fb::game::context::thread_index(const fb::game::map* map) const
{
    const auto&             threads = this->threads();
    auto count = threads.count();
    if(count == 0)
        return 0xFF;

    if(map == nullptr)
        return 0;

    return map->model.id % count;
}

fb::thread* fb::game::context::thread(const fb::game::object& obj) const
{
    auto index = this->thread_index(obj);
    return this->threads().at(index);
}

uint8_t fb::game::context::thread_index(const fb::game::object& obj) const
{
    auto map = obj.map();
    return this->thread_index(map);
}

const fb::thread* fb::game::context::current_thread() const
{
    return this->threads().current();
}

void fb::game::context::amqp_thread()
{
    auto& config = fb::config::get();
    auto timeout = timeval{ 5, 0 };
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
            queue1.handler<fb::protocol::internal::response::Pong>([](auto& response) -> async::task<void>
            {
                co_return;
            });
            queue1.handler<fb::protocol::internal::response::KickOut>([this](auto& response) -> async::task<void>
                {
                    auto socket = this->sockets.find([uid = response.uid](fb::socket<fb::game::character>& socket)
                        {
                            auto data = socket.data();
                            return data->id() == uid;
                        });

                    if (socket == nullptr)
                        co_return;

                    socket->cancel();
                });

            queue1.handler<fb::protocol::internal::response::Whisper>([this](auto& response) -> async::task<void>
                {
                    auto socket = this->sockets.find([uid = response.to](fb::socket<fb::game::character>& socket)
                        {
                            auto data = socket.data();
                            return data->id() == uid;
                        });
                    if (socket == nullptr)
                        co_return;
                    
                    auto session = socket->data();
                    if (session == nullptr)
                        co_return;
                    
                    session->message(std::format("{}> {}", CP949(response.from, PLATFORM::Windows), CP949(response.message, PLATFORM::Windows)), MESSAGE_TYPE::NOTIFY);
                });

            auto& queue2 = this->_amqp->declare_queue();
            queue2.bind("amq.direct", "fb.global");
            queue2.handler<fb::protocol::internal::response::Pong>([](auto& response) -> async::task<void>
            {
                co_return;
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
                if(this->_amqp->select(&timeout) == false)
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
void fb::game::context::handle_click_mob(fb::game::character& session, fb::game::mob& mob)
{
    this->send(session, fb::protocol::game::response::session::message(mob.name(), MESSAGE_TYPE::STATE), scope::SELF);
}

void fb::game::context::handle_click_npc(fb::game::character& session, fb::game::npc& npc)
{
    auto& model = npc.based<fb::model::npc>();
    if(model.script.empty())
        return;

    session.dialog.release();

    session.dialog
        .from(model.script.c_str())
        .func("on_interact")
        .pushobject(session)
        .pushobject(npc.based<fb::model::npc>())
        .resume(2);
}

async::task<bool> fb::game::context::handle_login(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::login& request)
{
    auto session = socket.data();
    if(session->inited())
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
        auto id = request.id;
        auto name = std::string(request.name);
        auto from = request.from;
        auto transfer = request.transfer;

        const auto& config = fb::config::get();
        auto delay = config["delay"].asInt();
        co_await this->sleep(std::chrono::seconds(delay));
        if (this->sockets.contains(fd) == false)
            co_return false;

        auto&& response = co_await this->get<fb::protocol::db::response::Login>("db", std::format("/user/login/{}", id));
        if (this->sockets.contains(fd) == false)
            co_return false;

        if (this->init_ch(response.character, *session, transfer) == false)
            co_return false;

        this->send(*session, fb::protocol::game::response::init(), scope::SELF);
        this->send(*session, fb::protocol::game::response::time(this->_time.hours()), scope::SELF);
        this->send(*session, fb::protocol::game::response::session::state(*session, STATE_LEVEL::LEVEL_MIN), scope::SELF);
        
        if(from == fb::protocol::internal::services::LOGIN)
        {
            auto msg = this->elapsed_message(response.character.last_login);
            if(msg.empty() == false)
                session->message(msg, MESSAGE_TYPE::STATE);
        }

        this->send(*session, fb::protocol::game::response::session::state(*session, STATE_LEVEL::LEVEL_MAX), scope::SELF);
        this->send(*session, fb::protocol::game::response::session::option(*session), scope::SELF);

        this->init_items(response.items, *session);
        this->init_spells(response.spells, *session);
    }
    catch(std::exception& /*e*/)
    {
        if (this->sockets.contains(fd) == false)
            co_return false;

        socket.close();
    }

    co_return true;
}

async::task<bool> fb::game::context::handle_direction(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::direction& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    if(session->direction(request.value) == false)
        co_return false;

    co_return true;
}

async::task<bool> fb::game::context::handle_logout(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::exit& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    const auto& config = fb::config::get();
    this->transfer(socket, config["login"]["ip"].asString(), config["login"]["port"].asInt(), fb::protocol::internal::services::GAME);
    co_return true;
}

async::task<bool> fb::game::context::handle_move(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::move& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto map = session->map();
    if(map == nullptr)
        co_return true;

    // TODO: 실제로 이동하지 않고 이동했을때의 위치를 구해서
    // 해당 위치에서 워프가 가능한지 확인하고
    // 워프가능하면 워프처리, 그렇지 않다면 해당 위치로 이동한다.
    auto forward = session->position_forward(request.direction);

    // 워프 위치라면 워프한다.
    const auto warp = map->warpable(forward);
    if(warp != nullptr)
    {
        if (session->condition(warp->condition) == false)
        {
            // 메시지 보냄
            session->message("감히 접근할 수 없습니다.");
        }

        switch (warp->dest.header)
        {
        case DSL::map:
        {
            auto params = dsl::map(warp->dest.params);
            auto& map = this->maps[params.id];
            co_await session->map(&map, point16_t(params.x, params.y));
        }
        break;

        case DSL::world:
        {
            auto params = dsl::world(warp->dest.params);
            auto& world = this->model.world[params.id][params.index];
            session->send(fb::protocol::game::response::map::worlds(this->model, params.id, params.index));
        }
        break;

        default:
            throw std::runtime_error("invalid dsl header");
        }
    }
    else
    {
        session->move(request.direction, request.position);
    }
    co_return true;
}

async::task<bool> fb::game::context::handle_update_move(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::update_move& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto map = session->map();
    if(map == nullptr)
        co_return true;

    if(co_await this->handle_move(socket, request))
        this->send(*session, fb::protocol::game::response::map::update(*map, request.begin, request.size), scope::SELF);

    co_return true;
}

async::task<bool> fb::game::context::handle_attack(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::attack& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    session->attack();
    co_return true;
}

async::task<bool> fb::game::context::handle_pickup(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::pick_up& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    session->items.pickup(request.boost);
    co_return true;
}

async::task<bool> fb::game::context::handle_emotion(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::emotion& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    session->action(ACTION(static_cast<int>(ACTION::EMOTION) + request.value), DURATION::EMOTION);
    co_return true;
}

async::task<bool> fb::game::context::handle_update_map(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::map::update& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto map = session->map();
    if(map == nullptr)
        co_return true;

    this->send(*session, fb::protocol::game::response::map::update(*map, request.position, request.size), scope::SELF);
    co_return true;
}

async::task<bool> fb::game::context::handle_refresh(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::refresh& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    this->send(*session, fb::protocol::game::response::session::position(*session), scope::SELF);
    co_return true;
}

async::task<bool> fb::game::context::handle_active_item(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::item::active& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    session->items.active(request.index);
    co_return true;
}

async::task<bool> fb::game::context::handle_inactive_item(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::item::inactive& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    session->items.inactive(request.parts);
    co_return true;
}

async::task<bool> fb::game::context::handle_drop_item(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::item::drop& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    session->items.drop(request.index, request.all ? -1 : 1);
    co_return true;
}

async::task<bool> fb::game::context::handle_drop_cash(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::item::drop_cash& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto map = session->map();
    if(map == nullptr)
        co_return false;

    auto chunk = std::min(session->money(), request.chunk);

    session->money_drop(chunk);
    co_return true;
}

// TODO : on_notify를 이용
async::task<bool> fb::game::context::handle_front_info(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::front_info& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto map = session->map();
    if(map == nullptr)
        co_return false;

    auto forwards = session->forwards();
    for(auto i = forwards.begin(); i != forwards.end(); i++)
    {
        auto object = *i;
        auto message = object->is(OBJECT_TYPE::ITEM) ? 
            static_cast<fb::game::item*>(*i)->inven_name() : 
            (*i)->name();
        
        session->message(message, MESSAGE_TYPE::STATE);
    }
    
    co_return true;
}

async::task<bool> fb::game::context::handle_self_info(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::self_info& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    this->send(*session, fb::protocol::game::response::session::internal_info(*session, this->model), scope::SELF);
    
    for(auto& [id, buff] : session->buffs)
        this->send(*session, fb::protocol::game::response::spell::buff(*buff), scope::SELF);
    co_return true;
}

async::task<bool> fb::game::context::handle_option_changed(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::change_option& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto option = CUSTOM_SETTING(request.option);

    if(option == CUSTOM_SETTING::RIDE)
    {
        if(session->state() == STATE::RIDING)
            co_await session->unride();
        else
            co_await session->ride();
    }
    else
    {
        session->option_toggle(option);
    }
    co_return true;
}

async::task<bool> fb::game::context::handle_click_object(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::click& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    if(request.fd == 0xFFFFFFFF) // Press F1
    {
        co_return true;
    }

    if(request.fd == 0xFFFFFFFE) // Preff F2
    {
        co_return true;
    }

    if(request.fd == 1 && session->dialog.active())
    {
        session->dialog.pushnil().resume(1);
    }
    else
    {
        auto map = session->map();
        auto you = map->objects[request.fd];
        if(you == nullptr)
            co_return true;

        switch(you->what())
        {
        case OBJECT_TYPE::CHARACTER:
            this->send(*session, fb::protocol::game::response::session::external_info(static_cast<fb::game::character&>(*you), this->model), scope::SELF);
            break;

        case OBJECT_TYPE::MOB:
            this->handle_click_mob(*session, static_cast<mob&>(*you));
            break;

        case OBJECT_TYPE::NPC:
            this->handle_click_npc(*session, static_cast<npc&>(*you));
            break;
        }
    }

    co_return true;
}

// TODO : on_item_detail
async::task<bool> fb::game::context::handle_item_info(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::item::info& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto item = session->items[request.slot];
    if(item == nullptr)
        co_return false;

    this->send(*session, fb::protocol::game::response::item::tip(request.position, item->tip_message()), scope::SELF);
    co_return true;
}

async::task<bool> fb::game::context::handle_itemmix(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::item::mix& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    if(request.indices.size() > CONTAINER_CAPACITY - 1)
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
    if(found == nullptr)
        throw std::runtime_error("no match exception");

    if(found->success.size() - found->source.size() > session->items.free_size())
        throw std::runtime_error("full inven exception");

    for (auto& x : found->source)
    {
        auto params = fb::model::dsl::item(x.params);
        auto deleted_count = uint32_t(0);
        while (deleted_count <= params.count)
        {
            auto item = session->items.find(this->model.item[params.id]);
            auto index = session->items.index(this->model.item[params.id]);
            if (item == nullptr)
                throw std::runtime_error("no match exception");

            auto count = item->count();
            auto deleted = session->items.remove(*item, count);
            if (deleted != nullptr)
                co_await deleted->destroy();

            deleted_count += count;
        }
    }

    auto listener = session->get_listener<fb::game::character>();
    auto success = (std::rand() % 100) < found->percent;
    auto& result = success ? found->success : found->failed;
    for (auto& dsl : result)
    {
        auto params = fb::model::dsl::item(dsl.params);
        auto& model = this->model.item[params.id];
        auto remain = params.count;
        while (remain > 0)
        {
            auto item = this->make<fb::game::item>(this->model.item[params.id]);
            auto count = std::min<uint16_t>(model.capacity, remain);
            item->count(count);
            session->items.add(item);
            remain -= count;
        }
    }

    if (listener != nullptr)
    {
        auto& message = success ? fb::game::message::mix::SUCCESS : fb::game::message::mix::FAILED;
        listener->on_notify(*session, message);
    }

    co_return true;
}

async::task<bool> fb::game::context::handle_trade(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::trade& request)
{
    auto me = socket.data();
    if (me->inited() == false)
        co_return true;

    auto map = me->map();
    if(map == nullptr)
        co_return true;

    auto you = static_cast<fb::game::character*>(map->objects[request.fd]);   // 파트너
    if(you == nullptr)
        co_return true;

    switch(static_cast<trade::state>(request.action))
    {
    case trade::state::REQUEST:
    {
        me->trade.begin(*you);
        break;
    }

    case trade::state::UP_ITEM: // 아이템 올릴때
    {
        auto item = me->items[request.parameter.index - 1];
        if(item == nullptr)
            break;

        me->trade.up(*item);
        break;
    }

    case trade::state::ITEM_COUNT: // 아이템 갯수까지 해서 올릴 때
    {
        me->trade.count(request.parameter.count);
        break;
    }

    case trade::state::UP_MONEY: // 금전 올릴 때
    {
        // 클라이언트가 입력한 금전 양
        me->trade.up(request.parameter.money);
        break;
    }

    case trade::state::CANCEL: // 취소한 경우
    {
        me->trade.cancel();
        break;
    }

    case trade::state::LOCK:
    {
        me->trade.lock();
        break;
    }
    }

    co_return true;
}

async::task<bool> fb::game::context::handle_group(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::group& request)
{
    auto me = socket.data();
    if (me->inited() == false)
        co_return true;

    try
    {
        std::stringstream sstream;
        auto you = this->find(request.name);
        if(you == nullptr)
        {
            sstream << request.name << fb::game::message::group::CANNOT_FIND_TARGET;
            throw std::runtime_error(sstream.str());
        }

        if(me == you)
        {
            throw std::runtime_error(fb::game::message::group::CANNOT_FIND_TARGET);
        }

        if(me->option(CUSTOM_SETTING::GROUP) == false)
        {
            throw std::runtime_error(fb::game::message::group::DISABLED_MINE);
        }

        if(you->option(CUSTOM_SETTING::GROUP) == false)
        {
            throw std::runtime_error(fb::game::message::group::DISABLED_TARGET);
        }

        auto mine = me->group();
        auto your = you->group();

        if(mine == nullptr) // 새로 그룹 만들기
        {
            if(your != nullptr)
            {
                sstream << request.name << fb::game::message::group::ALREADY_JOINED;
                throw std::runtime_error(sstream.str());
            }

            mine = fb::game::group::create(*me);
            mine->enter(*you);

            sstream << me->name() << fb::game::message::group::JOINED;
            this->send(*me, fb::protocol::game::response::message(sstream.str(), MESSAGE_TYPE::STATE), scope::GROUP);

            sstream.str("");
            sstream << request.name << fb::game::message::group::JOINED;
            this->send(*me, fb::protocol::game::response::message(sstream.str(), MESSAGE_TYPE::STATE), scope::GROUP);
        }
        else // 기존 그룹에 초대하기
        {
            auto& leader = mine->leader();
            if(me != &leader)
            {
                throw std::runtime_error(fb::game::message::group::NOT_OWNER);
            }

            if(mine != your && your != nullptr)
            {
                sstream << request.name << fb::game::message::group::ALREADY_JOINED;
                throw std::runtime_error(sstream.str());
            }

            if(mine == your)
            {
                sstream << request.name << fb::game::message::group::LEFT;
                this->send(*me, fb::protocol::game::response::message(sstream.str(), MESSAGE_TYPE::STATE), scope::GROUP);
                mine->leave(*you);
                co_return true;
            }

            if(mine->enter(*you) == nullptr)
            {
                throw std::runtime_error(fb::game::message::group::FULL_MEMBER);
            }
            
            sstream << request.name << fb::game::message::group::JOINED;
            this->send(*me, fb::protocol::game::response::message(sstream.str(), MESSAGE_TYPE::STATE), scope::GROUP);
            this->send(leader, fb::protocol::game::response::message(sstream.str(), MESSAGE_TYPE::STATE), scope::GROUP);
        }
    }
    catch(std::exception& e)
    {
        this->send(*me, fb::protocol::game::response::message(e.what(), MESSAGE_TYPE::STATE), scope::GROUP);
    }

    co_return true;
}

async::task<bool> fb::game::context::handle_user_list(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::user_list& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    this->send(*session, fb::protocol::game::response::user_list(*session, this->sockets), scope::SELF);
    co_return true;
}

async::task<bool> fb::game::context::handle_chat(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::chat& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    if (session->admin() && co_await handle_command(*session, request.message))
        co_return true;
    
    session->chat(request.message, request.shout);

    auto npcs = std::vector<fb::game::npc*>();
    if (request.shout)
    {
        for (auto& [fd, obj] : session->map()->objects)
        {
            if (obj.is(OBJECT_TYPE::NPC))
                npcs.push_back(static_cast<fb::game::npc*>(&obj));
        }
    }
    else
    {
        for (auto npc : session->showings(OBJECT_TYPE::NPC))
        {
            npcs.push_back(static_cast<fb::game::npc*>(npc));
        }
    }
    
    session->inline_interaction(request.message, npcs);
    
    co_return true;
}

async::task<bool> fb::game::context::handle_board(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::board::board& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto fd = session->fd();
    switch(request.action)
    {
    case BOARD_ACTION::SECTIONS:
    {
        this->send(*session, fb::protocol::game::response::board::sections(this->model), scope::SELF);
    } break;

    case BOARD_ACTION::ARTICLES:
    {
        try
        {
            if(this->model.board.contains(request.section) == false)
                throw std::runtime_error(fb::game::message::board::SECTION_NOT_EXIST);

            auto section = &this->model.board[request.section];
            auto offset = request.offset;

            auto&& response = co_await this->get<fb::protocol::db::response::GetArticleList>("db", std::format("/board/{}&offset={}", section->id, offset));
            if (this->sockets.contains(fd) == false)
                co_return false;

            auto articles = std::list<fb::game::board::article>();
            for (auto& summary : response.summary_list)
            {
                auto dt = datetime(summary.created_date);
                articles.push_back(fb::game::board::article
                { 
                    summary.id,
                    section->id,
                    summary.user,
                    summary.user_name,
                    summary.title,
                    (uint8_t)dt.month(),
                    (uint8_t)dt.day()
                });
            }

            auto button_flags = BOARD_BUTTON_ENABLE::UP;
            if (session->condition(section->condition) == false)
                button_flags |= BOARD_BUTTON_ENABLE::WRITE;
            this->send(*session, fb::protocol::game::response::board::articles(*section, articles, button_flags), scope::SELF);
        }
        catch (std::exception& e)
        {
            this->send(*session, fb::protocol::game::response::board::message(e.what(), false, false), scope::SELF);
        }
    } break;

    case BOARD_ACTION::ARTICLE:
    {
        try
        {
            if(this->model.board.contains(request.section) == false)
                throw std::runtime_error(fb::game::message::board::SECTION_NOT_EXIST);

            auto section = &this->model.board[request.section]; // 코루틴땜시 포인터로
            auto&& response = co_await this->get<fb::protocol::db::response::GetArticle>("db", std::format("/board/{}/{}", section->id, request.article));
            if (this->sockets.contains(fd) == false)
                co_return false;

            if (response.success == false)
            {
                this->send(*session, fb::protocol::game::response::board::message(fb::game::message::board::ARTICLE_NOT_EXIST, false, false), scope::SELF);
                co_return true;
            }

            auto dt = datetime(response.article.created_date);
            auto button_flags = BOARD_BUTTON_ENABLE::NONE;
            if (response.next)
                button_flags |= BOARD_BUTTON_ENABLE::NEXT;

            if (session->condition(section->condition) == false)
                button_flags |= BOARD_BUTTON_ENABLE::WRITE;

            this->send(*session, fb::protocol::game::response::board::article(fb::game::board::article
                {
                    response.article.id,
                    section->id,
                    response.article.user,
                    response.article.user_name,
                    response.article.title,
                    (uint8_t)dt.month(), (uint8_t)dt.day(),
                    response.article.contents
                }, button_flags), scope::SELF);
        }
        catch (std::exception& e)
        {
            this->send(*session, fb::protocol::game::response::board::message(e.what(), false, false), scope::SELF);
        }
    } break;

    case BOARD_ACTION::WRITE:
    {
        try
        {
            if (this->model.board.contains(request.section) == false)
                throw std::runtime_error(fb::game::message::board::SECTION_NOT_EXIST);

            auto section = &this->model.board[request.section];
            if (session->condition(section->condition) == false)
                throw std::runtime_error(fb::game::message::board::NOT_AUTH);

            if (request.title.length() > 64)
                throw std::runtime_error(fb::game::message::board::TOO_LONG_TITLE);

            if(request.contents.length() > 256)
                throw std::runtime_error(fb::game::message::board::TOO_LONG_CONTENTS);

            auto&& response = co_await this->post<fb::protocol::db::request::WriteArticle, fb::protocol::db::response::WriteArticle>("db", "/board/write", fb::protocol::db::request::WriteArticle
            {
                section->id, 
                session->id(), 
                request.title, 
                request.contents
            });

            if (this->sockets.contains(fd) == false)
                co_return false;

            if (response.success == false)
                throw std::runtime_error("게시글 작성 실패");

            this->send(*session, fb::protocol::game::response::board::message(fb::game::message::board::WRITE, true, true), scope::SELF);
        }
        catch (std::exception& e)
        {
            this->send(*session, fb::protocol::game::response::board::message(e.what(), false, false), scope::SELF);
        }
    } break;

    case BOARD_ACTION::DELETE:
    {
        try
        {
            if (this->model.board.contains(request.section) == false)
                throw std::runtime_error(fb::game::message::board::SECTION_NOT_EXIST);

            auto section = &this->model.board[request.section];
            if (session->condition(section->condition) == false)
                throw std::runtime_error(fb::game::message::board::NOT_AUTH);

            auto&& response = co_await this->post<fb::protocol::db::request::DeleteArticle, fb::protocol::db::response::DeleteArticle>("db", "/board/delete", fb::protocol::db::request::DeleteArticle
                {
                    request.article, session->id()
                });

            if (this->sockets.contains(fd) == false)
                co_return false;

            switch(response.result)
            {
                case -1: // article not found
                    throw std::runtime_error(fb::game::message::board::ARTICLE_NOT_EXIST);

                case -2: // article deleted
                    throw std::runtime_error(fb::game::message::board::ARTICLE_NOT_EXIST);

                case -3: // no authenticate
                    throw std::runtime_error(fb::game::message::board::NOT_AUTH);
            }

            this->send(*session, fb::protocol::game::response::board::message(fb::game::message::board::SUCCESS_DELETE, true, false), scope::SELF);
        }
        catch(std::exception& e)
        {
            this->send(*session, fb::protocol::game::response::board::message(e.what(), false, false), scope::SELF);
        }
    } break;

    default:
        co_return false;
    }

    co_return true;
}

async::task<bool> fb::game::context::handle_swap(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::swap& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    switch(request.type)
    {
    case SWAP_TYPE::SPELL:
    {
        session->spells.swap(request.src-1, request.dst-1);
        break;
    }

    case SWAP_TYPE::ITEM:
    {
        session->items.swap(request.src-1, request.dst-1);
        break;
    }

    default:
        co_return false;
    }

    co_return true;
}

async::task<bool> fb::game::context::handle_dialog(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::dialog& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    if(session->dialog.active() == false)
        co_return true;

    switch(request.interaction)
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
        if(request.action == 0x02) // OK button
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
//async::task<bool>bool fb::game::context::handle_dialog_1(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::dialog1& request)
//{
//    co_return this->handle_dialog(socket, (const fb::protocol::game::request::dialog&)request);
//}
//async::task<bool>bool fb::game::context::handle_dialog_2(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::dialog2& request)
//{
//    co_return this->handle_dialog(socket, (const fb::protocol::game::request::dialog&)request);
//}

async::task<bool> fb::game::context::handle_throw_item(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::item::throws& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    session->items.throws(request.index);
    co_return true;
}

async::task<bool> fb::game::context::handle_spell(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::spell::use& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    if(request.slot > CONTAINER_CAPACITY - 1)
        co_return false;

    auto model = session->spells[request.slot];
    if(model == nullptr)
        co_return false;

    request.parse(model->type);
    switch(model->type)
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

async::task<bool> fb::game::context::handle_door(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::door& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto thread = lua::get();
    if(thread == nullptr)
        co_return true;
    
    thread->from("scripts/common/door.lua")
        .func("on_door")
        .pushobject(session)
        .resume(1);
    co_return true;
}

async::task<bool> fb::game::context::handle_whisper(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::whisper& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto fd = session->fd();
    auto& from = session->name();
    auto to = std::string(request.name);
    auto message = std::string(request.message);
    try
    {
        auto&& response = co_await this->post<fb::protocol::internal::request::Whisper, fb::protocol::internal::response::Whisper>("internal", "/in-game/whisper", fb::protocol::internal::request::Whisper
            {
                UTF8(from, PLATFORM::Windows), 
                UTF8(to, PLATFORM::Windows), 
                UTF8(message, PLATFORM::Windows)
            });
        if (this->sockets.contains(fd) == false)
            co_return false;

        std::stringstream sstream;
        if(response.success)
            sstream << to << "< " << message;
        else
            sstream << to << "님은 바람의나라에 없습니다.";

        session->message(sstream.str(), MESSAGE_TYPE::NOTIFY);
    }
    catch(std::exception& /*e*/)
    {
        if (this->sockets.contains(fd) == false)
            co_return false;

        session->message("서버 오류", MESSAGE_TYPE::NOTIFY);
    }
    co_return true;
}

async::task<bool> fb::game::context::handle_world(fb::socket<fb::game::character>& socket, const fb::protocol::game::request::map::world& request)
{
    auto session = socket.data();
    if (session->inited() == false)
        co_return true;

    auto& world = this->model.world[request.value];
    auto& before = world[request.before];
    auto& after = world[request.after];

    if(session->map() == &this->maps[after.map])
    {
        session->refresh_map();
    }
    else
    {
        co_await session->map(&this->maps[after.map], after.position);
        this->save(*session);
    }
    co_return true;
}

void fb::game::context::handle_mob_action(const datetime& now, std::thread::id id)
{
    for(auto& [_, map] : this->maps)
    {
        if(map.active == false)
            continue;

        auto thread = this->thread(&map);
        if(thread != nullptr && thread->id() != id)
            continue;

        const auto mobs = map.activateds(OBJECT_TYPE::MOB);

        for(auto x : mobs)
        {
            auto mob = static_cast<fb::game::mob*>(x);
            if(mob->alive() == false)
                continue;

            auto target = mob->target();
            if(target == nullptr || map.objects.contains(*target) == false || target->alive() == false)
                mob->target(nullptr);

            if(mob->action())
                continue;

            mob->AI(now);
        }
    }
}

void fb::game::context::handle_mob_respawn(const datetime& now, std::thread::id id)
{
    for (auto& rezen : this->rezen)
    {
        rezen.spawn(id);
    }
}

void fb::game::context::handle_buff_timer(const datetime& now, std::thread::id id)
{
    for(auto& [_, map] : this->maps)
    {
        if(map.active == false)
            continue;

        auto thread = this->thread(&map);
        if(thread != nullptr && thread->id() != id)
            continue;

        if(map.objects.size() == 0)
            continue;

        for(auto& [fd, obj] : map.objects)
        {
            if(obj.buffs.size() == 0)
                continue;

            auto ended_buffs = std::vector<fb::game::buff*>();
            for (auto& [id, buff] : obj.buffs)
            {
                buff->time_dec(1);
                if (buff->time() <= 0ms)
                    ended_buffs.push_back(buff);
            }

            for (auto& buff : ended_buffs)
                obj.buffs.remove(buff->model);
        }
    }
}

void fb::game::context::handle_save_timer(const datetime& now, std::thread::id id)
{
    auto& c = console::get();

    for(auto& [_, map] : this->maps)
    {
        if(map.active == false)
            continue;

        auto thread = this->thread(&map);
        if(thread != nullptr && thread->id() != id)
            continue;

        if(map.objects.size() == 0)
            continue;

        for(auto& [fd, obj] : map.objects)
        {
            if(obj.is(OBJECT_TYPE::CHARACTER) == false)
                continue;

            auto session = static_cast<fb::game::character*>(&obj);
            this->save(*session);
        }
    }
}

void fb::game::context::handle_time(const datetime& now, std::thread::id id)
{
    auto updated = datetime();
    if(this->_time.hours() != updated.hours())
        this->send(fb::protocol::game::response::time(updated.hours()));

    this->_time = updated;
}

async::task<bool> fb::game::context::handle_command(fb::game::character& session, const std::string& message)
{
    if(message.starts_with('/') == false)
        co_return false;

    std::vector<std::string>        splitted;
    std::istringstream              sstream(message.substr(1));
    std::string                     unit;
    while (std::getline(sstream, unit, ' '))
    {
        splitted.push_back(unit);
    }

    if(splitted.empty())
        co_return false;

    auto found = this->_commands.find(splitted[0]);
    if (found == this->_commands.end())
        co_return false;

    if(found->second.admin && session.admin() == false)
        co_return false;

    Json::Value parameters;
    for (auto i = splitted.begin() + 1; i != splitted.end(); i++)
    {
        auto digit = std::all_of
        (
            (*i).begin(), (*i).end(),
            [] (uint8_t c)
            {
                return std::isdigit(c);
            }
        );
        if(digit)
            parameters.append(std::stoi(*i));
        else
            parameters.append(*i);
    }

    co_return co_await found->second.fn(session, parameters);
}
