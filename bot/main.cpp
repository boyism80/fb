#include <thread>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <fb/bot/bot.h>

using namespace std;
using namespace boost::asio;

int main(int, char**)
{
	using guard_type = executor_work_guard<io_context::executor_type>;

	constexpr auto io_size = 10;
	constexpr auto count = 200;

	io_context ios[io_size];
	auto guards = vector<unique_ptr<guard_type>>();
	auto bot_containers = vector<unique_ptr<fb::bot::bot_container>>();
	for(auto& io : ios)
	{
		guards.push_back(std::make_unique<guard_type>(io.get_executor()));
		bot_containers.push_back(make_unique<fb::bot::bot_container>(io));
	}

	auto endpoint = ip::tcp::endpoint(ip::address::from_string("127.0.0.1"), 3001);
	for(auto& bots : bot_containers)
	{
		for(int i = 0; i < count / io_size; i++)
		{
			auto bot = bots->create<fb::bot::gateway_bot>();
			bot->connect(endpoint);
		}
	}
	
	thread_pool thread_pool(io_size);
	for(auto& io : ios)
	{
		post(thread_pool, [&io] { io.run(); });
	}
	thread_pool.join();

	return 0;
}
