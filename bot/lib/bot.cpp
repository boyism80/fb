#include <fb/bot/bot.h>

using namespace fb::bot;

base_bot::base_bot(bot_container& owner, uint32_t id): fb::awaitable_socket<void*>(owner.context(), 
    std::bind(&base_bot::on_receive, this, std::placeholders::_1),
    std::bind(&base_bot::on_closed, this, std::placeholders::_1)), _owner(owner), id(id)
{}

base_bot::~base_bot()
{ }

fb::task<void> base_bot::on_receive(fb::base::socket<>& socket)
{
    static constexpr uint8_t    base_size = sizeof(uint8_t) + sizeof(uint16_t);

    co_await socket.in_stream<fb::task<void>>([this, &socket] (auto& in_stream) -> fb::task<void>
    {
        while (true)
        {
            try
            {
                if (in_stream.readable_size() < base_size)
                    break;

                auto                head = in_stream.read_u8();
                if (head != 0xAA)
                    throw std::runtime_error("header mismatch");

                auto                size = in_stream.read_u16(buffer::endian::BIG);
                if (size > in_stream.capacity())
                    throw std::runtime_error("limit packet size");

                if (in_stream.readable_size() < size)
                    break;

                auto cmd = in_stream.read_8();
                if (this->decrypt_policy(cmd))
                {
                    size = this->_cryptor.decrypt(in_stream, in_stream.offset() - 1, size);
                }

                if (this->_handler.contains(cmd))
                {
                    co_await this->_handler[cmd]();
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
    });
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

fb::task<void> base_bot::on_closed(fb::base::socket<>& socket)
{
    this->on_disconnected();
    this->_owner.remove(*this);
    co_return;
}

bool base_bot::on_encrypt(fb::ostream& out)
{
    return this->_cryptor.encrypt(out);
}

bool base_bot::on_wrap(fb::ostream& out)
{
    return this->_cryptor.wrap(out);
}

bool base_bot::decrypt_policy(int cmd) const
{
    switch(cmd)
    {
        case 0x03:
        return false;

        default:
        return true;
    }
}