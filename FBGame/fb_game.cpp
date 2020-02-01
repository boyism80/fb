#include "fb_game.h"
#include "db.h"

using namespace fb::game;

acceptor::acceptor(uint16_t port) : fb_acceptor<fb::game::session>(port)
{
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

    this->register_timer(100, &acceptor::handle_mob_action);            // �� �ൿ Ÿ�̸�
    this->register_timer(1000, &acceptor::handle_mob_respawn);          // �� ���� Ÿ�̸�
}

acceptor::~acceptor()
{ }

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
    session.item_add(static_cast<fb::game::item*>(items[1015]->make())); // ��ȭ�ǹ���
    session.item_add(static_cast<fb::game::item*>(items[243]->make())); // ������
    session.item_add(static_cast<fb::game::item*>(items[698]->make())); // ����
    session.item_add(static_cast<fb::game::item*>(items[3014]->make())); // ���丮
    session.item_add(static_cast<fb::game::item*>(items[2200]->make())); // ������

    // ������ ���·� ���� (������ �� ���� �� �ִ� ������� �����ص־� ��)
    session.weapon(static_cast<fb::game::weapon*>(items[15]->make())); // �ʽ����� ��
    session.helmet(static_cast<fb::game::helmet*>(items[1340]->make()));
    session.ring(static_cast<fb::game::ring*>(items[1689]->make()));
    session.ring(static_cast<fb::game::ring*>(items[1689]->make()));
    session.auxiliary(static_cast<fb::game::auxiliary*>(items[2135]->make()));
    session.auxiliary(static_cast<fb::game::auxiliary*>(items[2129]->make()));


    auto& spells = db::spells();
    for(auto pair : spells)
    {
        if(session.spell_add(pair.second) == -1)
            break;
    }

    return true;
}

bool acceptor::handle_disconnected(fb::game::session& session)
{
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
    uint8_t                 hours = 25;
    ostream.write_u8(0x20)      // cmd : 0x20
           .write_u8(hours%24)  // hours
           .write_u8(0x00)      // Unknown
           .write_u8(0x00);     // Unknown

    return ostream;
}

fb::ostream fb::game::acceptor::make_dialog_stream(const std::string& message, bool enabled_prev, bool enabled_next)
{
    fb::ostream             ostream;
    uint16_t                look = 0xC012;
    uint8_t                 color = 0x04;
    uint8_t                 icon_type = look < 0xC000 ? 0x01 : 0x02;

    ostream.write_u8(0x30)
        .write_u8(0x00)
        .write_u8(0x00)
        .write_u8(0x00)
        .write_u8(0x00)
        .write_u16(0x01) // id�ε�
        .write_u8(icon_type)
        .write_u8(0x01) // fixed
        .write_u16(look) // icon
        .write_u8(color) // color
        .write_u8(icon_type)
        .write_u16(look) // icon
        .write_u8(color) // color
        .write_u32(0x00000001)
        .write_u8(enabled_prev)
        .write_u8(enabled_next)
        .write(message, true);

    return ostream;
}

void fb::game::acceptor::send_stream(object& object, const fb::ostream& stream, acceptor::scope scope, bool exclude_self, bool encrypt)
{
    switch(scope)
    {
    case acceptor::scope::SELF:
        __super::send_stream(static_cast<fb::game::session&>(object), stream, encrypt);
        break;

    case acceptor::scope::PIVOT:
    {
        const auto sessions = object.looking_sessions();
        if(!exclude_self)
            __super::send_stream(static_cast<fb::game::session&>(object), stream, encrypt);

        for(const auto session : sessions)
            __super::send_stream(*session, stream, encrypt);
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

            __super::send_stream(*session, stream, encrypt);
        }

        break;
    }

    }
}

bool fb::game::acceptor::handle_move_life(fb::game::life* life, fb::game::direction direction)
{
    if(life->alive() == false)
        return false;

    bool                        result = false;

    try
    {
        std::vector<fb::game::session*> shown_sessions, hidden_sessions;

        if(life->move(direction, NULL, NULL, NULL, NULL, NULL, NULL, &shown_sessions, &hidden_sessions) == false)
            throw std::exception();

        for(auto session : shown_sessions)
            this->send_stream(*session, life->make_show_stream(), scope::SELF);

        for(auto session : hidden_sessions)
            this->send_stream(*session, life->make_hide_stream(), scope::SELF);

        for(const auto session : life->map()->sessions())
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

void fb::game::acceptor::handle_attack_mob(fb::game::session& session, fb::game::mob& mob, uint32_t random_damage)
{
    std::stringstream sstream;

    try
    {
        map* map = mob.map();

        // �� ü�� ��� ü�°����� ǥ��
        mob.hp_down(random_damage);
        this->send_stream(mob, mob.make_show_hp_stream(random_damage, true), scope::PIVOT, true);

        

        // �°� ��������� �� �̻� ������ �� ����
        if(mob.alive())
        {
            if(mob.offensive() != mob::offensive_type::NONE)
                mob.target(&session);

            return;
        }

        // �� ü���� �� ������� ���δ�.
        this->send_stream(mob, mob.make_die_stream(), scope::PIVOT, true);
        mob.dead_time(::GetTickCount64());

        // ��� ������ ������
        std::vector<object*> dropped_items;
        for(auto candidate : mob.items())
        {
            if(std::rand() % 100 > candidate.percentage)
                continue;

            item* item = static_cast<fb::game::item*>(candidate.item->make());
            item->map(map, mob.position());

            dropped_items.push_back(item);
        }

        if(dropped_items.size() != 0)
            this->send_stream(mob, object::make_show_stream(dropped_items), scope::PIVOT, true);

        // ���� 5�鼭 ���� �Ȱ������� ���� ������� �Ѵ�.
        if(session.level() >= 5 && session.cls() == 0)
            throw session::require_class_exception();

        // ����ġ�� �ִ� 3.3%�� �����Ͽ� ��´�.
        uint32_t require = session.max_level() ? 0xFFFFFFFF : db::required_exp(session.cls(), session.level()+1) - db::required_exp(session.cls(), session.level());
#if defined DEBUG | defined _DEBUG
        uint32_t limit_exp = require;
#else
        uint32_t limit_exp = max_level ? mob.experience() : std::min(uint32_t(require / 100.0f*3.3f + 1), mob.experience());
#endif
        session.experience_add(limit_exp);

        uint32_t exp = session.experience();
        uint32_t next_exp = db::required_exp(session.cls(), session.level()+1);

        if(exp >= next_exp && session.level_up())
        {
            auto& classes = db::classes();

            session.strength_up(classes[session.cls()]->level_abilities[session.level()]->strength);
            session.intelligence_up(classes[session.cls()]->level_abilities[session.level()]->intelligence);
            session.dexteritry_up(classes[session.cls()]->level_abilities[session.level()]->dexteritry);
            
            session.base_hp_up(classes[session.cls()]->level_abilities[session.level()]->base_hp + std::rand() % 10);
            session.base_mp_up(classes[session.cls()]->level_abilities[session.level()]->base_mp + std::rand() % 10);

            session.hp(session.base_hp());
            session.mp(session.base_mp());

            sstream << message::level::UP;
            this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
            this->send_stream(session, session.make_effet_stream(0x02), scope::PIVOT);
        }
        else
        {
            float percentage = 0.0f;
            uint32_t prev_exp = db::required_exp(session.cls(), session.level());
            if(exp > prev_exp)
                percentage = ((exp - prev_exp) / float(next_exp - prev_exp)) * 100;

            sstream << "����ġ�� " << limit_exp << '(' << int(percentage) << "%) �ö����ϴ�.";

            this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
        }
    }
    catch(std::exception& e)
    {
        sstream << e.what();
    }

    this->send_stream(session, message::make_stream(sstream.str(), message::type::MESSAGE_STATE), scope::SELF);
}

bool acceptor::handle_login(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    uint8_t                 enc_key[0x09] = {0,};

    uint8_t cmd = istream.read_u8();
    uint8_t enc_type = istream.read_u8();
    uint8_t key_size = istream.read_u8();
    istream.read(enc_key, key_size);
    session.crt(enc_type, enc_key);

    fb::ostream             ostream;
    ostream.write_u8(0x1E)
           .write_u8(0x06)
           .write_u8(0x00);
    this->send_stream(session, ostream, scope::SELF);
    
    this->send_stream(session, this->make_time_stream(), scope::SELF);

    this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);

    this->send_stream(session, message::make_stream("0�ð� 1�и��� �ٶ�����", message::type::MESSAGE_STATE), scope::SELF);

    this->send_stream(session, session.make_id_stream(), scope::SELF);

    this->send_stream(session, session.map()->make_config_stream(), scope::SELF);

    this->send_stream(session, session.map()->make_bgm_stream(), scope::SELF);

    this->send_stream(session, session.make_position_stream(), scope::SELF);

    this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);

    this->send_stream(session, session.make_show_objects_stream(), scope::SELF);

    this->send_stream(session, session.make_visual_stream(false), scope::PIVOT);
    this->send_stream(session, session.make_direction_stream(), scope::SELF);
    this->send_stream(session, session.make_option_stream(), scope::SELF);

    // Initialize spell list
    for(int i = 0; i < spell::MAX_SLOT; i++)
    {
        auto spell = session.spell(i);
        if(spell == nullptr)
            continue;

        this->send_stream(session, spell->make_show_stream(i+1), scope::SELF);
    }

    // Initialize mobs
    for(auto i : session.looking_sessions())
    {
        if(i == &session)
            continue;

        this->send_stream(session, i->make_visual_stream(false), scope::SELF);
    }

    for(int i = 0; i < fb::game::item::MAX_SLOT; i++)
        this->send_stream(session, session.make_update_item_slot_stream(i), scope::SELF);

    this->send_stream(session, this->make_dialog_stream("�ȳ��ϼ���?", false, false), scope::SELF);

    return true;
}

bool fb::game::acceptor::handle_direction(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    uint8_t                 cmd = istream.read_u8();
    fb::game::direction     direction = fb::game::direction(istream.read_u8());

    if(session.direction(direction) == false)
        return false;

    this->send_stream(session, session.make_direction_stream(), scope::PIVOT, true);
    return true;
}

bool fb::game::acceptor::handle_move(fb::game::session& session)
{
    auto&                   istream = session.in_stream();

    fb::game::map*          map = session.map();
    if(map == NULL)
        return false;

    uint8_t                 cmd = istream.read_u8();
    fb::game::direction     direction = fb::game::direction(istream.read_u8());
    uint8_t                 sequence = istream.read_u8();
    uint16_t                x = istream.read_u16();
    uint16_t                y = istream.read_u16();

    session.direction(direction);

    point16_t               before(x, y);
    if(session.position() != before)
        this->send_stream(session, session.make_position_stream(), scope::SELF);

    std::vector<object*>            show_objects, hide_objects;
    std::vector<fb::game::session*> show_sessions, hide_sessions, shown_sessions, hidden_sessions;
    if(session.move_forward(&show_objects, &hide_objects, &show_sessions, &hide_sessions, NULL, NULL, &shown_sessions, &hidden_sessions))
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
    if(warp != NULL)
    {
        this->handle_session_warp(session, warp);
        return true;
    }
    
    // ������Ʈ ����
    this->send_stream(session, object::make_show_stream(show_objects), scope::SELF);
    for(auto i : hide_objects)
        this->send_stream(session, i->make_hide_stream(), scope::SELF);

    // ������ ���ǿ��� ���� ���̴� ���� ����
    for(auto i : show_sessions)
        this->send_stream(session, i->make_visual_stream(false), scope::SELF);

    // ������ ���ǿ��� ����� ���� ����
    for(auto i : hide_sessions)
        this->send_stream(session, i->make_hide_stream(), scope::SELF);

    for(auto i : shown_sessions)
        this->send_stream(*i, session.make_visual_stream(false), scope::SELF);

    for(auto i : hidden_sessions)
        this->send_stream(*i, session.make_hide_stream(), scope::SELF);

    return true;
}

bool fb::game::acceptor::handle_update_move(fb::game::session& session)
{
    if(this->handle_move(session) == false)
        return false;

    auto&                   istream = session.in_stream();
    uint16_t                begin_x = istream.read_u16();
    uint16_t                begin_y = istream.read_u16();
    uint8_t                 width = istream.read_u8();
    uint8_t                 height = istream.read_u8();
    uint16_t                crc = istream.read_u16();
    this->send_stream(session, session.map()->make_update_stream(begin_x, begin_y, width, height, crc), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_attack(fb::game::session& session)
{
    try
    {
        session.state_assert(state::RIDING | state::GHOST);

        this->send_stream(session, session.make_action_stream(action::ATTACK, duration::DURATION_ATTACK), scope::PIVOT);
        auto*               weapon = session.weapon();
        if(weapon != NULL)
        {
            uint16_t        sound = weapon->sound();
            if(sound != 0)
                this->send_stream(session, session.make_sound_stream(fb::game::action_sounds(sound)), scope::PIVOT);
        }
        else
        {
            // �̰� ���� �������� Ȯ���ϵ��� ����
            this->send_stream(session, session.make_sound_stream(fb::game::action_sounds(0x015D)), scope::PIVOT);
        }

        auto*               front = session.forward_object(object::types::MOB);
        if(front == NULL)
            return true;

        auto*               front_mob = static_cast<fb::game::mob*>(front);

#if !defined DEBUG && !defined _DEBUG
        if(std::rand() % 3 == 0)
            return true;
#endif

        bool                critical = false;
        uint32_t            random_damage = session.random_damage(*front_mob, critical);

        this->handle_attack_mob(session, *front_mob, random_damage);
    }
    catch(std::exception& e)
    {
        this->send_stream(session, message::make_stream(e.what(), message::type::MESSAGE_STATE), scope::SELF);
    }

    return true;
}

bool fb::game::acceptor::handle_pickup(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    uint8_t                 cmd = istream.read_u8();
    bool                    boost = bool(istream.read_u8());

    try
    {
        auto                map = session.map();
        if(map == NULL)
            return false;

        session.state_assert(state::GHOST | state::RIDING);

        
        // Do action : pick up
        this->send_stream(session, session.make_action_stream(action::PICKUP, duration::DURATION_PICKUP), scope::PIVOT);


        std::string         message;
        const auto&         objects = map->objects();
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
                    this->send_stream(session, message::make_stream(message::money::FULL, message::type::MESSAGE_STATE), scope::SELF);

                this->send_stream(*cash, cash->make_show_stream(), scope::PIVOT, true);
                this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
            }
            else
            {
                uint8_t         index = session.item_add(below);
                if(index == -1)
                    break;

                this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);

                item_moved = (session.item(index) == below);
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
            map->object_delete(gain);
            if(gain->empty())
                delete gain;
        }
    }
    catch(std::exception& e)
    {
        this->send_stream(session, message::make_stream(e.what(), message::type::MESSAGE_STATE), scope::SELF);
    }

    return true;
}

bool fb::game::acceptor::handle_emotion(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    uint8_t                 cmd = istream.read_u8();
    uint8_t                 emotion = istream.read_u8();

    this->send_stream(session, session.make_action_stream(action(action::EMOTION + emotion), duration::DURATION_EMOTION), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_update_map(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    uint8_t                 cmd = istream.read_u8();
    uint16_t                x = istream.read_u16();
    uint16_t                y = istream.read_u16();
    uint8_t                 width = istream.read_u8();
    uint8_t                 height = istream.read_u8();
    uint16_t                crc = istream.read_u16();

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
    uint8_t                 cmd = istream.read_u8();
    uint8_t                 index = istream.read_u8() - 1;

    try
    {
        session.state_assert(state::RIDING | state::GHOST);

        uint8_t                 updated_index = 0;
        auto                    slot(equipment::eq_slots::UNKNOWN_SLOT);
        auto                    item = session.item_active(index, &updated_index, slot);

        if(item->attr() & fb::game::item::attrs::ITEM_ATTR_EQUIPMENT)
        {
            this->send_stream(session, session.make_delete_item_slot_stream(fb::game::item::delete_attr::DELETE_NONE, index), scope::SELF);
            if(updated_index != 0xFF)
                this->send_stream(session, session.make_update_item_slot_stream(updated_index), scope::SELF);

            this->send_stream(session, session.make_state_stream(fb::game::state_level::LEVEL_MAX), scope::SELF);
            this->send_stream(session, session.make_update_equipment_stream(slot), scope::SELF);
            this->send_stream(session, session.make_visual_stream(true), scope::PIVOT);
            this->send_stream(session, session.make_sound_stream(action_sounds::SOUND_EQUIPMENT_ON), scope::PIVOT);

            std::stringstream sstream;
            switch(slot)
            {
            case fb::game::equipment::eq_slots::WEAPON_SLOT:
                sstream << "w:����  :";
                break;

            case fb::game::equipment::eq_slots::ARMOR_SLOT:
                sstream << "a:����  :";
                break;

            case fb::game::equipment::eq_slots::SHIELD_SLOT:
                sstream << "s:����  :";
                break;

            case fb::game::equipment::eq_slots::HELMET_SLOT:
                sstream << "h:�Ӹ�  :";
                break;

            case fb::game::equipment::eq_slots::LEFT_HAND_SLOT:
                sstream << "l:�޼�  :";
                break;

            case fb::game::equipment::eq_slots::RIGHT_HAND_SLOT:
                sstream << "r:������  :";
                break;

            case fb::game::equipment::eq_slots::LEFT_AUX_SLOT:
                sstream << "[:����1  :";
                break;

            case fb::game::equipment::eq_slots::RIGHT_AUX_SLOT:
                sstream << "]:����2  :";
                break;
            }
            sstream << item->name();
            this->send_stream(session, message::make_stream(sstream.str(), message::type::MESSAGE_STATE), scope::SELF);

            sstream.str(std::string());
            sstream << "���� ����  " << session.defensive_physical() <<"  " << session.regenerative() << " S  " << session.defensive_magical();
            this->send_stream(session, message::make_stream(sstream.str(), message::type::MESSAGE_STATE), scope::SELF);

            return true;
        }


        if(item->attr() & fb::game::item::attrs::ITEM_ATTR_CONSUME)
        {   
            this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
            this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);
            this->send_stream(session, session.make_action_stream(action::EAT, duration::DURATION_EAT), scope::SELF);
            this->send_stream(session, session.make_sound_stream(action_sounds::SOUND_EAT), scope::SELF);

            if(item->empty())
            {
                this->send_stream(session, session.make_delete_item_slot_stream(fb::game::item::delete_attr::DELETE_EAT, index), scope::SELF);
                delete item;
            }
        }
    }
    catch(std::exception& e)
    {
        this->send_stream(session, message::make_stream(e.what(), message::type::MESSAGE_STATE), scope::PIVOT);
    }

    return true;
}

bool fb::game::acceptor::handle_inactive_item(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    uint8_t                 cmd = istream.read_u8();
    auto                    slot(equipment::eq_slots(istream.read_u8()));

    try
    {
        session.state_assert(state::RIDING | state::GHOST);

        uint8_t             item_index = session.equipment_off(slot);
        if(item_index == -1)
            return true;


        // ���� ������ ��Ʈ�� �ʿ�
        this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
        this->send_stream(session, session.make_equipment_off_stream(slot), scope::SELF);
        this->send_stream(session, session.make_visual_stream(true), scope::PIVOT);
        this->send_stream(session, session.make_sound_stream(action_sounds::SOUND_EQUIPMENT_OFF), scope::PIVOT);
        this->send_stream(session, session.make_update_item_slot_stream(item_index), scope::SELF);
    }
    catch(std::exception& e)
    {
        this->send_stream(session, message::make_stream(e.what(), message::type::MESSAGE_STATE), scope::PIVOT);
    }

    return true;
}

bool fb::game::acceptor::handle_drop_item(fb::game::session& session)
{
    try
    {
        session.state_assert(state::RIDING | state::GHOST);

        auto&               istream = session.in_stream();
        uint8_t             cmd = istream.read_u8();
        uint8_t             index = istream.read_u8() - 1;
        bool                drop_all = bool(istream.read_u8());

        auto                item = session.item(index);
        if(item == NULL)
            return true;

        auto                dropped = item->handle_drop(session, drop_all ? item->count() : 1);

        if(item == dropped)
        {
            this->send_stream(session, session.make_delete_item_slot_stream(fb::game::item::delete_attr::DELETE_DROP, index), scope::SELF);
            session.item_remove(index);
        }

        auto                map = session.map();
        this->send_stream(session, session.make_action_stream(action::PICKUP, duration::DURATION_PICKUP), scope::PIVOT);
        this->send_stream(session, dropped->make_show_stream(), scope::PIVOT);
        this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);
    }
    catch(std::exception& e)
    { 
        this->send_stream(session, message::make_stream(e.what(), message::type::MESSAGE_STATE), scope::SELF);
    }

    return true;
}

bool fb::game::acceptor::handle_drop_cash(fb::game::session& session)
{
    try
    {
        session.state_assert(state::RIDING | state::GHOST);

        auto                map = session.map();
        if(map == NULL)
            return false;

        auto&               istream = session.in_stream();
        uint8_t             cmd = istream.read_u8();
        uint32_t            chunk = std::min(session.money(), istream.read_u32());
        cash*               cash = new fb::game::cash(chunk);

        session.money_reduce(chunk);

        cash->map(map);
        cash->position(session.position());
        this->send_stream(session, session.make_action_stream(action::PICKUP, duration::DURATION_PICKUP), scope::PIVOT);
        this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
        this->send_stream(*cash, cash->make_show_stream(), scope::PIVOT, true);
        this->send_stream(session, message::make_stream(message::money::DROP, message::type::MESSAGE_STATE), scope::SELF);

        return true;
    }
    catch(std::exception& e)
    {
        this->send_stream(session, message::make_stream(e.what(), message::type::MESSAGE_STATE), scope::SELF);
    }
}

bool fb::game::acceptor::handle_front_info(fb::game::session& session)
{
    fb::game::map* map = session.map();
    if(map == NULL)
        return false;

    std::vector<fb::game::session*> sessions = session.forward_sessions();
    for(auto i : sessions)
    {
        this->send_stream(session, message::make_stream(i->name(), message::type::MESSAGE_STATE), scope::SELF);
    }

    std::vector<object*> objects = session.forward_objects(object::types::UNKNOWN);
    for(auto i : objects)
    {
        if(i->type() == fb::game::object::types::ITEM)
        {
            auto item = static_cast<fb::game::item*>(i);
            this->send_stream(session, message::make_stream(item->name_styled(), message::type::MESSAGE_STATE), scope::SELF);
        }
        else
        {
            this->send_stream(session, message::make_stream(i->name(), message::type::MESSAGE_STATE), scope::SELF);
        }
    }

    return true;
}

bool fb::game::acceptor::handle_self_info(fb::game::session& session)
{
    this->send_stream(session, session.make_internal_info_stream(), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_option_changed(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    uint8_t                 cmd = istream.read_u8();
    options                 option = options(istream.read_u8());


    bool                    enabled = session.option_toggle(option);
    std::stringstream       sstream;
    switch(option)
    {
    case options::RIDE:
    {
        static auto         horse_core = db::name2mob("��");
        if(horse_core == NULL)
            break;

        try
        {
            session.state_assert(state::GHOST | state::DISGUISE);

            object*         forward = NULL;
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
                if(forward == NULL || forward->based() != horse_core)
                    throw session::no_conveyance_exception();

                forward->map()->object_delete(forward);
                this->send_stream(*forward, forward->make_hide_stream(), scope::PIVOT, true);
                message = game::message::ride::OFF;
                session.state(fb::game::state::RIDING);

                forward->map()->object_delete(forward);
                delete forward;
            }

            this->send_stream(session, session.make_visual_stream(true), scope::PIVOT);
            this->send_stream(session, message::make_stream(message, message::type::MESSAGE_STATE), scope::SELF);
        }
        catch(std::exception& e)
        {
            this->send_stream(session, message::make_stream(e.what(), message::type::MESSAGE_STATE), scope::SELF);
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
    this->send_stream(session, message::make_stream(sstream.str(), message::type::MESSAGE_STATE), scope::SELF);
    this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
    this->send_stream(session, session.make_option_stream(), scope::SELF);

    return true;
}

bool fb::game::acceptor::handle_click_object(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    uint8_t                 cmd = istream.read_u8();
    uint8_t                 unknown = istream.read_u8();
    uint32_t                fd = istream.read_u32();

    if(fd == 0xFFFFFFFF) // Press F1
    {
        return true;
    }

    if(fd == 0xFFFFFFFE) // Preff F2
    {
        return true;
    }

    auto                    other = this->session(fd);
    if(other != NULL) // character
    {
        this->send_stream(session, other->make_external_info_stream(), scope::SELF);
        return true;
    }

    auto                    object = session.map()->object(fd);
    if(object != NULL) // object
    {
        this->send_stream(session, message::make_stream(object->name(), message::type::MESSAGE_STATE), scope::SELF);
        return true;
    }

    return true;
}

bool fb::game::acceptor::handle_item_info(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    uint8_t						cmd = istream.read_u8();
    uint16_t					position = istream.read_u16();
    uint8_t						unknown1 = istream.read_u8();
    uint8_t						unknown2 = istream.read_u8();
    uint8_t						unknown3 = istream.read_u8();
    uint8_t						slot = istream.read_u8() - 1;

    auto						item = session.item(slot);
    if(item == NULL)
        return false;

    this->send_stream(session, item->make_tip_stream(position), scope::SELF);

    return true;
}

bool fb::game::acceptor::handle_itemmix(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    uint8_t						cmd = istream.read_u8();
    uint8_t						count = istream.read_u8();
    if(count > item::MAX_SLOT - 1)
        return false;

    std::vector<item*>			items;
    for(int i = 0; i < count; i++)
    {
        uint8_t					index = istream.read_u8() - 1;
        auto					item = session.item(index);
        if(item == NULL)
            return true;

        items.push_back(item);
    }
    
    try
    {
        auto					itemmix = db::find_itemmix(items);
        if(itemmix == NULL)
            throw itemmix::no_match_exception();

        uint8_t					free_size = session.inventory_free_size();
        if(int(itemmix->success.size()) - int(itemmix->require.size()) > free_size)
            throw item::full_inven_exception();


        for(auto require : itemmix->require)
        {
            uint8_t				index = session.item2index(require.item);
            if(index == 0xFF)
                return true;

            auto				item = session.item(index);
            item->reduce(require.count);


            if(item->empty())
            {
                this->send_stream(session, session.make_delete_item_slot_stream(item::delete_attr::DELETE_NONE, index), scope::SELF);
                session.item_remove(index);
                delete item;
            }
            else
            {
                this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);
            }
        }

        bool                success = (std::rand() % 100) < itemmix->percentage;
        std::string         message;
        if(success)
        {
            for(auto success : itemmix->success)
            {
                auto		item = static_cast<fb::game::item*>(success.item->make());
                item->count(success.count);
                
				uint8_t		index = session.item_add(item);
                this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);
            }

            message = game::message::mix::SUCCESS;
        }
        else
        {
            for(auto failed : itemmix->failed)
            {
                auto		item = static_cast<fb::game::item*>(failed.item->make());
                item->count(failed.count);
                
				uint8_t		index = session.item_add(item);
                this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);
            }

            message = game::message::mix::FAILED;
        }

        this->send_stream(session, message::make_stream(message, message::type::MESSAGE_STATE), scope::SELF);
    }
    catch(std::exception& e)
    { 
        this->send_stream(session, message::make_stream(e.what(), message::type::MESSAGE_STATE), scope::SELF);
        return true;
    }

    

    return true;
}

bool fb::game::acceptor::handle_trade(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    uint8_t                     cmd = istream.read_u8();
    uint8_t                     action = istream.read_u8();
    uint32_t                    fd = istream.read_u32();

    auto                        partner = this->session(fd);            // ��Ʈ��
    auto&                       ts_mine = session.trade();       // ���� �ŷ��ý���
    auto&                       ts_your = partner->trade();      // ������ �ŷ��ý���

    if(partner == NULL)
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
            this->send_stream(session, message::make_stream(message::trade::REFUSED_BY_ME, message::type::MESSAGE_STATE), scope::SELF);
            break;
        }

        if(partner->option(options::TRADE) == false)
        {
            // ������ ��ȯ �ź���
            std::stringstream sstream;
            sstream << partner->name() << message::trade::REFUSED_BY_PARTNER;
            this->send_stream(session, message::make_stream(sstream.str(), message::type::MESSAGE_STATE), scope::SELF);
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
            this->send_stream(session, message::make_stream(sstream.str(), message::type::MESSAGE_STATE), scope::SELF);
            break;
        }

        if(session.sight(*partner) == false)
        {
            // ������ �þ߿��� ������ ����
            this->send_stream(session, message::make_stream(message::trade::PARTNER_INVISIBLE, message::type::MESSAGE_STATE), scope::SELF);
            break;
        }

        if(session.distance_sqrt(*partner) > 16)
        {
            // ������� �Ÿ��� �ʹ� ��
            std::stringstream sstream;
            sstream << partner->name() << message::trade::PARTNER_TOO_FAR;
            this->send_stream(session, message::make_stream(sstream.str(), message::type::MESSAGE_STATE), scope::SELF);
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
        uint8_t             index = istream.read_u8() - 1;
        auto                item = session.item(index);
        if(item == NULL)
            return true;

        // ��ȯ �Ұ����� ������ �ŷ� �õ�
        if(item->trade_enabled() == false)
        {
            this->send_stream(session, message::make_stream(message::trade::NOT_ALLOWED_TO_TRADE, message::type::MESSAGE_TRADE), scope::SELF);
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
            session.item_remove(index);
            this->send_stream(session, session.make_delete_item_slot_stream(item::delete_attr::DELETE_NONE, index), scope::PIVOT);

            // ���� ��� �� �ٿ��� �ø� �������� ǥ����
            this->send_stream(session, ts_mine.make_show_stream(true, trade_index), scope::SELF);
            this->send_stream(*partner, ts_mine.make_show_stream(false, trade_index), scope::SELF);
        }
    }
        break;

    case 2: // ������ �������� �ؼ� �ø� ��
    {
        // ������ �ø����� �õ��� ���� ������ ������
        auto                selected = ts_mine.selected();
        if(selected == NULL)
            return false;

        // �ø� ���� (Ŭ���̾�Ʈ�� �Է��� ��)
        uint16_t            count = istream.read_u16();
        if(selected->count() < count)
        {
            this->send_stream(session, message::make_stream(message::trade::INVALID_COUNT, message::type::MESSAGE_TRADE), scope::SELF);
            break;
        }

        uint8_t             index = session.item2index(selected->based<item::core>());
        if(selected->count() == count)
        {
            // ��� �� �ø��� ���, �������� ���� �������� �ʰ� �ִ� �״�� �ŷ�����Ʈ�� �Űܹ�����.
            uint8_t         trade_index = ts_mine.add(selected);
            if(index != 0xFF)
            {
                session.item_remove(index);
                this->send_stream(session, session.make_delete_item_slot_stream(item::delete_attr::DELETE_NONE, index), scope::SELF);

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

            this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);

            this->send_stream(session, ts_mine.make_show_stream(true, trade_index), scope::SELF);
            this->send_stream(*partner, ts_mine.make_show_stream(false, trade_index), scope::SELF);
        }
        break;
    }

    case 3: // ���� �ø� ��
    {
        // Ŭ���̾�Ʈ�� �Է��� ���� ��
        uint32_t            money = istream.read_u32();

        // �Է��� ���� ���� ����ؼ� ���� �ȵȴ�.
        // 100�� �Է��� ��� -1, -10, -100 �̷��� �����
        uint32_t            total = session.money() + ts_mine.money();
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
            this->send_stream(session, session.make_update_item_slot_stream(i), scope::SELF);

        // �޽��� ǥ���ϰ� �ŷ�����
        this->send_stream(session, ts_mine.make_close_stream(message::trade::CANCELLED_BY_ME), scope::SELF);
        ts_mine.end();



        // �ŷ�����Ʈ�� �÷ȴ� �����۰� ������ ���󺹱ͽ�Ų��.
        indices = ts_your.restore();
        this->send_stream(*partner, partner->make_state_stream(state_level::LEVEL_MIN), scope::SELF);
        for(auto i : indices)
            this->send_stream(*partner, partner->make_update_item_slot_stream(i), scope::SELF);

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
                    this->send_stream(*partner, partner->make_update_item_slot_stream(i), scope::SELF);
                this->send_stream(*partner, partner->make_state_stream(state_level::LEVEL_MIN), scope::SELF);

                // ���� �ŷ�����Ʈ ��ǰ���� ���� �ְ� ������Ʈ
                ts_your.flush(session, indices);
                for(auto i : indices)
                    this->send_stream(session, session.make_update_item_slot_stream(i), scope::SELF);
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
            this->send_stream(*partner, message::make_stream(message::trade::NOTIFY_LOCK_TO_PARTNER, message::type::MESSAGE_TRADE), scope::SELF);
        }
        break;
    }
    }

    return true;
}

bool fb::game::acceptor::handle_group(fb::game::session& session)
{
    auto&                       istream = session.in_stream();

    uint8_t                     cmd = istream.read_u8();
    uint8_t                     name_size = istream.read_u8();
    
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
    uint8_t                     cmd = istream.read_u8();
    uint8_t                     unknown = istream.read_u8();


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
    uint8_t                     cmd = istream.read_u8();
    
    uint8_t                     shout = istream.read_u8();
    uint8_t                     length = istream.read_u8();
    static char                 message[0xFF];
    istream.read(message, length);
    message[length] = 0;

    this->send_stream(session, session.make_chat_stream(message, shout), shout ? scope::MAP : scope::PIVOT);

    return true;
}

bool fb::game::acceptor::handle_board(fb::game::session& session)
{
    auto&                       board = db::board();
    auto&                       istream = session.in_stream();
    uint8_t                     cmd = istream.read_u8();
    uint8_t                     action = istream.read_u8();

    switch(action)
    {
    case 0x01: // section list
    {
        this->send_stream(session, board.make_sections_stream(), scope::SELF);
        break;
    }

    case 0x02: // article list
    {
        uint16_t                section_id = istream.read_u16();
        uint16_t                offset = istream.read_u16();

        this->send_stream(session, board.make_articles_stream(section_id, offset), scope::SELF);
        break;
    }

    case 0x03: // article
    {
        uint16_t                section_id = istream.read_u16();
        uint16_t                article_id = istream.read_u16();

        this->send_stream(session, board.make_article_stream(section_id, article_id, session), scope::SELF);
        break;
    }

    case 0x05: // delete
    {
        uint16_t                section_id = istream.read_u16();
        uint16_t                article_id = istream.read_u16();

        this->send_stream(session, board.make_delete_stream(section_id, article_id, session), scope::SELF);
        break;
    }

    }

    return true;
}

bool fb::game::acceptor::handle_swap(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    uint8_t                     cmd = istream.read_u8();
    uint8_t                     swap_type = istream.read_u8();
    uint8_t                     src = istream.read_u8();
    uint8_t                     dest = istream.read_u8();

    if(swap_type) // spell
    {
        if(session.spell_swap(src-1, dest-1) == false) // zero-based
            return true;

        const auto              right = session.spell(src-1);
        this->send_stream(session, right ? right->make_show_stream(src) : right->make_delete_stream(src), scope::SELF);

        const auto              left = session.spell(dest-1);
        this->send_stream(session, left ? left->make_show_stream(dest) : left->make_delete_stream(dest), scope::SELF);
    }
    else // item
    {}

    return true;
}

void fb::game::acceptor::handle_counter_mob_action(fb::game::mob* mob)
{
    fb::game::session* target = mob->target();
    if(target == NULL)
    {
        fb::game::direction before_direction = mob->direction();
        this->handle_move_life(mob, fb::game::direction(std::rand() % 4));
        return;
    }

    // �����¿�� Ÿ���� �ִ��� �˻��Ѵ�.
    for(int i = 0; i < 4; i++)
    {
        fb::game::direction direction = fb::game::direction(i);
        if(mob->near_session(direction) != target)
            continue;

        if(mob->direction() != direction)
        {
            mob->direction(direction);
            this->send_stream(*mob, mob->make_direction_stream(), scope::PIVOT, true);
        }

        uint32_t random_damage = mob->random_damage(*target);
        target->hp_down(random_damage);


        // �����ϴ� ��Ŷ ������.
        this->send_stream(*mob, mob->make_action_stream(fb::game::action::ATTACK, fb::game::duration::DURATION_ATTACK), scope::PIVOT, true);
        this->send_stream(*target, target->make_show_hp_stream(random_damage, false), scope::PIVOT);
        this->send_stream(*target, target->make_state_stream(state_level::LEVEL_MIDDLE), scope::SELF);
        return;
    }


    // Ÿ�� �������� �̵��� �����ϴٸ� �̵��Ѵ�.
    bool x_axis = bool(std::rand()%2);
    if(x_axis)
    {
        if(mob->x() > target->x() && this->handle_move_life(mob, fb::game::direction::LEFT))   return;
        if(mob->x() < target->x() && this->handle_move_life(mob, fb::game::direction::RIGHT))  return;
        if(mob->y() > target->y() && this->handle_move_life(mob, fb::game::direction::TOP))    return;
        if(mob->y() < target->y() && this->handle_move_life(mob, fb::game::direction::BOTTOM)) return;
    }
    else
    {
        if(mob->y() > target->y() && this->handle_move_life(mob, fb::game::direction::TOP))    return;
        if(mob->y() < target->y() && this->handle_move_life(mob, fb::game::direction::BOTTOM)) return;
        if(mob->x() > target->x() && this->handle_move_life(mob, fb::game::direction::LEFT))   return;
        if(mob->x() < target->x() && this->handle_move_life(mob, fb::game::direction::RIGHT))  return;
    }


    // �̵��� �� �ִ� �������� �ϴ� �̵��Ѵ�.
    uint8_t random_direction = std::rand() % 4;
    for(int i = 0; i < 4; i++)
    {
        if(this->handle_move_life(mob, fb::game::direction((random_direction + i) % 4)))
            return;
    }
}

void fb::game::acceptor::handle_containment_mob_action(fb::game::mob* mob)
{
    fb::game::session* target = mob->target();
    fb::game::direction before_direction = mob->direction();

    try
    {
        if(target == NULL || target->sight(*mob) == false)
            target = mob->autoset_target();

        if(target == NULL)
            throw std::runtime_error("no target");
    }
    catch(std::exception&)
    { }

    this->handle_counter_mob_action(mob);
}

void fb::game::acceptor::handle_mob_action(uint64_t now)
{
    auto&               maps = db::maps();

    for(auto map_i = maps.begin(); map_i != maps.end(); map_i++)
    {
        auto            map = map_i->second;
        const std::vector<object*>& objects = map->objects();

        for(auto obj_i = objects.begin(); obj_i != objects.end(); obj_i++)
        {
            object* object = (*obj_i);
            if(object->type() != object::types::MOB)
                continue;

            fb::game::mob* mob = static_cast<fb::game::mob*>(object);
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
    std::vector<object*> spawned_mobs;
    for(auto pair : db::maps())
    {
        fb::game::map* map = pair.second;
        for(auto object : map->objects())
        {
            if(object->type() != object::types::MOB)
                continue;

            auto mob = static_cast<fb::game::mob*>(object);
            if(mob == NULL)
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
        if(session == NULL)
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

void fb::game::acceptor::handle_session_warp(fb::game::session& session, const map::warp* warp)
{
    map*                    map = session.map();

    // ������ ���̴� ������Ʈ�� ���� ����
    for(auto i : session.shown_objects())
        this->send_stream(session, i->make_hide_stream(), scope::SELF);

    for(auto i : session.shown_sessions())
        this->send_stream(session, i->make_hide_stream(), scope::SELF);

    session.map(warp->map, warp->after);

    this->send_stream(session, session.make_id_stream(), scope::SELF);
    this->send_stream(session, warp->map->make_config_stream(), scope::SELF);
    this->send_stream(session, warp->map->make_bgm_stream(), scope::SELF);
    this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
    this->send_stream(session, session.make_position_stream(), scope::SELF);
    this->send_stream(session, session.make_visual_stream(false), scope::SELF);
    this->send_stream(session, session.make_direction_stream(), scope::SELF);

    // ���� ���̴� ������Ʈ ������
    this->send_stream(session, session.make_show_objects_stream(), scope::SELF);

    // ���� ���̴� ���ǵ� ������
    for(auto i : session.shown_sessions())
        this->send_stream(session, i->make_visual_stream(false), scope::SELF);
}