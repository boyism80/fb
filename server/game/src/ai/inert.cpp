#include <fb/game/ai/inert.h>

using namespace fb::game;

bool inert_ai::execute(mob&, const datetime&)
{
    return true;
}

MOB_ATTACK_TYPE inert_ai::get_type() const
{
    return MOB_ATTACK_TYPE::INERT;
}

void inert_ai::on_damage(mob&, std::shared_ptr<life>, const datetime&)
{ }
