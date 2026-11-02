#include <fb/bot/brain/brain.h>

#include <fb/bot/game_controller.h>
#include <fb/bot/integration/dialog_bot.h>
#include <fb/bot/integration/dialog_ext_bot.h>

#include <fb/logger.h>

#include <chrono>
#include <exception>
#include <string>

using namespace fb::bot;
using namespace std::chrono_literals;

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

constexpr auto TALK_TIMEOUT = 3s;
constexpr auto DIALOG_CLOSE = uint32_t{0xFFFFFFFD};

async::task<brain::event> brain::talk(std::shared_ptr<game_bot>             bot,
                                      std::shared_ptr<fb::protocol::header> opening,
                                      std::vector<answer>                   answers)
{
    using dialog_type     = fb::bot::integration::dialog_type;
    using dialog_ext_type = fb::bot::integration::dialog_ext_type;
    using list_type       = fb::game::dialog::list_type;
    using DIALOG_RESULT   = fb::model::enum_value::DIALOG_RESULT;

    auto settle = [bot]() -> async::task<void> {
        try
        {
            std::ignore = co_await bot->request_by_opcode(
                game_resp::internal_info<fb::protocol::CLIENT_VERSION::v550>::opcode,
                bot->make<game_reqs::self_info>().get(),
                [](const fb::protocol::header&) {
                    return true;
                },
                TALK_TIMEOUT);
        }
        catch (const std::exception&)
        { }
    };

    if (opening != nullptr)
    {
        auto closed     = std::optional<fb::bot::integration::dialog_bot>{};
        auto closed_ext = std::optional<fb::bot::integration::dialog_ext_bot>{};
        auto before     = bot->last_dialog(closed, closed_ext);
        bot->send(*opening);
        co_await settle();
        if (bot->aborted())
            co_return event{"TRANSFERRED", bot->map(), bot->position()};
        else if (bot->last_dialog(closed, closed_ext) == before)
            co_return event{"NO_DIALOG", bot->map(), bot->position()};
    }
    else if (this->talking(*bot) == false)
    {
        co_return event{"NO_DIALOG", bot->map(), bot->position()};
    }

    auto index  = size_t{0};
    auto result = std::string{"DONE"};
    while (true)
    {
        auto menu = std::optional<fb::bot::integration::dialog_bot>{};
        auto ext  = std::optional<fb::bot::integration::dialog_ext_bot>{};
        auto seq  = bot->last_dialog(menu, ext);

        auto asked = menu.has_value() && (menu->type == dialog_type::menu || menu->type == dialog_type::menu_no_ext ||
                                          menu->type == dialog_type::pursuit);
        auto listed =
            ext.has_value() && (ext->type == dialog_ext_type::list || ext->type == dialog_ext_type::list_no_msg);
        auto paged = ext.has_value() && ext->normal_button_next;

        if (this->active(bot->id) == false)
        {
            result = "INTERRUPTED";
            break;
        }
        else if ((asked || listed) && index >= answers.size())
        {
            result = "UNANSWERED";
            break;
        }
        else if (asked)
        {
            auto& chosen = answers[index++];
            auto  type =
                menu->type == dialog_type::pursuit ? fb::game::dialog::type::PURSUIT : fb::game::dialog::type::MENU;
            bot->send(
                *bot->make<game_reqs::dialog>(type, uint8_t{0}, std::string{}, chosen.index, uint16_t{0}, chosen.name));
        }
        else if (listed)
        {
            auto& chosen = answers[index++];
            bot->send(*bot->make<game_reqs::dialog_list>(list_type::LIST,
                                                         uint8_t{0},
                                                         std::string{},
                                                         chosen.index,
                                                         DIALOG_RESULT::NEXT));
        }
        else if (paged)
        {
            bot->send(*bot->make<game_reqs::dialog_list>(list_type::TEXT,
                                                         static_cast<uint8_t>(DIALOG_RESULT::NEXT),
                                                         std::string{},
                                                         uint16_t{0},
                                                         DIALOG_RESULT::NEXT));
        }
        else
        {
            break;
        }

        co_await settle();
        if (bot->aborted())
            co_return event{"TRANSFERRED", bot->map(), bot->position()};

        auto next_menu = std::optional<fb::bot::integration::dialog_bot>{};
        auto next_ext  = std::optional<fb::bot::integration::dialog_ext_bot>{};
        if (bot->last_dialog(next_menu, next_ext) == seq)
            break;
    }

    bot->send(*bot->make<game_reqs::dialog_list>(list_type::TEXT,
                                                 static_cast<uint8_t>(DIALOG_RESULT::QUIT),
                                                 std::string{},
                                                 uint16_t{0},
                                                 DIALOG_RESULT::QUIT));
    bot->send(*bot->make<game_reqs::click>(DIALOG_CLOSE));
    co_await settle();
    this->close(*bot);
    fb::logger::debug("brain: bot_id={} talk {} after {} answers", bot->id, result, index);
    co_return event{result, bot->map(), bot->position()};
}
