#include "fb_game.h"
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

IMPLEMENT_LUA_EXTENSION(fb::game::object::core, "fb.game.object.core")
{"name",                fb::game::object::core::builtin_name},
{"look",                fb::game::object::core::builtin_look},
{"color",               fb::game::object::core::builtin_color},
{"dialog",              fb::game::object::core::builtin_dialog},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::mob::core, "fb.game.mob.core")
{"speed",               fb::game::mob::core::builtin_speed},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::mob, "fb.game.mob")
{"__eq",                fb::game::object::builtin_eq},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::npc::core, "fb.game.npc.core")
{"input_dialog",        fb::game::npc::core::builtin_input_dialog},
{"menu_dialog",         fb::game::npc::core::builtin_menu_dialog},
{"item_dialog",         fb::game::npc::core::builtin_item_dialog},
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


IMPLEMENT_LUA_EXTENSION(fb::game::life::core, "fb.game.life.core")
{"hp",                  fb::game::life::core::builtin_hp},
{"mp",                  fb::game::life::core::builtin_mp},
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

IMPLEMENT_LUA_EXTENSION(fb::game::item::core, "fb.game.item.core")
{"make",                fb::game::item::core::builtin_make},
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
    lua::bind_class<object::core, lua::luable>();   lua::bind_class<object, lua::luable>();
    lua::bind_class<life::core, object::core>();    lua::bind_class<life, object>();
    lua::bind_class<mob::core, life::core>();       lua::bind_class<mob, life>();
    lua::bind_class<npc::core, object::core>();     lua::bind_class<npc, object>();
    lua::bind_class<item::core, object::core>();    lua::bind_class<item, object>();
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
    auto& maps = db::maps();
    db::name2map("부여왕초보사냥1")->objects.add(session, point16_t(2, 2));
    session.name(std::to_string(session.id()));
    session.look(0x61);
    session.color(0x0A);
    session.money(150);
    session.sex(fb::game::sex::MAN);
    session.legends_add(0x4A, 0x10, "갓승현 ㅋㅋ");
    session.legends_add(0x4A, 0x10, "똥진영 ㅋㅋ");
    session.base_hp(0xFFFFFFFF);
    session.hp(0xFFFFFFFF);
    session.title("갓승현 타이틀");

    auto& items = db::items();
    session.items.add(db::name2item("얼음칼")->make<item>());
    session.items.add(db::name2item("정화의방패")->make<item>());
    session.items.add(db::name2item("도씨검")->make<item>());
    session.items.add(db::name2item("낙랑의두루마리2")->make<item>());
    session.items.add(db::name2item("남자기모노")->make<item>());
    session.items.add(db::name2item("도토리")->make<item>());
    session.items.add(db::name2item("동동주")->make<item>());
    session.items.add(db::name2item("파란열쇠")->make<item>());
    

    // 착용한 상태로 설정 (내구도 등 변할 수 있는 내용들은 저장해둬야 함)
    session.items.weapon(db::name2item("양첨목봉")->make<weapon>()); // 초심자의 목도
    session.items.helmet(db::name2item("쇄자황금투구")->make<helmet>());
    session.items.ring(db::name2item("쇄자황금반지")->make<ring>());
    session.items.ring(db::name2item("쇄자황금반지")->make<ring>());
    session.items.auxiliary(db::name2item("보무의목걸이")->make<auxiliary>());
    session.items.auxiliary(db::name2item("해독의귀걸이")->make<auxiliary>());

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
    for(auto pair : db::maps())
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
    auto session = new fb::game::session(socket);
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

bool fb::game::acceptor::handle_move_life(fb::game::life* life, fb::game::direction direction)
{
    if(life->alive() == false)
        return false;

    bool                        result = false;

    try
    {
        std::vector<object*>    showns, hiddens;

        if(life->move(direction, nullptr, nullptr, &showns, &hiddens) == false)
            throw std::exception();

        for(auto session : showns)
            this->send_stream(*session, life->make_show_stream(), scope::SELF);

        for(auto session : hiddens)
            this->send_stream(*session, life->make_hide_stream(), scope::SELF);

        for(const auto session : life->map()->objects.sessions())
            this->send_stream(*session, life->make_move_stream(direction), scope::SELF);

        result = true;
    }
    catch(std::exception&)
    {
        result = false;
    }

    if(life->direction() != direction)
    {
        life->direction(direction);
        this->send_stream(*life, life->make_direction_stream(), scope::PIVOT, true);
    }

    return result;
}

void fb::game::acceptor::handle_damage(fb::game::life& me, fb::game::life& you, uint32_t damage)
{
    try
    {
        if(you.is(object::types::LIFE) == false)
            throw std::runtime_error(message::exception::INVALID_TARGET);

        // 몹 체력 깎고 체력게이지 표시
        if(this->handle_damage(static_cast<life&>(you), damage))
        {
            if(you.type() != object::types::MOB)
                return;

            auto& mob = static_cast<fb::game::mob&>(you);
            if(mob.offensive() != mob::offensive_type::NONE)
                mob.target(&me);
        }
        else
        {
            switch(you.type())
            {
            case object::types::SESSION:
                this->handle_die(static_cast<fb::game::session&>(you));
                break;

            case object::types::MOB:
                this->handle_die(static_cast<fb::game::mob&>(you));
                break;
            }

            if(me.is(object::types::SESSION) && you.is(object::types::MOB))
            {
                auto& session = static_cast<fb::game::session&>(me);
#if defined DEBUG | defined _DEBUG
                this->handle_experience(session, you.experience(), false);
#else
                this->handle_experience(session, mob.experience(), true);
#endif
            }
        }
    }
    catch(std::exception& e)
    {
        this->send_stream(me, message::make_stream(e.what(), message::type::STATE), scope::SELF);
    }
}

bool fb::game::acceptor::handle_damage(fb::game::life& life, uint32_t damage)
{
    life.hp_down(damage);
    this->send_stream(life, life.make_show_hp_stream(damage, false), scope::PIVOT);

    return life.alive();
}

void fb::game::acceptor::handle_damage(fb::game::session& session, uint32_t damage)
{
    handle_damage(static_cast<life&>(session), damage);
    this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIDDLE), scope::SELF);

    if(session.alive() == false)
        this->handle_die(session);
}

void fb::game::acceptor::handle_die(fb::game::mob& mob)
{
    // 몹 체력을 다 깎았으면 죽인다.
    this->send_stream(mob, mob.make_die_stream(), scope::PIVOT, true);
    mob.dead_time(fb::timer::now());

    // 드롭 아이템 떨구기
    std::vector<object*> dropped_items;
    for(auto candidate : mob.items())
    {
        if(std::rand() % 100 > candidate.percentage)
            continue;

        auto            item = static_cast<fb::game::item*>(candidate.item->make());
        mob.map()->objects.add(*item, mob.position());

        dropped_items.push_back(item);
    }

    if(dropped_items.size() != 0)
        this->send_stream(mob, object::make_show_stream(dropped_items), scope::PIVOT, true);
}

void fb::game::acceptor::handle_die(fb::game::session& session)
{
    session.state(state::GHOST);
    this->send_stream(session, session.make_visual_stream(true), scope::PIVOT);
}

void fb::game::acceptor::handle_experience(fb::game::session& session, uint32_t exp, bool limit)
{
    // 레벨 5면서 직업 안가졌으면 직업 가지라고 한다.
    if(session.level() >= 5 && session.cls() == 0)
        throw session::require_class_exception();

    // 다음 레벨로 가기 위한 경험치
    auto                    next_exp = session.max_level() ? 0xFFFFFFFF : db::required_exp(session.cls(), session.level()+1);
    auto                    prev_exp = session.max_level() ? 0x00000000 : db::required_exp(session.cls(), session.level());
    auto                    exp_range = next_exp - prev_exp;

    // 3.3% 제한한 경험치
    if(limit && session.max_level() == false)
        exp = std::min(uint32_t(exp_range / 100.0f*3.3f + 1), exp);


    // 경험치 상승
    session.experience_add(exp);
    this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);

    // 경험치 상승 메시지
    auto                    exp_now = session.experience();
    auto                    percentage = std::min(100.0f, ((exp_now - prev_exp) / float(exp_range)) * 100.0f);
    std::stringstream       sstream;
    sstream << "경험치가 " << exp << '(' << int(percentage) << "%) 올랐습니다.";
    this->send_stream(session, message::make_stream(sstream.str(), message::type::STATE), scope::SELF);

    // 레벨업 확인
    if(session.max_level() == false && (exp_now >= next_exp))
        this->handle_level_up(session);
}

void fb::game::acceptor::handle_level_up(fb::game::session& session)
{
    session.level_up();

    this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
    this->send_stream(session, session.make_effect_stream(0x02), scope::PIVOT);

    this->send_stream(session, message::make_stream(message::level::UP, message::type::STATE), scope::SELF);
}

void fb::game::acceptor::handle_click_mob(fb::game::session& session, fb::game::mob& mob)
{
    this->send_stream(session, message::make_stream(mob.name(), message::type::STATE), scope::SELF);
}

void fb::game::acceptor::handle_click_npc(fb::game::session& session, fb::game::npc& npc)
{
    if(session.dialog_thread != nullptr)
        delete session.dialog_thread;

    session.dialog_thread = new lua::thread();

    luaL_dofile(*session.dialog_thread, "scripts/script.lua");
    lua_getglobal(*session.dialog_thread, "handle_click");


    // 루아스크립트의 handle_click 함수의 리턴값 설정
    session.dialog_thread->pushobject(session);
    session.dialog_thread->pushobject(npc);
    session.dialog_thread->resume(2);
}

bool acceptor::handle_login(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    uint8_t                 enc_key[0x09] = {0,};

    auto                    cmd = istream.read_u8();
    auto                    enc_type = istream.read_u8();
    auto                    key_size = istream.read_u8();
    istream.read(enc_key, key_size);

    session.crt(enc_type, enc_key);

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


    // objects existed
    //this->send_stream(session, object::make_show_stream(session.showings(object::types::OBJECT)), scope::SELF);

    // session existed
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

    this->send_stream(session, session.make_direction_stream(), scope::PIVOT, true);
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

    session.direction(direction);

    point16_t               before(x, y);
    if(session.position() != before)
        this->send_stream(session, session.make_position_stream(), scope::SELF);

    std::vector<object*>            shows, hides, showns, hiddens;
    if(session.move_forward(&shows, &hides, &showns, &hiddens))
    {
        this->send_stream(session, session.make_move_stream(), scope::PIVOT, true);
    }
    else
    {
        this->send_stream(session, session.make_position_stream(), scope::SELF);
        return true;
    }

    // 워프 위치라면 워프한다.
    const auto              warp = map->warpable(session.position());
    if(warp != nullptr)
    {
        this->handle_session_warp(session, warp);
        return true;
    }

    this->macro_visible_update(session, &shows, &hides, &showns, &hiddens);
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
    try
    {
        session.state_assert(state::RIDING | state::GHOST);

        this->send_stream(session, session.make_action_stream(action::ATTACK, duration::DURATION_ATTACK), scope::PIVOT);
        auto*               weapon = session.items.weapon();
        if(weapon != nullptr)
        {
            auto            sound = weapon->sound();
            this->send_stream(session, session.make_sound_stream(sound != 0 ? game::sound::type(sound) : game::sound::SWING), scope::PIVOT);
        }


        auto                mobs = session.forward_objects(object::types::MOB);
        auto                i = std::find_if(
                                                mobs.begin(), 
                                                mobs.end(), 
                                                [] (game::object* x)
                                                { 
                                                    return static_cast<game::mob*>(x)->alive(); 
                                                }
                                            );

        if(i == mobs.end())
            return true;

        auto                mob = static_cast<game::mob*>(*i);
#if !defined DEBUG && !defined _DEBUG
        if(std::rand() % 3 == 0)
            return true;
#endif

        auto                critical = false;
        auto                random_damage = session.random_damage(*mob, critical);
        this->handle_damage(session, *mob, random_damage);

        if(weapon != nullptr)
            this->send_stream(session, session.make_sound_stream(sound::type::DAMAGE), scope::PIVOT);

        lua::thread         thread("scripts/common/attack.lua");
        thread.get("handle_attack");
        thread.pushobject(session);
        thread.pushobject(*mob);
        thread.resume(2);
    }
    catch(std::exception& e)
    {
        this->send_stream(session, message::make_stream(e.what(), message::type::STATE), scope::SELF);
    }

    return true;
}

bool fb::game::acceptor::handle_pickup(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    auto                    cmd = istream.read_u8();
    bool                    boost = bool(istream.read_u8());

    try
    {
        auto                map = session.map();
        if(map == nullptr)
            return false;

        session.state_assert(state::GHOST | state::RIDING);

        
        // Do action : pick up
        this->send_stream(session, session.make_action_stream(action::PICKUP, duration::DURATION_PICKUP), scope::PIVOT);


        std::string         message;
        const auto&         objects = map->objects;
        std::vector<item*>  gains;

        // Pick up items in reverse order
        for(auto i = objects.rbegin(); i != objects.rend(); i++)
        {
            auto            object = *i;
            if(object->position() != session.position())
                continue;

            if(object->type() != object::types::ITEM)
                continue;

            auto            below = static_cast<fb::game::item*>(object);
            auto            item_moved = false;
            if(below->attr() & fb::game::item::attrs::ITEM_ATTR_CASH)
            {
                auto        cash = static_cast<fb::game::cash*>(below);
                auto        remain = session.money_add(cash->chunk());
                cash->chunk(remain); // 먹고 남은 돈으로 설정

                if(remain != 0)
                    this->send_stream(session, message::make_stream(message::money::FULL, message::type::STATE), scope::SELF);

                this->send_stream(*cash, cash->make_show_stream(), scope::PIVOT, true);
                this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
            }
            else
            {
                auto            index = session.items.add(below);
                if(index == -1)
                    break;

                this->send_stream(session, session.items.make_update_stream(index), scope::SELF);

                item_moved = (session.items[index] == below);
            }

            if(item_moved || below->empty())
            {
                gains.push_back(below);
                this->send_stream(*below, below->make_hide_stream(), scope::PIVOT, true);
            }

            if(boost == false)
                break;
        }

        for(auto gain : gains)
        {
            map->objects.remove(*gain);
            if(gain->empty())
                delete gain;
        }

        lua::thread thread("scripts/common/pickup.lua");
        thread.get("handle_pickup");
        thread.pushobject(session);
        thread.resume(1);
    }
    catch(std::exception& e)
    {
        this->send_stream(session, message::make_stream(e.what(), message::type::STATE), scope::SELF);
    }

    return true;
}

bool fb::game::acceptor::handle_emotion(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    auto                    cmd = istream.read_u8();
    auto                    emotion = istream.read_u8();

    this->send_stream(session, session.make_action_stream(action(action::EMOTION + emotion), duration::DURATION_EMOTION), scope::PIVOT);
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

    try
    {
        session.state_assert(state::RIDING | state::GHOST);

        auto                updated_index = uint8_t(0);
        auto                slot(equipment::slot::UNKNOWN_SLOT);
        auto                item = session.items.active(index, &updated_index, slot);
        if(item == nullptr)
            return true;

        if(item->attr() & fb::game::item::attrs::ITEM_ATTR_EQUIPMENT)
        {
            this->send_stream(session, session.items.make_delete_stream(fb::game::item::delete_attr::DELETE_NONE, index), scope::SELF);
            if(updated_index != 0xFF)
                this->send_stream(session, session.items.make_update_stream(updated_index), scope::SELF);

            this->send_stream(session, session.make_state_stream(fb::game::state_level::LEVEL_MAX), scope::SELF);
            this->send_stream(session, session.items.make_update_stream(slot), scope::SELF);
            this->send_stream(session, session.make_visual_stream(true), scope::PIVOT);
            this->send_stream(session, session.make_sound_stream(sound::type::EQUIPMENT_ON), scope::PIVOT);

            std::stringstream sstream;
            switch(slot)
            {
            case fb::game::equipment::slot::WEAPON_SLOT:
                sstream << "w:무기  :";
                break;

            case fb::game::equipment::slot::ARMOR_SLOT:
                sstream << "a:갑옷  :";
                break;

            case fb::game::equipment::slot::SHIELD_SLOT:
                sstream << "s:방패  :";
                break;

            case fb::game::equipment::slot::HELMET_SLOT:
                sstream << "h:머리  :";
                break;

            case fb::game::equipment::slot::LEFT_HAND_SLOT:
                sstream << "l:왼손  :";
                break;

            case fb::game::equipment::slot::RIGHT_HAND_SLOT:
                sstream << "r:오른손  :";
                break;

            case fb::game::equipment::slot::LEFT_AUX_SLOT:
                sstream << "[:보조1  :";
                break;

            case fb::game::equipment::slot::RIGHT_AUX_SLOT:
                sstream << "]:보조2  :";
                break;
            }
            sstream << item->name();
            this->send_stream(session, message::make_stream(sstream.str(), message::type::STATE), scope::SELF);

            sstream.str(std::string());
            sstream << "갑옷 강도  " << session.defensive_physical() <<"  " << session.regenerative() << " S  " << session.defensive_magical();
            this->send_stream(session, message::make_stream(sstream.str(), message::type::STATE), scope::SELF);

            return true;
        }


        if(item->attr() & fb::game::item::attrs::ITEM_ATTR_CONSUME)
        {   
            this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
            this->send_stream(session, session.items.make_update_stream(index), scope::SELF);
            this->send_stream(session, session.make_action_stream(action::EAT, duration::DURATION_EAT), scope::SELF);
            this->send_stream(session, session.make_sound_stream(sound::type::EAT), scope::SELF);

            if(item->empty())
            {
                this->send_stream(session, session.items.make_delete_stream(fb::game::item::delete_attr::DELETE_EAT, index), scope::SELF);
                delete item;
            }
        }

        lua::thread         thread("scripts/item/%s.lua", item->active_script().c_str());
        thread.get("handle_active");
        thread.pushobject(session);
        thread.pushobject(*item);
        if(thread.resume(2) == false)
            return true;
    }
    catch(std::exception& e)
    {
        this->send_stream(session, message::make_stream(e.what(), message::type::STATE), scope::PIVOT);
    }

    return true;
}

bool fb::game::acceptor::handle_inactive_item(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    auto                    cmd = istream.read_u8();
    auto                    slot(equipment::slot(istream.read_u8()));

    try
    {
        session.state_assert(state::RIDING | state::GHOST);

        auto                item_index = session.items.inactive(slot);
        if(item_index == 0xFF)
            throw std::runtime_error(message::exception::INVENTORY_OVERFLOW);


        // 마법 딜레이 스트림 필요
        this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
        this->send_stream(session, session.items.make_unequip_stream(slot), scope::SELF);
        this->send_stream(session, session.make_visual_stream(true), scope::PIVOT);
        this->send_stream(session, session.make_sound_stream(sound::type::EQUIPMENT_OFF), scope::PIVOT);
        this->send_stream(session, session.items.make_update_stream(item_index), scope::SELF);
    }
    catch(std::exception& e)
    {
        this->send_stream(session, message::make_stream(e.what(), message::type::STATE), scope::PIVOT);
    }

    return true;
}

bool fb::game::acceptor::handle_drop_item(fb::game::session& session)
{
    auto&               istream = session.in_stream();
    auto                cmd = istream.read_u8();
    auto                index = istream.read_u8() - 1;
    bool                drop_all = bool(istream.read_u8());

    this->macro_drop_item(session, index, drop_all);
    return true;
}

bool fb::game::acceptor::handle_drop_cash(fb::game::session& session)
{
    try
    {
        session.state_assert(state::RIDING | state::GHOST);

        auto                map = session.map();
        if(map == nullptr)
            return false;

        auto&               istream = session.in_stream();
        auto                cmd = istream.read_u8();
        auto                chunk = std::min(session.money(), istream.read_u32());
        auto                cash = new fb::game::cash(chunk);

        session.money_reduce(chunk);

        map->objects.add(*cash, session.position());
        this->send_stream(session, session.make_action_stream(action::PICKUP, duration::DURATION_PICKUP), scope::PIVOT);
        this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
        this->send_stream(*cash, cash->make_show_stream(), scope::PIVOT, true);
        this->send_stream(session, message::make_stream(message::money::DROP, message::type::STATE), scope::SELF);

        return true;
    }
    catch(std::exception& e)
    {
        this->send_stream(session, message::make_stream(e.what(), message::type::STATE), scope::SELF);
    }
}

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


    bool                    enabled = session.option_toggle(option);
    std::stringstream       sstream;
    switch(option)
    {
    case options::RIDE:
    {
        static auto         horse_core = db::name2mob("말");
        if(horse_core == nullptr)
            break;

        try
        {
            session.state_assert(state::GHOST | state::DISGUISE);

            object*         forward = nullptr;
            std::string     message;

            if(session.state() == fb::game::state::RIDING)
            {
                message = game::message::ride::ON;
                session.state(fb::game::state::NORMAL);

                auto horse = static_cast<fb::game::mob*>(horse_core->make());
                session.map()->objects.add(*horse, session.position_forward());
                horse->heal();
                this->send_stream(*horse, horse->make_show_stream(), scope::PIVOT, true);
            }
            else
            {
                forward = session.forward_object(object::types::MOB);
                if(forward == nullptr || forward->based() != horse_core)
                    throw session::no_conveyance_exception();

                forward->map()->objects.remove(*forward);
                this->send_stream(*forward, forward->make_hide_stream(), scope::PIVOT, true);
                message = game::message::ride::OFF;
                session.state(fb::game::state::RIDING);

                forward->map()->objects.remove(*forward);
                delete forward;
            }

            this->send_stream(session, session.make_visual_stream(true), scope::PIVOT);
            this->send_stream(session, message::make_stream(message, message::type::STATE), scope::SELF);
        }
        catch(std::exception& e)
        {
            this->send_stream(session, message::make_stream(e.what(), message::type::STATE), scope::SELF);
        }
        return true;
    }

    case options::WHISPER:
        sstream << "귓속말듣기  ";
        break;

    case options::GROUP:
    {
        auto group = session.group();
        if(group != nullptr)
        {
            if(group->members().size() == 1)
            {
                this->send_stream(session, message::make_stream("그룹 해체", message::type::STATE), scope::GROUP);
                fb::game::group::destroy(*group);
            }
            else
            {
                auto leader = group->leave(session);
                std::stringstream sstream;
                sstream << session.name() << "님 그룹 탈퇴";
                this->send_stream(*leader, message::make_stream(sstream.str(), message::type::STATE), scope::GROUP);
            }
        }

        sstream << "그룹허가    ";
        break;
    }

    case options::ROAR:
        sstream << "외치기듣기  ";
        break;

    case options::ROAR_WORLDS:
        sstream << "세계후      ";
        break;

    case options::MAGIC_EFFECT:
        sstream << "마법이펙트  ";
        break;

    case options::WEATHER_EFFECT:
        sstream << "날씨변화    ";
        break;

    case options::FIXED_MOVE:
        sstream << "고정이동     ";
        break;

    case options::TRADE:
        sstream << "교환가능    ";
        break;

    case options::FAST_MOVE:
        sstream << "빠른이동    ";
        break;

    case options::EFFECT_SOUND:
        sstream << "소리듣기    ";
        break;

    case options::PK:
        sstream << "PK보호      ";
        break;

    default:
        return false;
    }

    sstream << ": " << (enabled ? "ON" : "OFF");
    this->send_stream(session, message::make_stream(sstream.str(), message::type::STATE), scope::SELF);
    this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
    this->send_stream(session, session.make_option_stream(), scope::SELF);

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
    auto                    other = map->objects[fd];
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

    std::vector<item*>          items;
    for(int i = 0; i < count; i++)
    {
        auto                    index = istream.read_u8() - 1;
        auto                    item = session.items[index];
        if(item == nullptr)
            return true;

        items.push_back(item);
    }
    
    try
    {
        auto                    itemmix = db::find_itemmix(items);
        if(itemmix == nullptr)
            throw itemmix::no_match_exception();

        auto                    free_size = session.items.free_size();
        if(int(itemmix->success.size()) - int(itemmix->require.size()) > free_size)
            throw item::full_inven_exception();


        for(auto require : itemmix->require)
        {
            auto                index = session.items.to_index(require.item);
            if(index == 0xFF)
                return true;

            auto                item = session.items[index];
            item->reduce(require.count);


            if(item->empty())
            {
                this->send_stream(session, session.items.make_delete_stream(item::delete_attr::DELETE_NONE, index), scope::SELF);
                session.items.remove(index);
                delete item;
            }
            else
            {
                this->send_stream(session, session.items.make_update_stream(index), scope::SELF);
            }
        }

        auto                success = (std::rand() % 100) < itemmix->percentage;
        std::string         message;
        if(success)
        {
            for(auto success : itemmix->success)
            {
                auto        item = static_cast<fb::game::item*>(success.item->make());
                item->count(success.count);
                
                auto        index = session.items.add(item);
                this->send_stream(session, session.items.make_update_stream(index), scope::SELF);
            }

            message = game::message::mix::SUCCESS;
        }
        else
        {
            for(auto failed : itemmix->failed)
            {
                auto        item = static_cast<fb::game::item*>(failed.item->make());
                item->count(failed.count);
                
                auto        index = session.items.add(item);
                this->send_stream(session, session.items.make_update_stream(index), scope::SELF);
            }

            message = game::message::mix::FAILED;
        }

        this->send_stream(session, message::make_stream(message, message::type::STATE), scope::SELF);
    }
    catch(std::exception& e)
    { 
        this->send_stream(session, message::make_stream(e.what(), message::type::STATE), scope::SELF);
        return true;
    }

    

    return true;
}

bool fb::game::acceptor::handle_trade(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    auto                        cmd = istream.read_u8();
    auto                        action = istream.read_u8();
    auto                        fd = istream.read_u32();

    auto                        map = session.map();
    if(map == nullptr)
        return true;

    auto                        you = static_cast<game::session*>(map->objects[fd]);   // 파트너
    if(you == nullptr)
        return true;

    switch(action)
    {
    case 0:
    {
        if(session.id() == you->id())
        {
            // 자기 자신과 거래를 하려고 시도하는 경우
            break;
        }

        if(session.option(options::TRADE) == false)
        {
            // 내가 교환 거부중
            this->send_stream(session, message::make_stream(message::trade::REFUSED_BY_ME, message::type::STATE), scope::SELF);
            break;
        }

        if(you->option(options::TRADE) == false)
        {
            // 상대방이 교환 거부중
            std::stringstream sstream;
            sstream << you->name() << message::trade::REFUSED_BY_PARTNER;
            this->send_stream(session, message::make_stream(sstream.str(), message::type::STATE), scope::SELF);
            break;
        }

        if(session.trade.trading())
        {
            // 내가 이미 교환중
            break;
        }

        if(you->trade.trading())
        {
            // 상대방이 이미 교환중
            std::stringstream sstream;
            sstream << you->name() << message::trade::PARTNER_ALREADY_TRADING;
            this->send_stream(session, message::make_stream(sstream.str(), message::type::STATE), scope::SELF);
            break;
        }

        if(session.sight(*you) == false)
        {
            // 상대방이 시야에서 보이지 않음
            this->send_stream(session, message::make_stream(message::trade::PARTNER_INVISIBLE, message::type::STATE), scope::SELF);
            break;
        }

        if(session.distance_sqrt(*you) > 16)
        {
            // 상대방과의 거리가 너무 멈
            std::stringstream sstream;
            sstream << you->name() << message::trade::PARTNER_TOO_FAR;
            this->send_stream(session, message::make_stream(sstream.str(), message::type::STATE), scope::SELF);
            break;
        }

        // 교환 시작
        session.trade.begin(you);
        this->send_stream(session, you->trade.make_dialog_stream(), scope::SELF);

        you->trade.begin(&session);
        this->send_stream(*you, session.trade.make_dialog_stream(), scope::SELF);
        break;
    }

    case 1: // 아이템 올릴때
    {
        auto                index = istream.read_u8() - 1;
        auto                item = session.items[index];
        if(item == nullptr)
            return true;

        // 교환 불가능한 아이템 거래 시도
        if(item->trade_enabled() == false)
        {
            this->send_stream(session, message::make_stream(message::trade::NOT_ALLOWED_TO_TRADE, message::type::POPUP), scope::SELF);
            break;
        }

        if((item->attr() & fb::game::item::attrs::ITEM_ATTR_BUNDLE) && (item->count() > 1))
        {
            // 묶음 단위의 아이템 형식 거래 시도
            session.trade.select(item);
            this->send_stream(session, session.trade.make_bundle_stream(), scope::SELF);
        }
        else
        {
            // 일반 아이템의 거래 시도

            uint8_t trade_index = session.trade.add(item); // 거래중인 아이템 리스트의 인덱스
            if(trade_index == 0xFF)
                return false;

            // 현재 인벤토리에서 거래중인 아이템 리스트로 아이템 이동
            session.items.remove(index);
            this->send_stream(session, session.items.make_delete_stream(item::delete_attr::DELETE_NONE, index), scope::SELF);

            // 나와 상대 둘 다에게 올린 아이템을 표시함
            this->send_stream(session, session.trade.make_show_stream(true, trade_index), scope::SELF);
            this->send_stream(*you, session.trade.make_show_stream(false, trade_index), scope::SELF);
        }
        break;
    }

    case 2: // 아이템 갯수까지 해서 올릴 때
    {
        // 이전에 올리려고 시도한 묶음 단위의 아이템
        auto                selected = session.trade.selected();
        if(selected == nullptr)
            return false;

        // 올릴 갯수 (클라이언트가 입력한 값)
        auto                count = istream.read_u16();
        if(selected->count() < count)
        {
            this->send_stream(session, message::make_stream(message::trade::INVALID_COUNT, message::type::POPUP), scope::SELF);
            break;
        }

        auto                index = session.items.to_index(selected->based<item::core>());
        if(selected->count() == count)
        {
            // 모두 다 올리는 경우, 아이템을 따로 복사하지 않고 있는 그대로 거래리스트에 옮겨버린다.
            auto            trade_index = session.trade.add(selected);
            if(index != 0xFF)
            {
                session.items.remove(index);
                this->send_stream(session, session.items.make_delete_stream(item::delete_attr::DELETE_NONE, index), scope::SELF);

                this->send_stream(session, session.trade.make_show_stream(true, trade_index), scope::SELF);
                this->send_stream(*you, session.trade.make_show_stream(false, trade_index), scope::SELF);
            }
        }
        else
        {
            // 일부만 올리는 경우, 기존의 것에서 갯수를 깎고 새로 복사된 아이템을 거래 리스트로 옮긴다.
            selected->reduce(count);
            auto            came_out = selected->clone<fb::game::item>();
            came_out->count(count);

            uint8_t trade_index = session.trade.add(came_out);

            this->send_stream(session, session.items.make_update_stream(index), scope::SELF);

            this->send_stream(session, session.trade.make_show_stream(true, trade_index), scope::SELF);
            this->send_stream(*you, session.trade.make_show_stream(false, trade_index), scope::SELF);
        }
        break;
    }

    case 3: // 금전 올릴 때
    {
        // 클라이언트가 입력한 금전 양
        auto                money = istream.read_u32();

        // 입력한 금전 양을 계속해서 빼면 안된다.
        // 100전 입력한 경우 -1, -10, -100 이렇게 까여버림
        auto                total = session.money() + session.trade.money();
        if(money > total)
            money = total;

        session.money(total - money);
        session.trade.money(money);

        this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
        this->send_stream(session, session.trade.make_money_stream(true), scope::SELF);
        this->send_stream(*you, session.trade.make_money_stream(false), scope::SELF);
        break;
    }

    case 4: // 취소한 경우
    {
        std::vector<uint8_t> indices;

        // 거래리스트에 올렸던 아이템과 금전을 원상복귀시킨다.
        indices = session.trade.restore();
        this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
        for(auto i : indices)
            this->send_stream(session, session.items.make_update_stream(i), scope::SELF);

        // 메시지 표시하고 거래종료
        this->send_stream(session, session.trade.make_close_stream(message::trade::CANCELLED_BY_ME), scope::SELF);
        session.trade.end();



        // 거래리스트에 올렸던 아이템과 금전을 원상복귀시킨다.
        indices = you->trade.restore();
        this->send_stream(*you, you->make_state_stream(state_level::LEVEL_MIN), scope::SELF);
        for(auto i : indices)
            this->send_stream(*you, you->items.make_update_stream(i), scope::SELF);

        // 메시지 표시하고 거래종료
        this->send_stream(*you, you->trade.make_close_stream(message::trade::CANCELLED_BY_PARTNER), scope::SELF);
        you->trade.end();
        break;
    }

    case 5:
    {
        this->send_stream(session, session.trade.make_lock_stream(), scope::SELF);
        session.trade.lock(true);

        if(you->trade.lock())
        {
            // 상대방이 이미 교환확인 누른 경우
            if(session.trade.flushable(*you) == false || you->trade.flushable(session) == false)
            {
                // 나 혹은 상대가 거래리스트에 올라온 아이템이나 금전을 다 받을 수 없는 상황일 때
                // 인벤토리가 가득 차거나, 더 이상 금전을 얻을 수 없는 경우
                this->send_stream(session, session.trade.make_close_stream(message::trade::FAILED), scope::SELF);
                this->send_stream(*you, you->trade.make_close_stream(message::trade::FAILED), scope::SELF);
            }
            else
            {
                std::vector<uint8_t> indices;
                
                // 상대의 거래리스트 물품들을 전부 받고 업데이트
                session.trade.flush(*you, indices);
                for(auto i : indices)
                    this->send_stream(*you, you->items.make_update_stream(i), scope::SELF);
                this->send_stream(*you, you->make_state_stream(state_level::LEVEL_MIN), scope::SELF);

                // 나의 거래리스트 물품들을 전부 주고 업데이트
                you->trade.flush(session, indices);
                for(auto i : indices)
                    this->send_stream(session, session.items.make_update_stream(i), scope::SELF);
                this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);


                // 메시지 표시하고 거래 종료
                this->send_stream(session, session.trade.make_close_stream(message::trade::SUCCESS), scope::SELF);
                session.trade.end();

                this->send_stream(*you, you->trade.make_close_stream(message::trade::SUCCESS), scope::SELF);
                you->trade.end();
            }
        }
        else
        {
            // 상대방이 아직 교환확인을 누르지 않은 경우
            this->send_stream(*you, message::make_stream(message::trade::NOTIFY_LOCK_TO_PARTNER, message::type::POPUP), scope::SELF);
        }
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
        .write_u16(sessions.size())
        .write_u16(sessions.size())
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
    auto&                       board = db::board();
    auto&                       istream = session.in_stream();
    auto                        cmd = istream.read_u8();
    auto                        action = istream.read_u8();

    switch(action)
    {
    case 0x01: // section list
    {
        this->send_stream(session, board.make_sections_stream(), scope::SELF);
        break;
    }

    case 0x02: // article list
    {
        auto                    section_id = istream.read_u16();
        auto                    offset = istream.read_u16();

        this->send_stream(session, board.make_articles_stream(section_id, offset), scope::SELF);
        break;
    }

    case 0x03: // article
    {
        auto                    section_id = istream.read_u16();
        auto                    article_id = istream.read_u16();

        this->send_stream(session, board.make_article_stream(section_id, article_id, session), scope::SELF);
        break;
    }

    case 0x04:
    {
        auto                    section_id = istream.read_u16();
        auto                    subject = istream.readstr_u8();
        auto                    content = istream.readstr_u16();

        auto                    section = board.sections().at(section_id);
        if(section->add(subject, content, session.name()) != nullptr)
            this->send_stream(session, board.make_message_stream("글을 작성하였습니다.", true, true), scope::SELF);
        break;
    }

    case 0x05: // delete
    {
        auto                    section_id = istream.read_u16();
        auto                    article_id = istream.read_u16();

        this->send_stream(session, board.make_delete_stream(section_id, article_id, session), scope::SELF);
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

        if(session.dialog_thread == nullptr)
            return true;

        // 루아스크립트 다이얼로그 함수의 리턴값 설정
        session.dialog_thread->pushinteger(action);
        if(session.dialog_thread->resume(1))
        {
            delete session.dialog_thread;
            session.dialog_thread = nullptr;
        }
        break;
    }

    case dialog::interaction::INPUT:
    {
        auto unknown1 = istream.read_u16();
        auto unknown2 = istream.read_u32();
        auto message = istream.readstr_u16();

        if(session.dialog_thread == nullptr)
            return true;

        session.dialog_thread->pushstring(message);
        if(session.dialog_thread->resume(1))
        {
            delete session.dialog_thread;
            session.dialog_thread = nullptr;
        }
        break;
    }

    case dialog::interaction::INPUT_EX:
    {
        if(session.dialog_thread == nullptr)
            return true;

        istream.read(nullptr, 0x07); // 7바이트 무시
        auto                    action = istream.read_u8();
        if(action == 0x02) // OK button
        {
            auto unknown1 = istream.read_u8();
            auto message = istream.readstr_u8();
            session.dialog_thread->pushstring(message);
        }
        else
        {
            session.dialog_thread->pushinteger(action);
        }

        if(session.dialog_thread->resume(1))
        {
            delete session.dialog_thread;
            session.dialog_thread = nullptr;
        }
        break;
    }

    case dialog::interaction::MENU:
    {
        auto unknown = istream.read_u32();
        auto index = istream.read_u16();

        if(session.dialog_thread == nullptr)
            return true;

        session.dialog_thread->pushinteger(index);
        if(session.dialog_thread->resume(1))
        {
            delete session.dialog_thread;
            session.dialog_thread = nullptr;
        }
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

        if(session.dialog_thread == nullptr)
            return true;

        session.dialog_thread->pushstring(item_name);
        if(session.dialog_thread->resume(1))
        {
            delete session.dialog_thread;
            session.dialog_thread = nullptr;
        }

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
    auto                        slot = istream.read_u8() - 1;

    try
    {
        auto                    item = session.items.at(slot);
        if(item == nullptr)
            throw std::exception();

        if(item->trade_enabled() == false)
            throw std::runtime_error(message::exception::CANNOT_THROW_ITEM);

        auto                    map = session.map();
        if(map == nullptr)
            throw std::exception();

        auto                    dropped = item->split(session);
        item->direction(session.direction());
        for(int i = 0; i < 7; i++)
        {
            if(map->movable_forward(*item) == false)
                break;

            item->position(item->position_forward());
        }

        if(item == dropped)
        {
            session.items.remove(slot);
            this->send_stream(session, session.items.make_delete_stream(item::delete_attr::DELETE_THROW, slot), scope::SELF);
        }
        else
        {
            this->send_stream(session, session.items.make_update_stream(slot), scope::SELF);
        }
        this->send_stream(session, session.make_throw_item_stream(*item), scope::PIVOT);
        this->send_stream(session, item->make_show_stream(), scope::PIVOT);
    }
    catch(std::runtime_error& e)
    {
        auto message = std::string(e.what());
        if(message.empty() == false)
            this->send_stream(session, message::make_stream(message, message::type::STATE), scope::SELF);
    }
    catch(std::exception& e)
    {

    }
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

    lua::thread             thread("scripts/spell/%s.lua", spell->cast().c_str());
    thread.get("handle_spell");


    switch(spell->type())
    {
    case spell::types::INPUT:
    {
        char                message[256];
        auto                size = istream.size() - (sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint8_t));
        istream.read(message, std::min(uint64_t(256), uint64_t(size))); message[size] = 0x00;

        thread.pushobject(session);
        thread.pushobject(spell);
        thread.pushstring(message);
        thread.resume(3);
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

        auto                target = map->objects.at(id);
        if(target == nullptr)
            return true;

        if(session.sight(*target) == false)
            return true;

        thread.pushobject(session);
        thread.pushobject(spell);
        thread.pushobject(target);
        thread.resume(3);
        break;
    }

    case spell::types::NORMAL:
    {
        thread.pushobject(session);
        thread.pushobject(spell);
        thread.resume(2);
        break;
    }
    }

    this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_door(fb::game::session& session)
{
    lua::thread             thread("scripts/common/door.lua");
    thread.get("handle_door");

    thread.pushobject(session);
    if(thread.resume(1) == false)
        return true;

    //auto redraw = thread.toboolean(1);
    //if(redraw == false)
    //    return true;

    //auto core = door->based();
    //this->send_stream(session, map->make_update_stream(door->position.x, door->position.y, core.size(), 1), scope::MAP);

    //if(door->opened())
    //    this->send_stream(session, message::make_stream(message::door::OPEN, message::type::STATE), scope::SELF);
    //else
    //    this->send_stream(session, message::make_stream(message::door::CLOSE, message::type::STATE), scope::SELF);

    return true;
}

void fb::game::acceptor::handle_counter_mob_action(fb::game::mob* mob)
{
    auto                    target = mob->target();
    if(target == nullptr)
    {
        this->handle_move_life(mob, direction(std::rand() % 4));
        return;
    }

    // 상하좌우로 타겟이 있는지 검사한다.
    for(int i = 0; i < 4; i++)
    {
        auto                direction = game::direction(i);
        if(mob->near_session(direction) != target)
            continue;

        if(mob->direction() != direction)
        {
            mob->direction(direction);
            this->send_stream(*mob, mob->make_direction_stream(), scope::PIVOT, true);
        }

        auto                damage = mob->random_damage(*target);

        this->send_stream(*mob, mob->make_action_stream(action::ATTACK, duration::DURATION_ATTACK), scope::PIVOT, true);
        this->handle_damage(*target, damage);
        return;
    }


    // 타겟 방향으로 이동이 가능하다면 이동한다.
    auto                    x_axis = bool(std::rand()%2);
    if(x_axis)
    {
        if(mob->x() > target->x() && this->handle_move_life(mob, direction::LEFT))   return;
        if(mob->x() < target->x() && this->handle_move_life(mob, direction::RIGHT))  return;
        if(mob->y() > target->y() && this->handle_move_life(mob, direction::TOP))    return;
        if(mob->y() < target->y() && this->handle_move_life(mob, direction::BOTTOM)) return;
    }
    else
    {
        if(mob->y() > target->y() && this->handle_move_life(mob, direction::TOP))    return;
        if(mob->y() < target->y() && this->handle_move_life(mob, direction::BOTTOM)) return;
        if(mob->x() > target->x() && this->handle_move_life(mob, direction::LEFT))   return;
        if(mob->x() < target->x() && this->handle_move_life(mob, direction::RIGHT))  return;
    }


    // 이동할 수 있는 방향으로 일단 이동한다.
    auto                    random_direction = std::rand() % 4;
    for(int i = 0; i < 4; i++)
    {
        if(this->handle_move_life(mob, direction((random_direction + i) % 4)))
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
    for(auto pair : db::maps())
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
    for(auto pair : db::maps())
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
    for(auto pair : db::maps())
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
                this->macro_object_unbuff(*object, finish->spell().name());
        }
    }
}

void fb::game::acceptor::handle_session_warp(fb::game::session& session, const map::warp* warp)
{
    auto                    map = session.map();

    this->macro_object_map(session, *warp->map, warp->after);
}

#if defined DEBUG | defined _DEBUG
bool fb::game::acceptor::handle_admin(fb::game::session& session, const std::string& message)
{
    if(message[0] != '/')
        return false;

    auto                            npc = db::name2npc("낙랑");
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
        auto item = db::name2item("남자기모노");
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
        std::vector<item::core*> items;
        int count = 0;
        for(auto pair : db::items())
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

        auto map = db::name2map(name);
        if(map == nullptr)
            return true;

        this->macro_object_map(session, *map, point16_t(x, y));
        return true;
    }

    if(splitted[0] == "변신")
    {
        auto name = splitted[1];
        auto mob = db::name2mob(name);
        if(mob == nullptr)
            return true;

        session.disguise(mob->look());
        session.state(state::DISGUISE);
        this->send_stream(session, session.make_visual_stream(true), scope::PIVOT);
        this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
        this->send_stream(session, session.make_effect_stream(0x03), scope::PIVOT);
        this->send_stream(session, session.make_action_stream(action::CAST_SPELL, duration::DURATION_SPELL), scope::PIVOT);
        this->send_stream(session, session.make_sound_stream(sound::type(0x0019)), scope::PIVOT);
        // TODO: spell delay
        return true;
    }

    if(splitted[0] == "변신해제")
    {
        session.disguise(0);
        session.state(state::NORMAL);
        this->send_stream(session, session.make_visual_stream(true), scope::PIVOT);
        this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
        return true;
    }

    if(splitted[0] == "마법배우기")
    {
        auto name = splitted[1];
        auto spell = db::name2spell(name);
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
        auto core = db::name2mob(name);
        if(core == nullptr)
            return true;

        auto mob = new fb::game::mob(core, true);
        auto map = session.map();
        map->objects.add(*mob, session.position());
        this->send_stream(session, mob->make_show_stream(), scope::PIVOT);
        return true;
    }

    if(splitted[0] == "직업바꾸기")
    {
        auto name = splitted[1];
        uint8_t cls, promotion;
        if(db::name2class(name, &cls, &promotion) == false)
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

item* fb::game::acceptor::macro_remove_item(fb::game::session& session, uint8_t index, int count, item::delete_attr attr)
{
    auto                item = session.items[index];
    if(item == nullptr)
        return nullptr;

    auto                dropped = item->split(session, count);
    if(item == dropped)
    {
        this->send_stream(session, session.items.make_delete_stream(attr, index), scope::SELF);
        session.items.remove(index);
    }
    
    return dropped;
}

fb::game::item* fb::game::acceptor::macro_drop_item(fb::game::session& session, uint8_t index, bool drop_all)
{
    try
    {
        session.state_assert(state::RIDING | state::GHOST);

        auto                item = session.items[index];
        if(item == nullptr)
            return nullptr;

        if(item->trade_enabled() == false)
            throw std::runtime_error(message::exception::CANNOT_DROP_ITEM);

        auto                dropped = this->macro_remove_item(session, index, drop_all ? item->count() : 1);
        this->send_stream(session, session.make_action_stream(action::PICKUP, duration::DURATION_PICKUP), scope::PIVOT);
        this->send_stream(session, dropped->make_show_stream(), scope::PIVOT);
        this->send_stream(session, session.items.make_update_stream(index), scope::SELF);
        return dropped;
    }
    catch(std::exception& e)
    {
        this->send_stream(session, message::make_stream(e.what(), message::type::STATE), scope::SELF);
        return nullptr;
    }
}

void fb::game::acceptor::macro_visible_update(fb::game::object& object, std::vector<fb::game::object*>* shows, std::vector<fb::game::object*>* hides, std::vector<fb::game::object*>* showns, std::vector<fb::game::object*>* hiddens)
{
    if(shows != nullptr)
    {
        for(auto i : *shows)
        {
            if(i->is(object::types::MOB) && i->visible() == false)
                continue;

            this->send_stream(object, i->make_show_stream(), scope::SELF);
        }
    }

    if(hides != nullptr)
    {
        for(auto i : *hides)
        {
            if(i->is(object::types::MOB) && i->visible() == false)
                continue;

            this->send_stream(object, i->make_hide_stream(), scope::SELF);
        }
    }

    if(showns != nullptr)
    {
        for(auto i : *showns)
            this->send_stream(*i, object.make_show_stream(), scope::SELF);
    }

    if(hiddens != nullptr)
    {
        for(auto i : *hiddens)
            this->send_stream(*i, object.make_hide_stream(), scope::SELF);
    }
}

void fb::game::acceptor::macro_object_map(fb::game::object& object, fb::game::map& map, const point16_t& position)
{
    // 이전에 보이던 오브젝트들 전부 제거
    for(auto i : object.showings())
        this->send_stream(object, i->make_hide_stream(), scope::SELF);

    // 이전에 나를 보던 오브젝트들에게서 나를 제거
    for(auto i : object.showns())
        this->send_stream(*i, object.make_hide_stream(), scope::SELF);

    map.objects.add(object, position);


    if(object.type() == object::types::SESSION)
    {
        auto& session = static_cast<game::session&>(object);
        this->send_stream(session, session.make_id_stream(), scope::SELF);
        this->send_stream(session, map.make_config_stream(), scope::SELF);
        this->send_stream(session, map.make_bgm_stream(), scope::SELF);
        this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
        this->send_stream(session, session.make_position_stream(), scope::SELF);
        this->send_stream(session, session.make_visual_stream(false), scope::SELF);
        this->send_stream(session, session.make_direction_stream(), scope::SELF);
    }

    // 지금 내가 볼 수 있는 오브젝트를 표시
    for(auto i : object.showings())
        this->send_stream(object, i->make_show_stream(), scope::SELF);

    // 지금 나를 볼 수 있는 오브젝트들에게 나를 표시
    for(auto i : object.showns())
        this->send_stream(*i, object.make_show_stream(), scope::SELF);
}

bool fb::game::acceptor::macro_object_unbuff(fb::game::object& object, const std::string& bufname)
{
    auto buff = object.buffs[bufname];
    if(buff == nullptr)
        return false;

    lua::thread thread("scripts/spell/%s.lua", buff->spell().uncast().c_str());
    thread.get("handle_uncast");
    thread.pushobject(object);
    thread.pushobject(buff->spell());
    thread.resume(2);

    object.buffs.remove(buff);
    this->send_stream(object, buff->make_clear_stream(), scope::SELF);
    return true;
}

