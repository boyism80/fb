#include <fb/game/clan.member.h>

#include <string>
#include <string_view>

using namespace fb::game;

clan_member::clan_member(std::string_view name, CLAN_ROLE role) :
    name(std::string(name)),
    role(role)
{ }