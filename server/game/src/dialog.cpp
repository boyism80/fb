#include <fb/game/character.h>
#include <fb/game/dialog.h>

#include <cstdint>
#include <memory>

using namespace fb::game::dialog;

std::unique_ptr<fb::game::appearance> speaker::look(const character& viewer) const
{
    if (this->custom != nullptr)
        return std::make_unique<fb::game::character_appearance<>>(*this->custom);
    else if (this->object != nullptr)
        return fb::game::appearance_factory::create(*this->object, viewer);
    else if (this->model != nullptr)
        return fb::game::appearance_factory::create(*this->model, viewer);
    else
        return nullptr;
}

uint32_t speaker::oid() const
{
    return this->object != nullptr && this->object->portrait() == false ? this->object->oid() : 0xFFFFFFFD;
}
