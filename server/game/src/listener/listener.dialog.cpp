#include <fb/game/appearance.h>
#include <fb/game/server.h>
#include <fb/protocol/client_version.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace fb::game;

namespace game_resp = fb::protocol::game::response;

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

void listener_impl::on_dialog(character&                              me,
                              std::unique_ptr<fb::game::appearance>&& appearance,
                              std::string_view                        message,
                              const std::vector<std::string>&         menus,
                              uint32_t                                oid,
                              std::optional<std::string>              ext)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_menu<V>(std::move(appearance), menus, message, oid, std::move(ext)));
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

void listener_impl::on_dialog(character&                              me,
                              std::unique_ptr<fb::game::appearance>&& appearance,
                              std::string_view                        message,
                              const std::vector<uint8_t>&             item_slots,
                              uint32_t                                oid)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_slot<V>(std::move(appearance), item_slots, message, oid));
    });
}

void listener_impl::on_dialog(character&                              me,
                              std::unique_ptr<fb::game::appearance>&& appearance,
                              std::string_view                        message,
                              const dialog::item_pairs&               pairs,
                              uint32_t                                oid,
                              uint16_t                                pursuit)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_item<V>(std::move(appearance), pairs, message, oid, pursuit));
    });
}

void listener_impl::on_dialog(character&                              me,
                              std::unique_ptr<fb::game::appearance>&& appearance,
                              std::string_view                        message,
                              uint32_t                                oid,
                              std::optional<std::string>              ext,
                              uint16_t                                pursuit)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_input<V>(std::move(appearance), message, oid, std::move(ext), pursuit));
    });
}

void listener_impl::on_dialog(character&                              me,
                              std::unique_ptr<fb::game::appearance>&& appearance,
                              std::optional<std::string>              message,
                              std::string_view                        top,
                              std::string_view                        bottom,
                              int                                     maxlen,
                              bool                                    prev,
                              bool                                    password,
                              uint32_t                                oid)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_input_ext<
                V>(std::move(appearance), std::move(message), top, bottom, maxlen, prev, password, oid));
    });
}

void listener_impl::on_dialog_pursuit(character&                              me,
                                      std::unique_ptr<fb::game::appearance>&& appearance,
                                      std::string_view                        message,
                                      const std::vector<std::string>&         options,
                                      uint32_t                                oid,
                                      uint16_t                                pursuit)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_pursuit<V>(std::move(appearance), options, message, oid, pursuit));
    });
}

void listener_impl::on_dialog_spell(character&                              me,
                                    std::unique_ptr<fb::game::appearance>&& appearance,
                                    std::string_view                        message,
                                    uint32_t                                oid,
                                    uint16_t                                pursuit)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_spell<V>(std::move(appearance), message, oid, pursuit));
    });
}

void listener_impl::on_dialog_buy(character&                              me,
                                  std::unique_ptr<fb::game::appearance>&& appearance,
                                  std::string_view                        message,
                                  const dialog::buy_entries&              entries,
                                  uint32_t                                oid,
                                  uint16_t                                pursuit)
{
    // 6.51 type 10 BUY is NEW_ONLY (g_ui_mode_new==1). OLD ignores subtype 10,
    // so fall back to PURSUIT name list — same simple-list UX as 5.50 BUY rows.
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        if constexpr (V == fb::protocol::CLIENT_VERSION::v651)
        {
            if (me.ui_mode == fb::protocol::CLIENT_UI_MODE::NEW)
            {
                me.send(game_resp::dialog_buy<V>(std::move(appearance), entries, message, oid, pursuit));
            }
            else
            {
                auto options = std::vector<std::string>{};
                options.reserve(entries.size());
                for (const auto& entry : entries)
                {
                    options.push_back(entry.model.name);
                }

                me.send(game_resp::dialog_pursuit<V>(std::move(appearance), options, message, oid, pursuit));
            }
        }
        else
        {
            me.send(game_resp::dialog_buy<V>(std::move(appearance), entries, message, oid, pursuit));
        }
    });
}

void listener_impl::on_dialog_email(character&                              me,
                                    std::unique_ptr<fb::game::appearance>&& appearance,
                                    std::string_view                        message,
                                    std::string_view                        str1,
                                    std::string_view                        str2,
                                    bool                                    button_prev,
                                    bool                                    button_next,
                                    uint32_t                                oid)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_email<V>(std::move(appearance), message, str1, str2, button_prev, button_next, oid));
    });
}

void listener_impl::on_dialog_0x30_10(character&                              me,
                                      std::unique_ptr<fb::game::appearance>&& appearance,
                                      std::string_view                        message,
                                      uint32_t                                oid)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::dialog_0x30_10<V>(std::move(appearance), message, false, false, oid));
    });
}
