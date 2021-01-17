#include "model/acceptor/acceptor.game.h"
using namespace fb::game;

IMPLEMENT_LUA_EXTENSION(fb::game::acceptor, "")
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::lua::luable, "fb.game.luable")
{"__gc",                fb::game::lua::luable::builtin_gc},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::spell, "fb.game.spell")
{"type",                fb::game::spell::builtin_type},
{"name",                fb::game::spell::builtin_name},
{"message",             fb::game::spell::builtin_message},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::map, "fb.game.map")
{"name",                fb::game::map::builtin_name},
{"objects",             fb::game::map::builtin_objects},
{"width",               fb::game::map::builtin_width},
{"height",              fb::game::map::builtin_height},
{"area",                fb::game::map::builtin_area},
{"movable",             fb::game::map::builtin_movable},
{"door",                fb::game::map::builtin_door},
{"doors",               fb::game::map::builtin_doors},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::object::master, "fb.game.object.core")
{"name",                fb::game::object::master::builtin_name},
{"look",                fb::game::object::master::builtin_look},
{"color",               fb::game::object::master::builtin_color},
{"dialog",              fb::game::object::master::builtin_dialog},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::mob::master, "fb.game.mob.core")
{"speed",               fb::game::mob::master::builtin_speed},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::mob, "fb.game.mob")
{"__eq",                fb::game::object::builtin_eq},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::npc::master, "fb.game.npc.core")
{"input_dialog",        fb::game::npc::master::builtin_input_dialog},
{"menu_dialog",         fb::game::npc::master::builtin_menu_dialog},
{"item_dialog",         fb::game::npc::master::builtin_item_dialog},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::npc, "fb.game.npc")
{"__eq",                fb::game::object::builtin_eq},
{"input_dialog",        fb::game::npc::builtin_input_dialog},
{"menu_dialog",         fb::game::npc::builtin_menu_dialog},
{"item_dialog",         fb::game::npc::builtin_item_dialog},
END_LUA_EXTENSION


IMPLEMENT_LUA_EXTENSION(fb::game::object, "fb.game.object")
{"core",                fb::game::object::builtin_core},
{"__eq",                fb::game::object::builtin_eq},
{"__tostring",          fb::game::object::builtin_tostring},
{"id",                  fb::game::object::builtin_id},
{"name",                fb::game::object::builtin_name},
{"dialog",              fb::game::object::builtin_dialog},
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


IMPLEMENT_LUA_EXTENSION(fb::game::life::master, "fb.game.life.core")
{"hp",                  fb::game::life::master::builtin_hp},
{"mp",                  fb::game::life::master::builtin_mp},
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
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::item::master, "fb.game.item.core")
{"make",                fb::game::item::master::builtin_make},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::item, "fb.game.item")
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::session, "fb.game.session")
{"__eq",                fb::game::object::builtin_eq},
{"look",                fb::game::session::builtin_look},
{"color",               fb::game::session::builtin_color},
{"money",               fb::game::session::builtin_money},
{"exp",                 fb::game::session::builtin_exp},
{"base_hp",             fb::game::session::builtin_base_hp},
{"base_mp",             fb::game::session::builtin_base_mp},
{"str",                 fb::game::session::builtin_strength},
{"dex",                 fb::game::session::builtin_dexterity},
{"int",                 fb::game::session::builtin_intelligence},
{"item",                fb::game::session::builtin_item},
{"items",               fb::game::session::builtin_items},
{"dropitem",            fb::game::session::builtin_item_drop},
{"mkitem",              fb::game::session::builtin_mkitem},
{"rmitem",              fb::game::session::builtin_rmitem},
{"state",               fb::game::session::builtin_state},
{"disguise",            fb::game::session::builtin_disguise},
{"class",               fb::game::session::builtin_class},
{"level",               fb::game::session::builtin_level},
{"group",               fb::game::session::builtin_group},
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

acceptor::acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay, const INTERNAL_CONNECTION& internal_connection) : 
    fb::acceptor<fb::game::session>(context, port, accept_delay, internal_connection),
    _timer(context)
{
    const auto& config = fb::config::get();

    lua::env<acceptor>("acceptor", this);
    lua::bind_class<lua::luable>();
    lua::bind_class<spell, lua::luable>();
    lua::bind_class<map, lua::luable>();
    lua::bind_class<door, lua::luable>();
    lua::bind_class<group, lua::luable>();
    lua::bind_class<object::master, lua::luable>();     lua::bind_class<object, lua::luable>();
    lua::bind_class<life::master, object::master>();    lua::bind_class<life, object>();
    lua::bind_class<mob::master, life::master>();       lua::bind_class<mob, life>();
    lua::bind_class<npc::master, object::master>();     lua::bind_class<npc, object>();
    lua::bind_class<item::master, object::master>();    lua::bind_class<item, object>();
    lua::bind_class<fb::game::session, life>();

    lua::bind_function("sleep",     builtin_sleep);
    lua::bind_function("name2mob",  builtin_name2mob);
    lua::bind_function("name2item", builtin_name2item);
    lua::bind_function("name2npc",  builtin_name2npc);
    lua::bind_function("name2map",  builtin_name2map);
    lua::bind_function("timer",     builtin_timer);
    lua::bind_function("weather",   builtin_weather);
    
    this->bind<fb::protocol::game::request::login>            (0x10, std::bind(&acceptor::handle_login,           this, std::placeholders::_1, std::placeholders::_2));   // 게임서버 접속 핸들러
    this->bind<fb::protocol::game::request::direction>        (0x11, std::bind(&acceptor::handle_direction,       this, std::placeholders::_1, std::placeholders::_2));   // 방향전환 핸들러
    this->bind<fb::protocol::game::request::exit>             (0x0B, std::bind(&acceptor::handle_exit,            this, std::placeholders::_1, std::placeholders::_2));   // 접속 종료
    this->bind<fb::protocol::game::request::update_move>      (0x06, std::bind(&acceptor::handle_update_move,     this, std::placeholders::_1, std::placeholders::_2));   // 이동과 맵 데이터 업데이트 핸들러
    this->bind<fb::protocol::game::request::move>             (0x32, std::bind(&acceptor::handle_move,            this, std::placeholders::_1, std::placeholders::_2));   // 이동 핸들러
    this->bind<fb::protocol::game::request::attack>           (0x13, std::bind(&acceptor::handle_attack,          this, std::placeholders::_1, std::placeholders::_2));   // 공격 핸들러
    this->bind<fb::protocol::game::request::pick_up>          (0x07, std::bind(&acceptor::handle_pickup,          this, std::placeholders::_1, std::placeholders::_2));   // 아이템 줍기 핸들러
    this->bind<fb::protocol::game::request::emotion>          (0x1D, std::bind(&acceptor::handle_emotion,         this, std::placeholders::_1, std::placeholders::_2));   // 감정표현 핸들러
    this->bind<fb::protocol::game::request::map::update>      (0x05, std::bind(&acceptor::handle_update_map,      this, std::placeholders::_1, std::placeholders::_2));   // 맵 데이터 업데이트 핸들러
    this->bind<fb::protocol::game::request::refresh>          (0x38, std::bind(&acceptor::handle_refresh,         this, std::placeholders::_1, std::placeholders::_2));   // 새로고침 핸들러
    this->bind<fb::protocol::game::request::item::active>     (0x1C, std::bind(&acceptor::handle_active_item,     this, std::placeholders::_1, std::placeholders::_2));   // 아이템 사용 핸들러
    this->bind<fb::protocol::game::request::item::inactive>   (0x1F, std::bind(&acceptor::handle_inactive_item,   this, std::placeholders::_1, std::placeholders::_2));   // 아이템 장착 해제 핸들러
    this->bind<fb::protocol::game::request::item::drop>       (0x08, std::bind(&acceptor::handle_drop_item,       this, std::placeholders::_1, std::placeholders::_2));   // 아이템 버리기 핸들러
    this->bind<fb::protocol::game::request::item::drop_cash>  (0x24, std::bind(&acceptor::handle_drop_cash,       this, std::placeholders::_1, std::placeholders::_2));   // 금전 버리기 핸들러
    this->bind<fb::protocol::game::request::front_info>       (0x09, std::bind(&acceptor::handle_front_info,      this, std::placeholders::_1, std::placeholders::_2));   // 앞방향 정보 핸들러
    this->bind<fb::protocol::game::request::self_info>        (0x2D, std::bind(&acceptor::handle_self_info,       this, std::placeholders::_1, std::placeholders::_2));   // 나 자신의 정보 핸들러
    this->bind<fb::protocol::game::request::change_option>    (0x1B, std::bind(&acceptor::handle_option_changed,  this, std::placeholders::_1, std::placeholders::_2));   // 옵션 설정 핸들러
    this->bind<fb::protocol::game::request::click>            (0x43, std::bind(&acceptor::handle_click_object,    this, std::placeholders::_1, std::placeholders::_2));   // 오브젝트 클릭 핸들러
    this->bind<fb::protocol::game::request::item::info>       (0x66, std::bind(&acceptor::handle_item_info,       this, std::placeholders::_1, std::placeholders::_2));   // 인벤토리 우클릭 핸들러
    this->bind<fb::protocol::game::request::item::mix>        (0x6B, std::bind(&acceptor::handle_itemmix,         this, std::placeholders::_1, std::placeholders::_2));   // 아이템 조합 핸들러
    this->bind<fb::protocol::game::request::trade>            (0x4A, std::bind(&acceptor::handle_trade,           this, std::placeholders::_1, std::placeholders::_2));   // 교환 핸들러
    this->bind<fb::protocol::game::request::group>            (0x2E, std::bind(&acceptor::handle_group,           this, std::placeholders::_1, std::placeholders::_2));   // 그룹 핸들러
    this->bind<fb::protocol::game::request::user_list>        (0x18, std::bind(&acceptor::handle_user_list,       this, std::placeholders::_1, std::placeholders::_2));   // 유저 리스트 핸들러
    this->bind<fb::protocol::game::request::chat>             (0x0E, std::bind(&acceptor::handle_chat,            this, std::placeholders::_1, std::placeholders::_2));   // 유저 채팅 핸들러
    this->bind<fb::protocol::game::request::board::board>     (0x3B, std::bind(&acceptor::handle_board,           this, std::placeholders::_1, std::placeholders::_2));   // 게시판 섹션 리스트 핸들러
    this->bind<fb::protocol::game::request::swap>             (0x30, std::bind(&acceptor::handle_swap,            this, std::placeholders::_1, std::placeholders::_2));   // 스펠 순서 변경
    this->bind<fb::protocol::game::request::dialog>           (0x3A, std::bind(&acceptor::handle_dialog,          this, std::placeholders::_1, std::placeholders::_2));   // 다이얼로그
    this->bind<fb::protocol::game::request::dialog>           (0x39, std::bind(&acceptor::handle_dialog,          this, std::placeholders::_1, std::placeholders::_2));   // 다이얼로그
    this->bind<fb::protocol::game::request::item::throws>     (0x17, std::bind(&acceptor::handle_throw_item,      this, std::placeholders::_1, std::placeholders::_2));   // 아이템 던지기 핸들러
    this->bind<fb::protocol::game::request::spell::use>       (0x0F, std::bind(&acceptor::handle_spell,           this, std::placeholders::_1, std::placeholders::_2));   // 스펠 핸들러
    this->bind<fb::protocol::game::request::door>             (0x20, std::bind(&acceptor::handle_door,            this, std::placeholders::_1, std::placeholders::_2));   // 도어 핸들러
    this->bind<fb::protocol::game::request::whisper>          (0x19, std::bind(&acceptor::handle_whisper,         this, std::placeholders::_1, std::placeholders::_2));   // 귓속말 핸들러
    this->bind<fb::protocol::game::request::map::world>       (0x3F, std::bind(&acceptor::handle_world,           this, std::placeholders::_1, std::placeholders::_2));   // 월드맵 핸들러

    this->bind<fb::protocol::internal::response::transfer>    (std::bind(&acceptor::handle_in_transfer,           this, std::placeholders::_1, std::placeholders::_2));   // 서버 이동
    this->bind<fb::protocol::internal::response::whisper>     (std::bind(&acceptor::handle_in_whisper,            this, std::placeholders::_1, std::placeholders::_2));   // 귓속말
    this->bind<fb::protocol::internal::response::message>     (std::bind(&acceptor::handle_in_message,            this, std::placeholders::_1, std::placeholders::_2));   // 월드 메시지
    this->bind<fb::protocol::internal::response::logout>      (std::bind(&acceptor::handle_in_logout,             this, std::placeholders::_1, std::placeholders::_2));   // 접속종료

    this->bind_timer(std::bind(&acceptor::handle_mob_action,   this, std::placeholders::_1), std::chrono::milliseconds(100));   // 몹 행동 타이머
    this->bind_timer(std::bind(&acceptor::handle_mob_respawn,  this, std::placeholders::_1), std::chrono::seconds(1));          // 몹 리젠 타이머
    this->bind_timer(std::bind(&acceptor::handle_buff_timer,   this, std::placeholders::_1), std::chrono::seconds(1));          // 버프 타이머

    this->bind_command("맵이동", std::bind(&acceptor::handle_command_map, this, std::placeholders::_1, std::placeholders::_2));
    this->bind_command("사운드", std::bind(&acceptor::handle_command_sound, this, std::placeholders::_1, std::placeholders::_2));
    this->bind_command("액션", std::bind(&acceptor::handle_command_action, this, std::placeholders::_1, std::placeholders::_2));
    this->bind_command("날씨", std::bind(&acceptor::handle_command_weather, this, std::placeholders::_1, std::placeholders::_2));
    this->bind_command("밝기", std::bind(&acceptor::handle_command_bright, this, std::placeholders::_1, std::placeholders::_2));
    this->bind_command("타이머", std::bind(&acceptor::handle_command_timer, this, std::placeholders::_1, std::placeholders::_2));
    this->bind_command("이펙트", std::bind(&acceptor::handle_command_effect, this, std::placeholders::_1, std::placeholders::_2));
    this->bind_command("변신", std::bind(&acceptor::handle_command_disguise, this, std::placeholders::_1, std::placeholders::_2));
    this->bind_command("변신해제", std::bind(&acceptor::handle_command_undisguise, this, std::placeholders::_1, std::placeholders::_2));
    this->bind_command("마법배우기", std::bind(&acceptor::handle_command_spell, this, std::placeholders::_1, std::placeholders::_2));
    this->bind_command("몬스터생성", std::bind(&acceptor::handle_command_mob, this, std::placeholders::_1, std::placeholders::_2));
    this->bind_command("직업바꾸기", std::bind(&acceptor::handle_command_class, this, std::placeholders::_1, std::placeholders::_2));
    this->bind_command("레벨바꾸기", std::bind(&acceptor::handle_command_level, this, std::placeholders::_1, std::placeholders::_2));
    this->bind_command("아이템생성", std::bind(&acceptor::handle_command_item, this, std::placeholders::_1, std::placeholders::_2));
    this->bind_command("월드맵", std::bind(&acceptor::handle_command_world, this, std::placeholders::_1, std::placeholders::_2));
    this->bind_command("스크립트", std::bind(&acceptor::handle_command_script, this, std::placeholders::_1, std::placeholders::_2));
    this->bind_command("머리바꾸기", std::bind(&acceptor::handle_command_hair, this, std::placeholders::_1, std::placeholders::_2));
    this->bind_command("머리염색", std::bind(&acceptor::handle_command_hair_color, this, std::placeholders::_1, std::placeholders::_2));
    this->bind_command("갑옷염색", std::bind(&acceptor::handle_command_armor_color, this, std::placeholders::_1, std::placeholders::_2));
}

acceptor::~acceptor()
{ 
    
}

bool acceptor::handle_connected(fb::socket<fb::game::session>& socket)
{
    return true;
}

bool acceptor::handle_disconnected(fb::socket<fb::game::session>& socket)
{
    auto session = socket.data();

    auto& c = fb::console::get();
    c.puts("%s님이 접속을 종료했습니다.", session->name().c_str());

    this->on_save(*session);
    session->map(nullptr);

    this->_internal->send(fb::protocol::internal::request::logout(session->name()));
    return true;
}

void fb::game::acceptor::handle_timer(uint64_t elapsed_milliseconds)
{
    for(auto pair : fb::game::table::maps)
        pair.second->handle_timer(elapsed_milliseconds);
}

fb::game::session* fb::game::acceptor::find(const std::string& name) const
{
    auto i = std::find_if(this->sockets.begin(), this->sockets.end(), 
        [&name] (std::pair<uint32_t, fb::socket<fb::game::session>*> pair) 
        {
            return pair.second->data()->name() == name;
        });

    if(i == this->sockets.end())
        return nullptr;

    return i->second->data();
}

bool fb::game::acceptor::exists(const fb::game::object& object) const
{
    return this->_hash_dict.contains(&object);
}

void fb::game::acceptor::bind_timer(std::function<void(uint64_t)> fn, const std::chrono::steady_clock::duration& duration)
{
    this->_timer.push(fn, duration);
}

void fb::game::acceptor::bind_command(const std::string& command, std::function<bool(fb::game::session&, Json::Value&)> fn)
{
    this->_command_dict.insert(std::make_pair(command, fn));
}

fb::game::session* fb::game::acceptor::handle_accepted(fb::socket<fb::game::session>& socket)
{
    auto session = new fb::game::session(socket, this);
    return session;
}

void fb::game::acceptor::send(object& object, const fb::protocol::base::header& header, acceptor::scope scope, bool exclude_self, bool encrypt)
{
    switch(scope)
    {
    case acceptor::scope::SELF:
        object.send(header, encrypt);
        break;

    case acceptor::scope::PIVOT:
    {
        auto nears = object.showings(object::types::SESSION);
        if(!exclude_self)
            object.send(header, encrypt);

        for(auto& x : nears)
            x->send(header, encrypt);
        break;
    }

    case acceptor::scope::GROUP:
    {
        if(object.is(object::types::SESSION) == false)
            return;

        auto& session = static_cast<fb::game::session&>(object);
        auto group = session.group();
        if(group == nullptr)
            return;

        for(const auto session : group->members())
            session->send(header, encrypt);

        break;
    }

    case acceptor::scope::MAP:
    {
        for(const auto pair : object.map()->objects)
        {
            if(exclude_self && pair.second == &object)
                continue;

            pair.second->send(header, encrypt);
        }

        break;
    }

    case acceptor::scope::WORLD:
    {
        this->send(header, encrypt);
        break;
    }

    }
}

void fb::game::acceptor::send(const fb::protocol::base::header& response, const fb::game::map& map, bool encrypt)
{
    for(auto pair : this->sockets)
    {
        auto session = pair.second->data();
        if(session->map() != &map)
            continue;

        session->send(response, encrypt);
    }
}

void fb::game::acceptor::send(const fb::protocol::base::header& response, bool encrypt)
{
    for(const auto pair : this->sockets)
    {
        auto session = pair.second->data();
        session->send(response, encrypt);
    }
}

// TODO : 클릭도 인터페이스로
void fb::game::acceptor::handle_click_mob(fb::game::session& session, fb::game::mob& mob)
{
    this->send(session, fb::protocol::game::response::session::message(mob.name(), message::type::STATE), scope::SELF);
}

void fb::game::acceptor::handle_click_npc(fb::game::session& session, fb::game::npc& npc)
{
    session.dialog
        .from("scripts/script.lua")
        .func("handle_click")
        .pushobject(session)
        .pushobject(npc)
        .resume(2);
}

bool fb::game::acceptor::handle_in_transfer(fb::internal::socket<>& socket, const fb::protocol::internal::response::transfer& response)
{
    auto client = this->sockets[response.fd];

    try
    {
        if(client == nullptr)
            return true;

        if(response.code != fb::protocol::internal::response::SUCCESS)
            throw std::runtime_error("비바람이 휘몰아치고 있습니다.");

        auto session = client->data();
        session->map(fb::game::table::maps[response.map], fb::game::point16_t(response.x, response.y), true);

        fb::ostream         parameter;
        parameter.write(response.name);
        this->transfer(*client, response.ip, response.port, parameter);
    }
    catch(std::exception& e)
    {
        auto session = client->data();
        this->on_notify(*session, e.what(), fb::game::message::type::STATE);

        auto before = session->before_map();
        if(before != nullptr)
        {
            session->map(before, session->before());
            session->before_map(nullptr);
        }
    }

    return true;
}

bool fb::game::acceptor::handle_in_whisper(fb::internal::socket<>& socket, const fb::protocol::internal::response::whisper& response)
{
    auto client = this->find(response.from);
    if(client == nullptr)
        return true;

    std::stringstream sstream;
    if(response.success)
        sstream << response.to << "> " << response.message;
    else
        sstream << response.to << "님은 바람의나라에 없습니다.";

    client->send(fb::protocol::game::response::message(sstream.str(), fb::game::message::type::NOTIFY));
    return true;
}

bool fb::game::acceptor::handle_in_message(fb::internal::socket<>& socket, const fb::protocol::internal::response::message& response)
{
    auto to = this->find(response.to);
    if(to != nullptr)
        this->send(*to, fb::protocol::game::response::message(response.contents, (fb::game::message::type)response.type), scope::SELF);

    return true;
}

bool fb::game::acceptor::handle_in_logout(fb::internal::socket<>& socket, const fb::protocol::internal::response::logout& response)
{
    auto session = this->find(response.name);
    if(session != nullptr)
        static_cast<fb::socket<fb::game::session>&>(*session).close();
    
    return true;
}

bool fb::game::acceptor::handle_login(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::login& request)
{
    auto session = socket.data();
    auto& c = fb::console::get();

    // Set crypt data
    socket.crt(request.enc_type, request.enc_key);

    session->name(request.name);
    c.puts("%s님이 접속했습니다.", request.name.c_str());

    fb::db::mquery
    (
        [this, session](daotk::mysql::connection& connection, std::vector<daotk::mysql::result>& results) 
        {
            auto&                       baseResult = results[0];
            if(baseResult.count() == 0)
                return false;

            uint32_t					id;
            std::string					name;
            std::string					pw;
            uint32_t					birth;
            datetime					date;
            uint32_t					look;
            uint32_t					color;
            uint32_t					sex;
            uint32_t					nation;
            uint32_t					creature;
            uint32_t					map;
            uint32_t					position_x;
            uint32_t					position_y;
            uint32_t					direction;
            uint32_t					state;
            uint32_t					cls;
            uint32_t					promotion;
            uint32_t					exp;
            uint32_t					money;
            std::optional<uint32_t>		disguise;
            uint32_t					hp;
            uint32_t					base_hp;
            uint32_t					additional_hp;
            uint32_t					mp;
            uint32_t					base_mp;
            uint32_t					additional_mp;
            std::optional<uint32_t>		weapon;
            std::optional<uint32_t>		weapon_color;
            std::optional<uint32_t>		helmet;
            std::optional<uint32_t>		helmet_color;
            std::optional<uint32_t>		armor;
            std::optional<uint32_t>		armor_color;
            std::optional<uint32_t>		shield;
            std::optional<uint32_t>		shield_color;
            std::optional<uint32_t>		ring_left;
            std::optional<uint32_t>		ring_left_color;
            std::optional<uint32_t>		ring_right;
            std::optional<uint32_t>		ring_right_color;
            std::optional<uint32_t>		aux_top;
            std::optional<uint32_t>		aux_top_color;
            std::optional<uint32_t>		aux_bot;
            std::optional<uint32_t>		aux_bot_color;
            std::optional<uint32_t>		clan;
            baseResult.fetch(id, name, pw, birth, date, look, color, sex, nation, creature, map, position_x, position_y, direction, state, cls, promotion, exp, money, disguise, hp, base_hp, additional_hp, mp, base_mp, additional_mp, weapon, weapon_color, helmet, helmet_color, armor, armor_color, shield, shield_color, ring_left, ring_left_color, ring_right, ring_right_color, aux_top, aux_top_color, aux_bot, aux_bot_color, clan);
            this->_internal->send(fb::protocol::internal::request::login(name, map));

            std::map<equipment::slot, std::optional<int>> equipments = 
            {
                {equipment::slot::WEAPON_SLOT, weapon},
                {equipment::slot::HELMET_SLOT, helmet},
                {equipment::slot::ARMOR_SLOT, armor},
                {equipment::slot::SHIELD_SLOT, shield},
                {equipment::slot::LEFT_HAND_SLOT, ring_left},
                {equipment::slot::RIGHT_HAND_SLOT, ring_right},
                {equipment::slot::LEFT_AUX_SLOT, aux_top},
                {equipment::slot::RIGHT_AUX_SLOT, aux_bot}
            };

            session->id(id);
            session->color(color);
            session->direction(fb::game::direction(direction));
            session->look(look);
            session->money(money);
            session->sex(fb::game::sex(sex));
            session->base_hp(base_hp);
            session->hp(hp);
            session->base_mp(base_mp);
            session->mp(mp);
            session->experience(exp);
            session->state(fb::game::state(state));
            session->armor_color(armor_color);

            if(disguise.has_value())
                session->disguise(disguise.value());
            else
                session->undisguise();

            if(fb::game::table::maps[map] == nullptr)
                return false;

            session->map(fb::game::table::maps[map], point16_t(position_x, position_y));

            this->send(*session, fb::protocol::game::response::init(), scope::SELF);
            this->send(*session, fb::protocol::game::response::time(25), scope::SELF);
            this->send(*session, fb::protocol::game::response::session::state(*session, state_level::LEVEL_MIN), scope::SELF);
            this->send(*session, fb::protocol::game::response::message("0시간 1분만에 바람으로", message::type::STATE), scope::SELF);
            this->send(*session, fb::protocol::game::response::session::state(*session, state_level::LEVEL_MAX), scope::SELF);
            this->send(*session, fb::protocol::game::response::session::option(*session), scope::SELF);

            auto&                           equipResult = results[1];
            equipResult.each([this, session, equipments] (uint32_t id, uint32_t master, uint32_t owner, std::optional<uint32_t> slot, uint32_t count, std::optional<uint32_t> duration)
            {
                auto& items = fb::game::table::items;
                auto item = items[master]->make<fb::game::item>(this);
                if(item == nullptr)
                    return true;

                item->id(id);
                item->count(count);
                if(slot.has_value())
                {
                    session->items.add(*item, slot.value());
                }
                else
                {
                    auto found = std::find_if
                    (
                        equipments.begin(), equipments.end(), 
                        [id](const std::pair<equipment::slot, std::optional<uint32_t>> x) 
                        {
                            return x.second.has_value() && x.second.value() == id;
                        }
                    );

                    if(found != equipments.end())
                        session->items.wear((*found).first, static_cast<fb::game::equipment*>(item));
                }
                return true;
            });
        }, 
        "SELECT * FROM user WHERE name='%s' LIMIT 1; SELECT * FROM item WHERE owner=(SELECT id FROM user WHERE name='%s');",
        session->name().c_str(),
        session->name().c_str()
    );

    return true;
}

bool fb::game::acceptor::handle_direction(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::direction& request)
{
    auto session = socket.data();
    if(session->direction(request.direction) == false)
        return false;

    return true;
}

bool fb::game::acceptor::handle_exit(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::exit& request)
{
    auto                    session = socket.data();
    const auto&             config = fb::config::get();
    this->transfer(socket, config["login"]["ip"].asString(), config["login"]["port"].asInt());
    return true;
}

bool fb::game::acceptor::handle_move(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::move& request)
{
    auto session = socket.data();
    auto                    map = session->map();
    if(map == nullptr)
        return true;

    point16_t               before(request.position);
    session->move(request.direction, before);

    // 워프 위치라면 워프한다.
    const auto              warp = map->warpable(session->position());
    if(warp != nullptr)
    {
        if(warp->offset != nullptr)
        {
            session->map(nullptr);
            session->send(fb::protocol::game::response::map::worlds(*warp->offset));
        }
        else
        {
            session->map(warp->map, warp->after);
        }
    }
    return true;
}

bool fb::game::acceptor::handle_update_move(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::update_move& request)
{
    auto session = socket.data();
    auto map = session->map();
    if(map == nullptr)
        return true;

    if(this->handle_move(socket, request) == false)
        return false;

    this->send(*session, fb::protocol::game::response::map::update(*map, request.begin, request.size), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_attack(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::attack& request)
{
    auto session = socket.data();
    session->attack();
    return true;
}

bool fb::game::acceptor::handle_pickup(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::pick_up& request)
{
    auto session = socket.data();
    session->items.pickup(request.boost);
    return true;
}

bool fb::game::acceptor::handle_emotion(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::emotion& request)
{
    auto session = socket.data();
    session->action(action(action::EMOTION + request.emotion), duration::DURATION_EMOTION);
    return true;
}

bool fb::game::acceptor::handle_update_map(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::map::update& request)
{
    auto session = socket.data();
    this->send(*session, fb::protocol::game::response::map::update(*session->map(), request.position, request.size), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_refresh(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::refresh& request)
{
    auto session = socket.data();
    this->send(*session, fb::protocol::game::response::session::position(*session), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_active_item(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::item::active& request)
{
    auto session = socket.data();
    session->items.active(request.index);
    return true;
}

bool fb::game::acceptor::handle_inactive_item(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::item::inactive& request)
{
    auto session = socket.data();
    session->items.inactive(request.slot);
    return true;
}

bool fb::game::acceptor::handle_drop_item(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::item::drop& request)
{
    auto session = socket.data();
    session->items.drop(request.index, request.all ? -1 : 1);
    return true;
}

bool fb::game::acceptor::handle_drop_cash(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::item::drop_cash& request)
{
    auto session = socket.data();
    auto                map = session->map();
    if(map == nullptr)
        return false;

    auto                chunk = std::min(session->money(), request.chunk);

    session->money_drop(chunk);
    return true;
}

// TODO : on_notify를 이용
bool fb::game::acceptor::handle_front_info(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::front_info& request)
{
    auto session = socket.data();
    auto                    map = session->map();
    if(map == nullptr)
        return false;

    auto forwards = session->forwards();
    for(auto i = forwards.begin(); i != forwards.end(); i++)
    {
        auto object = *i;
        auto message = object->is(object::types::ITEM) ? 
            static_cast<fb::game::item*>(*i)->name_styled() : 
            (*i)->name();
        
        this->send(*session, fb::protocol::game::response::message(message, message::type::STATE), scope::SELF);
    }
    
    return true;
}

bool fb::game::acceptor::handle_self_info(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::self_info& request)
{
    auto session = socket.data();
    this->send(*session, fb::protocol::game::response::session::internal_info(*session), scope::SELF);
    
    for(auto buff : session->buffs)
        this->send(*session, fb::protocol::game::response::spell::buff(*buff), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_option_changed(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::change_option& request)
{
    auto session = socket.data();
    auto                    option = options(request.option);

    if(option == options::RIDE)
    {
        if(session->state() == state::RIDING)
            session->unride(this);
        else
            session->ride();
    }
    else
    {
        session->option_toggle(option);
    }
    return true;
}

bool fb::game::acceptor::handle_click_object(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::click& request)
{
    auto session = socket.data();
    if(request.fd == 0xFFFFFFFF) // Press F1
    {
        return true;
    }

    if(request.fd == 0xFFFFFFFE) // Preff F2
    {
        return true;
    }

    auto                    map = session->map();
    auto                    you = map->objects[request.fd];
    if(you == nullptr)
        return true;

    switch(you->type())
    {
    case fb::game::object::types::SESSION:
        this->send(*session, fb::protocol::game::response::session::external_info(static_cast<fb::game::session&>(*you)), scope::SELF);
        break;

    case fb::game::object::types::MOB:
        this->handle_click_mob(*session, static_cast<mob&>(*you));
        break;

    case fb::game::object::types::NPC:
        this->handle_click_npc(*session, static_cast<npc&>(*you));
        break;
    }

    return true;
}

// TODO : on_item_detail
bool fb::game::acceptor::handle_item_info(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::item::info& request)
{
    auto session = socket.data();
    auto                        item = session->items[request.slot];
    if(item == nullptr)
        return false;

    this->send(*session, fb::protocol::game::response::item::tip(request.position, item->tip_message()), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_itemmix(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::item::mix& request)
{
    auto session = socket.data();
    if(request.indices.size() > item::MAX_SLOT - 1)
        return false;

    itemmix::builder            builder(*session, this);
    for(auto x : request.indices)
        builder.push(x - 1);

    builder.mix();
    return true;
}

bool fb::game::acceptor::handle_trade(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::trade& request)
{
    auto me = socket.data();
    auto                        map = me->map();
    if(map == nullptr)
        return true;

    auto                        you = static_cast<fb::game::session*>(map->objects[request.fd]);   // 파트너
    if(you == nullptr)
        return true;

    switch(request.action)
    {
    case trade::state::REQUEST:
    {
        me->trade.begin(*you);
        break;
    }

    case trade::state::UP_ITEM: // 아이템 올릴때
    {
        auto                item = me->items[request.parameter.index - 1];
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

    return true;
}

bool fb::game::acceptor::handle_group(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::group& request)
{
    auto session = socket.data();
    try
    {
        std::stringstream       sstream;
        auto                    you = this->find(request.name);
        if(you == nullptr)
        {
            sstream << request.name << fb::game::message::group::CANNOT_FIND_TARGET;
            throw std::runtime_error(sstream.str());
        }

        if(session == you)
        {
            throw std::runtime_error(fb::game::message::group::CANNOT_FIND_TARGET);
        }

        if(session->option(options::GROUP) == false)
        {
            throw std::runtime_error(fb::game::message::group::DISABLED_MINE);
        }

        if(you->option(options::GROUP) == false)
        {
            throw std::runtime_error(fb::game::message::group::DISABLED_TARGET);
        }

        auto mine = session->group();
        auto your = you->group();

        if(mine == nullptr) // 새로 그룹 만들기
        {
            if(your != nullptr)
            {
                sstream << request.name << fb::game::message::group::ALREADY_JOINED;
                throw std::runtime_error(sstream.str());
            }

            mine = fb::game::group::create(*session);
            mine->enter(*you);

            sstream << session->name() << fb::game::message::group::JOINED;
            this->send(*session, fb::protocol::game::response::message(sstream.str(), message::type::STATE), scope::GROUP);

            sstream.str("");
            sstream << request.name << fb::game::message::group::JOINED;
            this->send(*session, fb::protocol::game::response::message(sstream.str(), message::type::STATE), scope::GROUP);
        }
        else // 기존 그룹에 초대하기
        {
            auto& leader = mine->leader();
            if(session != &leader)
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
                this->send(*session, fb::protocol::game::response::message(sstream.str(), message::type::STATE), scope::GROUP);
                mine->leave(*you);
                return true;
            }

            if(mine->enter(*you) == nullptr)
            {
                throw std::runtime_error(fb::game::message::group::FULL_MEMBER);
            }
            
            sstream << request.name << fb::game::message::group::JOINED;
            this->send(*session, fb::protocol::game::response::message(sstream.str(), message::type::STATE), scope::GROUP);
            this->send(leader, fb::protocol::game::response::message(sstream.str(), message::type::STATE), scope::GROUP);
        }
    }
    catch(std::exception& e)
    {
        this->send(*session, fb::protocol::game::response::message(e.what(), message::type::STATE), scope::GROUP);
    }

    return true;
}

bool fb::game::acceptor::handle_user_list(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::user_list& request)
{
    auto session = socket.data();
    this->send(*session, fb::protocol::game::response::user_list(*session, this->sockets), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_chat(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::chat& request)
{
    auto session = socket.data();
    if(handle_command(*session, request.message) == true)
        return true;
    
    std::stringstream           sstream;
    if(request.shout)
        sstream << session->name() << "! " << request.message;
    else
        sstream << session->name() << ": " << request.message;

    this->send(*session, fb::protocol::game::response::chat(*session, sstream.str(), request.shout ? chat::SHOUT : chat::NORMAL), request.shout ? scope::MAP : scope::PIVOT);
    return true;
}

bool fb::game::acceptor::handle_board(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::board::board& request)
{
    auto session = socket.data();
    switch(request.action)
    {
    case 0x01: // section list
    {
        this->send(*session, fb::protocol::game::response::board::sections(), scope::SELF);
        break;
    }

    case 0x02: // article list
    {
        this->send(*session, fb::protocol::game::response::board::articles(request.section, request.offset), scope::SELF);
        break;
    }

    case 0x03: // article
    {
        this->send(*session, fb::protocol::game::response::board::article(request.section, request.article, *session), scope::SELF);
        break;
    }

    case 0x04:
    {
        auto                    section = fb::game::table::board.sections().at(request.section);
        if(section->add(request.title, request.contents, session->name()) != nullptr)
            this->send(*session, fb::protocol::game::response::board::message(fb::game::message::board::WRITE, true, true), scope::SELF);
        break;
    }

    case 0x05: // delete
    {
        try
        {
            auto                    section = fb::game::table::board.at(request.section);
            if(section == nullptr)
                throw board::section::not_found_exception();

            auto                    article = section->find(request.article);
            if(article == nullptr)
                throw board::article::not_found_exception();

            if(session->name() != article->writer())
                throw board::auth_exception();

            if(section->remove(article->id()) == false)
                throw board::article::not_found_exception();

            this->send(*session, fb::protocol::game::response::board::message(fb::game::message::board::SUCCESS_DELETE, true, false), scope::SELF);
        }
        catch(std::exception& e)
        {
            this->send(*session, fb::protocol::game::response::board::message(e.what(), false, false), scope::SELF);
        }
        break;
    }

    }

    return true;
}

bool fb::game::acceptor::handle_swap(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::swap& request)
{
    auto session = socket.data();
    switch(request.type)
    {
    case swap::type::SPELL:
    {
        session->spells.swap(request.src-1, request.dest-1);
        break;
    }

    case swap::type::ITEM:
    {
        session->items.swap(request.src-1, request.dest-1);
        break;
    }

    default:
        return false;
    }

    return true;
}

bool fb::game::acceptor::handle_dialog(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::dialog& request)
{
    auto session = socket.data();
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
        {
            session->dialog.pushstring(request.message);
        }
        else
        {
            session->dialog.pushinteger(request.action);
        }

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
        break;
    }

    case dialog::interaction::SELL:
    {
        session->dialog.pushstring(request.name).resume(1);
        break;
    }

    default:
    {
        break;
    }
    }

    return true;
}

bool fb::game::acceptor::handle_throw_item(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::item::throws& request)
{
    auto session = socket.data();
    session->items.throws(request.index);
    return true;
}

bool fb::game::acceptor::handle_spell(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::spell::use& request)
{
    auto session = socket.data();
    if(request.slot > spell::MAX_SLOT - 1)
        return false;

    auto                    spell = session->spells[request.slot];
    if(spell == nullptr)
        return false;

    request.parse(spell->type());
    switch(spell->type())
    {
    case spell::types::INPUT:
        session->active(*spell, request.message);
        break;

    case spell::types::TARGET:
        session->active(*spell, request.fd);
        break;

    case spell::types::NORMAL:
        session->active(*spell);
        break;
    }

    return true;
}

bool fb::game::acceptor::handle_door(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::door& request)
{
    auto session = socket.data();
    lua::thread("scripts/common/door.lua")
        .func("handle_door")
        .pushobject(session)
        .resume(1);
    return true;
}

bool fb::game::acceptor::handle_whisper(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::whisper& request)
{
    auto session = socket.data();
    this->_internal->send(fb::protocol::internal::request::whisper(session->name(), request.name, request.message));
    return true;
}

bool fb::game::acceptor::handle_world(fb::socket<fb::game::session>& socket, const fb::protocol::game::request::map::world& request)
{
    auto world = fb::game::table::worlds[request.world];
    if(world == nullptr)
        return false;

    const auto& offsets = world->offsets();
    const auto before = offsets[request.before]->dest;
    const auto after = offsets[request.after]->dest;

    auto session = socket.data();
    if(before.map->loaded())
        session->before_map(before.map);
    
    session->map(after.map, after.position);
    return true;
}

void fb::game::acceptor::handle_mob_action(uint64_t now)
{
    for(auto pair : fb::game::table::maps)
    {
        auto                map = pair.second;
        const auto          mobs = map->activateds(fb::game::object::types::MOB);

        for(auto x : mobs)
        {
            auto mob = static_cast<fb::game::mob*>(x);
            auto target = mob->target();
            if(this->exists(static_cast<fb::game::object&>(*target)) == false)
                mob->target(nullptr);

            if(mob->action())
                continue;

            mob->AI(now);
        }
    }
}

void fb::game::acceptor::handle_mob_respawn(uint64_t now)
{
    // 리젠된 전체 몹을 저장
    std::vector<object*>    spawned_mobs;
    for(auto pair : fb::game::table::maps)
    {
        auto map = pair.second;
        for(auto pair : map->objects)
        {
            if(pair.second->type() != object::types::MOB)
                continue;

            auto mob = static_cast<fb::game::mob*>(pair.second);
            if(mob == nullptr)
                continue;

            if(mob->spawn(now) == false)
                continue;

            mob->visible(true);
            mob->hp(mob->base_hp());
            mob->mp(mob->base_mp());
            spawned_mobs.push_back(mob);
        }
    }


    // 화면에 보이는 몹만 갱신
    std::vector<object*> shown_mobs;
    for(auto pair : this->sockets)
    {
        auto session = pair.second->data();
        if(session == nullptr)
            continue;

        shown_mobs.clear();
        for(auto spawned : spawned_mobs)
        {
            if(session->sight(*spawned) == false)
                continue;

            shown_mobs.push_back(spawned);
        }

        if(shown_mobs.size() > 0)
            this->send(*session, fb::protocol::game::response::object::show(shown_mobs), scope::SELF);
    }
}

void fb::game::acceptor::handle_buff_timer(uint64_t now)
{
    for(auto pair : fb::game::table::maps)
    {
        auto map = pair.second;
        if(map->objects.size() == 0)
            continue;

        for(auto pair : map->objects)
        {
            if(pair.second->buffs.size() == 0)
                continue;

            std::vector<buff*> finishes;
            for(auto buff : pair.second->buffs)
            {
                buff->time_dec(1);
                if(buff->time() <= 0)
                    finishes.push_back(buff);
            }

            for(auto finish : finishes)
                pair.second->buffs.remove(finish->spell().name());
        }
    }
}

bool fb::game::acceptor::handle_command(fb::game::session& session, const std::string& message)
{
    if(message.starts_with('/') == false)
        return false;

    std::vector<std::string>        splitted;
    std::istringstream              sstream(message.substr(1));
    std::string                     unit;
    while (std::getline(sstream, unit, ' '))
    {
        splitted.push_back(unit);
    }

    auto found = this->_command_dict.find(splitted[0]);
    if (found == this->_command_dict.end())
        return false;

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

    found->second(session, parameters);
    return true;
}
