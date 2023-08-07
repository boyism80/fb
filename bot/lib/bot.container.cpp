#include <fb/bot/bot.h>
#include <fb/core/thread.h>

using namespace fb::bot;

bot_container::bot_container(boost::asio::io_context& context) : _context(context)
{
	auto fn = [this]
	{
		constexpr auto term = 1000ms;

		while(this->_running)
		{
			auto now = fb::thread::now();
			for (auto& [k, v] : this->_bots)
			{
				v->on_timer(now);
			}
			auto elapsed = fb::thread::now() - now;
			if(elapsed < term)
				std::this_thread::sleep_for(term - elapsed);
		}
	};

	this->_running = true;
	fb::async::launch(fn);
}

bot_container::~bot_container()
{
	
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