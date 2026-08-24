#include <fb/game/appearance.h>
#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/model/model.h>
#include <stdexcept>

using namespace fb::game;
using namespace fb::model::enum_value;
using fb::big_endian;
using table = fb::model::table;

namespace {

template <typename App>
void fill_appearance_from_character(App& app, const character& ch)
{
    app.gender      = ch.gender();
    app.state       = ch.state();
    app.hair        = ch.hair();
    app.hair_color  = ch.color();
    app.armor_color = ch.armor_color();
    app.disguise    = std::nullopt;

    if (ch.items.armor() != nullptr)
    {
        app.armor = static_cast<uint8_t>(ch.items.armor()->model().dress);
        if (app.armor_color.has_value() == false)
            app.armor_color = ch.items.armor()->model().color;
    }

    if (ch.items.weapon() != nullptr)
    {
        app.weapon       = ch.items.weapon()->model().dress;
        app.weapon_color = ch.weapon_color().value_or(static_cast<uint8_t>(ch.items.weapon()->color()));
    }

    if (ch.items.shield() != nullptr)
    {
        app.shield       = ch.items.shield()->model().dress;
        app.shield_color = ch.shield_color().value_or(ch.items.shield()->color());
    }

    app.speed = ch.stat.speed();
}

} // namespace

template <fb::protocol::CLIENT_VERSION V>
character_appearance<V> character_appearance<V>::from(const character& ch)
{
    auto app = character_appearance<V>{};
    fill_appearance_from_character(app, ch);
    return app;
}

character_appearance<fb::protocol::CLIENT_VERSION::v651>
character_appearance<fb::protocol::CLIENT_VERSION::v651>::from(const character& ch)
{
    auto app = character_appearance<fb::protocol::CLIENT_VERSION::v651>{};
    fill_appearance_from_character(app, ch);
    app.face    = ch.face();
    auto helmet = ch.items.helmet();
    if (helmet != nullptr && ch.option(OPTION::VISIBLE_HELMET))
    {
        app.helmet       = static_cast<uint8_t>(helmet->model().dress);
        app.helmet_color = helmet->color();
    }
    return app;
}

template <fb::protocol::CLIENT_VERSION V>
character_appearance<V>::character_appearance(GENDER                  gender,
                                              std::optional<STATE>    state,
                                              uint16_t                hair,
                                              std::optional<uint8_t>  hair_color,
                                              std::optional<uint16_t> weapon,
                                              std::optional<uint8_t>  weapon_color,
                                              std::optional<uint8_t>  armor,
                                              std::optional<uint8_t>  armor_color,
                                              std::optional<uint8_t>  shield,
                                              std::optional<uint8_t>  shield_color,
                                              std::optional<uint16_t> disguise) :
    gender(gender),
    state(state),
    hair(hair),
    hair_color(hair_color),
    weapon(weapon),
    weapon_color(weapon_color),
    armor(armor),
    armor_color(armor_color),
    shield(shield),
    shield_color(shield_color),
    disguise(disguise)
{ }

character_appearance<fb::protocol::CLIENT_VERSION::v651>::character_appearance(GENDER                  gender,
                                                                               std::optional<STATE>    state,
                                                                               uint16_t                hair,
                                                                               std::optional<uint8_t>  hair_color,
                                                                               std::optional<uint16_t> weapon,
                                                                               std::optional<uint8_t>  weapon_color,
                                                                               std::optional<uint8_t>  armor,
                                                                               std::optional<uint8_t>  armor_color,
                                                                               std::optional<uint8_t>  shield,
                                                                               std::optional<uint8_t>  shield_color,
                                                                               std::optional<uint16_t> disguise) :
    character_appearance(character_appearance<fb::protocol::CLIENT_VERSION::v550>(gender,
                                                                                  state,
                                                                                  hair,
                                                                                  hair_color,
                                                                                  weapon,
                                                                                  weapon_color,
                                                                                  armor,
                                                                                  armor_color,
                                                                                  shield,
                                                                                  shield_color,
                                                                                  disguise))
{ }

template <fb::protocol::CLIENT_VERSION V>
void character_appearance<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    writer.write<uint8_t>(0x01);
    writer.write<uint8_t>(0x00);
    writer.write<uint8_t>(static_cast<uint8_t>(this->gender));
    writer.write<uint8_t>(static_cast<uint8_t>(this->state.value_or(STATE::NORMAL)));
    writer.write<uint16_t>(this->hair);
    writer.write<uint8_t>(this->hair_color.value_or(0x00));
    writer.write<uint8_t>(this->armor.value_or(static_cast<uint8_t>(this->gender)));
    writer.write<uint8_t>(this->armor_color.value_or(0x00));
    writer.write<uint16_t>(this->weapon.value_or(0xFFFF));
    writer.write<uint8_t>(this->weapon_color.value_or(0x00));
    writer.write<uint8_t>(this->shield.value_or(0xFF));
    writer.write<uint8_t>(this->shield_color.value_or(0x00));
    writer.write<uint32_t>(0x00);
}

void character_appearance<fb::protocol::CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    writer.write<uint8_t>(0x01);
    writer.write<uint8_t>(0x00);
    writer.write<uint8_t>(static_cast<uint8_t>(this->gender));
    writer.write<uint8_t>(static_cast<uint8_t>(this->state.value_or(STATE::NORMAL)));
    writer.write<uint16_t>(this->hair);
    writer.write<uint8_t>(this->hair_color.value_or(0x00));
    writer.write<uint8_t>(this->armor.value_or(static_cast<uint8_t>(this->gender)));
    writer.write<uint8_t>(this->armor_color.value_or(0x00));
    writer.write<uint16_t>(this->weapon.value_or(0xFFFF));
    writer.write<uint8_t>(this->weapon_color.value_or(0x00));
    writer.write<uint8_t>(this->shield.value_or(0xFF));
    writer.write<uint8_t>(this->shield_color.value_or(0x00));
    writer.write<uint32_t>(0x00);
}

void character_appearance<fb::protocol::CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer,
                                                                         fb::protocol::CLIENT_UI_MODE   ui_mode) const
{
    writer.write<uint8_t>(static_cast<uint8_t>(this->gender));
    writer.write<uint8_t>(static_cast<uint8_t>(this->state.value_or(STATE::NORMAL)));

    if (this->disguise.has_value())
    {
        writer.write<uint16_t>(this->disguise.value());
        writer.write<uint8_t>(this->hair_color.value_or(this->armor_color.value_or(0x00)));
        return;
    }

    writer.write<uint16_t>(this->ridable_id);
    writer.write<uint8_t>(walk_delay_from_speed(this->speed));
    if (ui_mode == fb::protocol::CLIENT_UI_MODE::NEW)
        writer.write<uint16_t>(static_cast<uint16_t>(200 + this->face));
    else
        writer.write<uint16_t>(this->hair);

    uint8_t slot7 = this->hair_style;
    if (slot7 == 0)
        slot7 = static_cast<uint8_t>(this->hair);
    writer.write<uint8_t>(slot7);
    writer.write<uint8_t>(this->face_hair_tint != 0 ? this->face_hair_tint : this->hair_color.value_or(0x00));
    writer.write<uint8_t>(this->armor.value_or(static_cast<uint8_t>(this->gender)));
    writer.write<uint8_t>(this->body_color != 0 ? this->body_color : this->armor_color.value_or(0x00));
    writer.write<uint16_t>(this->weapon.value_or(0xFFFF));
    writer.write<uint8_t>(this->weapon_color.value_or(0x00));
    writer.write<uint16_t>(this->shield.has_value() ? static_cast<uint16_t>(this->shield.value()) : 0xFFFF);
    writer.write<uint8_t>(this->shield_color.value_or(0x00));
    const auto show_helmet = (ui_mode == fb::protocol::CLIENT_UI_MODE::NEW) && this->helmet.has_value();
    writer.write<uint8_t>(show_helmet ? 1 : 0);
    writer.write<uint8_t>(show_helmet ? this->helmet.value() : 0);
    writer.write<uint8_t>(show_helmet ? this->helmet_color.value_or(0) : 0);
    writer.write<uint16_t>(this->accessory_pack);
    writer.write<uint8_t>(this->accessory_color);
}

template <fb::protocol::CLIENT_VERSION V>
void character_appearance<V>::to_lua(fb::lua::context* lua) const
{
    lua->new_table();
    if (this->disguise.has_value())
    {
        lua->pushstring("disguise");
        lua->pushinteger(static_cast<lua_Integer>(this->disguise.value()));
        lua->settable(-3);
    }
    lua->pushstring("hair");
    lua->pushinteger(static_cast<lua_Integer>(this->hair));
    lua->settable(-3);
    if (this->hair_color.has_value())
    {
        lua->pushstring("hair_color");
        lua->pushinteger(static_cast<lua_Integer>(this->hair_color.value()));
        lua->settable(-3);
    }
    lua->pushstring("gender");
    lua->pushinteger(static_cast<lua_Integer>(static_cast<uint8_t>(this->gender)));
    lua->settable(-3);
    if (this->state.has_value())
    {
        lua->pushstring("state");
        lua->pushinteger(static_cast<lua_Integer>(static_cast<uint8_t>(this->state.value())));
        lua->settable(-3);
    }
    if (this->weapon.has_value())
    {
        lua->pushstring("weapon");
        lua->pushinteger(static_cast<lua_Integer>(this->weapon.value()));
        lua->settable(-3);
    }
    if (this->weapon_color.has_value())
    {
        lua->pushstring("weapon_color");
        lua->pushinteger(static_cast<lua_Integer>(this->weapon_color.value()));
        lua->settable(-3);
    }
    if (this->armor.has_value())
    {
        lua->pushstring("armor");
        lua->pushinteger(static_cast<lua_Integer>(this->armor.value()));
        lua->settable(-3);
    }
    if (this->armor_color.has_value())
    {
        lua->pushstring("armor_color");
        lua->pushinteger(static_cast<lua_Integer>(this->armor_color.value()));
        lua->settable(-3);
    }
    if (this->shield.has_value())
    {
        lua->pushstring("shield");
        lua->pushinteger(static_cast<lua_Integer>(this->shield.value()));
        lua->settable(-3);
    }
    if (this->shield_color.has_value())
    {
        lua->pushstring("shield_color");
        lua->pushinteger(static_cast<lua_Integer>(this->shield_color.value()));
        lua->settable(-3);
    }
}

void character_appearance<fb::protocol::CLIENT_VERSION::v651>::to_lua(fb::lua::context* lua) const
{
    lua->new_table();
    if (this->disguise.has_value())
    {
        lua->pushstring("disguise");
        lua->pushinteger(static_cast<lua_Integer>(this->disguise.value()));
        lua->settable(-3);
    }
    lua->pushstring("hair");
    lua->pushinteger(static_cast<lua_Integer>(this->hair));
    lua->settable(-3);
    lua->pushstring("face");
    lua->pushinteger(static_cast<lua_Integer>(this->face));
    lua->settable(-3);
    if (this->hair_color.has_value())
    {
        lua->pushstring("hair_color");
        lua->pushinteger(static_cast<lua_Integer>(this->hair_color.value()));
        lua->settable(-3);
    }
    lua->pushstring("gender");
    lua->pushinteger(static_cast<lua_Integer>(static_cast<uint8_t>(this->gender)));
    lua->settable(-3);
    if (this->state.has_value())
    {
        lua->pushstring("state");
        lua->pushinteger(static_cast<lua_Integer>(static_cast<uint8_t>(this->state.value())));
        lua->settable(-3);
    }
    if (this->weapon.has_value())
    {
        lua->pushstring("weapon");
        lua->pushinteger(static_cast<lua_Integer>(this->weapon.value()));
        lua->settable(-3);
    }
    if (this->weapon_color.has_value())
    {
        lua->pushstring("weapon_color");
        lua->pushinteger(static_cast<lua_Integer>(this->weapon_color.value()));
        lua->settable(-3);
    }
    if (this->armor.has_value())
    {
        lua->pushstring("armor");
        lua->pushinteger(static_cast<lua_Integer>(this->armor.value()));
        lua->settable(-3);
    }
    if (this->armor_color.has_value())
    {
        lua->pushstring("armor_color");
        lua->pushinteger(static_cast<lua_Integer>(this->armor_color.value()));
        lua->settable(-3);
    }
    if (this->shield.has_value())
    {
        lua->pushstring("shield");
        lua->pushinteger(static_cast<lua_Integer>(this->shield.value()));
        lua->settable(-3);
    }
    if (this->shield_color.has_value())
    {
        lua->pushstring("shield_color");
        lua->pushinteger(static_cast<lua_Integer>(this->shield_color.value()));
        lua->settable(-3);
    }
    if (this->helmet.has_value())
    {
        lua->pushstring("helmet");
        lua->pushinteger(static_cast<lua_Integer>(this->helmet.value()));
        lua->settable(-3);
    }
    if (this->helmet_color.has_value())
    {
        lua->pushstring("helmet_color");
        lua->pushinteger(static_cast<lua_Integer>(this->helmet_color.value()));
        lua->settable(-3);
    }
}

object_appearance::object_appearance(uint16_t look, uint8_t color) :
    look(look),
    color(color)
{ }

object_appearance::object_appearance(const object_appearance& right) :
    look(right.look),
    color(right.color)
{ }

void object_appearance::serialize(fb::stream_writer<big_endian>& writer) const
{
    writer.write<uint8_t>(this->look > 0xBFFF ? 0x02 : 0x01);
    writer.write<uint8_t>(0x01);
    writer.write<uint16_t>(this->look);
    writer.write<uint8_t>(this->color);
    writer.write<uint8_t>(this->look > 0xBFFF ? 0x02 : 0x01);
    writer.write<uint16_t>(this->look);
    writer.write<uint8_t>(this->color);
}

void object_appearance::to_lua(fb::lua::context* lua) const
{
    lua->new_table();
    lua->pushstring("state");
    lua->pushinteger(static_cast<lua_Integer>(static_cast<uint8_t>(STATE::DISGUISE)));
    lua->settable(-3);
    lua->pushstring("disguise");
    lua->pushinteger(static_cast<lua_Integer>(this->look));
    lua->settable(-3);
    lua->pushstring("hair_color");
    lua->pushinteger(static_cast<lua_Integer>(this->color));
    lua->settable(-3);
}

std::unique_ptr<appearance> appearance_factory::create(const fb::model::object& obj)
{
    switch (obj.what())
    {
    case fb::model::enum_value::OBJECT_TYPE::NPC:
    {
        auto& npc_model = static_cast<const fb::model::npc&>(obj);
        if (npc_model.appearance.has_value())
        {
            auto  appearance_table = table::appearance;
            auto& app              = appearance_table[npc_model.appearance.value()];
            auto  ptr              = std::make_unique<character_appearance<>>();
            ptr->gender            = app.gender;
            ptr->state             = app.state;
            ptr->hair              = app.hair;
            ptr->hair_color        = app.hair_color;
            ptr->weapon            = app.weapon;
            ptr->weapon_color      = app.weapon_color;
            ptr->armor =
                app.armor.has_value() ? std::optional<uint8_t>{static_cast<uint8_t>(app.armor.value())} : std::nullopt;
            ptr->armor_color  = app.armor_color;
            ptr->shield       = app.shield;
            ptr->shield_color = app.shield_color;
            return std::move(ptr);
        }
        else
        {
            auto ptr   = std::make_unique<object_appearance>();
            ptr->look  = obj.look;
            ptr->color = obj.color;
            return std::move(ptr);
        }
    }

    default:
    {
        auto ptr   = std::make_unique<object_appearance>();
        ptr->look  = obj.look;
        ptr->color = obj.color;
        return std::move(ptr);
    }
    }
}

std::unique_ptr<appearance> appearance_factory::create(const fb::game::object& obj)
{
    switch (obj.what())
    {
    case fb::model::enum_value::OBJECT_TYPE::CHARACTER:
    {
        auto& ch = static_cast<const fb::game::character&>(obj);
        return std::make_unique<character_appearance<>>(character_appearance<>::from(ch));
    }
    default:
    {
        return create(obj.model());
    }
    }
}

std::unique_ptr<appearance> appearance_factory::create(const fb::game::object& obj, const fb::game::character& viewer)
{
    if (obj.is(OBJECT_TYPE::CHARACTER) == false)
        return create(obj);

    auto& subject = static_cast<const character&>(obj);
    auto  app     = character_appearance<>::from(subject);
    app.state     = subject.state_to(viewer, subject.state());
    return std::make_unique<character_appearance<>>(std::move(app));
}

std::shared_ptr<fb::game::appearance> fb::model::object::create_appearance() const
{
    throw std::runtime_error("create_appearance() not implemented");
}

std::shared_ptr<fb::game::appearance> fb::model::npc::create_appearance() const
{
    return std::make_shared<object_appearance>(this->look, this->color);
}

std::shared_ptr<fb::game::appearance> fb::model::mob::create_appearance() const
{
    return std::make_shared<object_appearance>(this->look, this->color);
}

template class character_appearance<fb::protocol::CLIENT_VERSION::v550>;
template class character_appearance<fb::protocol::CLIENT_VERSION::v565>;
