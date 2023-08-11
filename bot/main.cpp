#include <thread>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <fb/bot/bot.h>

int main(int, char**)
{
	constexpr auto io_size = 2;
	constexpr auto count = 100;

	boost::asio::io_context io1, io2;
	boost::asio::executor_work_guard<decltype(io1.get_executor())> work1{ io1.get_executor() };
	boost::asio::executor_work_guard<decltype(io2.get_executor())> work2{ io2.get_executor() };

	auto bots1 = fb::bot::bot_container(io1);
	auto bots2 = fb::bot::bot_container(io2);
	auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 3001);

	for (int i = 0; i < count / io_size; i++)
	{
		auto bot = bots1.create<fb::bot::gateway_bot>();
		bot->connect(endpoint);
	}

	for (int i = 0; i < count / io_size; i++)
	{
		auto bot = bots2.create<fb::bot::gateway_bot>();
		bot->connect(endpoint);
	}

	boost::asio::thread_pool thread_pool(io_size);
	post(thread_pool, [&io1] { io1.run(); });
	post(thread_pool, [&io2] { io2.run(); });
	thread_pool.join();

	return 0;
}
