#include <fb/game/server.h>
#include <fb/protocol/client_version.h>

using namespace fb::game;

namespace game_resp = fb::protocol::game::response;

void listener_impl::on_dialog(character&                 me,
                              std::optional<std::string> message,
                              bool                       button_prev,
                              bool                       button_next,
                              uint32_t                   oid)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog<V>(std::move(message), button_prev, button_next, oid));
    });
}

void listener_impl::on_dialog(character&                 me,
                              const fb::model::object&   object,
                              std::optional<std::string> message,
                              bool                       button_prev,
                              bool                       button_next,
                              uint32_t                   oid)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog<V>(object, std::move(message), button_prev, button_next, oid));
    });
}

void listener_impl::on_dialog(character&                 me,
                              const fb::game::object&    obj,
                              std::optional<std::string> message,
                              bool                       button_prev,
                              bool                       button_next,
                              uint32_t                   oid)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog<V>(obj, std::move(message), button_prev, button_next, oid));
    });
}

void listener_impl::on_dialog(character&                              me,
                              std::unique_ptr<fb::game::appearance>&& appearance,
                              std::optional<std::string>              message,
                              bool                                    button_prev,
                              bool                                    button_next,
                              uint32_t                                oid)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog<V>(std::move(appearance), std::move(message), button_prev, button_next, oid));
    });
}

void listener_impl::on_dialog(character&                      me,
                              const fb::model::object&        obj,
                              std::string_view                message,
                              const std::vector<std::string>& menus,
                              uint32_t                        oid,
                              std::optional<std::string>      ext)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_menu<V>(obj, menus, message, oid, std::move(ext)));
    });
}

void listener_impl::on_dialog(character&                      me,
                              const fb::game::object&         obj,
                              std::string_view                message,
                              const std::vector<std::string>& menus,
                              uint32_t                        oid,
                              std::optional<std::string>      ext)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_menu<V>(obj, menus, message, oid, std::move(ext)));
    });
}

void listener_impl::on_dialog(character&                      me,
                              const fb::model::object&        obj,
                              std::optional<std::string>      message,
                              const std::vector<std::string>& menus,
                              bool                            button_prev,
                              uint32_t                        oid)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_list<V>(obj, menus, std::move(message), button_prev, oid));
    });
}

void listener_impl::on_dialog(character&                      me,
                              const fb::game::object&         obj,
                              std::optional<std::string>      message,
                              const std::vector<std::string>& menus,
                              bool                            button_prev,
                              uint32_t                        oid)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_list<V>(obj, menus, std::move(message), button_prev, oid));
    });
}

void listener_impl::on_dialog(character&                              me,
                              std::unique_ptr<fb::game::appearance>&& appearance,
                              std::optional<std::string>              message,
                              const std::vector<std::string>&         menus,
                              bool                                    button_prev,
                              uint32_t                                oid)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_list<V>(std::move(appearance), menus, std::move(message), button_prev, oid));
    });
}

void listener_impl::on_dialog(character&                  me,
                              const fb::model::object&    obj,
                              std::string_view            message,
                              const std::vector<uint8_t>& item_slots,
                              uint32_t                    oid)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_slot<V>(obj, item_slots, message, oid));
    });
}

void listener_impl::on_dialog(character&                  me,
                              const fb::game::object&     obj,
                              std::string_view            message,
                              const std::vector<uint8_t>& item_slots,
                              uint32_t                    oid)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_slot<V>(obj, item_slots, message, oid));
    });
}

void listener_impl::on_dialog(character&                me,
                              const fb::model::object&  obj,
                              std::string_view          message,
                              const dialog::item_pairs& pairs,
                              uint32_t                  oid,
                              uint16_t                  pursuit)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_item<V>(obj, pairs, message, oid, pursuit));
    });
}

void listener_impl::on_dialog(character&                me,
                              const fb::game::object&   obj,
                              std::string_view          message,
                              const dialog::item_pairs& pairs,
                              uint32_t                  oid,
                              uint16_t                  pursuit)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_item<V>(obj, pairs, message, oid, pursuit));
    });
}

void listener_impl::on_dialog(character&                 me,
                              const fb::model::object&   obj,
                              std::string_view           message,
                              uint32_t                   oid,
                              std::optional<std::string> ext,
                              uint16_t                   pursuit)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_input<V>(obj, message, oid, std::move(ext), pursuit));
    });
}

void listener_impl::on_dialog(character&                 me,
                              const fb::game::object&    obj,
                              std::string_view           message,
                              uint32_t                   oid,
                              std::optional<std::string> ext,
                              uint16_t                   pursuit)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_input<V>(obj, message, oid, std::move(ext), pursuit));
    });
}

void listener_impl::on_dialog(character&                 me,
                              const fb::model::object&   obj,
                              std::optional<std::string> message,
                              std::string_view           top,
                              std::string_view           bottom,
                              int                        maxlen,
                              bool                       prev,
                              bool                       password,
                              uint32_t                   oid)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_input_ext<V>(obj, std::move(message), top, bottom, maxlen, prev, password, oid));
    });
}

void listener_impl::on_dialog(character&                 me,
                              const fb::game::object&    obj,
                              std::optional<std::string> message,
                              std::string_view           top,
                              std::string_view           bottom,
                              int                        maxlen,
                              bool                       prev,
                              bool                       password,
                              uint32_t                   oid)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_input_ext<V>(obj, std::move(message), top, bottom, maxlen, prev, password, oid));
    });
}

void listener_impl::on_dialog_pursuit(character&                      me,
                                      const fb::model::object&        obj,
                                      std::string_view                message,
                                      const std::vector<std::string>& options,
                                      uint32_t                        oid,
                                      uint16_t                        pursuit)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_pursuit<V>(obj, options, message, oid, pursuit));
    });
}

void listener_impl::on_dialog_pursuit(character&                      me,
                                      const fb::game::object&         obj,
                                      std::string_view                message,
                                      const std::vector<std::string>& options,
                                      uint32_t                        oid,
                                      uint16_t                        pursuit)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_pursuit<V>(obj, options, message, oid, pursuit));
    });
}

void listener_impl::on_dialog_spell(character&               me,
                                    const fb::model::object& obj,
                                    std::string_view         message,
                                    uint32_t                 oid,
                                    uint16_t                 pursuit)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_spell<V>(obj, message, oid, pursuit));
    });
}

void listener_impl::on_dialog_spell(character&              me,
                                    const fb::game::object& obj,
                                    std::string_view        message,
                                    uint32_t                oid,
                                    uint16_t                pursuit)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_spell<V>(obj, message, oid, pursuit));
    });
}

void listener_impl::on_dialog_dual_field(character&                                              me,
                                         const fb::model::object&                                obj,
                                         std::string_view                                        message,
                                         const std::vector<std::pair<std::string, std::string>>& pairs,
                                         uint32_t                                                oid,
                                         uint16_t                                                pursuit)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_dual_field<V>(obj, pairs, message, oid, pursuit));
    });
}

void listener_impl::on_dialog_dual_field(character&                                              me,
                                         const fb::game::object&                                 obj,
                                         std::string_view                                        message,
                                         const std::vector<std::pair<std::string, std::string>>& pairs,
                                         uint32_t                                                oid,
                                         uint16_t                                                pursuit)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_dual_field<V>(obj, pairs, message, oid, pursuit));
    });
}

void listener_impl::on_dialog_email(character&               me,
                                    const fb::model::object& obj,
                                    std::string_view         message,
                                    std::string_view         str1,
                                    std::string_view         str2,
                                    bool                     button_prev,
                                    bool                     button_next,
                                    uint32_t                 oid)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_email<V>(obj, message, str1, str2, button_prev, button_next, oid));
    });
}

void listener_impl::on_dialog_email(character&              me,
                                    const fb::game::object& obj,
                                    std::string_view        message,
                                    std::string_view        str1,
                                    std::string_view        str2,
                                    bool                    button_prev,
                                    bool                    button_next,
                                    uint32_t                oid)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_email<V>(obj, message, str1, str2, button_prev, button_next, oid));
    });
}

void listener_impl::on_dialog_0x30_10(character&               me,
                                      const fb::model::object& obj,
                                      std::string_view         message,
                                      uint32_t                 oid)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_0x30_10<V>(obj, message, false, false, oid));
    });
}

void listener_impl::on_dialog_0x30_10(character&              me,
                                      const fb::game::object& obj,
                                      std::string_view        message,
                                      uint32_t                oid)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_0x30_10<V>(obj, message, false, false, oid));
    });
}
