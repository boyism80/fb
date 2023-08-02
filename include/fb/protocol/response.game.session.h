#ifndef __PROTOCOL_RESPONSE_GAME_SESSION_H__
#define __PROTOCOL_RESPONSE_GAME_SESSION_H__

#include <fb/protocol/protocol.h>
#include <fb/game/session.h>
#include <fb/game/data_set.h>
#include <fb/game/clan.h>

using namespace fb::game;

namespace fb { namespace protocol { namespace game { namespace response { namespace session {

class message : public fb::protocol::base::header
{
public:
    const std::string               text;
    const fb::game::message::type   type;

public:
    message(const std::string& text, fb::game::message::type type) : fb::protocol::base::header(0x0A),
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
    const fb::game::session&    session;
    const fb::game::object&     to;
    const bool                  light;

public:
    show(const fb::game::session& session, const fb::game::object& to, bool light = false) : fb::protocol::base::header(light ? 0x1D : 0x33),
        session(session), to(to), light(light)
    { }

    show(const show&) = delete;

private:
    bool clock_visible() const
    {
        if(&this->session == &this->to)
            return true;

        if(this->to.is(fb::game::object::types::SESSION) == false)
            return false;

        // TODO: to 에게 걸린 버프가 있어서 그게 투명 다 감지하는 버프면
        // return true

        auto mine = this->session.group();
        if(mine == nullptr)
            return false;

        auto your = static_cast<const fb::game::session&>(this->to).group();
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
                  .write_u8(this->session.state() == fb::game::state::DISGUISE) // 변신유무
                  .write_u8(this->session.sex()); // sex


        switch(this->session.state())
        {
        case fb::game::state::HALF_CLOACK:
        {
            if(this->clock_visible())
                out_stream.write_8(fb::game::state::HALF_CLOACK);
            else
                out_stream.write_8(fb::game::state::CLOACK);
        } break;
        default:
        {
            out_stream.write_u8(this->session.state());
        } break;
        }

        if(this->session.state() == fb::game::state::DISGUISE)
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
                out_stream.write_u8((uint8_t)armor->based<fb::game::armor>()->dress)
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
                out_stream.write_u16(weapon->based<fb::game::weapon>()->dress)
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
                out_stream.write_u8((uint8_t)shield->based<fb::game::shield>()->dress)
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
    const fb::game::session&    session;

public:
    id(const fb::game::session& session) : fb::protocol::base::header(0x05),
        session(session)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u32(this->session.sequence())
                  .write_u32(this->session.direction()) // side
                  .write_u8(this->session.cls()) // class
                  .write_u16(0x00)
                  .write_u8(0x00);
    }
};

class state : public fb::protocol::base::header
{
public:
    const fb::game::session&    session;
    const fb::game::state_level level;

public:
    state(const fb::game::session& session, fb::game::state_level level) : fb::protocol::base::header(0x08),
        session(session), level(level)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->level);

        if(enum_in(this->level, state_level::BASED))
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

        if(enum_in(this->level, state_level::HP_MP))
        {
            out_stream.write_u32(this->session.hp())  // current hp
                      .write_u32(this->session.mp()); // current mp
        }

        if(enum_in(this->level, state_level::EXP_MONEY))
        {
            out_stream.write_u32(this->session.experience())  // exp
                      .write_u32(this->session.money()); // money
        }

        if(enum_in(this->level, state_level::CONDITION))
        {
            out_stream.write_u8(this->session.condition_contains(fb::game::condition::MOVE))  // condition::move
                      .write_u8(this->session.condition_contains(fb::game::condition::SIGHT))  // condition::sight
                      .write_u8(this->session.condition_contains(fb::game::condition::HEAR))  // condition::hear?
                      .write_u8(this->session.condition_contains(fb::game::condition::ORAL))  // condition:oral
                      .write_u8(this->session.condition_contains(fb::game::condition::MAP)); // condition:map?
        }

        out_stream.write_u8(0x00)  // mail count
                  .write_u8(true)  // fast move
                  .write_u8(0x00);
    }
};

class position : public fb::protocol::base::header
{
public:
    const fb::game::session&    session;

public:
    position(const fb::game::session& session) : fb::protocol::base::header(0x04),
        session(session)
    { }

public:
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
};

class internal_info : public fb::protocol::base::header
{
public:
    const fb::game::session&    session;

public:
    internal_info(const fb::game::session& session) : fb::protocol::base::header(0x39),
        session(session)
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
        out_stream.write_u8(this->session.option(options::GROUP));


        uint32_t                remained_exp = this->session.experience_remained();
        out_stream.write_u32(remained_exp);

        auto                    class_name = fb::game::data_set::classes.class2name(this->session.cls(), this->session.promotion());
        if(class_name == nullptr)
            return;

        out_stream.write(*class_name);

        fb::game::equipment*    equipments[] = {this->session.items.helmet(), this->session.items.ring(equipment::position::LEFT), this->session.items.ring(equipment::position::RIGHT), this->session.items.auxiliary(equipment::position::LEFT), this->session.items.auxiliary(equipment::position::RIGHT)};
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
                  .write_u8(this->session.option(options::TRADE))
                  .write_u8(this->session.option(options::PK));

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
    const fb::game::session&    session;

public:
    external_info(const fb::game::session& session) : fb::protocol::base::header(0x34),
        session(session)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write(this->session.title())
                  .write("클랜 이름")
                  .write("클랜 타이틀");

        // 클래스 이름
        const auto              class_name = fb::game::data_set::classes.class2name(this->session.cls(), this->session.promotion());
        if(class_name == nullptr)
            return;

        out_stream.write(*class_name)  // 직업
                  .write(this->session.name());    // 이름

        auto                    disguised = (this->session.state() == fb::game::state::DISGUISE);
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

            out_stream.write_u8(armor != nullptr ? armor->based<fb::game::armor>()->dress : 0xFF)
                      .write_u8(this->session.current_armor_color());

            out_stream.write_u16(weapon != nullptr ? weapon->based<fb::game::weapon>()->dress : 0xFFFF)
                      .write_u8(weapon != nullptr ? weapon->color() : 0x00);

            out_stream.write_u8(shield != nullptr ? shield->based<fb::game::shield>()->dress : 0xFF)
                      .write_u8(shield != nullptr ? shield->color() : 0x00);
        }

        // 장비정보
        std::stringstream       sstream;
        auto                    helmet = this->session.items.helmet(); // 투구
        out_stream.write_u16(helmet != nullptr ? helmet->look() : 0xFFFF)
                  .write_u8(helmet != nullptr ? helmet->color() : 0x00);

        auto                    ring_l = this->session.items.ring(equipment::position::LEFT); // 왼손
        out_stream.write_u16(ring_l != nullptr ? ring_l->look() : 0xFFFF)
                  .write_u8(ring_l != nullptr ? ring_l->color() : 0x00);

        auto                    ring_r = this->session.items.ring(equipment::position::RIGHT); // 오른손
        out_stream.write_u16(ring_r != nullptr ? ring_r->look() : 0xFFFF)
                  .write_u8(ring_r != nullptr ? ring_r->color() : 0x00);

        auto                    aux_l = this->session.items.auxiliary(equipment::position::LEFT); // 보조1
        out_stream.write_u16(aux_l != nullptr ? aux_l->look() : 0xFFFF)
                  .write_u8(aux_l != nullptr ? aux_l->color() : 0x00);

        auto                    aux_r = this->session.items.auxiliary(equipment::position::RIGHT); // 보조2
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
                  .write_u8(this->session.option(fb::game::options::GROUP))
                  .write_u8(this->session.option(fb::game::options::TRADE))
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
    const fb::game::session&    session;

public:
    option(const fb::game::session& session) : fb::protocol::base::header(0x23),
        session(session)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->session.option(options::WEATHER_EFFECT)) // weather
                  .write_u8(this->session.option(options::MAGIC_EFFECT)) // magic effect
                  .write_u8(this->session.option(options::ROAR_WORLDS)) // listen news
                  .write_u8(this->session.option(options::FAST_MOVE)) // fast move
                  .write_u8(this->session.option(options::EFFECT_SOUND)) // effect sound
                  .write_u8(0x00);
    }
};

class throws : public fb::protocol::base::header
{
public:
    const fb::game::session&    session;
    const fb::game::item&       item;
    const point16_t             to;

public:
    throws(const fb::game::session& session, const fb::game::item& item, const point16_t& to) : fb::protocol::base::header(0x16),
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
    const fb::game::session&        me;
    const fb::game::action          value;
    const fb::game::duration        duration;
    const uint8_t                   sound;

public:
    action(const fb::game::session& me, fb::game::action value, fb::game::duration duration, uint8_t sound = 0x00) : fb::protocol::base::header(0x1A),
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