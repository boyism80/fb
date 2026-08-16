#include <fb/bot/login_bot.h>
#include <fb/bot/game_bot.h>
#include <fb/bot/container.h>
#include <fb/bot/login_controller.h>

using namespace fb::bot;

login_bot::login_bot(bot_controller<login_bot>& bot_controller, uint32_t id) :
    bot<login_bot>(bot_controller, id)
{ }

login_bot::login_bot(bot_controller<login_bot>& bot_controller, uint32_t id, const fb::stream& params) :
    login_bot(bot_controller, id)
{
    auto clone    = fb::stream{params};
    auto reader   = fb::stream_reader<>(clone);
    auto enc_type = reader.read<uint8_t>();
    auto key_size = reader.read<uint8_t>();
    auto enc_key  = std::make_unique<uint8_t[]>(key_size);
    reader.read(enc_key.get(), key_size);
    this->encryption(enc_type, enc_key.get());

    // Transfer header: from (u8) + required CLIENT_VERSION (u16)
    if (reader.readable_size() >= sizeof(uint8_t) + sizeof(uint16_t))
    {
        this->_transfer_from = reader.read<uint8_t>();
        auto packed          = reader.read<uint16_t>();
        if (fb::protocol::is_supported(packed))
            this->_client_version = static_cast<fb::protocol::CLIENT_VERSION>(packed);
    }
}

login_bot::~login_bot()
{ }

std::string login_bot::generate_id() const
{
    constexpr auto min = 0xAC00; // 가
    constexpr auto max = 0xD7A3; // 힣

    while (true)
    {
        auto len = random(2, 6);
        auto wcs = std::wstring{};
        for (int i = 0; i < len; i++)
        {
            wcs += (wchar_t)random(min, max);
        }

        auto mbs   = fb::M(wcs);
        auto cp949 = fb::CP949(mbs); // Linux: UTF-8 -> CP949; Windows: already CP949
        if (fb::assert_korean(cp949) == false)
            continue;

        return mbs;
    }
}

uint8_t login_bot::transfer_from() const
{
    return this->_transfer_from;
}

fb::protocol::CLIENT_VERSION login_bot::client_version() const
{
    return this->_client_version;
}
