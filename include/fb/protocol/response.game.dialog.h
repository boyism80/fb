#ifndef __PROTOCOL_RESPONSE_DIALOG_ITEM_H__
#define __PROTOCOL_RESPONSE_DIALOG_ITEM_H__

#include <fb/protocol/protocol.h>
#ifndef BOT
#include <npc.h>
#endif

namespace fb::protocol::game::response::dialog {

class common : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x30;

public:
#ifndef BOT
    const fb::model::object&            object;
    const std::string                   message;
    const bool                          button_prev;
    const bool                          button_next;
    const fb::game::dialog::interaction interaction;
#else

#endif

public:
#ifndef BOT
    common(const fb::model::object&      object,
           const std::string&            message,
           bool                          button_prev,
           bool                          button_next,
           fb::game::dialog::interaction interaction) :
        object(object),
        message(message),
        button_prev(button_prev),
        button_next(button_next),
        interaction(interaction)
    { }

    common(const fb::game::object&       object,
           const std::string&            message,
           bool                          button_prev,
           bool                          button_next,
           fb::game::dialog::interaction interaction) :
        common(object.based(), message, button_prev, button_next, interaction)
    { }
#else
    common() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x00);                                    // unknown
        writer.write<uint8_t>(static_cast<uint8_t>(this->interaction)); // interaction
        writer.write<uint32_t>(0x01);
        writer.write<uint8_t>(this->object.look > 0xBFFF ? 0x02 : 0x01);
        writer.write<uint8_t>(0x01);
        writer.write<uint16_t>(this->object.look);
        writer.write<uint8_t>(this->object.color);
        writer.write<uint8_t>(this->object.look > 0xBFFF ? 0x02 : 0x01);
        writer.write<uint16_t>(this->object.look);
        writer.write<uint8_t>(this->object.color);
        writer.write<uint32_t>(0x01);
        writer.write<uint8_t>(this->button_prev);
        writer.write<uint8_t>(this->button_next);
        writer.write<std::string, uint16_t>(this->message);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
};

class menu : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x2F;

public:
#ifndef BOT
    const fb::model::npc&               npc;
    const std::vector<std::string>      menus;
    const std::string                   message;
    const fb::game::dialog::interaction interaction;
#else

#endif

public:
#ifndef BOT
    menu(const fb::model::npc&           npc,
         const std::vector<std::string>& menus,
         const std::string&              message,
         fb::game::dialog::interaction   interaction = fb::game::dialog::interaction::MENU) :
        npc(npc),
        menus(menus),
        message(message),
        interaction(interaction)
    { }
#else
    menu() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x01);
        writer.write<uint8_t>(static_cast<uint8_t>(interaction));
        writer.write<uint32_t>(0x01);
        writer.write<uint8_t>(this->npc.look > 0xBFFF ? 0x02 : 0x01);
        writer.write<uint8_t>(0x01);
        writer.write<uint16_t>(this->npc.look);
        writer.write<uint8_t>(this->npc.color);
        writer.write<uint8_t>(this->npc.look > 0xBFFF ? 0x02 : 0x01);
        writer.write<uint16_t>(this->npc.look);
        writer.write<uint8_t>(this->npc.color);
        writer.write<std::string, uint16_t>(message);

        writer.write<uint16_t>((uint16_t)menus.size());
        for (int i = 0; i < menus.size(); i++)
        {
            writer.write<std::string>(menus[i]);
            writer.write<uint16_t>(i);
        }

        writer.write<uint8_t>(0x00);

        for (int i = 0; i < this->menus.size(); i++)
        {
            writer.write<std::string>(menus[i]);
            writer.write<uint16_t>(i);
        }

        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
};

class slot : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x2F;

public:
#ifndef BOT
    const fb::model::npc&               npc;
    const std::vector<uint8_t>          slots;
    const std::string                   message;
    const fb::game::dialog::interaction interaction;
#else

#endif

public:
#ifndef BOT
    slot(const fb::model::npc&         npc,
         const std::vector<uint8_t>&   slots,
         const std::string&            message,
         fb::game::dialog::interaction interaction = fb::game::dialog::interaction::SLOT) :
        npc(npc),
        slots(slots),
        message(message),
        interaction(interaction)
    { }
#else
    slot() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x05);
        writer.write<uint8_t>(static_cast<uint8_t>(this->interaction));
        writer.write<uint32_t>(0x01);
        writer.write<uint8_t>(this->npc.look > 0xBFFF ? 0x02 : 0x01);
        writer.write<uint8_t>(0x01);
        writer.write<uint16_t>(this->npc.look);
        writer.write<uint8_t>(this->npc.color);
        writer.write<uint8_t>(this->npc.look > 0xBFFF ? 0x02 : 0x01);
        writer.write<uint16_t>(this->npc.look);
        writer.write<uint8_t>(this->npc.color);
        writer.write<std::string, uint16_t>(this->message);
        writer.write<uint16_t>(0xFFFF);
        writer.write<uint8_t>((uint8_t)this->slots.size());

        for (auto slot : this->slots)
            writer.write<uint8_t>(slot);
        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
};

class item : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x2F;

public:
#ifndef BOT
    const fb::model::npc&               npc;
    const fb::game::dialog::item_pairs& items;
    const std::string                   message;
    const uint16_t                      pursuit;
    const fb::game::dialog::interaction interaction;
#else

#endif

public:
#ifndef BOT
    item(const fb::model::npc&               npc,
         const fb::game::dialog::item_pairs& items,
         const std::string&                  message,
         uint16_t                            pursuit     = 0xFFFF,
         fb::game::dialog::interaction       interaction = fb::game::dialog::interaction::ITEM) :
        npc(npc),
        items(items),
        message(message),
        pursuit(pursuit),
        interaction(interaction)
    { }
#else
    item() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x04);
        writer.write<uint8_t>(static_cast<uint8_t>(interaction));
        writer.write<uint32_t>(0x01);
        writer.write<uint8_t>(this->npc.look > 0xBFFF ? 0x02 : 0x01);
        writer.write<uint8_t>(0x01);
        writer.write<uint16_t>(this->npc.look);
        writer.write<uint8_t>(this->npc.color);
        writer.write<uint8_t>(this->npc.look > 0xBFFF ? 0x02 : 0x01);
        writer.write<uint16_t>(this->npc.look);
        writer.write<uint8_t>(this->npc.color);
        writer.write<std::string, uint16_t>(this->message);
        writer.write<uint16_t>(this->pursuit);
        writer.write<uint16_t>((uint16_t)this->items.size());

        for (auto& [item, value] : this->items)
        {
            writer.write<uint16_t>(item.look);
            writer.write<uint8_t>(item.color);
            writer.write<uint32_t>(value);
            writer.write<std::string>(item.name);
            writer.write<std::string>(item.desc);
        }

        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
};

class input : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x2F;

public:
#ifndef BOT
    const fb::model::npc&               npc;
    const std::vector<uint8_t>          slots;
    const std::string                   message;
    const fb::game::dialog::interaction interaction;
#else

#endif

public:
#ifndef BOT
    input(const fb::model::npc&         npc,
          const std::string&            message,
          fb::game::dialog::interaction interaction = fb::game::dialog::interaction::INPUT) :
        npc(npc),
        message(message),
        interaction(interaction)
    { }
#else
    input() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x03);
        writer.write<uint8_t>(static_cast<uint8_t>(this->interaction));
        writer.write<uint32_t>(0x01);
        writer.write<uint8_t>(this->npc.look > 0xBFFF ? 0x02 : 0x01);
        writer.write<uint8_t>(0x01);
        writer.write<uint16_t>(this->npc.look);
        writer.write<uint8_t>(this->npc.color);
        writer.write<uint8_t>(this->npc.look > 0xBFFF ? 0x02 : 0x01);
        writer.write<uint16_t>(this->npc.look);
        writer.write<uint8_t>(this->npc.color);
        writer.write<std::string, uint16_t>(this->message);
        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
};

class input_ext : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x30;

public:
#ifndef BOT
    const fb::model::npc&               npc;
    const std::vector<uint8_t>          slots;
    const std::string                   message;
    const std::string                   top, bottom;
    const int                           maxlen;
    const bool                          button_prev;
    const fb::game::dialog::interaction interaction;
#else

#endif

public:
#ifndef BOT
    input_ext(const fb::model::npc&         npc,
              const std::string&            message,
              const std::string&            top,
              const std::string&            bottom,
              int                           maxlen      = 0xFF,
              bool                          button_prev = false,
              fb::game::dialog::interaction interaction = fb::game::dialog::interaction::INPUT_EX) :
        npc(npc),
        message(message),
        top(top),
        bottom(bottom),
        maxlen(maxlen),
        button_prev(button_prev),
        interaction(interaction)
    { }
#else
    input_ext() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x04);
        writer.write<uint8_t>(static_cast<uint8_t>(this->interaction));
        writer.write<uint32_t>(0x01);
        writer.write<uint8_t>(this->npc.look > 0xBFFF ? 0x02 : 0x01);
        writer.write<uint8_t>(0x01);
        writer.write<uint16_t>(this->npc.look);
        writer.write<uint8_t>(this->npc.color);
        writer.write<uint8_t>(this->npc.look > 0xBFFF ? 0x02 : 0x01);
        writer.write<uint16_t>(this->npc.look);
        writer.write<uint8_t>(this->npc.color);
        writer.write<uint32_t>(0x00000001);
        writer.write<uint8_t>(this->button_prev);
        writer.write<uint8_t>(0x00);
        writer.write<std::string, uint16_t>(this->message);
        writer.write<std::string, uint8_t>(this->top);
        writer.write<uint8_t>(this->maxlen);
        writer.write<std::string, uint8_t>(this->bottom);
        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
};

} // namespace fb::protocol::game::response::dialog

#endif // !__PROTOCOL_RESPONSE_DIALOG_ITEM_H__