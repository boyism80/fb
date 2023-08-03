#include <fb/bot/bot.h>

using namespace fb::bot;

game_bot::game_bot(bot_container& owner) : base_bot(owner)
{}

game_bot::game_bot(bot_container& owner, const fb::buffer& params) : base_bot(owner)
{}

game_bot::~game_bot()
{}