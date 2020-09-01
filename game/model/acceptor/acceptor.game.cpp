#include <model/acceptor/acceptor.game.h>
using namespace fb::game;

IMPLEMENT_LUA_EXTENSION(fb::game::acceptor, "")
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::lua::luable, "fb.game.luable")
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

acceptor::acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay) : 
    fb::acceptor<fb::game::session>(context, port, accept_delay),
    _timer(context)
{
    const auto& config = fb::config();
    this->_connection = new connection(
        config["database"]["ip"].asString(), 
        config["database"]["uid"].asString(), 
        config["database"]["pwd"].asString(), 
        config["database"]["name"].asString());

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
    lua::bind_class<game::session, life>();

    lua_register(lua::main::get(), "name2mob",  builtin_name2mob);
    lua_register(lua::main::get(), "name2item", builtin_name2item);
    lua_register(lua::main::get(), "name2npc",  builtin_name2npc);
    lua_register(lua::main::get(), "name2map",  builtin_name2map);
    lua_register(lua::main::get(), "timer",     builtin_timer);
    lua_register(lua::main::get(), "weather",   builtin_weather);
    
    this->bind<fb::protocol::request::game::login>            (0x10, std::bind(&acceptor::handle_login,           this, std::placeholders::_1, std::placeholders::_2));   // 게임서버 접속 핸들러
    this->bind<fb::protocol::request::game::direction>        (0x11, std::bind(&acceptor::handle_direction,       this, std::placeholders::_1, std::placeholders::_2));   // 방향전환 핸들러
    this->bind<fb::protocol::request::game::exit>             (0x0B, std::bind(&acceptor::handle_exit,            this, std::placeholders::_1, std::placeholders::_2));   // 접속 종료
    this->bind<fb::protocol::request::game::update_move>      (0x06, std::bind(&acceptor::handle_update_move,     this, std::placeholders::_1, std::placeholders::_2));   // 이동과 맵 데이터 업데이트 핸들러
    this->bind<fb::protocol::request::game::move>             (0x32, std::bind(&acceptor::handle_move,            this, std::placeholders::_1, std::placeholders::_2));   // 이동 핸들러
    this->bind<fb::protocol::request::game::attack>           (0x13, std::bind(&acceptor::handle_attack,          this, std::placeholders::_1, std::placeholders::_2));   // 공격 핸들러
    this->bind<fb::protocol::request::game::pick_up>          (0x07, std::bind(&acceptor::handle_pickup,          this, std::placeholders::_1, std::placeholders::_2));   // 아이템 줍기 핸들러
    this->bind<fb::protocol::request::game::emotion>          (0x1D, std::bind(&acceptor::handle_emotion,         this, std::placeholders::_1, std::placeholders::_2));   // 감정표현 핸들러
    this->bind<fb::protocol::request::game::map::update>      (0x05, std::bind(&acceptor::handle_update_map,      this, std::placeholders::_1, std::placeholders::_2));   // 맵 데이터 업데이트 핸들러
    this->bind<fb::protocol::request::game::refresh>          (0x38, std::bind(&acceptor::handle_refresh,         this, std::placeholders::_1, std::placeholders::_2));   // 새로고침 핸들러
    this->bind<fb::protocol::request::game::item::active>     (0x1C, std::bind(&acceptor::handle_active_item,     this, std::placeholders::_1, std::placeholders::_2));   // 아이템 사용 핸들러
    this->bind<fb::protocol::request::game::item::inactive>   (0x1F, std::bind(&acceptor::handle_inactive_item,   this, std::placeholders::_1, std::placeholders::_2));   // 아이템 장착 해제 핸들러
    this->bind<fb::protocol::request::game::item::drop>       (0x08, std::bind(&acceptor::handle_drop_item,       this, std::placeholders::_1, std::placeholders::_2));   // 아이템 버리기 핸들러
    this->bind<fb::protocol::request::game::item::drop_cash>  (0x24, std::bind(&acceptor::handle_drop_cash,       this, std::placeholders::_1, std::placeholders::_2));   // 금전 버리기 핸들러
    this->bind<fb::protocol::request::game::front_info>       (0x09, std::bind(&acceptor::handle_front_info,      this, std::placeholders::_1, std::placeholders::_2));   // 앞방향 정보 핸들러
    this->bind<fb::protocol::request::game::self_info>        (0x2D, std::bind(&acceptor::handle_self_info,       this, std::placeholders::_1, std::placeholders::_2));   // 나 자신의 정보 핸들러
    this->bind<fb::protocol::request::game::change_option>    (0x1B, std::bind(&acceptor::handle_option_changed,  this, std::placeholders::_1, std::placeholders::_2));   // 옵션 설정 핸들러
    this->bind<fb::protocol::request::game::click>            (0x43, std::bind(&acceptor::handle_click_object,    this, std::placeholders::_1, std::placeholders::_2));   // 오브젝트 클릭 핸들러
    this->bind<fb::protocol::request::game::item::info>       (0x66, std::bind(&acceptor::handle_item_info,       this, std::placeholders::_1, std::placeholders::_2));   // 인벤토리 우클릭 핸들러
    this->bind<fb::protocol::request::game::item::mix>        (0x6B, std::bind(&acceptor::handle_itemmix,         this, std::placeholders::_1, std::placeholders::_2));   // 아이템 조합 핸들러
    this->bind<fb::protocol::request::game::trade>            (0x4A, std::bind(&acceptor::handle_trade,           this, std::placeholders::_1, std::placeholders::_2));   // 교환 핸들러
    this->bind<fb::protocol::request::game::group>            (0x2E, std::bind(&acceptor::handle_group,           this, std::placeholders::_1, std::placeholders::_2));   // 그룹 핸들러
    this->bind<fb::protocol::request::game::user_list>        (0x18, std::bind(&acceptor::handle_user_list,       this, std::placeholders::_1, std::placeholders::_2));   // 유저 리스트 핸들러
    this->bind<fb::protocol::request::game::chat>             (0x0E, std::bind(&acceptor::handle_chat,            this, std::placeholders::_1, std::placeholders::_2));   // 유저 채팅 핸들러
    this->bind<fb::protocol::request::game::board::board>     (0x3B, std::bind(&acceptor::handle_board,           this, std::placeholders::_1, std::placeholders::_2));   // 게시판 섹션 리스트 핸들러
    this->bind<fb::protocol::request::game::swap>             (0x30, std::bind(&acceptor::handle_swap,            this, std::placeholders::_1, std::placeholders::_2));   // 스펠 순서 변경
    this->bind<fb::protocol::request::game::dialog>           (0x3A, std::bind(&acceptor::handle_dialog,          this, std::placeholders::_1, std::placeholders::_2));   // 다이얼로그
    this->bind<fb::protocol::request::game::dialog>           (0x39, std::bind(&acceptor::handle_dialog,          this, std::placeholders::_1, std::placeholders::_2));   // 다이얼로그
    this->bind<fb::protocol::request::game::item::throws>     (0x17, std::bind(&acceptor::handle_throw_item,      this, std::placeholders::_1, std::placeholders::_2));   // 아이템 던지기 핸들러
    this->bind<fb::protocol::request::game::spell::use>       (0x0F, std::bind(&acceptor::handle_spell,           this, std::placeholders::_1, std::placeholders::_2));   // 스펠 핸들러
    this->bind<fb::protocol::request::game::door>             (0x20, std::bind(&acceptor::handle_door,            this, std::placeholders::_1, std::placeholders::_2));   // 도어 핸들러

    this->_timer.push(std::bind(&acceptor::handle_mob_action,   this, std::placeholders::_1), 100);      // 몹 행동 타이머
    this->_timer.push(std::bind(&acceptor::handle_mob_respawn,  this, std::placeholders::_1), 1000);    // 몹 리젠 타이머
    this->_timer.push(std::bind(&acceptor::handle_buff_timer,   this, std::placeholders::_1), 1000);     // 버프 타이머
}

acceptor::~acceptor()
{ 
    delete this->_connection;
}

bool acceptor::handle_connected(fb::game::session& session)
{
    return true;
}

bool acceptor::handle_disconnected(fb::game::session& session)
{
    this->on_save(session);
    this->send(session, response::game::object::hide(session.sequence()), scope::PIVOT, true);

    auto map = session.map();
    if(map != nullptr)
        map->objects.remove(session);
    return true;
}

void fb::game::acceptor::handle_timer(uint64_t elapsed_milliseconds)
{
    for(auto pair : game::master::get().maps)
        pair.second->handle_timer(elapsed_milliseconds);
}

fb::game::session* fb::game::acceptor::find_session(const std::string& name) const
{
    auto i = std::find_if(this->sessions.begin(), this->sessions.end(), 
        [&name] (const fb::game::session* session) 
        {
            return session->name() == name;
        });

    if(i == this->sessions.end())
        return nullptr;

    return *i;
}

fb::game::session* fb::game::acceptor::handle_alloc_session(fb::socket* socket)
{
    auto session = new fb::game::session(socket, this);
    return session;
}

void fb::game::acceptor::send(object& object, const fb::protocol::base::response& response, acceptor::scope scope, bool exclude_self, bool encrypt)
{
    switch(scope)
    {
    case acceptor::scope::SELF:
        object.send(response, encrypt);
        break;

    case acceptor::scope::PIVOT:
    {
        const auto sessions = object.showns(object::types::SESSION);
        if(!exclude_self)
            object.send(response, encrypt);

        for(const auto session : sessions)
            session->send(response, encrypt);
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
            session->send(response, encrypt);

        break;
    }

    case acceptor::scope::MAP:
    {
        for(const auto session : this->sessions)
        {
            if(session->map() != object.map())
                continue;

            if(exclude_self && session == &object)
                continue;

            session->send(response, encrypt);
        }

        break;
    }

    case acceptor::scope::WORLD:
    {
        this->send(response, encrypt);
        break;
    }

    }
}

void fb::game::acceptor::send(const fb::protocol::base::response& response, const fb::game::map& map, bool encrypt)
{
    for(const auto session : this->sessions)
    {
        if(session->map() != &map)
            continue;

        session->send(response, encrypt);
    }
}

void fb::game::acceptor::send(const fb::protocol::base::response& response, bool encrypt)
{
    for(const auto session : this->sessions)
        session->send(response, encrypt);
}

// TODO : 클릭도 인터페이스로
void fb::game::acceptor::handle_click_mob(fb::game::session& session, fb::game::mob& mob)
{
    this->send(session, response::game::session::message(mob.name(), message::type::STATE), scope::SELF);
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

bool fb::game::acceptor::handle_login(fb::game::session& session, const fb::protocol::request::game::login& request)
{
    // Set crypt data
    session.crt(request.enc_type, request.enc_key);

    session.name(request.name);

    auto found = this->_connection->query
    (
        "SELECT * FROM user WHERE name LIKE '%s' LIMIT 1",      // sql injection 대처 필요
        session.name().c_str()
    );
    if(found.count() == 0)
        return false;

    found.each([this, &session] (uint32_t id, std::string name, std::string pw, uint32_t birth, datetime date, uint32_t look, uint32_t color, uint32_t sex, uint32_t nation, uint32_t creature, uint32_t map, uint32_t position_x, uint32_t position_y, uint32_t direction, uint32_t state, uint32_t cls, uint32_t promotion, uint32_t exp, uint32_t money, std::optional<uint32_t> disguise, uint32_t hp, uint32_t base_hp, uint32_t additional_hp, uint32_t mp, uint32_t base_mp, uint32_t additional_mp, uint32_t weapon, uint32_t weapon_color, uint32_t helmet, uint32_t helmet_color, uint32_t armor, uint32_t armor_color, uint32_t shield, uint32_t shield_color, uint32_t ring_left, uint32_t ring_left_color, uint32_t ring_right, uint32_t ring_right_color, uint32_t aux_top, uint32_t aux_top_color, uint32_t aux_bot, uint32_t aux_bot_color, std::optional<uint32_t> clan)
    {
        session.id(id);
        session.color(color);
        session.direction(fb::game::direction(direction));
        session.look(look);
        session.money(money);
        session.sex(fb::game::sex(sex));
        session.base_hp(base_hp);
        session.hp(hp);
        session.base_mp(base_mp);
        session.mp(mp);
        session.experience(exp);
        session.state(fb::game::state(state));

        if(disguise.has_value())
            session.disguise(disguise.value());
        else
            session.undisguise();

        if(game::master::get().maps[map] == nullptr)
            return false;

        game::master::get().maps[map]->objects.add(session, point16_t(position_x, position_y));

        this->send(session, response::game::init(), scope::SELF);
        this->send(session, response::game::time(25), scope::SELF);
        this->send(session, response::game::session::state(session, state_level::LEVEL_MIN), scope::SELF);
        this->send(session, response::game::message("0시간 1분만에 바람으로", message::type::STATE), scope::SELF);
        this->send(session, response::game::session::id(session), scope::SELF);
        this->send(session, response::game::map::config(*session.map()), scope::SELF);
        this->send(session, response::game::map::bgm(*session.map()), scope::SELF);
        this->send(session, response::game::session::position(session), scope::SELF);
        this->send(session, response::game::session::state(session, state_level::LEVEL_MAX), scope::SELF);
        this->send(session, response::game::session::show(session, false), scope::SELF);
        this->send(session, response::game::object::direction(session), scope::SELF);
        this->send(session, response::game::session::option(session), scope::SELF);

        return false;
    });

    found = this->_connection->query
    (
        "SELECT * FROM item WHERE owner=%d AND slot IS NOT NULL",
        session.id()
    );
    found.each([this, &session] (uint32_t id, uint32_t master, uint32_t owner, std::optional<uint32_t> slot, uint32_t count, std::optional<uint32_t> duration)
    {
        auto& items = fb::game::master::get().items;
        auto item = items[master]->make<fb::game::item>(this);
        if(item == nullptr)
            return false;

        item->id(id);
        item->count(count);
        session.items.add(*item, slot.value());
    });

    return true;
}

bool fb::game::acceptor::handle_direction(fb::game::session& session, const fb::protocol::request::game::direction& request)
{
    if(session.direction(request.direction) == false)
        return false;

    return true;
}

bool fb::game::acceptor::handle_exit(fb::game::session& session, const fb::protocol::request::game::exit& request)
{
    const auto&             config = fb::config();
    this->transfer(session, config["login"]["ip"].asString(), config["login"]["port"].asInt());
    return true;
}

bool fb::game::acceptor::handle_move(fb::game::session& session, const fb::protocol::request::game::move& request)
{
    auto                    map = session.map();
    if(map == nullptr)
        return false;

    point16_t               before(request.position);
    session.move(request.direction, before);

    // 워프 위치라면 워프한다.
    const auto              warp = map->warpable(session.position());
    if(warp != nullptr)
        session.warp(*warp->map, warp->after);
    return true;
}

bool fb::game::acceptor::handle_update_move(fb::game::session& session, const fb::protocol::request::game::update_move& request)
{
    if(this->handle_move(session, request) == false)
        return false;

    this->send(session, response::game::map::update(*session.map(), request.begin, request.size), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_attack(fb::game::session& session, const fb::protocol::request::game::attack& request)
{
    session.attack();
    return true;
}

bool fb::game::acceptor::handle_pickup(fb::game::session& session, const fb::protocol::request::game::pick_up& request)
{
    session.items.pickup(request.boost);
    return true;
}

bool fb::game::acceptor::handle_emotion(fb::game::session& session, const fb::protocol::request::game::emotion& request)
{
    session.action(action(action::EMOTION + request.emotion), duration::DURATION_EMOTION);
    return true;
}

bool fb::game::acceptor::handle_update_map(fb::game::session& session, const fb::protocol::request::game::map::update& request)
{
    this->send(session, response::game::map::update(*session.map(), request.position, request.size), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_refresh(fb::game::session& session, const fb::protocol::request::game::refresh& request)
{
    this->send(session, response::game::session::position(session), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_active_item(fb::game::session& session, const fb::protocol::request::game::item::active& request)
{
    session.items.active(request.index);
    return true;
}

bool fb::game::acceptor::handle_inactive_item(fb::game::session& session, const fb::protocol::request::game::item::inactive& request)
{
    session.items.inactive(request.slot);
    return true;
}

bool fb::game::acceptor::handle_drop_item(fb::game::session& session, const fb::protocol::request::game::item::drop& request)
{
    session.items.drop(request.index, request.all ? -1 : 1);
    return true;
}

bool fb::game::acceptor::handle_drop_cash(fb::game::session& session, const fb::protocol::request::game::item::drop_cash& request)
{
    auto                map = session.map();
    if(map == nullptr)
        return false;

    auto                chunk = std::min(session.money(), request.chunk);
    //auto                cash = new fb::game::cash(chunk, this);

    session.money_drop(chunk);
    return true;
}

// TODO : on_notify를 이용
bool fb::game::acceptor::handle_front_info(fb::game::session& session, const fb::protocol::request::game::front_info& request)
{
    auto                    map = session.map();
    if(map == nullptr)
        return false;

    for(auto i : session.forward_sessions())
    {
        this->send(session, response::game::message(i->name(), message::type::STATE), scope::SELF);
    }

    for(auto i : session.forward_objects(object::types::OBJECT))
    {
        if(i->type() == fb::game::object::types::ITEM)
        {
            auto item = static_cast<fb::game::item*>(i);
            this->send(session, response::game::message(item->name_styled(), message::type::STATE), scope::SELF);
        }
        else
        {
            this->send(session, response::game::message(i->name(), message::type::STATE), scope::SELF);
        }
    }

    return true;
}

bool fb::game::acceptor::handle_self_info(fb::game::session& session, const fb::protocol::request::game::self_info& request)
{
    this->send(session, response::game::session::internal_info(session), scope::SELF);
    
    for(auto buff : session.buffs)
        this->send(session, response::game::spell::buff(*buff), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_option_changed(fb::game::session& session, const fb::protocol::request::game::change_option& request)
{
    auto                    option = options(request.option);

    if(option == options::RIDE)
    {
        if(session.state() == state::RIDING)
            session.unride(this);
        else
            session.ride();
    }
    else
    {
        session.option_toggle(option);
    }
    return true;
}

bool fb::game::acceptor::handle_click_object(fb::game::session& session, const fb::protocol::request::game::click& request)
{
    if(request.fd == 0xFFFFFFFF) // Press F1
    {
        return true;
    }

    if(request.fd == 0xFFFFFFFE) // Preff F2
    {
        return true;
    }

    auto                    map = session.map();
    auto                    other = map->objects.find(request.fd);
    if(other == nullptr)
        return true;

    switch(other->type())
    {
    case fb::game::object::types::SESSION:
        this->send(session, response::game::session::external_info(session), scope::SELF);
        break;

    case fb::game::object::types::MOB:
        this->handle_click_mob(session, *static_cast<mob*>(other));
        break;

    case fb::game::object::types::NPC:
        this->handle_click_npc(session, *static_cast<npc*>(other));
        break;
    }

    return true;
}

// TODO : on_item_detail
bool fb::game::acceptor::handle_item_info(fb::game::session& session, const fb::protocol::request::game::item::info& request)
{
    auto                        item = session.items[request.slot];
    if(item == nullptr)
        return false;

    this->send(session, response::game::item::tip(request.position, item->tip_message()), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_itemmix(fb::game::session& session, const fb::protocol::request::game::item::mix& request)
{
    if(request.indices.size() > item::MAX_SLOT - 1)
        return false;

    itemmix::builder            builder(session, this);
    for(auto x : request.indices)
        builder.push(x - 1);

    builder.mix();
    return true;
}

bool fb::game::acceptor::handle_trade(fb::game::session& me, const fb::protocol::request::game::trade& request)
{
    auto                        map = me.map();
    if(map == nullptr)
        return true;

    auto                        you = static_cast<game::session*>(map->objects.find(request.fd));   // 파트너
    if(you == nullptr)
        return true;

    switch(request.action)
    {
    case trade::state::REQUEST:
    {
        me.trade.begin(*you);
        break;
    }

    case trade::state::UP_ITEM: // 아이템 올릴때
    {
        auto                item = me.items[request.parameter.index];
        if(item == nullptr)
            break;

        me.trade.up(*item);
        break;
    }

    case trade::state::ITEM_COUNT: // 아이템 갯수까지 해서 올릴 때
    {
        me.trade.count(request.parameter.count);
        break;
    }

    case trade::state::UP_MONEY: // 금전 올릴 때
    {
        // 클라이언트가 입력한 금전 양
        me.trade.up(request.parameter.money);
        break;
    }

    case trade::state::CANCEL: // 취소한 경우
    {
        me.trade.cancel();
        break;
    }

    case trade::state::LOCK:
    {
        me.trade.lock();
        break;
    }
    }

    return true;
}

bool fb::game::acceptor::handle_group(fb::game::session& session, const fb::protocol::request::game::group& request)
{
    try
    {
        std::stringstream       sstream;
        auto                    you = this->find_session(request.name);
        if(you == nullptr)
        {
            sstream << request.name << "님을 찾을 수 없습니다.";
            throw std::runtime_error(sstream.str());
        }

        if(session == *you)
        {
            throw std::runtime_error("대상을 찾을 수 없습니다.");
        }

        if(session.option(options::GROUP) == false)
        {
            throw std::runtime_error("그룹 거부 상태입니다.");
        }

        if(you->option(options::GROUP) == false)
        {
            throw std::runtime_error("상대방이 그룹 거부 상태입니다.");
        }

        auto mine = session.group();
        auto your = you->group();

        if(mine == nullptr) // 새로 그룹 만들기
        {
            if(your != nullptr)
            {
                sstream << request.name << "님은 이미 그룹 참여 중입니다.";
                throw std::runtime_error(sstream.str());
            }

            mine = fb::game::group::create(session);
            mine->enter(*you);

            sstream << session.name() << "님 그룹에 참여";
            this->send(session, response::game::message(sstream.str(), message::type::STATE), scope::GROUP);

            sstream.str("");
            sstream << request.name << "님 그룹에 참여";
            this->send(session, response::game::message(sstream.str(), message::type::STATE), scope::GROUP);
        }
        else // 기존 그룹에 초대하기
        {
            auto& leader = mine->leader();
            if(session != leader)
            {
                throw std::runtime_error("그룹장만 할 수 있습니다.");
            }

            if(mine != your && your != nullptr)
            {
                sstream << request.name << "님은 다른 그룹에 참여 중입니다.";
                throw std::runtime_error(sstream.str());
            }

            if(mine == your)
            {
                sstream << request.name << "님 그룹 탈퇴";
                this->send(session, response::game::message(sstream.str(), message::type::STATE), scope::GROUP);
                mine->leave(*you);
                return true;
            }

            if(mine->enter(*you) == nullptr)
            {
                throw std::runtime_error("자리가 없습니다.");
            }
            
            sstream << request.name << "님 그룹에 참여";
            this->send(session, response::game::message(sstream.str(), message::type::STATE), scope::GROUP);
            this->send(leader, response::game::message(sstream.str(), message::type::STATE), scope::GROUP);
        }
    }
    catch(std::exception& e)
    {
        this->send(session, response::game::message(e.what(), message::type::STATE), scope::GROUP);
    }

    return true;
}

bool fb::game::acceptor::handle_user_list(fb::game::session& session, const fb::protocol::request::game::user_list& request)
{
    this->send(session, response::game::user_list(session, this->sessions), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_chat(fb::game::session& session, const fb::protocol::request::game::chat& request)
{
#if defined DEBUG | defined _DEBUG
    if(this->handle_admin(session, request.message))
        return true;
#endif

    std::stringstream           sstream;
    if(request.shout)
        sstream << session.name() << "! " << request.message;
    else
        sstream << session.name() << ": " << request.message;

    this->send(session, response::game::chat(session, sstream.str(), request.shout ? chat::SHOUT : chat::NORMAL), request.shout ? scope::MAP : scope::PIVOT);
    return true;
}

bool fb::game::acceptor::handle_board(fb::game::session& session, const fb::protocol::request::game::board::board& request)
{
    switch(request.action)
    {
    case 0x01: // section list
    {
        this->send(session, response::game::board::sections(), scope::SELF);
        break;
    }

    case 0x02: // article list
    {
        this->send(session, response::game::board::articles(request.section, request.offset), scope::SELF);
        break;
    }

    case 0x03: // article
    {
        this->send(session, response::game::board::article(request.section, request.article, session), scope::SELF);
        break;
    }

    case 0x04:
    {
        auto                    section = game::master::get().board.sections().at(request.section);
        if(section->add(request.title, request.contents, session.name()) != nullptr)
            this->send(session, response::game::board::message("글을 작성하였습니다.", true, true), scope::SELF);
        break;
    }

    case 0x05: // delete
    {
        try
        {
            auto                    section = game::master::get().board.at(request.section);
            if(section == nullptr)
                throw board::section::not_found_exception();

            auto                    article = section->find(request.article);
            if(article == nullptr)
                throw board::article::not_found_exception();

            if(session.name() != article->writer())
                throw board::auth_exception();

            if(section->remove(article->id()) == false)
                throw board::article::not_found_exception();

            this->send(session, response::game::board::message(message::board::SUCCESS_DELETE, true, false), scope::SELF);
        }
        catch(std::exception& e)
        {
            this->send(session, response::game::board::message(e.what(), false, false), scope::SELF);
        }
        break;
    }

    }

    return true;
}

bool fb::game::acceptor::handle_swap(fb::game::session& session, const fb::protocol::request::game::swap& request)
{
    switch(request.type)
    {
    case swap::type::SPELL:
    {
        session.spells.swap(request.src-1, request.dest-1);
        break;
    }

    case swap::type::ITEM:
    {
        session.items.swap(request.src-1, request.dest-1);
        break;
    }

    default:
        return false;
    }

    return true;
}

bool fb::game::acceptor::handle_dialog(fb::game::session& session, const fb::protocol::request::game::dialog& request)
{
    switch(request.interaction)
    {
    case dialog::interaction::NORMAL: // 일반 다이얼로그
    {
        session.dialog.pushinteger(request.action).resume(1);
        break;
    }

    case dialog::interaction::INPUT:
    {
        session.dialog.pushstring(request.message).resume(1);
        break;
    }

    case dialog::interaction::INPUT_EX:
    {
        if(request.action == 0x02) // OK button
        {
            session.dialog.pushstring(request.message);
        }
        else
        {
            session.dialog.pushinteger(request.action);
        }

        session.dialog.resume(1);
        break;
    }

    case dialog::interaction::MENU:
    {
        session.dialog.pushinteger(request.index).resume(1);
        break;
    }

    case dialog::interaction::SLOT:
    {
        break;
    }

    case dialog::interaction::SELL:
    {
        session.dialog.pushstring(request.name).resume(1);
        break;
    }

    default:
    {
        break;
    }
    }

    return true;
}

bool fb::game::acceptor::handle_throw_item(fb::game::session& session, const fb::protocol::request::game::item::throws& request)
{
    session.items.throws(request.index);
    return true;
}

bool fb::game::acceptor::handle_spell(fb::game::session& session, const fb::protocol::request::game::spell::use& request)
{
    if(request.slot > spell::MAX_SLOT - 1)
        return false;

    auto                    spell = session.spells[request.slot];
    if(spell == nullptr)
        return false;

    lua::thread             thread;
    thread.from("scripts/spell/%s.lua", spell->cast().c_str())
          .func("handle_spell");

    request.parse(spell->type());
    switch(spell->type())
    {
    case spell::types::INPUT:
    {
        char                message[256];

        thread.pushobject(session)
            .pushobject(spell)
            .pushstring(request.message)
            .resume(3);
        break;
    }

    case spell::types::TARGET:
    {
        auto                map = session.map();
        if(map == nullptr)
            return false;

        auto                target = map->objects.find(request.fd);
        if(target == nullptr)
            return true;

        if(session.sight(*target) == false)
            return true;

        thread.pushobject(session)
            .pushobject(spell)
            .pushobject(target)
            .resume(3);
        break;
    }

    case spell::types::NORMAL:
    {
        thread.pushobject(session)
            .pushobject(spell)
            .resume(2);
        break;
    }
    }

    // 마나를 쓰니까 업데이트 해주겠지?
    // 마나 안쓰면 업데이트 안해줘도 되니까
    // 이건 hp, mp가 변경될 때 리스너
    this->send(session, response::game::session::state(session, state_level::LEVEL_MAX), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_door(fb::game::session& session, const fb::protocol::request::game::door& request)
{
    lua::thread()
        .from("scripts/common/door.lua")
        .func("handle_door")
        .pushobject(session)
        .resume(1);
    return true;
}

void fb::game::acceptor::handle_counter_mob_action(fb::game::mob* mob)
{
    auto                    target = mob->target();
    if(target == nullptr)
    {
        mob->move(direction(std::rand() % 4));
        return;
    }

    // 상하좌우로 타겟이 있는지 검사한다.
    for(int i = 0; i < 4; i++)
    {
        auto                direction = game::direction(i);
        if(mob->near_session(direction) != target)
            continue;

        if(mob->direction() != direction)
            mob->direction(direction);

        mob->attack();
        return;
    }


    // 타겟 방향으로 이동이 가능하다면 이동한다.
    auto                    x_axis = bool(std::rand()%2);
    if(x_axis)
    {
        if(mob->x() > target->x() && mob->move(direction::LEFT))   return;
        if(mob->x() < target->x() && mob->move(direction::RIGHT))  return;
        if(mob->y() > target->y() && mob->move(direction::TOP))    return;
        if(mob->y() < target->y() && mob->move(direction::BOTTOM)) return;
    }
    else
    {
        if(mob->y() > target->y() && mob->move(direction::TOP))    return;
        if(mob->y() < target->y() && mob->move(direction::BOTTOM)) return;
        if(mob->x() > target->x() && mob->move(direction::LEFT))   return;
        if(mob->x() < target->x() && mob->move(direction::RIGHT))  return;
    }


    // 이동할 수 있는 방향으로 일단 이동한다.
    auto                    random_direction = std::rand() % 4;
    for(int i = 0; i < 4; i++)
    {
        if(mob->move(direction((random_direction + i) % 4)))
            return;
    }
}

void fb::game::acceptor::handle_containment_mob_action(fb::game::mob* mob)
{
    try
    {
        auto                    target = mob->target();
        if(target == nullptr || target->sight(*mob) == false)
            target = mob->autoset_target();

        if(target == nullptr)
            throw std::runtime_error("no target");
    }
    catch(std::exception&)
    { }

    this->handle_counter_mob_action(mob);
}

void fb::game::acceptor::handle_mob_action(uint64_t now)
{
    for(auto pair : game::master::get().maps)
    {
        auto                map = pair.second;
        const auto&         mobs = map->objects.active_mobs();

        for(auto mob : mobs)
        {
            if(now < mob->action_time() + mob->speed())
                continue;

            switch(mob->offensive())
            {
            case mob::offensive_type::COUNTER:
                this->handle_counter_mob_action(mob);
                break;

            case mob::offensive_type::CONTAINMENT:
                this->handle_containment_mob_action(mob);
                break;

            default:
                this->handle_counter_mob_action(mob);
                break;
            }

            mob->action_time(now);
        }
    }
}

void fb::game::acceptor::handle_mob_respawn(uint64_t now)
{
    // 리젠된 전체 몹을 저장
    std::vector<object*>    spawned_mobs;
    for(auto pair : game::master::get().maps)
    {
        auto map = pair.second;
        for(auto object : map->objects)
        {
            if(object->type() != object::types::MOB)
                continue;

            auto mob = static_cast<fb::game::mob*>(object);
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
    for(auto session : this->sessions)
    {
        if(session == nullptr)
            continue;

        shown_mobs.clear();
        for(auto spawned : spawned_mobs)
        {
            if(session->sight(*spawned) == false)
                continue;

            shown_mobs.push_back(spawned);
        }

        this->send(*session, response::game::object::show(shown_mobs), scope::SELF);
    }
}

void fb::game::acceptor::handle_buff_timer(uint64_t now)
{
    for(auto pair : game::master::get().maps)
    {
        auto map = pair.second;
        if(map->objects.size() == 0)
            continue;

        for(auto object : map->objects)
        {
            if(object->buffs.size() == 0)
                continue;

            std::vector<buff*> finishes;
            for(auto buff : object->buffs)
            {
                buff->time_dec(1);
                if(buff->time() <= 0)
                    finishes.push_back(buff);
            }

            for(auto finish : finishes)
                object->buffs.remove(finish->spell().name());
        }
    }
}

#if defined DEBUG | defined _DEBUG
bool fb::game::acceptor::handle_admin(fb::game::session& session, const std::string& message)
{
    if(message[0] != '/')
        return false;

    auto                            npc = game::master::get().name2npc("낙랑");
    std::string                     command(message.begin() + 1, message.end());
    std::vector<std::string>        splitted;
    std::istringstream              sstream(command);
    std::string                     unit;

    while(std::getline(sstream, unit, ' '))
    {
        splitted.push_back(unit);
    }

    if(command == "show")
    {
        session.dialog.show(*game::master::get().name2item("남자기모노"), "갓승현님 사랑합니다.", true, true);
        return true;
    }

    if(command == "show short list")
    {
        std::vector<std::string> menus = {"갓", "승", "현"};
        session.dialog.show(*npc, "갓승현님 존경합니다.", menus);
        return true;
    }

    if(command == "show long list")
    {
        std::vector<std::string> menus;
        std::string message = "갓승현님은 위대하신 게임서버 개발계의 큰 별이시다.";
        int i = 0;
        int mod = 0;
        for(auto c : message)
        {
            i++;
            
            if(c == ' ')
                mod = int(!(bool(mod)));

            if(i % 2 != mod)
                continue;

            menus.push_back(std::string(message.begin(), message.begin() + i));
        }
        session.dialog.show(*npc, "갓승현님 존경합니다.", menus);
        return true;
    }

    if(command == "show inventory items")
    {
        std::vector<uint8_t> slots;
        for(int i = 0; i < item::MAX_SLOT; i++)
        {
            const auto item = session.items[i];
            if(item == nullptr)
                continue;

            slots.push_back(i+1);
        }
        session.dialog.show(*npc, "갓승현님의 인벤토리를 보여드립니다.", slots);
        return true;
    }

    if(command == "show item core list")
    {
        std::vector<item::master*> items;
        int count = 0;
        for(auto pair : game::master::get().items)
        {
            items.push_back(pair.second);
            if(count++ > 100)
                break;
        }
        session.dialog.show(*npc, "아시발 다 판다", items);
        return true;
    }

    if(command == "show question")
    {
        session.dialog.input(*npc, "What is your name?");
        return true;
    }

    if(command == "show extend input")
    {
        session.dialog.input(*npc, "안녕", "탑", "바텀", 0xFF, true);
        return true;
    }

    if(command == "be ghost")
    {
        if(session.state() == state::NORMAL)
            session.state(state::GHOST);
        else
            session.state(state::NORMAL);

        this->send(session, response::game::session::show(session, true), scope::PIVOT);
        return true;
    }

    if(splitted[0] == "weather")
    {
        auto value = std::stoi(splitted[1]);
        this->send(session, response::game::weather(weather::type(value)), scope::SELF);
        return true;
    }

    if(splitted[0] == "bright")
    {
        auto value = std::stoi(splitted[1]);
        this->send(session, response::game::bright(value), scope::SELF);
        return true;
    }

    if(splitted[0] == "timer")
    {
        auto value = std::stoi(splitted[1]);
        this->send(session, response::game::timer(value), scope::SELF);
        return true;
    }

    if(splitted[0] == "effect")
    {
        auto value = std::stoi(splitted[1]);
        this->send(session, response::game::object::effect(session, value), scope::SELF);
        return true;
    }

    if(splitted[0] == "action")
    {
        auto value = std::stoi(splitted[1]);
        this->send(session, response::game::session::action(session, game::action(value), duration::DURATION_SPELL), scope::SELF);
        return true;
    }

    if(splitted[0] == "sound")
    {
        auto value = std::stoi(splitted[1]);
        this->send(session, response::game::object::sound(session, game::sound::type(value)), scope::SELF);
        return true;
    }

    if(splitted[0] == "map")
    {
        auto name = splitted[1];
        auto x = 0;
        auto y = 0;

        if(splitted.size() > 2)
            x = std::stoi(splitted[2]);

        if(splitted.size() > 3)
            y = std::stoi(splitted[3]);

        auto map = game::master::get().name2map(name);
        if(map == nullptr)
            return true;
        
        session.warp(*map, point16_t(x, y));
        return true;
    }

    if(splitted[0] == "변신")
    {
        auto name = splitted[1];
        auto mob = game::master::get().name2mob(name);
        if(mob == nullptr)
            return true;

        session.disguise(mob->look());
        this->send(session, response::game::object::effect(session, 0x03), scope::PIVOT);
        this->send(session, response::game::session::action(session, action::CAST_SPELL, duration::DURATION_SPELL), scope::PIVOT);
        this->send(session, response::game::object::sound(session, sound::type(0x0019)), scope::PIVOT);
        return true;
    }

    if(splitted[0] == "변신해제")
    {
        session.undisguise();
        return true;
    }

    if(splitted[0] == "마법배우기")
    {
        auto name = splitted[1];
        auto spell = game::master::get().name2spell(name);
        if(spell == nullptr)
            return true;

        auto slot = session.spells.add(spell);
        if(slot == 0xFF)
            return true;

        return true;
    }

    if(splitted[0] == "몬스터생성")
    {
        auto name = splitted[1];
        auto core = game::master::get().name2mob(name);
        if(core == nullptr)
            return true;

        auto mob = new fb::game::mob(core, this, true);
        auto map = session.map();
        map->objects.add(*mob, session.position());
        this->send(session, response::game::object::show(*mob), scope::PIVOT);
        return true;
    }

    if(splitted[0] == "직업바꾸기")
    {
        auto name = splitted[1];
        uint8_t cls, promotion;
        if(game::master::get().name2class(name, &cls, &promotion) == false)
            return true;

        session.cls(cls);
        session.promotion(promotion);
        this->send(session, response::game::session::id(session), scope::SELF);
        this->send(session, response::game::session::state(session, state_level::LEVEL_MAX), scope::SELF);
        return true;
    }

    if(splitted[0] == "레벨바꾸기")
    {
        auto level = std::stoi(splitted[1]);
        session.level(level);
        this->send(session, response::game::session::state(session, state_level::LEVEL_MAX), scope::SELF);
        return true;
    }

    if(splitted[0] == "아이템생성")
    {
        auto name = splitted[1];
        auto item = master::get().name2item(name)->make<fb::game::item>(this);
        session.map()->objects.add(*item, session.position());
        return true;
    }

    return false;
}
#endif