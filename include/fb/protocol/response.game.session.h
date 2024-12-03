#ifndef __PROTOCOL_RESPONSE_GAME_SESSION_H__
#define __PROTOCOL_RESPONSE_GAME_SESSION_H__

#include <fb/protocol/protocol.h>
#include <character.h>
#include <fb/model/model.h>
#include <clan.h>
#include <group.h>

using namespace fb::game;

namespace fb::protocol::game::response::session {

class message : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x0A;

public:
    const std::string  text;
    const MESSAGE_TYPE type;

public:
    message(const std::string& text, MESSAGE_TYPE type) :
        text(text),
        type(type)
    { }

public:
    async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(static_cast<uint8_t>(this->type));
        writer.write<std::string, uint16_t>(this->text);
    }
};

class show : public fb::protocol::base::header
{
public:
    const fb::game::character& session;
    const fb::game::object&    to;
    const bool                 light;

public:
    show(const fb::game::character& session, const fb::game::object& to, bool light = false) :
        session(session),
        to(to),
        light(light)
    { }

    show(const show&) = delete;

private:
    bool clock_visible() const
    {
        if (&this->session == &this->to)
            return true;

        if (this->to.is(OBJECT_TYPE::CHARACTER) == false)
            return false;

        // TODO: to 에게 걸린 버프가 있어서 그게 투명 다 감지하는 버프면
        // return true

        auto mine = this->session.group();
        if (mine == nullptr)
            return false;

        auto your = static_cast<const fb::game::character&>(this->to).group();
        if (your == nullptr)
            return false;

        return mine == your;
    }

public:
    async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        auto map = this->session.map();
        if (map == nullptr)
            co_return;

        if (this->light == false)
        {
            writer.write<uint8_t>(0x33);                                            // id
            writer.write<uint16_t>(this->session.x());                              // x
            writer.write<uint16_t>(this->session.y());                              // y
            writer.write<uint8_t>(static_cast<uint8_t>(this->session.direction())); // side
        }
        else
        {
            writer.write<uint8_t>(0x1D); // id
        }

        writer.write<uint32_t>(this->session.sequence());
        writer.write<uint8_t>(this->session.state() == STATE::DISGUISE);  // 변신유무
        writer.write<uint8_t>(static_cast<uint8_t>(this->session.sex())); // sex

        switch (this->session.state())
        {
        case STATE::HALF_CLOACK:
        {
            if (this->clock_visible())
                writer.write<uint8_t>(static_cast<uint8_t>(STATE::HALF_CLOACK));
            else
                writer.write<uint8_t>(static_cast<uint8_t>(STATE::CLOACK));
        }
        break;
        default:
        {
            writer.write<uint8_t>(static_cast<uint8_t>(this->session.state()));
        }
        break;
        }

        if (this->session.state() == STATE::DISGUISE)
        {
            writer.write<uint16_t>(this->session.disguise().value());
            writer.write<uint8_t>(this->session.current_armor_color());
        }
        else
        {
            writer.write<uint16_t>(this->session.look()); // face
            writer.write<uint8_t>(this->session.color()); // hair color

            auto armor = this->session.items.armor();
            if (armor != nullptr)
            {
                writer.write<uint8_t>((uint8_t)armor->based<fb::model::armor>().dress);
                writer.write<uint8_t>(session.current_armor_color());
            }
            else
            {
                writer.write<uint8_t>(static_cast<uint8_t>(this->session.sex())); // sex
                writer.write<uint8_t>(0x00);
            }

            auto weapon = this->session.items.weapon();
            if (weapon != nullptr)
            {
                writer.write<uint16_t>(weapon->based<fb::model::weapon>().dress);
                writer.write<uint8_t>(weapon->color());
            }
            else
            {
                writer.write<uint16_t>(0xFFFF);
                writer.write<uint8_t>(0x00);
            }

            auto shield = this->session.items.shield();
            if (shield != nullptr)
            {
                writer.write<uint8_t>((uint8_t)shield->based<fb::model::shield>().dress);
                writer.write<uint8_t>(shield->color());
            }
            else
            {
                writer.write<uint8_t>(0xFF); // about shield
                writer.write<uint8_t>(0x00);
            }
        }

        writer.write<uint8_t>(0x04);                              // head mark
        writer.write<std::string, uint8_t>(this->session.name()); // name
    }
};

class id : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x05;

public:
#ifndef BOT
    const fb::game::character& session;
#else
    uint32_t sequence  = 0;
    uint32_t direction = 0;
    uint8_t  cls       = 0;
#endif

public:
#ifndef BOT
    id(const fb::game::character& session) :
        session(session)
    { }
#else
    id() = default;
#endif

public:
#ifndef BOT
    async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint32_t>(this->session.sequence());
        writer.write<uint32_t>(static_cast<uint32_t>(this->session.direction())); // side
        writer.write<uint8_t>(static_cast<uint8_t>(this->session.cls()));         // class
        writer.write<uint16_t>(0x00);
        writer.write<uint8_t>(0x00);
    }
#else
    async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->sequence  = reader.read<uint32_t>();
        this->direction = reader.read<uint32_t>();
        this->cls       = reader.read<uint8_t>();
    }
#endif
};

class state : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x08;

public:
#ifdef BOT
    STATE_LEVEL STATE_LEVEL;
    uint8_t     nation       = 0;
    uint8_t     creature     = 0;
    uint8_t     level        = 0;
    uint32_t    base_hp      = 0;
    uint32_t    base_mp      = 0;
    uint8_t     strength     = 0;
    uint8_t     intelligence = 0;
    uint8_t     dexteritry   = 0;
    uint32_t    hp           = 0;
    uint32_t    mp           = 0;
    uint32_t    experience   = 0;
    uint32_t    money        = 0;
    uint32_t    condition    = 0;
    uint8_t     mail         = 0;
    uint8_t     fast_move    = 0;
#else
    const fb::game::character& session;
    const STATE_LEVEL          level;
#endif

public:
#ifdef BOT
    state() = default;
#else
    state(const fb::game::character& session, STATE_LEVEL level) :
        session(session),
        level(level)
    { }
#endif

public:
#ifndef BOT
    async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(static_cast<uint8_t>(this->level));

        if (enum_in(this->level, STATE_LEVEL::BASED))
        {
            writer.write<uint8_t>(static_cast<uint8_t>(this->session.nation()));   // nation
            writer.write<uint8_t>(static_cast<uint8_t>(this->session.creature())); // creature
            writer.write<uint8_t>(0x00);                                           // Unknown (clan?)
            writer.write<uint8_t>(this->session.level());                          // level
            writer.write<uint32_t>(this->session.base_hp());                       // base hp
            writer.write<uint32_t>(this->session.base_mp());                       // base mp
            writer.write<uint8_t>(this->session.strength());
            writer.write<uint8_t>(this->session.intelligence());
            writer.write<uint8_t>(0x03);
            writer.write<uint8_t>(0x03);
            writer.write<uint8_t>(this->session.dexteritry());
            writer.write<uint8_t>(0x03);
            writer.write<uint32_t>(0x00);
            writer.write<uint8_t>(0x00);
        }

        if (enum_in(this->level, STATE_LEVEL::HP_MP))
        {
            writer.write<uint32_t>(this->session.hp()); // current hp
            writer.write<uint32_t>(this->session.mp()); // current mp
        }

        if (enum_in(this->level, STATE_LEVEL::EXP_MONEY))
        {
            writer.write<uint32_t>(this->session.experience()); // exp
            writer.write<uint32_t>(this->session.money());      // money
        }

        if (enum_in(this->level, STATE_LEVEL::CONDITION))
        {
            writer.write<uint8_t>(this->session.condition_contains(CONDITION::MOVE));  // condition::move
            writer.write<uint8_t>(this->session.condition_contains(CONDITION::SIGHT)); // condition::sight
            writer.write<uint8_t>(this->session.condition_contains(CONDITION::HEAR));  // condition::hear?
            writer.write<uint8_t>(this->session.condition_contains(CONDITION::ORAL));  // condition:oral
            writer.write<uint8_t>(this->session.condition_contains(CONDITION::MAP));   // condition:map?
        }

        writer.write<uint8_t>(0x00); // mail count
        writer.write<uint8_t>(true); // fast move
        writer.write<uint8_t>(0x00);
    }
#else
    async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->STATE_LEVEL = static_cast<fb::model::enum_value::STATE_LEVEL>(reader.read<uint8_t>());
        if (enum_in(this->STATE_LEVEL, STATE_LEVEL::BASED))
        {
            this->nation   = reader.read<uint8_t>();
            this->creature = reader.read<uint8_t>();
            reader.read<uint8_t>();
            this->level        = reader.read<uint8_t>();
            this->base_hp      = reader.read<uint32_t>();
            this->base_mp      = reader.read<uint32_t>();
            this->strength     = reader.read<uint8_t>();
            this->intelligence = reader.read<uint8_t>();
            reader.read<uint8_t>();
            reader.read<uint8_t>();
            this->dexteritry = reader.read<uint8_t>();
            reader.read<uint8_t>();
            reader.read<uint32_t>();
            reader.read<uint8_t>();
        }

        if (enum_in(this->STATE_LEVEL, STATE_LEVEL::HP_MP))
        {
            this->hp = reader.read<uint32_t>();
            this->mp = reader.read<uint32_t>();
        }

        if (enum_in(this->STATE_LEVEL, STATE_LEVEL::EXP_MONEY))
        {
            this->experience = reader.read<uint32_t>();
            this->money      = reader.read<uint32_t>();
        }

        if (enum_in(this->STATE_LEVEL, STATE_LEVEL::CONDITION))
        {
            if (reader.read<uint8_t>())
                this->condition |= (uint32_t)CONDITION::MOVE;
            if (reader.read<uint8_t>())
                this->condition |= (uint32_t)CONDITION::SIGHT;
            if (reader.read<uint8_t>())
                this->condition |= (uint32_t)CONDITION::HEAR;
            if (reader.read<uint8_t>())
                this->condition |= (uint32_t)CONDITION::ORAL;
            if (reader.read<uint8_t>())
                this->condition |= (uint32_t)CONDITION::MAP;
        }

        this->mail      = reader.read<uint8_t>();
        this->fast_move = reader.read<uint8_t>();
        reader.read<uint8_t>();
    }
#endif
};

class position : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x04;

public:
#ifndef BOT
    const fb::game::character& session;
#else
    point16_t abs;
    point16_t rel;
#endif

public:
#ifndef BOT
    position(const fb::game::character& session) :
        session(session)
    { }
#else
    position() = default;
#endif

public:
#ifndef BOT
    async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint16_t>(this->session.x()); // 실제 x 좌표
        writer.write<uint16_t>(this->session.y()); // 실제 y 좌표

        auto map = this->session.map();
        if (map->width() < fb::game::map::MAX_SCREEN_WIDTH)
            writer.write<uint16_t>(this->session.x() + fb::game::map::HALF_SCREEN_WIDTH - (map->width() / 2));
        else if (this->session.x() < fb::game::map::HALF_SCREEN_WIDTH)
            writer.write<uint16_t>(this->session.x());
        else if (this->session.x() >= map->width() - fb::game::map::HALF_SCREEN_WIDTH)
            writer.write<uint16_t>(this->session.x() + fb::game::map::MAX_SCREEN_WIDTH - map->width());
        else
            writer.write<uint16_t>(fb::game::map::HALF_SCREEN_WIDTH);

        // 스크린에서의 y 좌표
        if (map->height() < fb::game::map::MAX_SCREEN_HEIGHT)
            writer.write<uint16_t>(this->session.y() + fb::game::map::HALF_SCREEN_HEIGHT - (map->height() / 2));
        else if (this->session.y() < fb::game::map::HALF_SCREEN_HEIGHT)
            writer.write<uint16_t>(this->session.y());
        else if (this->session.y() >= (map->height() - fb::game::map::HALF_SCREEN_HEIGHT))
            writer.write<uint16_t>(this->session.y() + fb::game::map::MAX_SCREEN_HEIGHT - map->height());
        else
            writer.write<uint16_t>(fb::game::map::HALF_SCREEN_HEIGHT);

        writer.write<uint8_t>(0x00);
    }
#else
    async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->abs.x = reader.read<uint16_t>();
        this->abs.y = reader.read<uint16_t>();
        this->rel.x = reader.read<uint16_t>();
        this->rel.y = reader.read<uint16_t>();
    }
#endif
};

class internal_info : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x39;

public:
    const fb::game::character& session;
    const fb::model::model&    model;

public:
    internal_info(const fb::game::character& session, const fb::model::model& model) :
        session(session),
        model(model)
    { }

public:
    async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        auto clan = this->session.clan();
        writer.write<uint8_t>(header);
        writer.write<uint8_t>((uint8_t)this->session.defensive_physical());
        writer.write<uint8_t>(this->session.damage());
        writer.write<uint8_t>(this->session.hit());
        writer.write(clan != nullptr ? clan->name() : "");
        writer.write(clan != nullptr ? clan->title() : "");
        writer.write(this->session.title());

        // TODO: serialize coroutine
        // auto group = this->session.group();
        // if (group != nullptr)
        //{
        //     group->lock<void>([&writer](fb::game::group& g) {
        //         auto sstream = std::stringstream();
        //         sstream << "그룹원" << std::endl << "  * " << g.master() << std::endl;

        //        for (auto& member : g.members())
        //            sstream << "    " << member << std::endl;
        //        writer.write<std::string>(sstream.str());
        //    });
        //}
        // else
        //{
        writer.write<std::string>("그룹 없음.");
        //}
        writer.write<uint8_t>(this->session.option(SETTING::GROUP));

        uint32_t remained_exp = this->session.experience_remained();
        writer.write<uint32_t>(remained_exp);

        auto& class_name = model.promotion[this->session.cls()][this->session.promotion()].name;
        writer.write<std::string>(class_name);

        fb::game::equipment* equipments[] = {this->session.items.helmet(),
                                             this->session.items.ring(EQUIPMENT_POSITION::LEFT),
                                             this->session.items.ring(EQUIPMENT_POSITION::RIGHT),
                                             this->session.items.auxiliary(EQUIPMENT_POSITION::LEFT),
                                             this->session.items.auxiliary(EQUIPMENT_POSITION::RIGHT)};
        for (int i = 0, size = sizeof(equipments) / sizeof(fb::game::equipment*); i < size; i++)
        {
            if (equipments[i] == nullptr)
            {
                writer.write<uint16_t>(0xFFFF);
                writer.write<uint8_t>(0x00);
            }
            else
            {
                writer.write<uint16_t>(equipments[i]->look());
                writer.write<uint8_t>(equipments[i]->color());
            }
        }

        writer.write<uint8_t>(0x00); // fixed
        writer.write<uint8_t>(this->session.option(SETTING::TRADE));
        writer.write<uint8_t>(this->session.option(SETTING::PK_PROTECT));

        writer.write<uint8_t>((uint8_t)this->session.legends.size());
        for (auto legend : this->session.legends)
        {
            writer.write<uint8_t>(legend.look);
            writer.write<uint8_t>(legend.color);
            writer.write(legend.content);
        }
        writer.write<uint8_t>(0x00);
    }
};

class external_info : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x34;

public:
    const fb::game::character& session;
    const fb::model::model&    model;

public:
    external_info(const fb::game::character& session, const fb::model::model& model) :
        session(session),
        model(model)
    { }

public:
    async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<std::string>(this->session.title());
        writer.write("클랜 이름");
        writer.write("클랜 타이틀");

        // 클래스 이름
        const auto& class_name = model.promotion[this->session.cls()][this->session.promotion()].name;
        writer.write<std::string>(class_name);           // 직업
        writer.write<std::string>(this->session.name()); // 이름

        auto disguised = (this->session.state() == STATE::DISGUISE);
        writer.write<uint8_t>(disguised);
        writer.write<uint8_t>(static_cast<uint8_t>(this->session.sex()));
        writer.write<uint8_t>(static_cast<uint8_t>(this->session.state()));

        auto armor  = this->session.items.armor();  // 갑옷
        auto weapon = this->session.items.weapon(); // 무기
        auto shield = this->session.items.shield(); // 방패
        if (disguised)
        {
            writer.write<uint16_t>(this->session.disguise().value());
            writer.write<uint8_t>(this->session.current_armor_color());
        }
        else
        {
            writer.write<uint16_t>(this->session.look());
            writer.write<uint8_t>(this->session.color());

            writer.write<uint8_t>(armor != nullptr ? armor->based<fb::model::armor>().dress : 0xFF);
            writer.write<uint8_t>(this->session.current_armor_color());

            writer.write<uint16_t>(weapon != nullptr ? weapon->based<fb::model::weapon>().dress : 0xFFFF);
            writer.write<uint8_t>(weapon != nullptr ? weapon->color() : 0x00);

            writer.write<uint8_t>(shield != nullptr ? shield->based<fb::model::shield>().dress : 0xFF);
            writer.write<uint8_t>(shield != nullptr ? shield->color() : 0x00);
        }

        // 장비정보
        auto sstream = std::stringstream();
        auto helmet  = this->session.items.helmet(); // 투구
        writer.write<uint16_t>(helmet != nullptr ? helmet->look() : 0xFFFF);
        writer.write<uint8_t>(helmet != nullptr ? helmet->color() : 0x00);

        auto ring_l = this->session.items.ring(EQUIPMENT_POSITION::LEFT); // 왼손
        writer.write<uint16_t>(ring_l != nullptr ? ring_l->look() : 0xFFFF);
        writer.write<uint8_t>(ring_l != nullptr ? ring_l->color() : 0x00);

        auto ring_r = this->session.items.ring(EQUIPMENT_POSITION::RIGHT); // 오른손
        writer.write<uint16_t>(ring_r != nullptr ? ring_r->look() : 0xFFFF);
        writer.write<uint8_t>(ring_r != nullptr ? ring_r->color() : 0x00);

        auto aux_l = this->session.items.auxiliary(EQUIPMENT_POSITION::LEFT); // 보조1
        writer.write<uint16_t>(aux_l != nullptr ? aux_l->look() : 0xFFFF);
        writer.write<uint8_t>(aux_l != nullptr ? aux_l->color() : 0x00);

        auto aux_r = this->session.items.auxiliary(EQUIPMENT_POSITION::RIGHT); // 보조2
        writer.write<uint16_t>(aux_r != nullptr ? aux_r->look() : 0xFFFF);
        writer.write<uint8_t>(aux_r != nullptr ? aux_r->color() : 0x00);

        // 장비정보 텍스트
        sstream << " w:무기  :" << (weapon != nullptr ? weapon->name() : "없음") << std::endl;
        sstream << " a:갑옷  :" << (armor != nullptr ? armor->name() : "없음") << std::endl;
        sstream << " s:방패  :" << (shield != nullptr ? shield->name() : "없음") << std::endl;
        sstream << " h:머리  :" << (helmet != nullptr ? helmet->name() : "없음") << std::endl;
        sstream << " l:왼손  :" << (ring_l != nullptr ? ring_l->name() : "없음") << std::endl;
        sstream << " r:오른손:" << (ring_r != nullptr ? ring_r->name() : "없음") << std::endl;
        sstream << " [:보조1 :" << (aux_l != nullptr ? aux_l->name() : "없음") << std::endl;
        sstream << " ]:보조2 :" << (aux_r != nullptr ? aux_r->name() : "없음") << std::endl;
        writer.write<std::string>(sstream.str());

        writer.write<uint32_t>(this->session.sequence());
        writer.write<uint8_t>(this->session.option(SETTING::GROUP));
        writer.write<uint8_t>(this->session.option(SETTING::TRADE));
        writer.write<uint32_t>(0x00000000); // unknown

        // 업적
        writer.write<uint8_t>((uint8_t)this->session.legends.size());
        for (auto& legend : this->session.legends)
        {
            writer.write<uint8_t>(legend.look);
            writer.write<uint8_t>(legend.color);
            writer.write(legend.content);
        }
        writer.write<uint8_t>(0x00);
    }
};

class option : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x23;

public:
#ifdef BOT
    bool weather_effect = false;
    bool magic_effect   = false;
    bool roar_worlds    = false;
    bool fast_move      = false;
    bool effect_sound   = false;
#else
    const fb::game::character& session;
#endif

public:
#ifdef BOT
    option() = default;
#else
    option(const fb::game::character& session) :
        session(session)
    { }
#endif

public:
#ifndef BOT
    async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(this->session.option(SETTING::WEATHER_EFFECT)); // weather
        writer.write<uint8_t>(this->session.option(SETTING::MAGIC_EFFECT));   // magic effect
        writer.write<uint8_t>(this->session.option(SETTING::ROAR_WORLDS));    // listen news
        writer.write<uint8_t>(this->session.option(SETTING::FAST_MOVE));      // fast move
        writer.write<uint8_t>(this->session.option(SETTING::EFFECT_SOUND));   // effect sound
        writer.write<uint8_t>(0x00);
    }
#else
    async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->weather_effect = reader.read<uint8_t>();
        this->magic_effect   = reader.read<uint8_t>();
        this->roar_worlds    = reader.read<uint8_t>();
        this->fast_move      = reader.read<uint8_t>();
        this->effect_sound   = reader.read<uint8_t>();
        reader.read<uint8_t>();
    }
#endif
};

class throws : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x16;

public:
    const fb::game::character& session;
    const fb::game::item&      item;
    const point16_t            to;

public:
    throws(const fb::game::character& session, const fb::game::item& item, const point16_t& to) :
        session(session),
        item(item),
        to(to)
    { }

public:
    async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint32_t>(this->session.sequence());
        writer.write<uint16_t>(this->item.look());
        writer.write<uint8_t>(this->item.color());
        writer.write<uint32_t>(this->item.sequence());
        writer.write<uint16_t>(this->session.x());
        writer.write<uint16_t>(this->session.y());
        writer.write<uint16_t>(this->to.x);
        writer.write<uint16_t>(this->to.y);
        writer.write<uint32_t>(0x00000000);
        writer.write<uint8_t>(0x02);
        writer.write<uint8_t>(0x00);
    }
};

class action : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x1A;

public:
    const fb::game::character& me;
    const ACTION               value;
    const DURATION             duration;
    const uint8_t              sound;

public:
    action(const fb::game::character& me, ACTION value, DURATION duration, uint8_t sound = 0x00) :
        me(me),
        value(value),
        duration(duration),
        sound(sound)
    { }

public:
    async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint32_t>(this->me.sequence());
        writer.write<uint8_t>(static_cast<uint8_t>(this->value));      // type
        writer.write<uint16_t>(static_cast<uint16_t>(this->duration)); // duration
        writer.write<uint8_t>(this->sound);                            // sound
    }
};

} // namespace fb::protocol::game::response::session

#endif // !__PROTOCOL_RESPONSE_GAME_SESSION_H__