#include "fb_game.h"
using namespace fb::game;

IMPLEMENT_LUA_EXTENSION(fb::game::acceptor, "")
END_LUA_EXTENSION

acceptor::acceptor(uint16_t port) : fb_acceptor<fb::game::session>(port)
{
    lua::env<acceptor>("acceptor", this);

    lua::bind_class<lua::luable>();
    lua::bind_class<spell, lua::luable>();
    lua::bind_class<map, lua::luable>();
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


    this->register_handle(0x10, &acceptor::handle_login);               // ���Ӽ��� ���� �ڵ鷯
    this->register_handle(0x11, &acceptor::handle_direction);           // ������ȯ �ڵ鷯
    this->register_handle(0x06, &acceptor::handle_update_move);         // �̵��� �� ������ ������Ʈ �ڵ鷯
    this->register_handle(0x32, &acceptor::handle_move);                // �̵� �ڵ鷯
    this->register_handle(0x13, &acceptor::handle_attack);              // ���� �ڵ鷯
    this->register_handle(0x07, &acceptor::handle_pickup);              // ������ �ݱ� �ڵ鷯
    this->register_handle(0x1D, &acceptor::handle_emotion);             // ����ǥ�� �ڵ鷯
    this->register_handle(0x05, &acceptor::handle_update_map);          // �� ������ ������Ʈ �ڵ鷯
    this->register_handle(0x38, &acceptor::handle_refresh);             // ���ΰ�ħ �ڵ鷯
    this->register_handle(0x1C, &acceptor::handle_active_item);         // ������ ��� �ڵ鷯
    this->register_handle(0x1F, &acceptor::handle_inactive_item);       // ������ ���� ���� �ڵ鷯
    this->register_handle(0x08, &acceptor::handle_drop_item);           // ������ ������ �ڵ鷯
    this->register_handle(0x24, &acceptor::handle_drop_cash);           // ���� ������ �ڵ鷯
    this->register_handle(0x09, &acceptor::handle_front_info);          // �չ��� ���� �ڵ鷯
    this->register_handle(0x2D, &acceptor::handle_self_info);           // �� �ڽ��� ���� �ڵ鷯
    this->register_handle(0x1B, &acceptor::handle_option_changed);      // �ɼ� ���� �ڵ鷯
    this->register_handle(0x43, &acceptor::handle_click_object);        // ������Ʈ Ŭ�� �ڵ鷯
    this->register_handle(0x66, &acceptor::handle_item_info);           // �κ��丮 ��Ŭ�� �ڵ鷯
    this->register_handle(0x6B, &acceptor::handle_itemmix);             // ������ ���� �ڵ鷯
    this->register_handle(0x4A, &acceptor::handle_trade);               // ��ȯ �ڵ鷯
    this->register_handle(0x2E, &acceptor::handle_group);               // �׷� �ڵ鷯
    this->register_handle(0x18, &acceptor::handle_user_list);           // ���� ����Ʈ �ڵ鷯
    this->register_handle(0x0E, &acceptor::handle_chat);                // ���� ä�� �ڵ鷯
    this->register_handle(0x3B, &acceptor::handle_board);               // �Խ��� ���� ����Ʈ �ڵ鷯
	this->register_handle(0x30, &acceptor::handle_swap);                // ���� ���� ����
    this->register_handle(0x3A, &acceptor::handle_dialog);              // ���̾�α�
    this->register_handle(0x39, &acceptor::handle_dialog);              // ���̾�α�
    this->register_handle(0x17, &acceptor::handle_throw_item);          // ������ ������ �ڵ鷯
    this->register_handle(0x0F, &acceptor::handle_spell);               // ���� �ڵ鷯
    this->register_handle(0x20, &acceptor::handle_door);               // ���� �ڵ鷯

    this->register_timer(100, &acceptor::handle_mob_action);            // �� �ൿ Ÿ�̸�
    this->register_timer(1000, &acceptor::handle_mob_respawn);          // �� ���� Ÿ�̸�
    this->register_timer(1000, &acceptor::handle_buff_timer);          // �� ���� Ÿ�̸�
}

acceptor::~acceptor()
{ 
    
}

bool acceptor::handle_connected(fb::game::session& session)
{
    auto& maps = db::maps();
    session.map(maps[315], point16_t(6, 8));
    session.name("ä����");
    session.look(0x61);
    session.color(0x0A);
    session.money(150);
    session.sex(fb::game::sex::MAN);
    session.legends_add(0x4A, 0x10, "������ ����");
    session.legends_add(0x4A, 0x10, "������ ����");
    session.base_hp(0xFFFFFFFF);
    session.hp(0xFFFFFFFF);
    session.title("������ Ÿ��Ʋ");

    auto& items = db::items();
    session.items.add(db::name2item("����Į")->make<item>());
    session.items.add(db::name2item("��ȭ�ǹ���")->make<item>());
    session.items.add(db::name2item("������")->make<item>());
    session.items.add(db::name2item("�����ǵη縶��2")->make<item>());
    session.items.add(db::name2item("���ڱ���")->make<item>());
    session.items.add(db::name2item("���丮")->make<item>());
    session.items.add(db::name2item("������")->make<item>());
    

    // ������ ���·� ���� (������ �� ���� �� �ִ� ������� �����ص־� ��)
    session.items.weapon(db::name2item("��÷���")->make<weapon>()); // �ʽ����� ��
    session.items.helmet(db::name2item("����Ȳ������")->make<helmet>());
    session.items.ring(db::name2item("����Ȳ�ݹ���")->make<ring>());
    session.items.ring(db::name2item("����Ȳ�ݹ���")->make<ring>());
    session.items.auxiliary(db::name2item("�����Ǹ����")->make<auxiliary>());
    session.items.auxiliary(db::name2item("�ص��ǱͰ���")->make<auxiliary>());

    auto& spells = db::spells();
    for(auto pair : spells)
    {
        if(session.spells.add(pair.second) == -1)
            break;
    }

    return true;
}

bool acceptor::handle_disconnected(fb::game::session& session)
{
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

fb::game::session* fb::game::acceptor::handle_allocate_session(SOCKET fd)
{
    auto session = new fb::game::session(fd);
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

    case acceptor::scope::MAP:
    {
        const auto& sessions = this->sessions();
        for(const auto session : sessions)
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

void fb::game::acceptor::send_stream(const fb::ostream& stream, bool encrypt)
{
    for(const auto session : this->sessions())
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

        // �� ü�� ��� ü�°����� ǥ��
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
    // �� ü���� �� ������� ���δ�.
    this->send_stream(mob, mob.make_die_stream(), scope::PIVOT, true);
    mob.dead_time(::GetTickCount64());

    // ��� ������ ������
    std::vector<object*> dropped_items;
    for(auto candidate : mob.items())
    {
        if(std::rand() % 100 > candidate.percentage)
            continue;

        auto            item = static_cast<fb::game::item*>(candidate.item->make());
        item->map(mob.map(), mob.position());

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
    // ���� 5�鼭 ���� �Ȱ������� ���� ������� �Ѵ�.
    if(session.level() >= 5 && session.cls() == 0)
        throw session::require_class_exception();

    // ���� ������ ���� ���� ����ġ
    auto                    next_exp = session.max_level() ? 0xFFFFFFFF : db::required_exp(session.cls(), session.level()+1);
    auto                    prev_exp = session.max_level() ? 0x00000000 : db::required_exp(session.cls(), session.level());
    auto                    exp_range = next_exp - prev_exp;

    // 3.3% ������ ����ġ
    if(limit && session.max_level() == false)
        exp = std::min(uint32_t(exp_range / 100.0f*3.3f + 1), exp);


    // ����ġ ���
    session.experience_add(exp);
    this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);

    // ����ġ ��� �޽���
    auto                    exp_now = session.experience();
    auto                    percentage = std::min(100.0f, ((exp_now - prev_exp) / float(exp_range)) * 100.0f);
    std::stringstream       sstream;
    sstream << "����ġ�� " << exp << '(' << int(percentage) << "%) �ö����ϴ�.";
    this->send_stream(session, message::make_stream(sstream.str(), message::type::STATE), scope::SELF);

    // ������ Ȯ��
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


    // ��ƽ�ũ��Ʈ�� handle_click �Լ��� ���ϰ� ����
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

    auto&                   socket = static_cast<crtsocket&>(session);
    socket.crt(enc_type, enc_key);

    fb::ostream             ostream;
    ostream.write_u8(0x1E)
           .write_u8(0x06)
           .write_u8(0x00);
    this->send_stream(session, ostream, scope::SELF);
    
    this->send_stream(session, this->make_time_stream(), scope::SELF);

    this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);

    this->send_stream(session, message::make_stream("0�ð� 1�и��� �ٶ�����", message::type::STATE), scope::SELF);

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

    // ���� ��ġ��� �����Ѵ�.
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


        game::mob*          mob = nullptr;
        for(auto front : session.forward_objects(object::types::MOB))
        {
            if(front->alive() == false)
                continue;

            mob = static_cast<game::mob*>(front);
            break;
        }

        if(mob == nullptr)
            return true;

#if !defined DEBUG && !defined _DEBUG
        if(std::rand() % 3 == 0)
            return true;
#endif

        auto                critical = false;
        auto                random_damage = session.random_damage(*mob, critical);
        this->handle_damage(session, *mob, random_damage);

        if(weapon != nullptr)
            this->send_stream(session, session.make_sound_stream(sound::type::DAMAGE), scope::PIVOT);
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
            bool            item_moved = false;
            if(below->attr() & fb::game::item::attrs::ITEM_ATTR_CASH)
            {
                auto        cash = static_cast<fb::game::cash*>(below);
                uint32_t    remain = session.money_add(cash->chunk());
                cash->chunk(remain); // �԰� ���� ������ ����

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
                sstream << "w:����  :";
                break;

            case fb::game::equipment::slot::ARMOR_SLOT:
                sstream << "a:����  :";
                break;

            case fb::game::equipment::slot::SHIELD_SLOT:
                sstream << "s:����  :";
                break;

            case fb::game::equipment::slot::HELMET_SLOT:
                sstream << "h:�Ӹ�  :";
                break;

            case fb::game::equipment::slot::LEFT_HAND_SLOT:
                sstream << "l:�޼�  :";
                break;

            case fb::game::equipment::slot::RIGHT_HAND_SLOT:
                sstream << "r:������  :";
                break;

            case fb::game::equipment::slot::LEFT_AUX_SLOT:
                sstream << "[:����1  :";
                break;

            case fb::game::equipment::slot::RIGHT_AUX_SLOT:
                sstream << "]:����2  :";
                break;
            }
            sstream << item->name();
            this->send_stream(session, message::make_stream(sstream.str(), message::type::STATE), scope::SELF);

            sstream.str(std::string());
            sstream << "���� ����  " << session.defensive_physical() <<"  " << session.regenerative() << " S  " << session.defensive_magical();
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


        // ���� ������ ��Ʈ�� �ʿ�
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

        cash->map(map);
        cash->position(session.position());
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
        static auto         horse_core = db::name2mob("��");
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
                horse->map(session.map(), session.position_forward());
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
        sstream << "�ӼӸ����  ";
        break;

    case options::GROUP:
        sstream << "�׷��㰡    ";
        break;

    case options::ROAR:
        sstream << "��ġ����  ";
        break;

    case options::ROAR_WORLDS:
        sstream << "������      ";
        break;

    case options::MAGIC_EFFECT:
        sstream << "��������Ʈ  ";
        break;

    case options::WEATHER_EFFECT:
        sstream << "������ȭ    ";
        break;

    case options::FIXED_MOVE:
        sstream << "�����̵�     ";
        break;

    case options::TRADE:
        sstream << "��ȯ����    ";
        break;

    case options::FAST_MOVE:
        sstream << "�����̵�    ";
        break;

    case options::EFFECT_SOUND:
        sstream << "�Ҹ����    ";
        break;

    case options::PK:
        sstream << "PK��ȣ      ";
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
    auto                    other = map->objects.at(fd);
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

    auto                        partner = this->session(fd);   // ��Ʈ��
    auto&                       ts_mine = session.trade;       // ���� �ŷ��ý���
    auto&                       ts_your = partner->trade;      // ������ �ŷ��ý���

    if(partner == nullptr)
        return true;

    switch(action)
    {
    case 0:
    {
        if(session.id() == partner->id())
        {
            // �ڱ� �ڽŰ� �ŷ��� �Ϸ��� �õ��ϴ� ���
            break;
        }

        if(session.option(options::TRADE) == false)
        {
            // ���� ��ȯ �ź���
            this->send_stream(session, message::make_stream(message::trade::REFUSED_BY_ME, message::type::STATE), scope::SELF);
            break;
        }

        if(partner->option(options::TRADE) == false)
        {
            // ������ ��ȯ �ź���
            std::stringstream sstream;
            sstream << partner->name() << message::trade::REFUSED_BY_PARTNER;
            this->send_stream(session, message::make_stream(sstream.str(), message::type::STATE), scope::SELF);
            break;
        }

        if(ts_mine.trading())
        {
            // ���� �̹� ��ȯ��
            break;
        }

        if(ts_your.trading())
        {
            // ������ �̹� ��ȯ��
            std::stringstream sstream;
            sstream << partner->name() << message::trade::PARTNER_ALREADY_TRADING;
            this->send_stream(session, message::make_stream(sstream.str(), message::type::STATE), scope::SELF);
            break;
        }

        if(session.sight(*partner) == false)
        {
            // ������ �þ߿��� ������ ����
            this->send_stream(session, message::make_stream(message::trade::PARTNER_INVISIBLE, message::type::STATE), scope::SELF);
            break;
        }

        if(session.distance_sqrt(*partner) > 16)
        {
            // ������� �Ÿ��� �ʹ� ��
            std::stringstream sstream;
            sstream << partner->name() << message::trade::PARTNER_TOO_FAR;
            this->send_stream(session, message::make_stream(sstream.str(), message::type::STATE), scope::SELF);
            break;
        }

        // ��ȯ ����
        ts_mine.begin(partner);
        this->send_stream(session, ts_your.make_dialog_stream(), scope::SELF);

        ts_your.begin(&session);
        this->send_stream(*partner, ts_mine.make_dialog_stream(), scope::SELF);
        break;
    }

    case 1: // ������ �ø���
    {
        auto                index = istream.read_u8() - 1;
        auto                item = session.items[index];
        if(item == nullptr)
            return true;

        // ��ȯ �Ұ����� ������ �ŷ� �õ�
        if(item->trade_enabled() == false)
        {
            this->send_stream(session, message::make_stream(message::trade::NOT_ALLOWED_TO_TRADE, message::type::POPUP), scope::SELF);
            break;
        }

        if((item->attr() & fb::game::item::attrs::ITEM_ATTR_BUNDLE) && (item->count() > 1))
        {
            // ���� ������ ������ ���� �ŷ� �õ�
            ts_mine.select(item);
            this->send_stream(session, ts_mine.make_bundle_stream(), scope::SELF);
        }
        else
        {
            // �Ϲ� �������� �ŷ� �õ�

            uint8_t trade_index = ts_mine.add(item); // �ŷ����� ������ ����Ʈ�� �ε���
            if(trade_index == 0xFF)
                return false;

            // ���� �κ��丮���� �ŷ����� ������ ����Ʈ�� ������ �̵�
            session.items.remove(index);
            this->send_stream(session, session.items.make_delete_stream(item::delete_attr::DELETE_NONE, index), scope::PIVOT);

            // ���� ��� �� �ٿ��� �ø� �������� ǥ����
            this->send_stream(session, ts_mine.make_show_stream(true, trade_index), scope::SELF);
            this->send_stream(*partner, ts_mine.make_show_stream(false, trade_index), scope::SELF);
        }
        break;
    }

    case 2: // ������ �������� �ؼ� �ø� ��
    {
        // ������ �ø����� �õ��� ���� ������ ������
        auto                selected = ts_mine.selected();
        if(selected == nullptr)
            return false;

        // �ø� ���� (Ŭ���̾�Ʈ�� �Է��� ��)
        auto                count = istream.read_u16();
        if(selected->count() < count)
        {
            this->send_stream(session, message::make_stream(message::trade::INVALID_COUNT, message::type::POPUP), scope::SELF);
            break;
        }

        auto                index = session.items.to_index(selected->based<item::core>());
        if(selected->count() == count)
        {
            // ��� �� �ø��� ���, �������� ���� �������� �ʰ� �ִ� �״�� �ŷ�����Ʈ�� �Űܹ�����.
            auto            trade_index = ts_mine.add(selected);
            if(index != 0xFF)
            {
                session.items.remove(index);
                this->send_stream(session, session.items.make_delete_stream(item::delete_attr::DELETE_NONE, index), scope::SELF);

                this->send_stream(session, ts_mine.make_show_stream(true, trade_index), scope::SELF);
                this->send_stream(*partner, ts_mine.make_show_stream(false, trade_index), scope::SELF);
            }
        }
        else
        {
            // �Ϻθ� �ø��� ���, ������ �Ϳ��� ������ ��� ���� ����� �������� �ŷ� ����Ʈ�� �ű��.
            selected->reduce(count);
            auto            came_out = selected->clone<fb::game::item>();
            came_out->count(count);

            uint8_t trade_index = ts_mine.add(came_out);

            this->send_stream(session, session.items.make_update_stream(index), scope::SELF);

            this->send_stream(session, ts_mine.make_show_stream(true, trade_index), scope::SELF);
            this->send_stream(*partner, ts_mine.make_show_stream(false, trade_index), scope::SELF);
        }
        break;
    }

    case 3: // ���� �ø� ��
    {
        // Ŭ���̾�Ʈ�� �Է��� ���� ��
        auto                money = istream.read_u32();

        // �Է��� ���� ���� ����ؼ� ���� �ȵȴ�.
        // 100�� �Է��� ��� -1, -10, -100 �̷��� �����
        auto                total = session.money() + ts_mine.money();
        if(money > total)
            money = total;

        session.money(total - money);
        ts_mine.money(money);

        this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
        this->send_stream(session, ts_mine.make_money_stream(true), scope::SELF);
        this->send_stream(*partner, ts_mine.make_money_stream(false), scope::SELF);
        break;
    }

    case 4: // ����� ���
    {
        std::vector<uint8_t> indices;

        // �ŷ�����Ʈ�� �÷ȴ� �����۰� ������ ���󺹱ͽ�Ų��.
        indices = ts_mine.restore();
        this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
        for(auto i : indices)
            this->send_stream(session, session.items.make_update_stream(i), scope::SELF);

        // �޽��� ǥ���ϰ� �ŷ�����
        this->send_stream(session, ts_mine.make_close_stream(message::trade::CANCELLED_BY_ME), scope::SELF);
        ts_mine.end();



        // �ŷ�����Ʈ�� �÷ȴ� �����۰� ������ ���󺹱ͽ�Ų��.
        indices = ts_your.restore();
        this->send_stream(*partner, partner->make_state_stream(state_level::LEVEL_MIN), scope::SELF);
        for(auto i : indices)
            this->send_stream(*partner, partner->items.make_update_stream(i), scope::SELF);

        // �޽��� ǥ���ϰ� �ŷ�����
        this->send_stream(*partner, ts_your.make_close_stream(message::trade::CANCELLED_BY_PARTNER), scope::SELF);
        ts_your.end();
        break;
    }

    case 5:
    {
        this->send_stream(session, ts_mine.make_lock_stream(), scope::SELF);
        ts_mine.lock(true);

        if(ts_your.lock())
        {
            // ������ �̹� ��ȯȮ�� ���� ���
            if(ts_mine.flushable(*partner) == false || ts_your.flushable(session) == false)
            {
                // �� Ȥ�� ��밡 �ŷ�����Ʈ�� �ö�� �������̳� ������ �� ���� �� ���� ��Ȳ�� ��
                // �κ��丮�� ���� ���ų�, �� �̻� ������ ���� �� ���� ���
                this->send_stream(session, ts_mine.make_close_stream(message::trade::FAILED), scope::SELF);
                this->send_stream(*partner, ts_your.make_close_stream(message::trade::FAILED), scope::SELF);
            }
            else
            {
                std::vector<uint8_t> indices;
                
                // ����� �ŷ�����Ʈ ��ǰ���� ���� �ް� ������Ʈ
                ts_mine.flush(*partner, indices);
                for(auto i : indices)
                    this->send_stream(*partner, partner->items.make_update_stream(i), scope::SELF);
                this->send_stream(*partner, partner->make_state_stream(state_level::LEVEL_MIN), scope::SELF);

                // ���� �ŷ�����Ʈ ��ǰ���� ���� �ְ� ������Ʈ
                ts_your.flush(session, indices);
                for(auto i : indices)
                    this->send_stream(session, session.items.make_update_stream(i), scope::SELF);
                this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);


                // �޽��� ǥ���ϰ� �ŷ� ����
                this->send_stream(session, ts_mine.make_close_stream(message::trade::SUCCESS), scope::SELF);
                ts_mine.end();

                this->send_stream(*partner, ts_your.make_close_stream(message::trade::SUCCESS), scope::SELF);
                ts_your.end();
            }
        }
        else
        {
            // ������ ���� ��ȯȮ���� ������ ���� ���
            this->send_stream(*partner, message::make_stream(message::trade::NOTIFY_LOCK_TO_PARTNER, message::type::POPUP), scope::SELF);
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
    auto                        name_size = istream.read_u8();
    
    char*                       buffer = new char[name_size];
    istream.read(buffer, name_size);
    std::string                 name(buffer);
    delete[] buffer;


    // ���⼭ �������ִ� ���� ��ü ������� ������ �� �� �ƴ϶�
    // DB�� �����ؼ� �ϴ� ����� ���� �� ���ٴ� ������ ��±���
    
    return true;
}

bool fb::game::acceptor::handle_user_list(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    auto                        cmd = istream.read_u8();
    auto                        unknown = istream.read_u8();


    fb::ostream                 ostream;
    auto&                       sessions = this->sessions();
    ostream.write_u8(0x36)
        .write_u16(sessions.size())
        .write_u16(sessions.size())
        .write_u8(0x00);

    for(const auto& i : sessions)
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
	auto						interaction = istream.read_u8();

	switch(interaction)
	{
    case dialog::interaction::NORMAL: // �Ϲ� ���̾�α�
	{
		istream.read(nullptr, 0x07); // 7����Ʈ ����
		auto					action = istream.read_u8();

        if(session.dialog_thread == nullptr)
            return true;

		// ��ƽ�ũ��Ʈ ���̾�α� �Լ��� ���ϰ� ����
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

		istream.read(nullptr, 0x07); // 7����Ʈ ����
		auto					action = istream.read_u8();
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

    char                    path[256];
    sprintf(path, "scripts/spell/%s.lua", spell->cast().c_str());

    lua::thread             thread;
    thread.fromfile(path, "handle_spell");


    switch(spell->type())
    {
    case spell::types::INPUT:
    {
        char                message[256];
        auto                size = istream.size() - (sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint8_t));
        istream.read(message, std::min(uint32_t(256), size)); message[size] = 0x00;

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
    auto direction = session.direction();
    if(direction == fb::game::direction::LEFT || direction == fb::game::direction::RIGHT)
        return true;

    auto map = session.map();
    if(map == nullptr)
        return false;

    auto forward = point16_t(session.x(), session.y() + (direction == fb::game::direction::TOP ? -1 : 1));
    auto door = map->find_door(forward);
    if(door == nullptr)
        return true;

    door->toggle();

    auto door_core = door->core();
    this->send_stream(session, map->make_update_stream(door->position.x, door->position.y, door_core.size(), 1), scope::MAP);

    if(door->opened())
        this->send_stream(session, message::make_stream(message::door::OPEN, message::type::STATE), scope::SELF);
    else
        this->send_stream(session, message::make_stream(message::door::CLOSE, message::type::STATE), scope::SELF);

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

    // �����¿�� Ÿ���� �ִ��� �˻��Ѵ�.
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


    // Ÿ�� �������� �̵��� �����ϴٸ� �̵��Ѵ�.
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


    // �̵��� �� �ִ� �������� �ϴ� �̵��Ѵ�.
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
        auto                sessions = map->objects.sessions();
        if(sessions.size() == 0)
            continue;

        const auto&         objects = map->objects;

        for(auto object : objects)
        {
            if(object->type() != object::types::MOB)
                continue;

            auto            mob = static_cast<fb::game::mob*>(object);
            if(now < mob->action_time() + mob->speed())
                continue;

            if(mob->alive() == false)
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
    // ������ ��ü ���� ����
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


    // ȭ�鿡 ���̴� ���� ����
    std::vector<object*> shown_mobs;
    for(auto session : this->sessions())
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
            {
                this->macro_object_unbuff(*object, finish->spell().name());
                object->buffs.remove(finish);
            }
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

    auto                            npc = db::name2npc("����");
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
        auto item = db::name2item("���ڱ���");
        this->send_stream(session, item->make_dialog_stream("�������� ����մϴ�.", true, true), scope::SELF);
        return true;
    }

    if(splitted[0] == "show short list")
    {
        std::vector<std::string> menus = {"��", "��", "��"};
        this->send_stream(session, npc->make_dialog_stream("�������� �����մϴ�.", menus), scope::SELF);
        return true;
    }

    if(splitted[0] == "show long list")
    {
        std::vector<std::string> menus;
        std::string message = "���������� �����Ͻ� ���Ӽ��� ���߰��� ū ���̽ô�.";
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
        this->send_stream(session, npc->make_dialog_stream("�������� �����մϴ�.", menus), scope::SELF);
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
        this->send_stream(session, npc->make_dialog_stream("���������� �κ��丮�� �����帳�ϴ�.", slots), scope::SELF);
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
        this->send_stream(session, npc->make_dialog_stream("�ƽù� �� �Ǵ�", items), scope::SELF);
        return true;
    }

    if(splitted[0] == "show question")
    {
        this->send_stream(session, npc->make_input_dialog_stream("What is your name?"), scope::SELF);
        return true;
    }

	if(splitted[0] == "show extend input")
	{
		this->send_stream(session, npc->make_input_dialog_stream("�ȳ�", "ž", "����", 0xFF, true), scope::SELF);
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

    return false;
}

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
            if(i->is(object::types::MOB) && i->alive() == false)
                continue;

            this->send_stream(object, i->make_show_stream(), scope::SELF);
        }
    }

    if(hides != nullptr)
    {
        for(auto i : *hides)
        {
            if(i->is(object::types::MOB) && i->alive() == false)
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
    // ������ ���̴� ������Ʈ�� ���� ����
    for(auto i : object.showings())
        this->send_stream(object, i->make_hide_stream(), scope::SELF);

    // ������ ���� ���� ������Ʈ�鿡�Լ� ���� ����
    for(auto i : object.showns())
        this->send_stream(*i, object.make_hide_stream(), scope::SELF);

    object.map(&map, position);


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

    // ���� ���� �� �� �ִ� ������Ʈ�� ǥ��
    for(auto i : object.showings())
        this->send_stream(object, i->make_show_stream(), scope::SELF);

    // ���� ���� �� �� �ִ� ������Ʈ�鿡�� ���� ǥ��
    for(auto i : object.showns())
        this->send_stream(*i, object.make_show_stream(), scope::SELF);
}

void fb::game::acceptor::macro_object_unbuff(fb::game::object& object, const std::string& bufname)
{
    auto buff = object.buffs[bufname];
    if(buff == nullptr)
        return;

    lua::thread thread;
    thread.fromfile(buff->spell().uncast(), "handle_uncast");
    thread.pushobject(object);
    thread.pushobject(buff->spell());
    thread.resume(2);

    object.buffs.remove(buff);
    this->send_stream(object, buff->make_clear_stream(), scope::SELF);
}

#endif