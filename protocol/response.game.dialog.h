#ifndef __PROTOCOL_RESPONSE_DIALOG_ITEM_H__
#define __PROTOCOL_RESPONSE_DIALOG_ITEM_H__

#include <protocol/protocol.h>
#include <model/npc/npc.h>

namespace fb { namespace protocol { namespace response { namespace game { namespace dialog {

class common : public fb::protocol::base::response
{
public:
    const fb::game::object&             object;
    const std::string                   message;
    const bool                          button_prev;
    const bool                          button_next;
    const fb::game::dialog::interaction interaction;

public:
    common(const fb::game::object& object, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction) : 
        object(object), message(message), button_prev(button_prev), button_next(button_next), interaction(interaction)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x30)
                  .write_u8(0x00)     // unknown
                  .write_u8(this->interaction)     // interaction
                  .write_u32(0x01)
                  .write_u8(this->object.look() > 0xBFFF ? 0x02 : 0x01)
                  .write_u8(0x01)
                  .write_u16(this->object.look())
                  .write_u8(this->object.color())
                  .write_u8(this->object.look() > 0xBFFF ? 0x02 : 0x01)
                  .write_u16(this->object.look())
                  .write_u8(this->object.color())
                  .write_u32(0x01)
                  .write_u8(this->button_prev)
                  .write_u8(this->button_next)
                  .write(this->message, true);
    }
};

class menu : public fb::protocol::base::response
{
public:
    const fb::game::npc&                npc;
    const std::vector<std::string>      menus;
    const std::string                   message;
    const bool                          button_prev;
    const bool                          button_next;
    const fb::game::dialog::interaction interaction;

public:
    menu(const fb::game::npc& npc, const std::vector<std::string>& menus, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::MENU) : 
        npc(npc), menus(menus), message(message), button_prev(button_prev), button_next(button_next), interaction(interaction)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x30)
                  .write_u8(0x00)     // unknown
                  .write_u8(this->interaction)     // interaction
                  .write_u32(0x01)
                  .write_u8(this->npc.look() > 0xBFFF ? 0x02 : 0x01)
                  .write_u8(0x01)
                  .write_u16(this->npc.look())
                  .write_u8(this->npc.color())
                  .write_u8(this->npc.look() > 0xBFFF ? 0x02 : 0x01)
                  .write_u16(this->npc.look())
                  .write_u8(this->npc.color())
                  .write_u32(0x01)
                  .write_u8(this->button_prev)
                  .write_u8(this->button_next)
                  .write(this->message, true)
                  .write_u16((uint16_t)this->menus.size());

        for(int i = 0; i < this->menus.size(); i++)
        {
            out_stream.write(menus[i])
                      .write_u16(i);
        }

        out_stream.write_u8(0x00);
    }
};

class slot : public fb::protocol::base::response
{
public:
    const fb::game::npc&                npc;
    const std::vector<uint8_t>          slots;
    const std::string                   message;
    const fb::game::dialog::interaction interaction;

public:
    slot(const fb::game::npc& npc, const std::vector<uint8_t>& slots, const std::string& message, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::SLOT) : 
        npc(npc), slots(slots), message(message), interaction(interaction)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x2F)
                  .write_u8(0x05)
                  .write_u8(this->interaction)
                  .write_u32(0x01)
                  .write_u8(this->npc.look() > 0xBFFF ? 0x02 : 0x01)
                  .write_u8(0x01)
                  .write_u16(this->npc.look())
                  .write_u8(this->npc.color())
                  .write_u8(this->npc.look() > 0xBFFF ? 0x02 : 0x01)
                  .write_u16(this->npc.look())
                  .write_u8(this->npc.color())
                  .write(this->message, true)
                  .write_u16(0xFFFF)
                  .write_u8((uint8_t)this->slots.size());

        for(auto slot : this->slots)
            out_stream.write_u8(slot);
        out_stream.write_u8(0x00);
    }
};


class input : public fb::protocol::base::response
{
public:
    const fb::game::npc&                npc;
    const std::vector<uint8_t>          slots;
    const std::string                   message;
    const fb::game::dialog::interaction interaction;

public:
    input(const fb::game::npc& npc, const std::string& message, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::INPUT) : 
        npc(npc), message(message), interaction(interaction)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x2F)
                  .write_u8(0x03)
                  .write_u8(this->interaction)
                  .write_u32(0x01)
                  .write_u8(this->npc.look() > 0xBFFF ? 0x02 : 0x01)
                  .write_u8(0x01)
                  .write_u16(this->npc.look())
                  .write_u8(this->npc.color())
                  .write_u8(this->npc.look() > 0xBFFF ? 0x02 : 0x01)
                  .write_u16(this->npc.look())
                  .write_u8(this->npc.color())
                  .write(this->message, true)
                  .write_u8(0x00);
    }
};

class input_ext : public fb::protocol::base::response
{
public:
    const fb::game::npc&                npc;
    const std::vector<uint8_t>          slots;
    const std::string                   message;
    const std::string                   top, bottom;
    const int                           maxlen;
    const bool                          button_prev;
    const fb::game::dialog::interaction interaction;

public:
    input_ext(const fb::game::npc& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool button_prev = false, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::INPUT_EX) : 
        npc(npc), message(message), top(top), bottom(bottom), maxlen(maxlen), button_prev(button_prev), interaction(interaction)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x30)
                  .write_u8(0x04)
                  .write_u8(this->interaction)
                  .write_u32(0x01)
                  .write_u8(this->npc.look() > 0xBFFF ? 0x02 : 0x01)
                  .write_u8(0x01)
                  .write_u16(this->npc.look())
                  .write_u8(this->npc.color())
                  .write_u8(this->npc.look() > 0xBFFF ? 0x02 : 0x01)
                  .write_u16(this->npc.look())
                  .write_u8(this->npc.color())
                  .write_u32(0x00000001)
                  .write_u8(this->button_prev)
                  .write_u8(0x00)
                  .write(this->message, true)
                  .write(this->top, false)
                  .write_u8(this->maxlen)
                  .write(this->bottom, false)
                  .write_u8(0x00);
    }
};

} } } } }

#endif // !__PROTOCOL_RESPONSE_DIALOG_ITEM_H__