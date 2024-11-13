#include <fb/bot/bot.h>

using namespace fb::bot;

base_bot::base_bot(bot_container& owner, uint32_t id) :
    fb::socket<void*>(owner.context(), std::bind(&base_bot::on_receive, this, std::placeholders::_1), std::bind(&base_bot::on_closed, this, std::placeholders::_1)),
    _owner(owner),
    id(id)
{ }

base_bot::~base_bot()
{ }

async::task<void> base_bot::on_receive(fb::socket<>& socket)
{
    static constexpr uint8_t base_size = sizeof(uint8_t) + sizeof(uint16_t);

    co_await socket.reader<async::task<void>>([this, &socket](auto& reader) -> async::task<void> {
        while (true)
        {
            try
            {
                if (reader.readable_size() < base_size)
                    break;

                auto head = reader.read<uint8_t>();
                if (head != 0xAA)
                    throw std::runtime_error("header mismatch");

                auto size = reader.read<uint16_t>();
                if (size > reader.capacity())
                    throw std::runtime_error("limit packet size");

                if (reader.readable_size() < size)
                    break;

                auto cmd = reader.read_8();
                if (this->decrypt_policy(cmd))
                {
                    size = this->_cryptor.decrypt(reader, reader.offset() - 1, size);
                }

                if (this->_handler.contains(cmd))
                {
                    co_await this->_handler[cmd]([&reader, size] {
                        reader.reset();
                        reader.shift(base_size + size);
                        reader.flush();
                    });
                }
            }
            catch (std::exception&)
            {
                reader.clear();
                break;
            }
            catch (...)
            {
                reader.clear();
                break;
            }
        }

        reader.reset();
    });
}

void base_bot::connect(const boost::asio::ip::tcp::endpoint& endpoint)
{
    this->async_connect(endpoint, [&](const auto& e) {
        this->recv();
        this->on_connected();
    });
}

void base_bot::on_connected()
{ }

void base_bot::on_disconnected()
{ }

async::task<void> base_bot::on_closed(fb::socket<>& socket)
{
    this->on_disconnected();
    this->_owner.remove(*this);
    co_return;
}

bool base_bot::on_encrypt(fb::stream_writer<>& out)
{
    return this->_cryptor.encrypt(out);
}

bool base_bot::on_wrap(fb::stream_writer<>& out)
{
    return this->_cryptor.wrap(out);
}

bool base_bot::decrypt_policy(int cmd) const
{
    switch (cmd)
    {
    case 0x03:
        return false;

    default:
        return true;
    }
}