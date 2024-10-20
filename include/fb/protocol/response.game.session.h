#ifndef __PROTOCOL_RESPONSE_GAME_SESSION_H__
#define __PROTOCOL_RESPONSE_GAME_SESSION_H__

#include <fb/protocol/protocol.h>
#include <character.h>
#include <model.h>
#include <clan.h>
#include <group.h>

using namespace fb::game;

namespace fb { namespace protocol { namespace game { namespace response { namespace session {

class message : public fb::protocol::base::header
{
public:
    const std::string               text;
    const MESSAGE_TYPE    type;

public:
    message(const std::string& text, MESSAGE_TYPE type) : fb::protocol::base::header(0x0A),
        text(text), type(type)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->type)
                  .write(this->text, true);
    }
};

class show : public fb::protocol::base::header
{
public:
    const fb::game::character&    session;
    const fb::game::object&     to;
    const bool                  light;

public:
    show(const fb::game::character& session, const fb::game::object& to, bool light = false) : fb::protocol::base::header(light ? 0x1D : 0x33),
        session(session), to(to), light(light)
    { }

    show(const show&) = delete;

private:
    bool clock_visible() const
    {
        if(&this->session == &this->to)
            return true;

        if(this->to.is(OBJECT_TYPE::CHARACTER) == false)
            return false;

        // TODO: to 에게 걸린 버프가 있어서 그게 투명 다 감지하는 버프면
        // return true

        auto mine = this->session.group();
        if(mine == nullptr)
            return false;

        auto your = static_cast<const fb::game::character&>(this->to).group();
        if(your == nullptr)
            return false;

        return mine == your;
    }

public:
    void serialize(fb::ostream& out_stream) const
    {
        auto map = this->session.map();
        if(map == nullptr)
            return;

        base::header::serialize(out_stream);
        if(this->light == false)
        {
            out_stream.write_u16(this->session.x()) // x
                      .write_u16(this->session.y()) // y
                      .write_u8(this->session.direction()); // side
        }

        out_stream.write_u32(this->session.sequence())
                  .write_u8(this->session.state() == STATE::DISGUISE) // 변신유무
                  .write_u8(this->session.sex()); // sex


        switch(this->session.state())
        {
        case STATE::HALF_CLOACK:
        {
            if(this->clock_visible())
                out_stream.write_8(STATE::HALF_CLOACK);
            else
                out_stream.write_8(STATE::CLOACK);
        } break;
        default:
        {
            out_stream.write_u8(this->session.state());
        } break;
        }

        if(this->session.state() == STATE::DISGUISE)
        {
            out_stream.write_u16(this->session.disguise().value())
                      .write_u8(this->session.current_armor_color());
        }
        else
        {
            out_stream.write_u16(this->session.look()) // face
                      .write_u8(this->session.color()); // hair color

            auto armor = this->session.items.armor();
            if(armor != nullptr)
            {
                out_stream.write_u8((uint8_t)armor->based<fb::model::armor>().dress)
                          .write_u8(session.current_armor_color());
            }
            else
            {
                out_stream.write_u8(this->session.sex()) // sex
                          .write_u8(0x00);
            }

            auto weapon = this->session.items.weapon();
            if(weapon != nullptr)
            {
                out_stream.write_u16(weapon->based<fb::model::weapon>().dress)
                          .write_u8(weapon->color());
            }
            else
            {
                out_stream.write_u16(0xFFFF)
                          .write_u8(0x00);
            }

            auto shield = this->session.items.shield();
            if(shield != nullptr)
            {
                out_stream.write_u8((uint8_t)shield->based<fb::model::shield>().dress)
                          .write_u8(shield->color());
            }
            else
            {
                out_stream.write_u8(0xFF) // about shield
                          .write_u8(0x00);
            }
        }

        out_stream.write_u8(0x04) // head mark
                  .write(this->session.name(), false); // name
    }
};

class id : public fb::protocol::base::header
{
public:
#ifndef BOT
    const fb::game::character&    session;
#else
    uint32_t                    sequence = 0;
    uint32_t                    direction = 0;
    uint8_t                     cls = 0;
#endif

public:
#ifndef BOT
    id(const fb::game::character& session) : fb::protocol::base::header(0x05),
        session(session)
    { }
#else
    id() : fb::protocol::base::header(0x05)
    { }
#endif

public:
#ifndef BOT
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u32(this->session.sequence())
                  .write_u32(this->session.direction()) // side
                  .write_u8(this->session.cls()) // class
                  .write_u16(0x00)
                  .write_u8(0x00);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->sequence = in_stream.read_u32();
        this->direction = in_stream.read_u32();
        this->cls = in_stream.read_u8();
    }
#endif
};

class state : public fb::protocol::base::header
{
public:
#ifdef BOT
    STATE_LEVEL                 STATE_LEVEL;
    uint8_t                     nation       = 0;
    uint8_t                     creature     = 0;
    uint8_t                     level        = 0;
    uint32_t                    base_hp      = 0;
    uint32_t                    base_mp      = 0;
    uint8_t                     strength     = 0;
    uint8_t                     intelligence = 0;
    uint8_t                     dexteritry   = 0;
    uint32_t                    hp           = 0;
    uint32_t                    mp           = 0;
    uint32_t                    experience   = 0;
    uint32_t                    money        = 0;
    uint32_t                    condition    = 0;
    uint8_t                     mail         = 0;
    uint8_t                     fast_move    = 0;
#else
    const fb::game::character&    session;
    const STATE_LEVEL level;
#endif

public:
#ifdef BOT
    state() : fb::protocol::base::header(0x08)
    { }
#else
    state(const fb::game::character& session, STATE_LEVEL level) : fb::protocol::base::header(0x08),
        session(session), level(level)
    { }
#endif

public:
#ifndef BOT
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->level);

        if(enum_in(this->level, STATE_LEVEL::BASED))
        {
            out_stream.write_u8(this->session.nation()) // nation
                      .write_u8(this->session.creature()) // creature
                      .write_u8(0x00) // Unknown (clan?)
                      .write_u8(this->session.level()) // level
                      .write_u32(this->session.base_hp()) // base hp
                      .write_u32(this->session.base_mp()) // base mp
                      .write_u8(this->session.strength())
                      .write_u8(this->session.intelligence())
                      .write_u8(0x03)
                      .write_u8(0x03)
                      .write_u8(this->session.dexteritry())
                      .write_u8(0x03)
                      .write_u32(0x00)
                      .write_u8(0x00);
        }

        if(enum_in(this->level, STATE_LEVEL::HP_MP))
        {
            out_stream.write_u32(this->session.hp())  // current hp
                      .write_u32(this->session.mp()); // current mp
        }

        if(enum_in(this->level, STATE_LEVEL::EXP_MONEY))
        {
            out_stream.write_u32(this->session.experience())  // exp
                      .write_u32(this->session.money()); // money
        }

        if(enum_in(this->level, STATE_LEVEL::CONDITION))
        {
            out_stream.write_u8(this->session.condition_contains(CONDITION::MOVE))  // condition::move
                      .write_u8(this->session.condition_contains(CONDITION::SIGHT))  // condition::sight
                      .write_u8(this->session.condition_contains(CONDITION::HEAR))  // condition::hear?
                      .write_u8(this->session.condition_contains(CONDITION::ORAL))  // condition:oral
                      .write_u8(this->session.condition_contains(CONDITION::MAP)); // condition:map?
        }

        out_stream.write_u8(0x00)  // mail count
                  .write_u8(true)  // fast move
                  .write_u8(0x00);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->STATE_LEVEL = static_cast<fb::model::enum_value::STATE_LEVEL>(in_stream.read_u8());
        if(enum_in(this->STATE_LEVEL, STATE_LEVEL::BASED))
        {
            this->nation = in_stream.read_u8();
            this->creature = in_stream.read_u8();
            in_stream.read_u8();
            this->level = in_stream.read_u8();
            this->base_hp = in_stream.read_u32();
            this->base_mp = in_stream.read_u32();
            this->strength = in_stream.read_u8();
            this->intelligence = in_stream.read_u8();
            in_stream.read_u8();
            in_stream.read_u8();
            this->dexteritry = in_stream.read_u8();
            in_stream.read_u8();
            in_stream.read_u32();
            in_stream.read_u8();
        }

        if(enum_in(this->STATE_LEVEL, STATE_LEVEL::HP_MP))
        {
            this->hp = in_stream.read_u32();
            this->mp = in_stream.read_u32();
        }

        if(enum_in(this->STATE_LEVEL, STATE_LEVEL::EXP_MONEY))
        {
            this->experience = in_stream.read_u32();
            this->money = in_stream.read_u32();
        }

        if(enum_in(this->STATE_LEVEL, STATE_LEVEL::CONDITION))
        {
            if (in_stream.read_u8())
                this->condition |= (uint32_t)CONDITION::MOVE;
            if (in_stream.read_u8())
                this->condition |= (uint32_t)CONDITION::SIGHT;
            if (in_stream.read_u8())
                this->condition |= (uint32_t)CONDITION::HEAR;
            if (in_stream.read_u8())
                this->condition |= (uint32_t)CONDITION::ORAL;
            if (in_stream.read_u8())
                this->condition |= (uint32_t)CONDITION::MAP;
        }

        this->mail = in_stream.read_u8();
        this->fast_move = in_stream.read_u8();
        in_stream.read_u8();
    }
#endif
};

class position : public fb::protocol::base::header
{
public:
#ifndef BOT
    const fb::game::character&    session;
#else
    point16_t                   abs;
    point16_t                   rel;
#endif

public:
#ifndef BOT
    position(const fb::game::character& session) : fb::protocol::base::header(0x04),
        session(session)
    { }
#else
    position() : fb::protocol::base::header(0x04)
    { }
#endif

public:
#ifndef BOT
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u16(this->session.x())  // 실제 x 좌표
                  .write_u16(this->session.y()); // 실제 y 좌표

        auto                    map = this->session.map();
        if(map->width() < fb::game::map::MAX_SCREEN_WIDTH)
            out_stream.write_u16(this->session.x() + fb::game::map::HALF_SCREEN_WIDTH - (map->width() / 2));
        else if(this->session.x() < fb::game::map::HALF_SCREEN_WIDTH)
            out_stream.write_u16(this->session.x());
        else if(this->session.x() >= map->width() - fb::game::map::HALF_SCREEN_WIDTH)
            out_stream.write_u16(this->session.x() + fb::game::map::MAX_SCREEN_WIDTH - map->width());
        else
            out_stream.write_u16(fb::game::map::HALF_SCREEN_WIDTH);

        // 스크린에서의 y 좌표
        if(map->height() < fb::game::map::MAX_SCREEN_HEIGHT)
            out_stream.write_u16(this->session.y() + fb::game::map::HALF_SCREEN_HEIGHT - (map->height() / 2));
        else if(this->session.y() < fb::game::map::HALF_SCREEN_HEIGHT)
            out_stream.write_u16(this->session.y());
        else if(this->session.y() >= (map->height() - fb::game::map::HALF_SCREEN_HEIGHT))
            out_stream.write_u16(this->session.y() + fb::game::map::MAX_SCREEN_HEIGHT - map->height());
        else
            out_stream.write_u16(fb::game::map::HALF_SCREEN_HEIGHT);

        out_stream.write_u8(0x00);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->abs.x = in_stream.read_u16();
        this->abs.y = in_stream.read_u16();
        this->rel.x = in_stream.read_u16();
        this->rel.y = in_stream.read_u16();
    }
#endif
};

class internal_info : public fb::protocol::base::header
{
public:
    const fb::game::character&    session;
    const fb::model::model& model;

public:
    internal_info(const fb::game::character& session, const fb::model::model& model) : fb::protocol::base::header(0x39),
        session(session), model(model)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        auto clan = this->session.clan();
        base::header::serialize(out_stream);
        out_stream.write_u8((uint8_t)this->session.defensive_physical())
                  .write_u8(this->session.damage())
                  .write_u8(this->session.hit())
                  .write(clan != nullptr ? clan->name() : "")
                  .write(clan != nullptr ? clan->title() : "")
                  .write(this->session.title());

        auto group = this->session.group();
        if(group != nullptr)
        {
            std::stringstream   sstream;
            sstream << "그룹원" 
                << std::endl 
                << "  * "
                << group->leader().name()
                << std::endl;

            for(auto member : group->members())
            {
                if(group->leader() == *member)
                    continue;

                sstream << "    "
                    << member->name()
                    << std::endl;
            }
            out_stream.write(sstream.str());
        }
        else
        {
            out_stream.write("그룹 없음.");
        }
        out_stream.write_u8(this->session.option(CUSTOM_SETTING::GROUP));


        uint32_t                remained_exp = this->session.experience_remained();
        out_stream.write_u32(remained_exp);

        auto&                   class_name = model.promotion[this->session.cls()][this->session.promotion()].name;
        out_stream.write(class_name);

        fb::game::equipment*    equipments[] = {this->session.items.helmet(), this->session.items.ring(EQUIPMENT_POSITION::LEFT), this->session.items.ring(EQUIPMENT_POSITION::RIGHT), this->session.items.auxiliary(EQUIPMENT_POSITION::LEFT), this->session.items.auxiliary(EQUIPMENT_POSITION::RIGHT)};
        for(int i = 0, size = sizeof(equipments) / sizeof(fb::game::equipment*); i < size; i++)
        {
            if(equipments[i] == nullptr)
            {
                out_stream.write_u16(0xFFFF)
                          .write_u8(0x00);
            }
            else
            {
                out_stream.write_u16(equipments[i]->look())
                          .write_u8(equipments[i]->color());
            }
        }

        out_stream.write_u8(0x00) // fixed;
                  .write_u8(this->session.option(CUSTOM_SETTING::TRADE))
                  .write_u8(this->session.option(CUSTOM_SETTING::PK));

        out_stream.write_u8((uint8_t)this->session.legends.size());
        for(auto legend : this->session.legends)
        {
            out_stream.write_u8(legend.look)
                      .write_u8(legend.color)
                      .write(legend.content);
        }
        out_stream.write_u8(0x00);
    }
};

class external_info : public fb::protocol::base::header
{
public:
    const fb::game::character&    session;
    const fb::model::model& model;

public:
    external_info(const fb::game::character& session, const fb::model::model& model) : fb::protocol::base::header(0x34),
        session(session), model(model)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write(this->session.title())
                  .write("클랜 이름")
                  .write("클랜 타이틀");

        // 클래스 이름
        const auto&              class_name = model.promotion[this->session.cls()][this->session.promotion()].name;
        out_stream.write(class_name)  // 직업
                  .write(this->session.name());    // 이름

        auto                    disguised = (this->session.state() == STATE::DISGUISE);
        out_stream.write_u8(disguised)
                  .write_u8(this->session.sex())
                  .write_u8(this->session.state());

        auto                    armor = this->session.items.armor(); // 갑옷
        auto                    weapon = this->session.items.weapon(); // 무기
        auto                    shield = this->session.items.shield(); // 방패
        if(disguised)
        {
            out_stream.write_u16(this->session.disguise().value())
                      .write_u8(this->session.current_armor_color());
        }
        else
        {
            out_stream.write_u16(this->session.look())
                      .write_u8(this->session.color());

            out_stream.write_u8(armor != nullptr ? armor->based<fb::model::armor>().dress : 0xFF)
                      .write_u8(this->session.current_armor_color());

            out_stream.write_u16(weapon != nullptr ? weapon->based<fb::model::weapon>().dress : 0xFFFF)
                      .write_u8(weapon != nullptr ? weapon->color() : 0x00);

            out_stream.write_u8(shield != nullptr ? shield->based<fb::model::shield>().dress : 0xFF)
                      .write_u8(shield != nullptr ? shield->color() : 0x00);
        }

        // 장비정보
        std::stringstream       sstream;
        auto                    helmet = this->session.items.helmet(); // 투구
        out_stream.write_u16(helmet != nullptr ? helmet->look() : 0xFFFF)
                  .write_u8(helmet != nullptr ? helmet->color() : 0x00);

        auto                    ring_l = this->session.items.ring(EQUIPMENT_POSITION::LEFT); // 왼손
        out_stream.write_u16(ring_l != nullptr ? ring_l->look() : 0xFFFF)
                  .write_u8(ring_l != nullptr ? ring_l->color() : 0x00);

        auto                    ring_r = this->session.items.ring(EQUIPMENT_POSITION::RIGHT); // 오른손
        out_stream.write_u16(ring_r != nullptr ? ring_r->look() : 0xFFFF)
                  .write_u8(ring_r != nullptr ? ring_r->color() : 0x00);

        auto                    aux_l = this->session.items.auxiliary(EQUIPMENT_POSITION::LEFT); // 보조1
        out_stream.write_u16(aux_l != nullptr ? aux_l->look() : 0xFFFF)
                  .write_u8(aux_l != nullptr ? aux_l->color() : 0x00);

        auto                    aux_r = this->session.items.auxiliary(EQUIPMENT_POSITION::RIGHT); // 보조2
        out_stream.write_u16(aux_r != nullptr ? aux_r->look() : 0xFFFF)
                  .write_u8(aux_r != nullptr ? aux_r->color() : 0x00);


        // 장비정보 텍스트
        sstream << " w:무기  :" << (weapon != nullptr ? weapon->name() : "없음") << std::endl;
        sstream << " a:갑옷  :" << (armor  != nullptr ? armor->name()  : "없음") << std::endl;
        sstream << " s:방패  :" << (shield != nullptr ? shield->name() : "없음") << std::endl;
        sstream << " h:머리  :" << (helmet != nullptr ? helmet->name() : "없음") << std::endl;
        sstream << " l:왼손  :" << (ring_l != nullptr ? ring_l->name() : "없음") << std::endl;
        sstream << " r:오른손:" << (ring_r != nullptr ? ring_r->name() : "없음") << std::endl;
        sstream << " [:보조1 :" << (aux_l  != nullptr ? aux_l->name()  : "없음") << std::endl;
        sstream << " ]:보조2 :" << (aux_r  != nullptr ? aux_r->name()  : "없음") << std::endl;
        out_stream.write(sstream.str());


        out_stream.write_u32(this->session.sequence())
                  .write_u8(this->session.option(CUSTOM_SETTING::GROUP))
                  .write_u8(this->session.option(CUSTOM_SETTING::TRADE))
                  .write_u32(0x00000000); // unknown

                                    // 업적
        out_stream.write_u8((uint8_t)this->session.legends.size());
        for(auto legend : this->session.legends)
        {
            out_stream.write_u8(legend.look)
                      .write_u8(legend.color)
                      .write(legend.content);
        }
        out_stream.write_u8(0x00);
    }
};

class option : public fb::protocol::base::header
{
public:
#ifdef BOT
    bool                        weather_effect = false;
    bool                        magic_effect = false;
    bool                        roar_worlds = false;
    bool                        fast_move = false;
    bool                        effect_sound = false;
#else
    const fb::game::character&    session;
#endif

public:
#ifdef BOT
    option() : fb::protocol::base::header(0x23)
    { }
#else
    option(const fb::game::character& session) : fb::protocol::base::header(0x23),
        session(session)
    { }
#endif

public:
#ifndef BOT
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->session.option(CUSTOM_SETTING::WEATHER_EFFECT)) // weather
                  .write_u8(this->session.option(CUSTOM_SETTING::MAGIC_EFFECT)) // magic effect
                  .write_u8(this->session.option(CUSTOM_SETTING::ROAR_WORLDS)) // listen news
                  .write_u8(this->session.option(CUSTOM_SETTING::FAST_MOVE)) // fast move
                  .write_u8(this->session.option(CUSTOM_SETTING::EFFECT_SOUND)) // effect sound
                  .write_u8(0x00);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->weather_effect = in_stream.read_u8();
        this->magic_effect = in_stream.read_u8();
        this->roar_worlds = in_stream.read_u8();
        this->fast_move = in_stream.read_u8();
        this->effect_sound = in_stream.read_u8();
        in_stream.read_u8();
    }
#endif
};

class throws : public fb::protocol::base::header
{
public:
    const fb::game::character&    session;
    const fb::game::item&       item;
    const point16_t             to;

public:
    throws(const fb::game::character& session, const fb::game::item& item, const point16_t& to) : fb::protocol::base::header(0x16),
        session(session), item(item), to(to)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u32(this->session.sequence())
                  .write_u16(this->item.look())
                  .write_u8(this->item.color())
                  .write_u32(this->item.sequence())
                  .write_u16(this->session.x())
                  .write_u16(this->session.y())
                  .write_u16(this->to.x)
                  .write_u16(this->to.y)
                  .write_u32(0x00000000)
                  .write_u8(0x02)
                  .write_u8(0x00);
    }
};

class action : public fb::protocol::base::header
{
public:
    const fb::game::character&        me;
    const ACTION     value;
    const DURATION        duration;
    const uint8_t                   sound;

public:
    action(const fb::game::character& me, ACTION value, DURATION duration, uint8_t sound = 0x00) : fb::protocol::base::header(0x1A),
        me(me), value(value), duration(duration), sound(sound)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u32(this->me.sequence())
                  .write_u8(this->value) // type
                  .write_u16(this->duration) // duration
                  .write_u8(this->sound); // sound
    }
};

} } } } }

#endif // !__PROTOCOL_RESPONSE_GAME_SESSION_H__