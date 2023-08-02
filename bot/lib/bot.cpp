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
    this->bind<fb::protocol::gateway::response::welcome>(std::bind(&gateway_bot::handle_welcome, this, std::placeholders::_1));
    this->bind<fb::protocol::gateway::response::crt>(std::bind(&gateway_bot::handle_crt, this, std::placeholders::_1));
    this->bind<fb::protocol::gateway::response::hosts>(std::bind(&gateway_bot::handle_hosts, this, std::placeholders::_1));
    this->bind<fb::protocol::response::transfer>(std::bind(&gateway_bot::handle_transfer, this, std::placeholders::_1));
}

gateway_bot::~gateway_bot()
{}

void gateway_bot::handle_welcome(const fb::protocol::gateway::response::welcome& response)
{
    this->send(fb::protocol::gateway::request::assert_version { 0x0226, 0xD7 }, false, true);
}

void gateway_bot::handle_crt(const fb::protocol::gateway::response::crt& response)
{
    this->_cryptor = response.cryptor;
    this->send(fb::protocol::gateway::request::entry_list { 0x01, 0 });
}

void gateway_bot::handle_hosts(const fb::protocol::gateway::response::hosts& response)
{
    this->send(fb::protocol::gateway::request::entry_list { 0x00, 1 });
}

void gateway_bot::handle_transfer(const fb::protocol::response::transfer& response)
{
    /*
        .write_u8(crt.type())
        .write_u8(cryptor::KEY_SIZE)
        .write(crt.key(), cryptor::KEY_SIZE)
        .write_u8(from);
    */
}

bool gateway_bot::on_encrypt(fb::ostream& out)
{
    return this->_cryptor.encrypt(out);
}

bool gateway_bot::on_wrap(fb::ostream& out)
{
    return this->_cryptor.wrap(out);
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