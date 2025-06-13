#include <fb/game/protocol/character/external_info.h>

namespace fb::protocol::game::response {

#ifndef BOT
external_info::external_info(const fb::game::character& ch, const fb::model::model& model) :
    ch(ch),
    model(model)
{ }
#endif

#ifndef BOT
async::task<void> external_info::serialize(fb::stream_writer<big_endian>& writer) const
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
        clan_lock->read([&writer](auto& clan) {
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
    writer.write<uint8_t>(this->ch.option(OPTION::GROUP));
    writer.write<uint8_t>(this->ch.option(OPTION::TRADE));
    writer.write<uint32_t>(0x00000000); // unknown

    // 업적
    writer.write<uint8_t>((uint8_t)this->ch.achievements.size());
    for (auto& [_, achievement] : this->ch.achievements)
    {
        auto& model = achievement->model;
        writer.write<uint8_t>(model.look);
        writer.write<uint8_t>(model.color);
        writer.write<std::string>(achievement->text.value_or(model.text));
    }
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> external_info::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: deserialize bytes
}
#endif
} // namespace fb::protocol::game::response
