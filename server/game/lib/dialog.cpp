#include <fb/game/dialog.h>
#include <fb/game/server.h>

using namespace fb::game::dialog;

void character_portrait::serialize(fb::stream_writer<big_endian>& writer) const
{
    writer.write<uint8_t>(0x01);
    writer.write<uint8_t>(0x00);
    writer.write<uint8_t>(static_cast<uint8_t>(this->sex));
    writer.write<uint8_t>(static_cast<uint8_t>(this->state));
    writer.write<uint16_t>(this->hair);
    writer.write<uint8_t>(this->hair_color);
    writer.write<uint8_t>(this->armor.value_or(static_cast<uint8_t>(this->sex)));
    writer.write<uint8_t>(this->armor_color.value_or(0x00));
    writer.write<uint16_t>(this->weapon.value_or(0xFFFF));
    writer.write<uint8_t>(this->weapon_color.value_or(0x00));
    writer.write<uint8_t>(this->shield.value_or(0xFF));
    writer.write<uint8_t>(this->shield_color.value_or(0x00));
    writer.write<uint32_t>(0x00);
}

void object_portrait::serialize(fb::stream_writer<big_endian>& writer) const
{
    writer.write<uint8_t>(this->look > 0xBFFF ? 0x02 : 0x01);
    writer.write<uint8_t>(0x01);
    writer.write<uint16_t>(this->look);
    writer.write<uint8_t>(this->color);
    writer.write<uint8_t>(this->look > 0xBFFF ? 0x02 : 0x01);
    writer.write<uint16_t>(this->look);
    writer.write<uint8_t>(this->color);
}

std::unique_ptr<portrait> portrait_factory::create(fb::model::model& model, const fb::model::object& obj)
{
    switch (obj.what())
    {
    case fb::model::enum_value::OBJECT_TYPE::NPC:
    {
        auto& npc_model = static_cast<const fb::model::npc&>(obj);
        if (npc_model.preset.has_value())
        {
            auto& preset      = model.preset[npc_model.preset.value()];
            auto  ptr         = new character_portrait();
            ptr->sex          = preset.sex;
            ptr->state        = preset.state;
            ptr->hair         = preset.hair;
            ptr->hair_color   = preset.hair_color;
            ptr->weapon       = preset.weapon;
            ptr->weapon_color = preset.weapon_color;
            ptr->armor        = preset.armor;
            ptr->armor_color  = preset.armor_color;
            ptr->shield       = preset.shield;
            ptr->shield_color = preset.shield_color;
            return std::unique_ptr<portrait>(ptr);
        }
        else
        {
            auto ptr   = new object_portrait();
            ptr->look  = obj.look;
            ptr->color = obj.color;
            return std::unique_ptr<portrait>(ptr);
        }
    }

    default:
    {
        auto ptr   = new object_portrait();
        ptr->look  = obj.look;
        ptr->color = obj.color;
        return std::unique_ptr<portrait>(ptr);
    }
    }
}

std::unique_ptr<portrait> portrait_factory::create(const fb::game::object& obj)
{
    switch (obj.what())
    {
    case fb::model::enum_value::OBJECT_TYPE::CHARACTER:
    {
        auto& ch        = static_cast<const fb::game::character&>(obj);
        auto  ptr       = new character_portrait();
        ptr->sex        = ch.sex();
        ptr->state      = ch.state();
        ptr->hair       = ch.look();
        ptr->hair_color = ch.color();

        if (ch.items.weapon() != nullptr)
        {
            ptr->weapon       = ch.items.weapon()->based<fb::model::weapon>().dress;
            ptr->weapon_color = std::nullopt;
        }

        if (ch.items.armor() != nullptr)
        {
            ptr->armor       = ch.items.armor()->based<fb::model::armor>().dress;
            ptr->armor_color = ch.armor_color();
        }

        if (ch.items.shield() != nullptr)
        {
            ptr->shield       = ch.items.shield()->based<fb::model::shield>().dress;
            ptr->shield_color = std::nullopt;
        }

        return std::unique_ptr<portrait>(ptr);
    }
    default:
    {
        return create(obj.server.model, obj.based<fb::model::object>());
    }
    }
}