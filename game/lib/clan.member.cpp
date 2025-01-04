#include <fb/game/clan.member.h>

using namespace fb::game;

clan_member::clan_member(const std::string& name, CLAN_POSITION position) :
    name(name),
    position(position)
{ }