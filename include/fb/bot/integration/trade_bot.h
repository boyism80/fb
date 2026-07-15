#ifndef __BOT_INTEGRATION_TRADE_BOT_H__
#define __BOT_INTEGRATION_TRADE_BOT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::bot::integration {

class trade_bot : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x42;

public:
    enum class trade_type : uint8_t
    {
        dialog = 0x00,
        bundle = 0x01,
        upload = 0x02,
        money  = 0x03,
        close  = 0x04,
        lock   = 0x05,
    };

    trade_type type;

    // trade_dialog data
    uint32_t    dialog_oid;
    std::string dialog_name;

    // trade_upload data
    uint8_t     upload_index;
    uint16_t    upload_look;
    uint8_t     upload_color;
    std::string upload_name;
    bool        upload_mine;

    // trade_money data
    uint32_t money;
    bool     mine;

    // trade_close data
    std::string close_message;

public:
    trade_bot() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::bot::integration

#endif // __BOT_INTEGRATION_TRADE_BOT_H__