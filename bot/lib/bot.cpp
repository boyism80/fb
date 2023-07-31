#include <fb/bot/bot.h>

using namespace fb::bot;

base_bot::base_bot(bot_container& owner)
	 : fb::base::socket<void*>(owner.context(), 
	 	std::bind(&base_bot::on_receive, this, std::placeholders::_1),
	 	std::bind(&base_bot::on_closed, this, std::placeholders::_1)), _owner(owner)
{}

base_bot::~base_bot()
{
    
}

void base_bot::on_receive(fb::base::socket<>& socket)
{
	static constexpr uint8_t    base_size     = sizeof(uint8_t) + sizeof(uint16_t);
    auto&                       in_stream     = socket.in_stream();
	
    while (true)
    {
        try
        {
            if (in_stream.readable_size() < base_size)
                break;

            auto                head = in_stream.read_u8();
            if (head != 0xAA)
                throw std::exception();

            auto                size = in_stream.read_u16(buffer::endian::BIG);
            if (size > in_stream.capacity())
                throw std::exception();

            if (in_stream.readable_size() < size)
                break;

            auto cmd = in_stream.read_8();
            if (this->_handler_dict.contains(cmd))
            {
                this->_handler_dict[cmd]();
            }

            in_stream.reset();
            in_stream.shift(base_size + size);
            in_stream.flush();
        }
        catch (std::exception&)
        {
            in_stream.clear();
            break;
        }
        catch (...)
        {
            in_stream.clear();
            break;
        }
    }

    in_stream.reset();
}

void base_bot::on_closed(fb::base::socket<>& socket)
{}

gateway_bot::gateway_bot(bot_container& owner) : base_bot(owner)
{
    this->bind<fb::protocol::gateway::response::welcome>(0x7E, std::bind(&gateway_bot::handle_welcome, this, std::placeholders::_1));
}

gateway_bot::~gateway_bot()
{}

void gateway_bot::handle_welcome(const fb::protocol::gateway::response::welcome& response)
{
    std::cout << "dd" << std::endl;
}

login_bot::login_bot(bot_container& owner) : base_bot(owner)
{}

login_bot::~login_bot()
{}

game_bot::game_bot(bot_container& owner) : base_bot(owner)
{}

game_bot::~game_bot()
{}

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