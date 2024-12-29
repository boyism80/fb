#ifndef __PROTOCOL_RESPONSE_GAME_SESSION_H__
#define __PROTOCOL_RESPONSE_GAME_SESSION_H__

#include <fb/protocol/protocol.h>
#include <fb/model/model.h>
#ifndef BOT
#include <character.h>
#include <clan.h>
#include <group.h>
#else
#include <enum_ext.h>
#endif

#ifndef BOT
using namespace fb::game;
#else
using namespace fb::model;
#endif

namespace fb::protocol::game::response::character {

class message : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x0A;

public:
#ifndef BOT
    const std::string  text;
    const MESSAGE_TYPE type;
#else

#endif

public:
#ifndef BOT
    message(const std::string& text, MESSAGE_TYPE type) :
        text(text),
        type(type)
    { }
#else
    message() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(static_cast<uint8_t>(this->type));
        writer.write<std::string, uint16_t>(this->text);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
};

class show : public fb::protocol::base::header
{
public:
#ifndef BOT
    const fb::game::character& ch;
    const fb::game::object&    to;
    const bool                 light;
#else

#endif

public:
#ifndef BOT
    show(const fb::game::character& ch, const fb::game::object& to, bool light = false) :
        ch(ch),
        to(to),
        light(light)
    { }

    show(const show&) = delete;
#else
    show() = default;
#endif

private:
#ifndef BOT
    bool clock_visible() const
    {
        if (&this->ch == &this->to)
            return true;

        if (this->to.is(OBJECT_TYPE::CHARACTER) == false)
            return false;

        // TODO: to 에게 걸린 버프가 있어서 그게 투명 다 감지하는 버프면
        // return true

        auto& mine = this->ch.group();
        if (mine == nullptr)
            return false;

        auto& your = static_cast<const fb::game::character&>(this->to).group();
        if (your == nullptr)
            return false;

        return mine.get() == your.get();
    }
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        auto map = this->ch.map();
        if (map == nullptr)
            co_return;

        if (this->light == false)
        {
            writer.write<uint8_t>(0x33);                                       // id
            writer.write<uint16_t>(this->ch.x());                              // x
            writer.write<uint16_t>(this->ch.y());                              // y
            writer.write<uint8_t>(static_cast<uint8_t>(this->ch.direction())); // side
        }
        else
        {
            writer.write<uint8_t>(0x1D); // id
        }

        writer.write<uint32_t>(this->ch.sequence());
        writer.write<uint8_t>(this->ch.state() == STATE::DISGUISE);  // 변신유무
        writer.write<uint8_t>(static_cast<uint8_t>(this->ch.sex())); // sex

        switch (this->ch.state())
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
            writer.write<uint8_t>(static_cast<uint8_t>(this->ch.state()));
        }
        break;
        }

        if (this->ch.state() == STATE::DISGUISE)
        {
            writer.write<uint16_t>(this->ch.disguise().value());
            writer.write<uint8_t>(this->ch.current_armor_color());
        }
        else
        {
            writer.write<uint16_t>(this->ch.look()); // face
            writer.write<uint8_t>(this->ch.color()); // hair color

            auto armor = this->ch.items.armor();
            if (armor != nullptr)
            {
                writer.write<uint8_t>((uint8_t)armor->based<fb::model::armor>().dress);
                writer.write<uint8_t>(ch.current_armor_color());
            }
            else
            {
                writer.write<uint8_t>(static_cast<uint8_t>(this->ch.sex())); // sex
                writer.write<uint8_t>(0x00);
            }

            auto weapon = this->ch.items.weapon();
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

            auto shield = this->ch.items.shield();
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

        writer.write<uint8_t>(0x04);                         // head mark
        writer.write<std::string, uint8_t>(this->ch.name()); // name
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
};

class id : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x05;

public:
#ifndef BOT
    const fb::game::character& ch;
#else
    uint32_t sequence  = 0;
    uint32_t direction = 0;
    uint8_t  cls       = 0;
#endif

public:
#ifndef BOT
    id(const fb::game::character& ch) :
        ch(ch)
    { }
#else
    id() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint32_t>(this->ch.sequence());
        writer.write<uint32_t>(static_cast<uint32_t>(this->ch.direction())); // side
        writer.write<uint8_t>(static_cast<uint8_t>(this->ch.cls()));         // class
        writer.write<uint16_t>(0x00);
        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
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
    STATE_LEVEL level;
    uint8_t     ch_nation       = 0;
    uint8_t     ch_creature     = 0;
    uint8_t     ch_level        = 0;
    uint32_t    ch_base_hp      = 0;
    uint32_t    ch_base_mp      = 0;
    uint8_t     ch_strength     = 0;
    uint8_t     ch_intelligence = 0;
    uint8_t     ch_dexteritry   = 0;
    uint32_t    ch_hp           = 0;
    uint32_t    ch_mp           = 0;
    uint32_t    ch_exp          = 0;
    uint32_t    ch_money        = 0;
    uint32_t    ch_condition    = 0;
    uint8_t     ch_mail         = 0;
    uint8_t     ch_fast_move    = 0;
#else
    const fb::game::character& ch;
    const STATE_LEVEL          level;
#endif

public:
#ifdef BOT
    state() = default;
#else
    state(const fb::game::character& ch, STATE_LEVEL level) :
        ch(ch),
        level(level)
    { }
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(static_cast<uint8_t>(this->level));

        if (enum_in(this->level, STATE_LEVEL::BASED))
        {
            writer.write<uint8_t>(static_cast<uint8_t>(this->ch.nation()));   // nation
            writer.write<uint8_t>(static_cast<uint8_t>(this->ch.creature())); // creature
            writer.write<uint8_t>(0x00);                                      // Unknown (clan?)
            writer.write<uint8_t>(this->ch.level());                          // level
            writer.write<uint32_t>(this->ch.base_hp());                       // base hp
            writer.write<uint32_t>(this->ch.base_mp());                       // base mp
            writer.write<uint8_t>(this->ch.strength());
            writer.write<uint8_t>(this->ch.intelligence());
            writer.write<uint8_t>(0x03);
            writer.write<uint8_t>(0x03);
            writer.write<uint8_t>(this->ch.dexteritry());
            writer.write<uint8_t>(0x03);
            writer.write<uint32_t>(0x00);
            writer.write<uint8_t>(0x00);
        }

        if (enum_in(this->level, STATE_LEVEL::HP_MP))
        {
            writer.write<uint32_t>(this->ch.hp()); // current hp
            writer.write<uint32_t>(this->ch.mp()); // current mp
        }

        if (enum_in(this->level, STATE_LEVEL::EXP_MONEY))
        {
            writer.write<uint32_t>(this->ch.exp());   // exp
            writer.write<uint32_t>(this->ch.money()); // money
        }

        if (enum_in(this->level, STATE_LEVEL::CONDITION))
        {
            writer.write<uint8_t>(this->ch.condition_contains(CONDITION::MOVE));  // condition::move
            writer.write<uint8_t>(this->ch.condition_contains(CONDITION::SIGHT)); // condition::sight
            writer.write<uint8_t>(this->ch.condition_contains(CONDITION::HEAR));  // condition::hear?
            writer.write<uint8_t>(this->ch.condition_contains(CONDITION::ORAL));  // condition:oral
            writer.write<uint8_t>(this->ch.condition_contains(CONDITION::MAP));   // condition:map?
        }

        writer.write<uint8_t>(0x00); // mail count
        writer.write<uint8_t>(true); // fast move
        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->level = static_cast<fb::model::enum_value::STATE_LEVEL>(reader.read<uint8_t>());
        if (enum_in(this->level, STATE_LEVEL::BASED))
        {
            this->ch_nation   = reader.read<uint8_t>();
            this->ch_creature = reader.read<uint8_t>();
            reader.read<uint8_t>();
            this->ch_level        = reader.read<uint8_t>();
            this->ch_base_hp      = reader.read<uint32_t>();
            this->ch_base_mp      = reader.read<uint32_t>();
            this->ch_strength     = reader.read<uint8_t>();
            this->ch_intelligence = reader.read<uint8_t>();
            reader.read<uint8_t>();
            reader.read<uint8_t>();
            this->ch_dexteritry = reader.read<uint8_t>();
            reader.read<uint8_t>();
            reader.read<uint32_t>();
            reader.read<uint8_t>();
        }

        if (enum_in(this->level, STATE_LEVEL::HP_MP))
        {
            this->ch_hp = reader.read<uint32_t>();
            this->ch_mp = reader.read<uint32_t>();
        }

        if (enum_in(this->level, STATE_LEVEL::EXP_MONEY))
        {
            this->ch_exp   = reader.read<uint32_t>();
            this->ch_money = reader.read<uint32_t>();
        }

        if (enum_in(this->level, STATE_LEVEL::CONDITION))
        {
            if (reader.read<uint8_t>())
                this->ch_condition |= (uint32_t)CONDITION::MOVE;
            if (reader.read<uint8_t>())
                this->ch_condition |= (uint32_t)CONDITION::SIGHT;
            if (reader.read<uint8_t>())
                this->ch_condition |= (uint32_t)CONDITION::HEAR;
            if (reader.read<uint8_t>())
                this->ch_condition |= (uint32_t)CONDITION::ORAL;
            if (reader.read<uint8_t>())
                this->ch_condition |= (uint32_t)CONDITION::MAP;
        }

        this->ch_mail      = reader.read<uint8_t>();
        this->ch_fast_move = reader.read<uint8_t>();
        std::ignore        = reader.read<uint8_t>();
    }
#endif
};

class position : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x04;

public:
#ifndef BOT
    const fb::game::character& ch;
#else
    point<uint16_t> abs;
    point<uint16_t> rel;
#endif

public:
#ifndef BOT
    position(const fb::game::character& ch) :
        ch(ch)
    { }
#else
    position() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint16_t>(this->ch.x()); // 실제 x 좌표
        writer.write<uint16_t>(this->ch.y()); // 실제 y 좌표

        auto map = this->ch.map();
        if (map->width() < fb::game::map::MAX_SCREEN_WIDTH)
            writer.write<uint16_t>(this->ch.x() + fb::game::map::HALF_SCREEN_WIDTH - (map->width() / 2));
        else if (this->ch.x() < fb::game::map::HALF_SCREEN_WIDTH)
            writer.write<uint16_t>(this->ch.x());
        else if (this->ch.x() >= map->width() - fb::game::map::HALF_SCREEN_WIDTH)
            writer.write<uint16_t>(this->ch.x() + fb::game::map::MAX_SCREEN_WIDTH - map->width());
        else
            writer.write<uint16_t>(fb::game::map::HALF_SCREEN_WIDTH);

        // 스크린에서의 y 좌표
        if (map->height() < fb::game::map::MAX_SCREEN_HEIGHT)
            writer.write<uint16_t>(this->ch.y() + fb::game::map::HALF_SCREEN_HEIGHT - (map->height() / 2));
        else if (this->ch.y() < fb::game::map::HALF_SCREEN_HEIGHT)
            writer.write<uint16_t>(this->ch.y());
        else if (this->ch.y() >= (map->height() - fb::game::map::HALF_SCREEN_HEIGHT))
            writer.write<uint16_t>(this->ch.y() + fb::game::map::MAX_SCREEN_HEIGHT - map->height());
        else
            writer.write<uint16_t>(fb::game::map::HALF_SCREEN_HEIGHT);

        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
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
#ifndef BOT
    fb::game::character&    ch;
    const fb::model::model& model;
#else

#endif

public:
#ifndef BOT
    internal_info(fb::game::character& ch, const fb::model::model& model) :
        ch(ch),
        model(model)
    { }
#else
    internal_info() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>((uint8_t)this->ch.defensive_physical());
        writer.write<uint8_t>(this->ch.damage());
        writer.write<uint8_t>(this->ch.hit());

        auto& clan_lock_ptr = this->ch.clan();
        if (clan_lock_ptr != nullptr)
        {
            clan_lock_ptr->lock([&writer](auto& clan) {
                writer.write<std::string>(clan.name());
                writer.write<std::string>(clan.title().value_or(""));
            });
        }
        else
        {
            writer.write<std::string>("");
            writer.write<std::string>("");
        }
        writer.write<std::string>(this->ch.title());

        auto& shared_group_lock = this->ch.group();
        if (shared_group_lock != nullptr)
        {
            shared_group_lock->lock([&writer](auto& group) {
                if (group.inited())
                {
                    auto sstream = std::stringstream();
                    sstream << "그룹원" << std::endl << "  * " << group.master() << std::endl;

                    for (auto& member : group.members())
                        sstream << "    " << member << std::endl;
                    writer.write<std::string>(sstream.str());
                }
                else
                {
                    writer.write<std::string>("그룹 정보 가져오는중.");
                }
            });
        }
        else
        {
            writer.write<std::string>("그룹 없음.");
        }
        writer.write<uint8_t>(this->ch.option(SETTING::GROUP));

        uint32_t remained_exp = this->ch.experience_remained();
        writer.write<uint32_t>(remained_exp);

        auto& class_name = model.promotion[this->ch.cls()][this->ch.promotion()].name;
        writer.write<std::string>(class_name);

        fb::game::equipment* equipments[] = {this->ch.items.helmet(),
                                             this->ch.items.ring(EQUIPMENT_POSITION::LEFT),
                                             this->ch.items.ring(EQUIPMENT_POSITION::RIGHT),
                                             this->ch.items.auxiliary(EQUIPMENT_POSITION::LEFT),
                                             this->ch.items.auxiliary(EQUIPMENT_POSITION::RIGHT)};
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
        writer.write<uint8_t>(this->ch.option(SETTING::TRADE));
        writer.write<uint8_t>(this->ch.option(SETTING::PK_PROTECT));

        writer.write<uint8_t>((uint8_t)this->ch.traces.size());
        for (auto& [_, trace] : this->ch.traces)
        {
            auto& model = trace->model;
            writer.write<uint8_t>(model.look);
            writer.write<uint8_t>(model.color);
            writer.write<std::string>(trace->text.value_or(model.text));
        }
        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
};

class external_info : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x34;

public:
#ifndef BOT
    fb::game::character&    ch;
    const fb::model::model& model;
#else

#endif

public:
#ifndef BOT
    external_info(fb::game::character& ch, const fb::model::model& model) :
        ch(ch),
        model(model)
    { }
#else
    external_info() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<std::string>(this->ch.title());

        auto& clan_lock = this->ch.clan();
        if (clan_lock == nullptr)
        {
            writer.write<std::string>("");
            writer.write<std::string>("");
        }
        else
        {
            clan_lock->lock([&writer](auto& clan) {
                writer.write<std::string>(clan.name());
                writer.write<std::string>(clan.title().value_or(""));
            });
        }

        // 클래스 이름
        const auto& class_name = model.promotion[this->ch.cls()][this->ch.promotion()].name;
        writer.write<std::string>(class_name);      // 직업
        writer.write<std::string>(this->ch.name()); // 이름

        auto disguised = (this->ch.state() == STATE::DISGUISE);
        writer.write<uint8_t>(disguised);
        writer.write<uint8_t>(static_cast<uint8_t>(this->ch.sex()));
        writer.write<uint8_t>(static_cast<uint8_t>(this->ch.state()));

        auto armor  = this->ch.items.armor();  // 갑옷
        auto weapon = this->ch.items.weapon(); // 무기
        auto shield = this->ch.items.shield(); // 방패
        if (disguised)
        {
            writer.write<uint16_t>(this->ch.disguise().value());
            writer.write<uint8_t>(this->ch.current_armor_color());
        }
        else
        {
            writer.write<uint16_t>(this->ch.look());
            writer.write<uint8_t>(this->ch.color());

            writer.write<uint8_t>(armor != nullptr ? armor->based<fb::model::armor>().dress : 0xFF);
            writer.write<uint8_t>(this->ch.current_armor_color());

            writer.write<uint16_t>(weapon != nullptr ? weapon->based<fb::model::weapon>().dress : 0xFFFF);
            writer.write<uint8_t>(weapon != nullptr ? weapon->color() : 0x00);

            writer.write<uint8_t>(shield != nullptr ? shield->based<fb::model::shield>().dress : 0xFF);
            writer.write<uint8_t>(shield != nullptr ? shield->color() : 0x00);
        }

        // 장비정보
        auto sstream = std::stringstream();
        auto helmet  = this->ch.items.helmet(); // 투구
        writer.write<uint16_t>(helmet != nullptr ? helmet->look() : 0xFFFF);
        writer.write<uint8_t>(helmet != nullptr ? helmet->color() : 0x00);

        auto ring_l = this->ch.items.ring(EQUIPMENT_POSITION::LEFT); // 왼손
        writer.write<uint16_t>(ring_l != nullptr ? ring_l->look() : 0xFFFF);
        writer.write<uint8_t>(ring_l != nullptr ? ring_l->color() : 0x00);

        auto ring_r = this->ch.items.ring(EQUIPMENT_POSITION::RIGHT); // 오른손
        writer.write<uint16_t>(ring_r != nullptr ? ring_r->look() : 0xFFFF);
        writer.write<uint8_t>(ring_r != nullptr ? ring_r->color() : 0x00);

        auto aux_l = this->ch.items.auxiliary(EQUIPMENT_POSITION::LEFT); // 보조1
        writer.write<uint16_t>(aux_l != nullptr ? aux_l->look() : 0xFFFF);
        writer.write<uint8_t>(aux_l != nullptr ? aux_l->color() : 0x00);

        auto aux_r = this->ch.items.auxiliary(EQUIPMENT_POSITION::RIGHT); // 보조2
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

        writer.write<uint32_t>(this->ch.sequence());
        writer.write<uint8_t>(this->ch.option(SETTING::GROUP));
        writer.write<uint8_t>(this->ch.option(SETTING::TRADE));
        writer.write<uint32_t>(0x00000000); // unknown

        // 업적
        writer.write<uint8_t>((uint8_t)this->ch.traces.size());
        for (auto& [_, trace] : this->ch.traces)
        {
            auto& model = trace->model;
            writer.write<uint8_t>(model.look);
            writer.write<uint8_t>(model.color);
            writer.write<std::string>(trace->text.value_or(model.text));
        }
        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
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
    const fb::game::character& ch;
#endif

public:
#ifdef BOT
    option() = default;
#else
    option(const fb::game::character& ch) :
        ch(ch)
    { }
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(this->ch.option(SETTING::WEATHER_EFFECT)); // weather
        writer.write<uint8_t>(this->ch.option(SETTING::MAGIC_EFFECT));   // magic effect
        writer.write<uint8_t>(this->ch.option(SETTING::ROAR_WORLDS));    // listen news
        writer.write<uint8_t>(this->ch.option(SETTING::FAST_MOVE));      // fast move
        writer.write<uint8_t>(this->ch.option(SETTING::EFFECT_SOUND));   // effect sound
        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
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
#ifndef BOT
    const fb::game::character& ch;
    const fb::game::item&      item;
    const point<uint16_t>      to;
#else

#endif

public:
#ifndef BOT
    throws(const fb::game::character& ch, const fb::game::item& item, const point<uint16_t>& to) :
        ch(ch),
        item(item),
        to(to)
    { }
#else
    throws() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint32_t>(this->ch.sequence());
        writer.write<uint16_t>(this->item.look());
        writer.write<uint8_t>(this->item.color());
        writer.write<uint32_t>(this->item.sequence());
        writer.write<uint16_t>(this->ch.x());
        writer.write<uint16_t>(this->ch.y());
        writer.write<uint16_t>(this->to.x);
        writer.write<uint16_t>(this->to.y);
        writer.write<uint32_t>(0x00000000);
        writer.write<uint8_t>(0x02);
        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
};

class action : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x1A;

public:
#ifndef BOT
    const fb::game::character& me;
    const ACTION               value;
    const DURATION             duration;
    const uint8_t              sound;
#else

#endif

public:
#ifndef BOT
    action(const fb::game::character& me, ACTION value, DURATION duration, uint8_t sound = 0x00) :
        me(me),
        value(value),
        duration(duration),
        sound(sound)
    { }
#else
    action() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint32_t>(this->me.sequence());
        writer.write<uint8_t>(static_cast<uint8_t>(this->value));      // type
        writer.write<uint16_t>(static_cast<uint16_t>(this->duration)); // duration
        writer.write<uint8_t>(this->sound);                            // sound
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
};

} // namespace fb::protocol::game::response::character

#endif // !__PROTOCOL_RESPONSE_GAME_SESSION_H__