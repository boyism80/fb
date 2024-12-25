#include <bot.h>

using namespace fb::bot;

base_bot::base_bot(bot_container& owner, uint32_t id) :
    fb::socket<void*>(owner,
                      std::bind(&bot_container::on_receive, &owner, std::placeholders::_1, std::placeholders::_2),
                      std::bind(&bot_container::on_closed, &owner, std::placeholders::_1)),
    _owner(owner),
    id(id)
{ }

base_bot::~base_bot()
{ }

async::task<void> base_bot::on_receive(fb::stream& stream)
{
    static constexpr uint8_t base_size = sizeof(uint8_t) + sizeof(uint16_t);

    auto reader = fb::stream_reader<big_endian>(stream);
    while (true)
    {
        try
        {
            if (reader.readable_size() < base_size)
                co_return;

            auto head = reader.read<uint8_t>();
            if (head != 0xAA)
                throw std::runtime_error("magic code mismatch");

            auto size = reader.read<uint16_t>();
            if (size > reader.readable_size())
            {
                reader.seek(0);
                co_return;
            }

            auto cmd = reader.read<uint8_t>();
            if (this->decrypt_policy(cmd))
            {
                size = this->_cryptor.decrypt(stream, reader.seek() - 1, size);
            }

            reader.flush();
            if (this->_deserializer.contains(cmd) == false)
            {
            }
            else if (this->_handler.contains(cmd) == false)
            {
            }
            else
            {
                auto protocol = std::shared_ptr<fb::protocol::base::header>(co_await this->_deserializer[cmd](reader));
                this->thread()->enqueue(
                    [this, cmd, protocol](auto&) -> async::task<void> {
                        // TODO: check socket alive
                        co_await this->_handler[cmd](*protocol.get());
                    },
                    [](auto& error) { // error
                        fb::logger::fatal(error.what());
                    },
                    []() { // success

                    });
            }

            reader.seek(size - sizeof(uint8_t));
            reader.flush(); // remove packet body
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
}

void base_bot::connect(const boost::asio::ip::tcp::endpoint& endpoint)
{
    this->async_connect(endpoint, [this, id = this->id, thread = this->thread()](const auto& e) {
        boost::asio::co_spawn(static_cast<boost::asio::io_context&>(this->_owner), this->recv(), boost::asio::detached);
        std::ignore = this->on_connected();
    });
}

async::task<void> base_bot::on_connected()
{
    co_return;
}

async::task<void> base_bot::on_disconnected()
{
    co_return;
}

async::task<void> base_bot::on_closed()
{
    co_await this->on_disconnected();
}

bool base_bot::on_encrypt(fb::stream& out)
{
    return this->_cryptor.encrypt(out);
}

bool base_bot::on_wrap(fb::stream& out)
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

fb::thread* base_bot::thread() const
{
    return this->_owner.threads.modular(this->id);
}