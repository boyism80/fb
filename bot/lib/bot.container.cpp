#include <fb/bot/bot.h>

using namespace fb::bot;

bot_container::bot_container(boost::asio::io_context& context) : _context(context)
{}

bot_container::~bot_container()
{
    for (auto& [k, v] : this->_bots)
    {
        delete v;
    }
    this->_bots.clear();
}

boost::asio::io_context& bot_container::context() const
{
	return this->_context;
} 

void bot_container::close(base_bot& bot)
{
	auto fd = bot.fd();
	auto i = this->_bots.find(fd);
	if(i == this->_bots.end())
		return;

	delete i->second;
	this->_bots.erase(i);
}