#include <fb/bot/builtin/game_bot.h>
#include <fb/bot/integration/bulletin_bot.h>
#include <fb/bot/game_bot.h>
#include <fb/bot/integration/lua_integration_protocol.h>
#include <fb/bot/integration/lua_integration_test.h>
#include <fb/game/protocol.h>
#include <fb/model/model.h>

#include <fb/lua.h>

#include <algorithm>
#include <memory>
#include <string>

using namespace std::chrono_literals;
using namespace fb::bot::integration;
using namespace fb::model::enum_value;

// The bot speaks the v550 C2S layout; versioned requests are instantiated for it.
constexpr auto BOT_CLIENT_VERSION = fb::protocol::CLIENT_VERSION::v550;

namespace {

#ifdef _DEBUG
constexpr auto INTEGRATION_DEFAULT_TIMEOUT = 30s;
#else
constexpr auto INTEGRATION_DEFAULT_TIMEOUT = 10s;
#endif

void push_article(fb::lua::context* lua, const bulletin_bot::article_data& article)
{
    lua->new_table();
    lua->pushstring("id");
    lua->pushinteger(article.id);
    lua->settable(-3);
    lua->pushstring("uname");
    lua->pushstring(article.uname);
    lua->settable(-3);
    lua->pushstring("month");
    lua->pushinteger(article.month);
    lua->settable(-3);
    lua->pushstring("day");
    lua->pushinteger(article.day);
    lua->settable(-3);
    lua->pushstring("title");
    lua->pushstring(article.title);
    lua->settable(-3);
}

void push_mail(fb::lua::context* lua, const bulletin_bot::mail_data& mail)
{
    lua->new_table();
    lua->pushstring("id");
    lua->pushinteger(mail.id);
    lua->settable(-3);
    lua->pushstring("sender_name");
    lua->pushstring(mail.sender_name);
    lua->settable(-3);
    lua->pushstring("month");
    lua->pushinteger(mail.month);
    lua->settable(-3);
    lua->pushstring("day");
    lua->pushinteger(mail.day);
    lua->settable(-3);
    lua->pushstring("title");
    lua->pushstring(mail.title);
    lua->settable(-3);
}

} // namespace

namespace fb::bot {

int builtin::game_bot::builtin_bulletin_write(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto section      = static_cast<uint16_t>(luaL_checkinteger(L, 2));
    auto title_str    = lua->tostring(3);
    auto contents_str = lua->tostring(4);
    auto bot_ptr      = bot;
    auto result       = std::make_shared<bool>(false);

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, section, title_str, contents_str, result]() -> async::task<void> {
        auto bot = bot_ptr;
        if (bot == nullptr)
            co_return;

        namespace game_reqs = fb::protocol::game::request;
        auto&& resp         = co_await bot->request<bulletin_bot>(
            game_reqs::bulletin<BOT_CLIENT_VERSION>(BULLETIN_ACTION::WRITE, section, 0, 0, title_str, contents_str),
            INTEGRATION_DEFAULT_TIMEOUT);

        *result = resp.type == bulletin_bot::bulletin_type::message &&
                  resp.message_text == fb::model::const_value::string::MESSAGE_BULLETIN_WRITE;
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_bulletin_get_sections(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto bot_ptr = bot;

    struct sections_result
    {
        std::vector<std::pair<uint32_t, std::string>> items;
    };
    auto result = std::make_shared<sections_result>();

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, result]() -> async::task<void> {
        auto bot = bot_ptr;
        if (bot == nullptr)
            co_return;

        namespace game_reqs = fb::protocol::game::request;
        auto&& resp         = co_await bot->request<bulletin_bot>(
            game_reqs::bulletin<BOT_CLIENT_VERSION>(BULLETIN_ACTION::SECTIONS),
            [](auto& r) -> bool {
                return r.type == bulletin_bot::bulletin_type::sections;
            },
            INTEGRATION_DEFAULT_TIMEOUT);

        for (auto& section : resp.bulletins)
            result->items.emplace_back(section.id, section.title);
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->new_table();
        auto i = 1;
        for (auto& [id, title] : result->items)
        {
            lua->pushinteger(i++);
            lua->new_table();
            lua->pushstring("id");
            lua->pushinteger(id);
            lua->settable(-3);
            lua->pushstring("title");
            lua->pushstring(title);
            lua->settable(-3);
            lua->settable(-3);
        }
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_bulletin_get_articles(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto section = static_cast<uint16_t>(luaL_checkinteger(L, 2));
    auto offset  = static_cast<uint16_t>(luaL_optinteger(L, 3, 0x7FFF));
    auto bot_ptr = bot;
    auto result  = std::make_shared<std::vector<bulletin_bot::article_data>>();

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, section, offset, result]() -> async::task<void> {
        auto bot = bot_ptr;
        if (bot == nullptr)
            co_return;

        namespace game_reqs = fb::protocol::game::request;
        auto&& resp         = co_await bot->request<bulletin_bot>(
            game_reqs::bulletin<BOT_CLIENT_VERSION>(BULLETIN_ACTION::ARTICLES, section, 0, offset),
            [section](auto& r) -> bool {
                return r.type == bulletin_bot::bulletin_type::articles && r.articles_bulletin_id == section;
            },
            INTEGRATION_DEFAULT_TIMEOUT);

        *result = resp.articles;
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->new_table();
        auto i = 1;
        for (auto& article : *result)
        {
            lua->pushinteger(i++);
            push_article(lua, article);
            lua->settable(-3);
        }
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_bulletin_read_article(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto section    = static_cast<uint16_t>(luaL_checkinteger(L, 2));
    auto article_id = static_cast<uint16_t>(luaL_checkinteger(L, 3));
    auto bot_ptr    = bot;

    struct article_full
    {
        uint16_t    id;
        std::string uname, title, contents;
    };
    auto result = std::make_shared<article_full>();

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, section, article_id, result]() -> async::task<void> {
        auto bot = bot_ptr;
        if (bot == nullptr)
            co_return;

        namespace game_reqs = fb::protocol::game::request;
        auto&& resp         = co_await bot->request<bulletin_bot>(
            game_reqs::bulletin<BOT_CLIENT_VERSION>(BULLETIN_ACTION::ARTICLE, section, article_id),
            [article_id](auto& r) -> bool {
                return r.type == bulletin_bot::bulletin_type::article && r.article_id == article_id;
            },
            INTEGRATION_DEFAULT_TIMEOUT);

        result->id       = resp.article_id;
        result->uname    = resp.article_uname;
        result->title    = resp.article_title;
        result->contents = resp.article_contents;
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->new_table();
        lua->pushstring("id");
        lua->pushinteger(result->id);
        lua->settable(-3);
        lua->pushstring("uname");
        lua->pushstring(result->uname);
        lua->settable(-3);
        lua->pushstring("title");
        lua->pushstring(result->title);
        lua->settable(-3);
        lua->pushstring("contents");
        lua->pushstring(result->contents);
        lua->settable(-3);
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_bulletin_delete_article(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto section    = static_cast<uint16_t>(luaL_checkinteger(L, 2));
    auto article_id = static_cast<uint16_t>(luaL_checkinteger(L, 3));
    auto bot_ptr    = bot;
    auto result     = std::make_shared<bool>(false);

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, section, article_id, result]() -> async::task<void> {
        auto bot = bot_ptr;
        if (bot == nullptr)
            co_return;

        namespace game_reqs = fb::protocol::game::request;
        auto&& resp         = co_await bot->request<bulletin_bot>(
            game_reqs::bulletin<BOT_CLIENT_VERSION>(BULLETIN_ACTION::DELETE, section, article_id),
            [](auto& r) -> bool {
                switch (r.type)
                {
                case bulletin_bot::bulletin_type::message:
                case bulletin_bot::bulletin_type::message_mail:
                    return true;
                default:
                    return false;
                }
            },
            INTEGRATION_DEFAULT_TIMEOUT);

        *result = resp.message_text == fb::model::const_value::string::MESSAGE_BULLETIN_SUCCESS_DELETE;
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_bulletin_send_mail(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto to_str       = lua->tostring(2);
    auto title_str    = lua->tostring(3);
    auto contents_str = lua->tostring(4);
    auto bot_ptr      = bot;
    auto result       = std::make_shared<bool>(false);

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, to_str, title_str, contents_str, result]() -> async::task<void> {
        auto bot = bot_ptr;
        if (bot == nullptr)
            co_return;

        namespace game_reqs = fb::protocol::game::request;
        auto&& resp         = co_await bot->request<bulletin_bot>(
            game_reqs::bulletin<BOT_CLIENT_VERSION>(BULLETIN_ACTION::SEND_MAIL,
                                                    0,
                                                    0,
                                                    0,
                                                    title_str,
                                                    contents_str,
                                                    to_str),
            [](auto& r) -> bool {
                switch (r.type)
                {
                case bulletin_bot::bulletin_type::message:
                case bulletin_bot::bulletin_type::message_mail:
                    return true;
                default:
                    return false;
                }
            },
            INTEGRATION_DEFAULT_TIMEOUT);

        *result = resp.message_success;
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_bulletin_get_mails(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto bot_ptr = bot;
    auto result  = std::make_shared<std::vector<bulletin_bot::mail_data>>();

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, result]() -> async::task<void> {
        auto bot = bot_ptr;
        if (bot == nullptr)
            co_return;

        namespace game_reqs = fb::protocol::game::request;
        auto&& resp         = co_await bot->request<bulletin_bot>(
            game_reqs::bulletin<BOT_CLIENT_VERSION>(BULLETIN_ACTION::ARTICLES, 0xFFFF, 0, 0x7FFF),
            [](auto& r) -> bool {
                return r.type == bulletin_bot::bulletin_type::mails;
            },
            INTEGRATION_DEFAULT_TIMEOUT);

        *result = resp.mails;
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->new_table();
        auto i = 1;
        for (auto& mail : *result)
        {
            lua->pushinteger(i++);
            push_mail(lua, mail);
            lua->settable(-3);
        }
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_bulletin_read_mail(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto mail_id = static_cast<uint16_t>(luaL_checkinteger(L, 2));
    auto bot_ptr = bot;

    struct mail_full
    {
        uint16_t    id;
        std::string sender_name, title, contents;
    };
    auto result = std::make_shared<mail_full>();

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, mail_id, result]() -> async::task<void> {
        auto bot = bot_ptr;
        if (bot == nullptr)
            co_return;

        namespace game_reqs = fb::protocol::game::request;
        auto&& resp         = co_await bot->request<bulletin_bot>(
            game_reqs::bulletin<BOT_CLIENT_VERSION>(BULLETIN_ACTION::ARTICLE, 0xFFFF, mail_id, 0),
            [mail_id](auto& r) -> bool {
                return r.type == bulletin_bot::bulletin_type::mail && r.mail_id == mail_id;
            },
            INTEGRATION_DEFAULT_TIMEOUT);

        result->id          = resp.mail_id;
        result->sender_name = resp.mail_sender_name;
        result->title       = resp.mail_title;
        result->contents    = resp.mail_contents;
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->new_table();
        lua->pushstring("id");
        lua->pushinteger(result->id);
        lua->settable(-3);
        lua->pushstring("sender_name");
        lua->pushstring(result->sender_name);
        lua->settable(-3);
        lua->pushstring("title");
        lua->pushstring(result->title);
        lua->settable(-3);
        lua->pushstring("contents");
        lua->pushstring(result->contents);
        lua->settable(-3);
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_bulletin_delete_mail(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto mail_id = static_cast<uint16_t>(luaL_checkinteger(L, 2));
    auto bot_ptr = bot;
    auto result  = std::make_shared<bool>(false);

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, mail_id, result]() -> async::task<void> {
        auto bot = bot_ptr;
        if (bot == nullptr)
            co_return;

        namespace game_reqs = fb::protocol::game::request;
        auto&& resp         = co_await bot->request<bulletin_bot>(
            game_reqs::bulletin<BOT_CLIENT_VERSION>(BULLETIN_ACTION::DELETE, 0xFFFF, mail_id, 0),
            [](auto& r) -> bool {
                switch (r.type)
                {
                case bulletin_bot::bulletin_type::message:
                case bulletin_bot::bulletin_type::message_mail:
                    return true;
                default:
                    return false;
                }
            },
            INTEGRATION_DEFAULT_TIMEOUT);

        *result = resp.message_text == fb::model::const_value::string::MESSAGE_BULLETIN_SUCCESS_DELETE;
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
}

} // namespace fb::bot
