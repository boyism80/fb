#include <fb/bot/bot.h>

using namespace fb::bot;

base_bot::base_bot(bot_container& owner): fb::awaitable_socket<void*>(owner.context(), 
    std::bind(&base_bot::on_receive, this, std::placeholders::_1),
    std::bind(&base_bot::on_closed, this, std::placeholders::_1)), _owner(owner)
{}

base_bot::~base_bot()
{ }

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
            if (this->is_decrypt(cmd))
            {
                size = this->_cryptor.decrypt(in_stream, in_stream.offset() - 1, size);
            }

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

void base_bot::connect(const boost::asio::ip::tcp::endpoint& endpoint)
{
    this->async_connect(endpoint, [&](const auto& e) 
    {
        this->recv();
        this->on_connected();
    });
}

void base_bot::on_connected()
{ }

void base_bot::on_disconnected()
{ }

void base_bot::on_closed(fb::base::socket<>& socket)
{
    this->on_disconnected();
}

bool base_bot::on_encrypt(fb::ostream& out)
{
    return this->_cryptor.encrypt(out);
}

bool base_bot::on_wrap(fb::ostream& out)
{
    return this->_cryptor.wrap(out);
}

bool base_bot::is_decrypt(int cmd) const
{
    switch(cmd)
    {
        case 0x03:
        return false;

        default:
        return true;
    }
}