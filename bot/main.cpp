#include <thread>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <fb/bot/bot.h>

int main(int, char**)
{
	boost::asio::io_context io;
	auto bots = fb::bot::bot_container(io);
	auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 3001);
	auto bot = bots.create<fb::bot::gateway_bot>();
	bot->connect(endpoint);

	io.run();

	return 0;
}
