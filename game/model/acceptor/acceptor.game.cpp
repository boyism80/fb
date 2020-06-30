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

acceptor::acceptor(boost::asio::io_context& context, uint16_t port) : 
    fb::acceptor<fb::game::session>(context, port),
    _timer(context)
{
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


    this->register_fn(0x10, std::bind(&acceptor::handle_login, this, std::placeholders::_1));          // 게임서버 접속 핸들러
    this->register_fn(0x11, std::bind(&acceptor::handle_direction, this, std::placeholders::_1));      // 방향전환 핸들러
    this->register_fn(0x0B, std::bind(&acceptor::handle_exit, this, std::placeholders::_1));           // 접속 종료
    this->register_fn(0x06, std::bind(&acceptor::handle_update_move, this, std::placeholders::_1));    // 이동과 맵 데이터 업데이트 핸들러
    this->register_fn(0x32, std::bind(&acceptor::handle_move, this, std::placeholders::_1));           // 이동 핸들러
    this->register_fn(0x13, std::bind(&acceptor::handle_attack, this, std::placeholders::_1));         // 공격 핸들러
    this->register_fn(0x07, std::bind(&acceptor::handle_pickup, this, std::placeholders::_1));         // 아이템 줍기 핸들러
    this->register_fn(0x1D, std::bind(&acceptor::handle_emotion, this, std::placeholders::_1));        // 감정표현 핸들러
    this->register_fn(0x05, std::bind(&acceptor::handle_update_map, this, std::placeholders::_1));     // 맵 데이터 업데이트 핸들러
    this->register_fn(0x38, std::bind(&acceptor::handle_refresh, this, std::placeholders::_1));        // 새로고침 핸들러
    this->register_fn(0x1C, std::bind(&acceptor::handle_active_item, this, std::placeholders::_1));    // 아이템 사용 핸들러
    this->register_fn(0x1F, std::bind(&acceptor::handle_inactive_item, this, std::placeholders::_1));  // 아이템 장착 해제 핸들러
    this->register_fn(0x08, std::bind(&acceptor::handle_drop_item, this, std::placeholders::_1));      // 아이템 버리기 핸들러
    this->register_fn(0x24, std::bind(&acceptor::handle_drop_cash, this, std::placeholders::_1));      // 금전 버리기 핸들러
    this->register_fn(0x09, std::bind(&acceptor::handle_front_info, this, std::placeholders::_1));     // 앞방향 정보 핸들러
    this->register_fn(0x2D, std::bind(&acceptor::handle_self_info, this, std::placeholders::_1));      // 나 자신의 정보 핸들러
    this->register_fn(0x1B, std::bind(&acceptor::handle_option_changed, this, std::placeholders::_1)); // 옵션 설정 핸들러
    this->register_fn(0x43, std::bind(&acceptor::handle_click_object, this, std::placeholders::_1));   // 오브젝트 클릭 핸들러
    this->register_fn(0x66, std::bind(&acceptor::handle_item_info, this, std::placeholders::_1));      // 인벤토리 우클릭 핸들러
    this->register_fn(0x6B, std::bind(&acceptor::handle_itemmix, this, std::placeholders::_1));        // 아이템 조합 핸들러
    this->register_fn(0x4A, std::bind(&acceptor::handle_trade, this, std::placeholders::_1));          // 교환 핸들러
    this->register_fn(0x2E, std::bind(&acceptor::handle_group, this, std::placeholders::_1));          // 그룹 핸들러
    this->register_fn(0x18, std::bind(&acceptor::handle_user_list, this, std::placeholders::_1));      // 유저 리스트 핸들러
    this->register_fn(0x0E, std::bind(&acceptor::handle_chat, this, std::placeholders::_1));           // 유저 채팅 핸들러
    this->register_fn(0x3B, std::bind(&acceptor::handle_board, this, std::placeholders::_1));          // 게시판 섹션 리스트 핸들러
    this->register_fn(0x30, std::bind(&acceptor::handle_swap, this, std::placeholders::_1));           // 스펠 순서 변경
    this->register_fn(0x3A, std::bind(&acceptor::handle_dialog, this, std::placeholders::_1));         // 다이얼로그
    this->register_fn(0x39, std::bind(&acceptor::handle_dialog, this, std::placeholders::_1));         // 다이얼로그
    this->register_fn(0x17, std::bind(&acceptor::handle_throw_item, this, std::placeholders::_1));     // 아이템 던지기 핸들러
    this->register_fn(0x0F, std::bind(&acceptor::handle_spell, this, std::placeholders::_1));          // 스펠 핸들러
    this->register_fn(0x20, std::bind(&acceptor::handle_door, this, std::placeholders::_1));           // 도어 핸들러

    this->_timer.push(std::bind(&acceptor::handle_mob_action, this, std::placeholders::_1), 100);      // 몹 행동 타이머
    this->_timer.push(std::bind(&acceptor::handle_mob_respawn, this, std::placeholders::_1), 1000);    // 몹 리젠 타이머
    this->_timer.push(std::bind(&acceptor::handle_buff_timer, this, std::placeholders::_1), 1000);     // 버프 타이머
}

acceptor::~acceptor()
{ 
    
}

bool acceptor::handle_connected(fb::game::session& session)
{
    return true;
}

bool acceptor::handle_disconnected(fb::game::session& session)
{
    this->send_stream(session, session.make_hide_stream(), scope::PIVOT, true);

    auto map = session.map();
    if(map != nullptr)
        map->objects.remove(session);

    return false;
}

void fb::game::acceptor::handle_timer(uint64_t elapsed_milliseconds)
{
    for(auto pair : game::master::get().maps)
        pair.second->handle_timer(elapsed_milliseconds);
}

fb::ostream fb::game::acceptor::make_time_stream()
{
    fb::ostream             ostream;
    auto                    hours = 25;
    ostream.write_u8(0x20)      // cmd : 0x20
           .write_u8(hours%24)  // hours
           .write_u8(0x00)      // Unknown
           .write_u8(0x00);     // Unknown

    return ostream;
}

fb::ostream fb::game::acceptor::make_bright_stream(uint8_t value)
{
    fb::ostream             ostream;

    ostream.write_u8(0x20)
        .write_u8(0x00)
        .write_u8(std::max(0, 20 - value));

    return ostream;
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

void fb::game::acceptor::send_stream(object& object, const fb::ostream& stream, acceptor::scope scope, bool exclude_self, bool encrypt)
{
    if(stream.empty())
        return;

    switch(scope)
    {
    case acceptor::scope::SELF:
        object.send(stream, encrypt);
        break;

    case acceptor::scope::PIVOT:
    {
        const auto sessions = object.showns(object::types::SESSION);
        if(!exclude_self)
            object.send(stream, encrypt);

        for(const auto session : sessions)
            session->send(stream, encrypt);
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
            session->send(stream, encrypt);

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

            session->send(stream, encrypt);
        }

        break;
    }

    case acceptor::scope::WORLD:
    {
        this->send_stream(stream, encrypt);
        break;
    }

    }
}

void fb::game::acceptor::send_stream(const fb::ostream& stream, const fb::game::map& map, bool encrypt)
{
    for(const auto session : this->sessions)
    {
        if(session->map() != &map)
            continue;

        session->send(stream, encrypt);
    }
}

void fb::game::acceptor::send_stream(const fb::ostream& stream, bool encrypt)
{
    for(const auto session : this->sessions)
        session->send(stream, encrypt);
}

// TODO : 클릭도 인터페이스로
void fb::game::acceptor::handle_click_mob(fb::game::session& session, fb::game::mob& mob)
{
    this->send_stream(session, message::make_stream(mob.name(), message::type::STATE), scope::SELF);
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

bool fb::game::acceptor::handle_login(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    uint8_t                 enc_key[0x09] = {0,};

    auto                    cmd = istream.read_u8();
    auto                    enc_type = istream.read_u8();
    auto                    key_size = istream.read_u8();
    istream.read(enc_key, key_size);

    // Set crypt data
    session.crt(enc_type, enc_key);

    session.name(istream.readstr_u8());

    game::master::get().name2map("부여왕초보사냥2")->objects.add(session, point16_t(2, 2));
    session.look(0x61);
    session.color(0x0A);
    session.money(150);
    session.sex(fb::game::sex::MAN);
    session.legends.push(0x4A, 0x10, "갓승현 ㅋㅋ")
                   .push(0x4A, 0x10, "똥진영 ㅋㅋ");
    session.base_hp(0xFFFFFFFF);
    session.hp(0xFFFFFFFF);
    session.title("갓승현 타이틀");

    session.items.add(game::master::get().name2item("얼음칼")->make<item>(this));
    session.items.add(game::master::get().name2item("정화의방패")->make<item>(this));
    session.items.add(game::master::get().name2item("도씨검")->make<item>(this));
    session.items.add(game::master::get().name2item("낙랑의두루마리2")->make<item>(this));
    session.items.add(game::master::get().name2item("남자기모노")->make<item>(this));
    session.items.add(game::master::get().name2item("도토리")->make<item>(this));
    session.items.add(game::master::get().name2item("동동주")->make<item>(this));
    session.items.add(game::master::get().name2item("파란열쇠")->make<item>(this));


    // 착용한 상태로 설정 (내구도 등 변할 수 있는 내용들은 저장해둬야 함)
    session.items.weapon(game::master::get().name2item("양첨목봉")->make<weapon>(this)); // 초심자의 목도
    session.items.helmet(game::master::get().name2item("쇄자황금투구")->make<helmet>(this));
    session.items.ring(game::master::get().name2item("쇄자황금반지")->make<ring>(this));
    session.items.ring(game::master::get().name2item("쇄자황금반지")->make<ring>(this));
    session.items.auxiliary(game::master::get().name2item("보무의목걸이")->make<auxiliary>(this));
    session.items.auxiliary(game::master::get().name2item("해독의귀걸이")->make<auxiliary>(this));

    fb::ostream             ostream;
    ostream.write_u8(0x1E)
           .write_u8(0x06)
           .write_u8(0x00);
    this->send_stream(session, ostream, scope::SELF);
    
    this->send_stream(session, this->make_time_stream(), scope::SELF);

    this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);

    this->send_stream(session, message::make_stream("0시간 1분만에 바람으로", message::type::STATE), scope::SELF);

    this->send_stream(session, session.make_id_stream(), scope::SELF);

    this->send_stream(session, session.map()->make_config_stream(), scope::SELF);

    this->send_stream(session, session.map()->make_bgm_stream(), scope::SELF);

    this->send_stream(session, session.make_position_stream(), scope::SELF);

    this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);

    this->send_stream(session, session.make_show_stream(), scope::PIVOT);
    this->send_stream(session, session.make_direction_stream(), scope::SELF);
    this->send_stream(session, session.make_option_stream(), scope::SELF);


    // spell slots
    for(int i = 0; i < spell::MAX_SLOT; i++)
        this->send_stream(session, session.spells.make_update_stream(i), scope::SELF);

    // item slots
    for(int i = 0; i < fb::game::item::MAX_SLOT; i++)
        this->send_stream(session, session.items.make_update_stream(i), scope::SELF);

    // me existed
    for(auto i : session.showns())
        this->send_stream(session, i->make_show_stream(), scope::SELF);

    return true;
}

bool fb::game::acceptor::handle_direction(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    auto                    cmd = istream.read_u8();
    auto                    direction = fb::game::direction(istream.read_u8());

    if(session.direction(direction) == false)
        return false;

    return true;
}

bool fb::game::acceptor::handle_exit(fb::game::session& session)
{
    const auto&             config = fb::config();
    this->transfer(session, inet_addr(config["login"]["ip"].asCString()), config["login"]["port"].asInt());
    return true;
}

bool fb::game::acceptor::handle_move(fb::game::session& session)
{
    auto&                   istream = session.in_stream();

    fb::game::map*          map = session.map();
    if(map == nullptr)
        return false;

    auto                    cmd = istream.read_u8();
    auto                    direction = fb::game::direction(istream.read_u8());
    auto                    sequence = istream.read_u8();
    auto                    x = istream.read_u16();
    auto                    y = istream.read_u16();

    point16_t               before(x, y);
    session.move(direction, before);

    // 워프 위치라면 워프한다.
    const auto              warp = map->warpable(session.position());
    if(warp != nullptr)
        warp->map->objects.add(session, warp->after);
    return true;
}

bool fb::game::acceptor::handle_update_move(fb::game::session& session)
{
    if(this->handle_move(session) == false)
        return false;

    auto&                   istream = session.in_stream();
    auto                    begin_x = istream.read_u16();
    auto                    begin_y = istream.read_u16();
    auto                    width = istream.read_u8();
    auto                    height = istream.read_u8();
    auto                    crc = istream.read_u16();
    this->send_stream(session, session.map()->make_update_stream(begin_x, begin_y, width, height, crc), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_attack(fb::game::session& session)
{
    session.attack();
    return true;
}

bool fb::game::acceptor::handle_pickup(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    auto                    cmd = istream.read_u8();
    bool                    boost = bool(istream.read_u8());

    session.items.pickup(boost);
    return true;
}

bool fb::game::acceptor::handle_emotion(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    auto                    cmd = istream.read_u8();
    auto                    emotion = istream.read_u8();

    session.action(action(action::EMOTION + emotion), duration::DURATION_EMOTION);
    return true;
}

bool fb::game::acceptor::handle_update_map(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    auto                    cmd = istream.read_u8();
    auto                    x = istream.read_u16();
    auto                    y = istream.read_u16();
    auto                    width = istream.read_u8();
    auto                    height = istream.read_u8();
    auto                    crc = istream.read_u16();

    this->send_stream(session, session.map()->make_update_stream(x, y, width, height, crc), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_refresh(fb::game::session& session)
{
    this->send_stream(session, session.make_position_stream(), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_active_item(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    auto                    cmd = istream.read_u8();
    auto                    index = istream.read_u8() - 1;

    session.items.active(index);
    return true;
}

bool fb::game::acceptor::handle_inactive_item(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    auto                    cmd = istream.read_u8();
    auto                    slot(equipment::slot(istream.read_u8()));

    session.items.inactive(slot);
    return true;
}

bool fb::game::acceptor::handle_drop_item(fb::game::session& session)
{
    auto&               istream = session.in_stream();
    auto                cmd = istream.read_u8();
    auto                index = istream.read_u8() - 1;
    bool                drop_all = bool(istream.read_u8());

    session.items.drop(index, drop_all ? -1 : 1);
    return true;
}

bool fb::game::acceptor::handle_drop_cash(fb::game::session& session)
{
    auto                map = session.map();
    if(map == nullptr)
        return false;

    auto&               istream = session.in_stream();
    auto                cmd = istream.read_u8();
    auto                chunk = std::min(session.money(), istream.read_u32());
    auto                cash = new fb::game::cash(chunk, this);

    session.money_drop(chunk);
    return true;
}

// TODO : on_notify를 이용
bool fb::game::acceptor::handle_front_info(fb::game::session& session)
{
    auto                    map = session.map();
    if(map == nullptr)
        return false;

    for(auto i : session.forward_sessions())
    {
        this->send_stream(session, message::make_stream(i->name(), message::type::STATE), scope::SELF);
    }

    for(auto i : session.forward_objects(object::types::OBJECT))
    {
        if(i->type() == fb::game::object::types::ITEM)
        {
            auto item = static_cast<fb::game::item*>(i);
            this->send_stream(session, message::make_stream(item->name_styled(), message::type::STATE), scope::SELF);
        }
        else
        {
            this->send_stream(session, message::make_stream(i->name(), message::type::STATE), scope::SELF);
        }
    }

    return true;
}

bool fb::game::acceptor::handle_self_info(fb::game::session& session)
{
    this->send_stream(session, session.make_internal_info_stream(), scope::SELF);
    
    for(auto buff : session.buffs)
        this->send_stream(session, buff->make_stream(), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_option_changed(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    auto                    cmd = istream.read_u8();
    auto                    option = options(istream.read_u8());

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

bool fb::game::acceptor::handle_click_object(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    auto                    cmd = istream.read_u8();
    auto                    unknown = istream.read_u8();
    auto                    fd = istream.read_u32();

    if(fd == 0xFFFFFFFF) // Press F1
    {
        return true;
    }

    if(fd == 0xFFFFFFFE) // Preff F2
    {
        return true;
    }

    auto                    map = session.map();
    auto                    other = map->objects.find(fd);
    if(other == nullptr)
        return true;

    switch(other->type())
    {
    case fb::game::object::types::SESSION:
        this->send_stream(session, static_cast<game::session*>(other)->make_external_info_stream(), scope::SELF);
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
bool fb::game::acceptor::handle_item_info(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    auto                        cmd = istream.read_u8();
    auto                        position = istream.read_u16();
    auto                        unknown1 = istream.read_u8();
    auto                        unknown2 = istream.read_u8();
    auto                        unknown3 = istream.read_u8();
    auto                        slot = istream.read_u8() - 1;

    auto                        item = session.items[slot];
    if(item == nullptr)
        return false;

    this->send_stream(session, item->make_tip_stream(position), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_itemmix(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    auto                        cmd = istream.read_u8();
    auto                        count = istream.read_u8();
    if(count > item::MAX_SLOT - 1)
        return false;

    itemmix::builder            builder(session, this);
    for(int i = 0; i < count; i++)
        builder.push(istream.read_u8() - 1);

    builder.mix();
    return true;
}

bool fb::game::acceptor::handle_trade(fb::game::session& me)
{
    auto&                       istream = me.in_stream();
    auto                        cmd = istream.read_u8();
    auto                        action = istream.read_u8();
    auto                        fd = istream.read_u32();

    auto                        map = me.map();
    if(map == nullptr)
        return true;

    auto                        you = static_cast<game::session*>(map->objects.find(fd));   // 파트너
    if(you == nullptr)
        return true;

    switch(action)
    {
    case trade::state::REQUEST:
    {
        me.trade.begin(*you);
        break;
    }

    case trade::state::UP_ITEM: // 아이템 올릴때
    {
        auto                index = istream.read_u8() - 1;
        auto                item = me.items[index];
        if(item == nullptr)
            break;

        me.trade.up(*item);
        break;
    }

    case trade::state::ITEM_COUNT: // 아이템 갯수까지 해서 올릴 때
    {
        auto                count = istream.read_u16();
        me.trade.count(count);
        break;
    }

    case trade::state::UP_MONEY: // 금전 올릴 때
    {
        // 클라이언트가 입력한 금전 양
        auto                money = istream.read_u32();
        me.trade.up(money);
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

bool fb::game::acceptor::handle_group(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    auto                        cmd = istream.read_u8();
    auto                        name = istream.readstr_u8();
    

    try
    {
        std::stringstream       sstream;
        auto                    you = this->find_session(name);
        if(you == nullptr)
        {
            sstream << name << "님을 찾을 수 없습니다.";
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
                sstream << name << "님은 이미 그룹 참여 중입니다.";
                throw std::runtime_error(sstream.str());
            }

            mine = fb::game::group::create(session);
            mine->enter(*you);

            sstream << session.name() << "님 그룹에 참여";
            this->send_stream(session, message::make_stream(sstream.str(), message::type::STATE), scope::GROUP);

            sstream.str("");
            sstream << name << "님 그룹에 참여";
            this->send_stream(session, message::make_stream(sstream.str(), message::type::STATE), scope::GROUP);
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
                sstream << name << "님은 다른 그룹에 참여 중입니다.";
                throw std::runtime_error(sstream.str());
            }

            if(mine == your)
            {
                sstream << name << "님 그룹 탈퇴";
                this->send_stream(leader, message::make_stream(sstream.str(), message::type::STATE), scope::GROUP);
                mine->leave(*you);
                return true;
            }

            if(mine->enter(*you) == nullptr)
            {
                throw std::runtime_error("자리가 없습니다.");
            }
            
            sstream << name << "님 그룹에 참여";
            this->send_stream(leader, message::make_stream(sstream.str(), message::type::STATE), scope::GROUP);
        }
    }
    catch(std::exception& e)
    {
        this->send_stream(session, message::make_stream(e.what(), message::type::STATE), scope::SELF);
    }

    return true;
}

bool fb::game::acceptor::handle_user_list(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    auto                        cmd = istream.read_u8();
    auto                        unknown = istream.read_u8();


    fb::ostream                 ostream;
    ostream.write_u8(0x36)
        .write_u16((uint16_t)sessions.size())
        .write_u16((uint16_t)sessions.size())
        .write_u8(0x00);

    for(const auto& i : this->sessions)
    {
        const auto& name = i->name();

        ostream.write_u8(0x10 * i->nation())
            .write_u8(0x10 * i->promotion())
            .write_u8((&session == i) ? 0x88 : 0x0F)
            .write(name, false);
    }

    this->send_stream(session, ostream, scope::SELF);

    return true;
}

bool fb::game::acceptor::handle_chat(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    auto                        cmd = istream.read_u8();
    
    auto                        shout = istream.read_u8();
    auto                        length = istream.read_u8();
    static char                 message[0xFF];
    istream.read(message, length);
    message[length] = 0;

#if defined DEBUG | defined _DEBUG
    if(this->handle_admin(session, message))
        return true;
#endif

    std::stringstream           sstream;
    if(shout)
        sstream << session.name() << "! " << message;
    else
        sstream << session.name() << ": " << message;

    this->send_stream(session, session.make_chat_stream(sstream.str(), shout ? chat::SHOUT : chat::NORMAL), shout ? scope::MAP : scope::PIVOT);

    return true;
}

bool fb::game::acceptor::handle_board(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    auto                        cmd = istream.read_u8();
    auto                        action = istream.read_u8();

    switch(action)
    {
    case 0x01: // section list
    {
        this->send_stream(session, game::master::get().board.make_sections_stream(), scope::SELF);
        break;
    }

    case 0x02: // article list
    {
        auto                    section_id = istream.read_u16();
        auto                    offset = istream.read_u16();

        this->send_stream(session, game::master::get().board.make_articles_stream(section_id, offset), scope::SELF);
        break;
    }

    case 0x03: // article
    {
        auto                    section_id = istream.read_u16();
        auto                    article_id = istream.read_u16();

        this->send_stream(session, game::master::get().board.make_article_stream(section_id, article_id, session), scope::SELF);
        break;
    }

    case 0x04:
    {
        auto                    section_id = istream.read_u16();
        auto                    subject = istream.readstr_u8();
        auto                    content = istream.readstr_u16();

        auto                    section = game::master::get().board.sections().at(section_id);
        if(section->add(subject, content, session.name()) != nullptr)
            this->send_stream(session, game::master::get().board.make_message_stream("글을 작성하였습니다.", true, true), scope::SELF);
        break;
    }

    case 0x05: // delete
    {
        auto                    section_id = istream.read_u16();
        auto                    article_id = istream.read_u16();

        this->send_stream(session, game::master::get().board.make_delete_stream(section_id, article_id, session), scope::SELF);
        break;
    }

    }

    return true;
}

bool fb::game::acceptor::handle_swap(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    auto                        cmd = istream.read_u8();
    auto                        type = swap::type(istream.read_u8());
    auto                        src = istream.read_u8();
    auto                        dest = istream.read_u8();

    switch(type)
    {
    case swap::type::SPELL:
    {
        if(session.spells.swap(src-1, dest-1) == false) // zero-based
            return true;

        const auto              right = session.spells[src-1];
        this->send_stream(session, right ? session.spells.make_update_stream(src-1) : session.spells.make_delete_stream(src-1), scope::SELF);

        const auto              left = session.spells[dest-1];
        this->send_stream(session, left ? session.spells.make_update_stream(dest-1) : session.spells.make_delete_stream(dest-1), scope::SELF);

        break;
    }

    case swap::type::ITEM:
    {
        if(session.items.swap(src-1, dest-1) == false)
            return true;

        const auto              right = session.items[src-1];
        this->send_stream(session, right ? session.items.make_update_stream(src-1) : session.items.make_delete_stream(item::delete_attr::DELETE_REMOVED, src-1), scope::SELF);

        const auto              left = session.items[dest-1];
        this->send_stream(session, left ? session.items.make_update_stream(dest-1) : session.items.make_delete_stream(item::delete_attr::DELETE_REMOVED, dest-1), scope::SELF);
        break;
    }

    default:
        return false;
    }

    return true;
}

bool fb::game::acceptor::handle_dialog(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    auto                        cmd = istream.read_u8();
    auto                        interaction = istream.read_u8();

    switch(interaction)
    {
    case dialog::interaction::NORMAL: // 일반 다이얼로그
    {
        istream.read(nullptr, 0x07); // 7바이트 무시
        auto                    action = istream.read_u8();

        session.dialog.pushinteger(action).resume(1);
        break;
    }

    case dialog::interaction::INPUT:
    {
        auto unknown1 = istream.read_u16();
        auto unknown2 = istream.read_u32();
        auto message = istream.readstr_u16();

        session.dialog.pushstring(message).resume(1);
        break;
    }

    case dialog::interaction::INPUT_EX:
    {
        istream.read(nullptr, 0x07); // 7바이트 무시
        auto                    action = istream.read_u8();
        if(action == 0x02) // OK button
        {
            auto unknown1 = istream.read_u8();
            auto message = istream.readstr_u8();
            session.dialog.pushstring(message);
        }
        else
        {
            session.dialog.pushinteger(action);
        }

        session.dialog.resume(1);
        break;
    }

    case dialog::interaction::MENU:
    {
        auto unknown = istream.read_u32();
        auto index = istream.read_u16();

        session.dialog.pushinteger(index).resume(1);
        break;
    }

    case dialog::interaction::SLOT:
    {
        break;
    }

    case dialog::interaction::SELL:
    {
        auto unknown = istream.read_u32();
        auto pursuit = istream.read_u16();
        auto item_name = istream.readstr_u8();

        session.dialog.pushstring(item_name).resume(1);
        break;
    }

    default:
    {
        break;
    }
    }

    return true;
}

bool fb::game::acceptor::handle_throw_item(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    auto                        cmd = istream.read_u8();
    auto                        unknown = istream.read_u8();
    auto                        index = istream.read_u8() - 1;

    session.items.throws(index);
    return true;
}

bool fb::game::acceptor::handle_spell(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    auto                    cmd = istream.read_u8();
    auto                    slot = istream.read_u8() - 1;

    if(slot > spell::MAX_SLOT - 1)
        return false;

    auto                    spell = session.spells[slot];
    if(spell == nullptr)
        return false;

    lua::thread             thread;
    thread.from("scripts/spell/%s.lua", spell->cast().c_str())
          .func("handle_spell");


    switch(spell->type())
    {
    case spell::types::INPUT:
    {
        char                message[256];
        auto                size = istream.size() - (sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint8_t));
        istream.read(message, std::min(uint64_t(256), uint64_t(size))); message[size] = 0x00;

        thread.pushobject(session)
            .pushobject(spell)
            .pushstring(message)
            .resume(3);
        break;
    }

    case spell::types::TARGET:
    {
        auto                id = istream.read_u32();
        auto                x = istream.read_u16();
        auto                y = istream.read_u16();

        auto                map = session.map();
        if(map == nullptr)
            return false;

        auto                target = map->objects.find(id);
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
    this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_door(fb::game::session& session)
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

        this->send_stream(*session, object::make_show_stream(shown_mobs), scope::SELF);
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

    if(splitted[0] == "show")
    {
        auto item = game::master::get().name2item("남자기모노");
        this->send_stream(session, item->make_dialog_stream("갓승현님 사랑합니다.", true, true), scope::SELF);
        return true;
    }

    if(splitted[0] == "show short list")
    {
        std::vector<std::string> menus = {"갓", "승", "현"};
        this->send_stream(session, npc->make_dialog_stream("갓승현님 존경합니다.", menus), scope::SELF);
        return true;
    }

    if(splitted[0] == "show long list")
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
        this->send_stream(session, npc->make_dialog_stream("갓승현님 존경합니다.", menus), scope::SELF);
        return true;
    }

    if(splitted[0] == "show inventory items")
    {
        std::vector<uint8_t> slots;
        for(int i = 0; i < item::MAX_SLOT; i++)
        {
            const auto item = session.items[i];
            if(item == nullptr)
                continue;

            slots.push_back(i+1);
        }
        this->send_stream(session, npc->make_dialog_stream("갓승현님의 인벤토리를 보여드립니다.", slots), scope::SELF);
        return true;
    }

    if(splitted[0] == "show item core list")
    {
        std::vector<item::master*> items;
        int count = 0;
        for(auto pair : game::master::get().items)
        {
            items.push_back(pair.second);
            if(count++ > 100)
                break;
        }
        this->send_stream(session, npc->make_dialog_stream("아시발 다 판다", items), scope::SELF);
        return true;
    }

    if(splitted[0] == "show question")
    {
        this->send_stream(session, npc->make_input_dialog_stream("What is your name?"), scope::SELF);
        return true;
    }

    if(splitted[0] == "show extend input")
    {
        this->send_stream(session, npc->make_input_dialog_stream("안녕", "탑", "바텀", 0xFF, true), scope::SELF);
        return true;
    }

    if(splitted[0] == "be ghost")
    {
        if(session.state() == state::NORMAL)
            session.state(state::GHOST);
        else
            session.state(state::NORMAL);

        this->send_stream(session, session.make_visual_stream(true), scope::PIVOT);
        return true;
    }

    if(splitted[0] == "weather")
    {
        auto value = std::stoi(splitted[1]);
        this->send_stream(session, weather::make_stream(weather::type(value)), scope::SELF);
        return true;
    }

    if(splitted[0] == "bright")
    {
        auto value = std::stoi(splitted[1]);
        this->send_stream(session, this->make_bright_stream(value), scope::SELF);
        return true;
    }

    if(splitted[0] == "timer")
    {
        auto value = std::stoi(splitted[1]);
        this->send_stream(session, timer::make_stream(value), scope::SELF);
        return true;
    }

    if(splitted[0] == "effect")
    {
        auto value = std::stoi(splitted[1]);
        this->send_stream(session, session.make_effect_stream(value), scope::SELF);
        return true;
    }

    if(splitted[0] == "action")
    {
        auto value = std::stoi(splitted[1]);
        this->send_stream(session, session.make_action_stream(game::action(value), duration::DURATION_SPELL), scope::SELF);
        return true;
    }

    if(splitted[0] == "sound")
    {
        auto value = std::stoi(splitted[1]);
        this->send_stream(session, session.make_sound_stream(game::sound::type(value)), scope::SELF);
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

        map->objects.add(session, point16_t(x, y));
        return true;
    }

    if(splitted[0] == "변신")
    {
        auto name = splitted[1];
        auto mob = game::master::get().name2mob(name);
        if(mob == nullptr)
            return true;

        session.disguise(mob->look());
        this->send_stream(session, session.make_effect_stream(0x03), scope::PIVOT);
        this->send_stream(session, session.make_action_stream(action::CAST_SPELL, duration::DURATION_SPELL), scope::PIVOT);
        this->send_stream(session, session.make_sound_stream(sound::type(0x0019)), scope::PIVOT);
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

        this->send_stream(session, session.spells.make_update_stream(slot), scope::SELF);
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
        this->send_stream(session, mob->make_show_stream(), scope::PIVOT);
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
        this->send_stream(session, session.make_id_stream(), scope::SELF);
        this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
        return true;
    }

    if(splitted[0] == "레벨바꾸기")
    {
        auto level = std::stoi(splitted[1]);
        session.level(level);
        this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
        return true;
    }

    return false;
}
#endif