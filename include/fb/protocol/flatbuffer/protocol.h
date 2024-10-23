#ifndef __FLATBUFFER_PROTOCOL_H__
#define __FLATBUFFER_PROTOCOL_H__

#include <string>
#include <optional>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.position_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.character_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.item_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.spell_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.articlesummary_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.article_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.account_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.changepw_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.initcharacter_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.authenticate_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.login_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.makecharacter_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.reservename_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.save_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.getarticle_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.getarticlelist_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.writearticle_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.deletearticle_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.deletearticle_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.getarticle_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.getarticlelist_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.writearticle_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.changepw_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.getuid_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.initcharacter_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.authenticate_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.login_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.makecharacter_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.reservename_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.save_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.service_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.transferresult_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.login_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.logout_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.ping_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.transfer_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.whisper_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.kickout_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.kickout_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.login_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.logout_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.pong_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.transfer_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.whisper_generated.h>

namespace fb::protocol::db
{
    class Position;
    class Character;
    class Item;
    class Spell;
    class ArticleSummary;
    class Article;
} // end of namespace fb::protocol::db
namespace fb::protocol::db::request
{
    class Account;
    class ChangePw;
    class InitCharacter;
    class Authenticate;
    class Login;
    class MakeCharacter;
    class ReserveName;
    class Save;
    class GetArticle;
    class GetArticleList;
    class WriteArticle;
    class DeleteArticle;
} // end of namespace fb::protocol::db::request
namespace fb::protocol::db::response
{
    class DeleteArticle;
    class GetArticle;
    class GetArticleList;
    class WriteArticle;
    class ChangePw;
    class GetUid;
    class InitCharacter;
    class Authenticate;
    class Login;
    class MakeCharacter;
    class ReserveName;
    class Save;
} // end of namespace fb::protocol::db::response
namespace fb::protocol::internal
{
    enum class Service : int8_t;
    enum class TransferResult : int8_t;
} // end of namespace fb::protocol::internal
namespace fb::protocol::internal::request
{
    class Login;
    class Logout;
    class Ping;
    class Transfer;
    class Whisper;
    class KickOut;
} // end of namespace fb::protocol::internal::request
namespace fb::protocol::internal::response
{
    class KickOut;
    class Login;
    class Logout;
    class Pong;
    class Transfer;
    class Whisper;
} // end of namespace fb::protocol::internal::response


namespace flatbuffers {
template<typename>   constexpr bool is_optional_impl = false;
template<typename T> constexpr bool is_optional_impl<std::optional<T>> = true;
template<>           constexpr bool is_optional_impl<std::nullopt_t> = true;
template<typename T> constexpr bool is_optional = is_optional_impl<std::decay_t<T>>;

template<typename>   constexpr bool is_vector_impl = false;
template<typename T> constexpr bool is_vector_impl<std::vector<T>> = true;
template<typename T> constexpr bool is_vector = is_vector_impl<std::decay_t<T>>;


template <typename T> struct FlatBufferOffset { typedef T type; };
template <> struct FlatBufferOffset<std::string> { typedef flatbuffers::Offset<flatbuffers::String> type; };
template <> struct FlatBufferOffset<std::optional<uint16_t>> { typedef flatbuffers::Offset< nullable::nullable_ushort> type; };
template <> struct FlatBufferOffset<std::optional<uint8_t>> { typedef flatbuffers::Offset< nullable::nullable_ubyte> type; };
template <> struct FlatBufferOffset<std::optional<uint32_t>> { typedef flatbuffers::Offset< nullable::nullable_uint> type; };
template <> struct FlatBufferOffset<fb::protocol::db::Position> { typedef flatbuffers::Offset<fb::protocol::db::raw::Position> type; };
template <> struct FlatBufferOffset<fb::protocol::db::Character> { typedef flatbuffers::Offset<fb::protocol::db::raw::Character> type; };
template <> struct FlatBufferOffset<fb::protocol::db::Item> { typedef flatbuffers::Offset<fb::protocol::db::raw::Item> type; };
template <> struct FlatBufferOffset<fb::protocol::db::Spell> { typedef flatbuffers::Offset<fb::protocol::db::raw::Spell> type; };
template <> struct FlatBufferOffset<fb::protocol::db::ArticleSummary> { typedef flatbuffers::Offset<fb::protocol::db::raw::ArticleSummary> type; };
template <> struct FlatBufferOffset<fb::protocol::db::Article> { typedef flatbuffers::Offset<fb::protocol::db::raw::Article> type; };
template <> struct FlatBufferOffset<fb::protocol::db::request::Account> { typedef flatbuffers::Offset<fb::protocol::db::request::raw::Account> type; };
template <> struct FlatBufferOffset<fb::protocol::db::request::ChangePw> { typedef flatbuffers::Offset<fb::protocol::db::request::raw::ChangePw> type; };
template <> struct FlatBufferOffset<fb::protocol::db::request::InitCharacter> { typedef flatbuffers::Offset<fb::protocol::db::request::raw::InitCharacter> type; };
template <> struct FlatBufferOffset<fb::protocol::db::request::Authenticate> { typedef flatbuffers::Offset<fb::protocol::db::request::raw::Authenticate> type; };
template <> struct FlatBufferOffset<fb::protocol::db::request::Login> { typedef flatbuffers::Offset<fb::protocol::db::request::raw::Login> type; };
template <> struct FlatBufferOffset<fb::protocol::db::request::MakeCharacter> { typedef flatbuffers::Offset<fb::protocol::db::request::raw::MakeCharacter> type; };
template <> struct FlatBufferOffset<fb::protocol::db::request::ReserveName> { typedef flatbuffers::Offset<fb::protocol::db::request::raw::ReserveName> type; };
template <> struct FlatBufferOffset<fb::protocol::db::request::Save> { typedef flatbuffers::Offset<fb::protocol::db::request::raw::Save> type; };
template <> struct FlatBufferOffset<fb::protocol::db::request::GetArticle> { typedef flatbuffers::Offset<fb::protocol::db::request::raw::GetArticle> type; };
template <> struct FlatBufferOffset<fb::protocol::db::request::GetArticleList> { typedef flatbuffers::Offset<fb::protocol::db::request::raw::GetArticleList> type; };
template <> struct FlatBufferOffset<fb::protocol::db::request::WriteArticle> { typedef flatbuffers::Offset<fb::protocol::db::request::raw::WriteArticle> type; };
template <> struct FlatBufferOffset<fb::protocol::db::request::DeleteArticle> { typedef flatbuffers::Offset<fb::protocol::db::request::raw::DeleteArticle> type; };
template <> struct FlatBufferOffset<fb::protocol::db::response::DeleteArticle> { typedef flatbuffers::Offset<fb::protocol::db::response::raw::DeleteArticle> type; };
template <> struct FlatBufferOffset<fb::protocol::db::response::GetArticle> { typedef flatbuffers::Offset<fb::protocol::db::response::raw::GetArticle> type; };
template <> struct FlatBufferOffset<fb::protocol::db::response::GetArticleList> { typedef flatbuffers::Offset<fb::protocol::db::response::raw::GetArticleList> type; };
template <> struct FlatBufferOffset<fb::protocol::db::response::WriteArticle> { typedef flatbuffers::Offset<fb::protocol::db::response::raw::WriteArticle> type; };
template <> struct FlatBufferOffset<fb::protocol::db::response::ChangePw> { typedef flatbuffers::Offset<fb::protocol::db::response::raw::ChangePw> type; };
template <> struct FlatBufferOffset<fb::protocol::db::response::GetUid> { typedef flatbuffers::Offset<fb::protocol::db::response::raw::GetUid> type; };
template <> struct FlatBufferOffset<fb::protocol::db::response::InitCharacter> { typedef flatbuffers::Offset<fb::protocol::db::response::raw::InitCharacter> type; };
template <> struct FlatBufferOffset<fb::protocol::db::response::Authenticate> { typedef flatbuffers::Offset<fb::protocol::db::response::raw::Authenticate> type; };
template <> struct FlatBufferOffset<fb::protocol::db::response::Login> { typedef flatbuffers::Offset<fb::protocol::db::response::raw::Login> type; };
template <> struct FlatBufferOffset<fb::protocol::db::response::MakeCharacter> { typedef flatbuffers::Offset<fb::protocol::db::response::raw::MakeCharacter> type; };
template <> struct FlatBufferOffset<fb::protocol::db::response::ReserveName> { typedef flatbuffers::Offset<fb::protocol::db::response::raw::ReserveName> type; };
template <> struct FlatBufferOffset<fb::protocol::db::response::Save> { typedef flatbuffers::Offset<fb::protocol::db::response::raw::Save> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::Login> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::Login> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::Logout> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::Logout> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::Ping> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::Ping> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::Transfer> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::Transfer> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::Whisper> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::Whisper> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::KickOut> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::KickOut> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::KickOut> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::KickOut> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::Login> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::Login> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::Logout> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::Logout> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::Pong> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::Pong> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::Transfer> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::Transfer> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::Whisper> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::Whisper> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::Service> { typedef fb::protocol::internal::raw::Service type; };
template <> struct FlatBufferOffset<fb::protocol::internal::TransferResult> { typedef fb::protocol::internal::raw::TransferResult type; };
template <typename T> struct FlatBufferOffset<std::optional<T>> { typedef FlatBufferOffset<typename T>::type type; };
template <typename T> struct FlatBufferOffset<std::vector<T>> { typedef flatbuffers::Offset<flatbuffers::Vector<typename FlatBufferOffset<T>::type>> type; };

template <typename T> inline static
typename FlatBufferOffset<T>::type build(FlatBufferBuilder& builder, const T& value);
template <> inline static
flatbuffers::Offset<flatbuffers::String> build<std::string>(FlatBufferBuilder& builder, const std::string& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::raw::Position> build<fb::protocol::db::Position>(FlatBufferBuilder& builder, const fb::protocol::db::Position& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::raw::Character> build<fb::protocol::db::Character>(FlatBufferBuilder& builder, const fb::protocol::db::Character& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::raw::Item> build<fb::protocol::db::Item>(FlatBufferBuilder& builder, const fb::protocol::db::Item& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::raw::Spell> build<fb::protocol::db::Spell>(FlatBufferBuilder& builder, const fb::protocol::db::Spell& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::raw::ArticleSummary> build<fb::protocol::db::ArticleSummary>(FlatBufferBuilder& builder, const fb::protocol::db::ArticleSummary& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::raw::Article> build<fb::protocol::db::Article>(FlatBufferBuilder& builder, const fb::protocol::db::Article& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::request::raw::Account> build<fb::protocol::db::request::Account>(FlatBufferBuilder& builder, const fb::protocol::db::request::Account& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::request::raw::ChangePw> build<fb::protocol::db::request::ChangePw>(FlatBufferBuilder& builder, const fb::protocol::db::request::ChangePw& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::request::raw::InitCharacter> build<fb::protocol::db::request::InitCharacter>(FlatBufferBuilder& builder, const fb::protocol::db::request::InitCharacter& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::request::raw::Authenticate> build<fb::protocol::db::request::Authenticate>(FlatBufferBuilder& builder, const fb::protocol::db::request::Authenticate& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::request::raw::Login> build<fb::protocol::db::request::Login>(FlatBufferBuilder& builder, const fb::protocol::db::request::Login& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::request::raw::MakeCharacter> build<fb::protocol::db::request::MakeCharacter>(FlatBufferBuilder& builder, const fb::protocol::db::request::MakeCharacter& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::request::raw::ReserveName> build<fb::protocol::db::request::ReserveName>(FlatBufferBuilder& builder, const fb::protocol::db::request::ReserveName& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::request::raw::Save> build<fb::protocol::db::request::Save>(FlatBufferBuilder& builder, const fb::protocol::db::request::Save& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::request::raw::GetArticle> build<fb::protocol::db::request::GetArticle>(FlatBufferBuilder& builder, const fb::protocol::db::request::GetArticle& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::request::raw::GetArticleList> build<fb::protocol::db::request::GetArticleList>(FlatBufferBuilder& builder, const fb::protocol::db::request::GetArticleList& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::request::raw::WriteArticle> build<fb::protocol::db::request::WriteArticle>(FlatBufferBuilder& builder, const fb::protocol::db::request::WriteArticle& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::request::raw::DeleteArticle> build<fb::protocol::db::request::DeleteArticle>(FlatBufferBuilder& builder, const fb::protocol::db::request::DeleteArticle& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::response::raw::DeleteArticle> build<fb::protocol::db::response::DeleteArticle>(FlatBufferBuilder& builder, const fb::protocol::db::response::DeleteArticle& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::response::raw::GetArticle> build<fb::protocol::db::response::GetArticle>(FlatBufferBuilder& builder, const fb::protocol::db::response::GetArticle& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::response::raw::GetArticleList> build<fb::protocol::db::response::GetArticleList>(FlatBufferBuilder& builder, const fb::protocol::db::response::GetArticleList& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::response::raw::WriteArticle> build<fb::protocol::db::response::WriteArticle>(FlatBufferBuilder& builder, const fb::protocol::db::response::WriteArticle& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::response::raw::ChangePw> build<fb::protocol::db::response::ChangePw>(FlatBufferBuilder& builder, const fb::protocol::db::response::ChangePw& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::response::raw::GetUid> build<fb::protocol::db::response::GetUid>(FlatBufferBuilder& builder, const fb::protocol::db::response::GetUid& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::response::raw::InitCharacter> build<fb::protocol::db::response::InitCharacter>(FlatBufferBuilder& builder, const fb::protocol::db::response::InitCharacter& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::response::raw::Authenticate> build<fb::protocol::db::response::Authenticate>(FlatBufferBuilder& builder, const fb::protocol::db::response::Authenticate& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::response::raw::Login> build<fb::protocol::db::response::Login>(FlatBufferBuilder& builder, const fb::protocol::db::response::Login& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::response::raw::MakeCharacter> build<fb::protocol::db::response::MakeCharacter>(FlatBufferBuilder& builder, const fb::protocol::db::response::MakeCharacter& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::response::raw::ReserveName> build<fb::protocol::db::response::ReserveName>(FlatBufferBuilder& builder, const fb::protocol::db::response::ReserveName& value);
template <> inline static
flatbuffers::Offset<fb::protocol::db::response::raw::Save> build<fb::protocol::db::response::Save>(FlatBufferBuilder& builder, const fb::protocol::db::response::Save& value);
template <> inline static
flatbuffers::Offset<fb::protocol::internal::request::raw::Login> build<fb::protocol::internal::request::Login>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Login& value);
template <> inline static
flatbuffers::Offset<fb::protocol::internal::request::raw::Logout> build<fb::protocol::internal::request::Logout>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Logout& value);
template <> inline static
flatbuffers::Offset<fb::protocol::internal::request::raw::Ping> build<fb::protocol::internal::request::Ping>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Ping& value);
template <> inline static
flatbuffers::Offset<fb::protocol::internal::request::raw::Transfer> build<fb::protocol::internal::request::Transfer>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Transfer& value);
template <> inline static
flatbuffers::Offset<fb::protocol::internal::request::raw::Whisper> build<fb::protocol::internal::request::Whisper>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Whisper& value);
template <> inline static
flatbuffers::Offset<fb::protocol::internal::request::raw::KickOut> build<fb::protocol::internal::request::KickOut>(FlatBufferBuilder& builder, const fb::protocol::internal::request::KickOut& value);
template <> inline static
flatbuffers::Offset<fb::protocol::internal::response::raw::KickOut> build<fb::protocol::internal::response::KickOut>(FlatBufferBuilder& builder, const fb::protocol::internal::response::KickOut& value);
template <> inline static
flatbuffers::Offset<fb::protocol::internal::response::raw::Login> build<fb::protocol::internal::response::Login>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Login& value);
template <> inline static
flatbuffers::Offset<fb::protocol::internal::response::raw::Logout> build<fb::protocol::internal::response::Logout>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Logout& value);
template <> inline static
flatbuffers::Offset<fb::protocol::internal::response::raw::Pong> build<fb::protocol::internal::response::Pong>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Pong& value);
template <> inline static
flatbuffers::Offset<fb::protocol::internal::response::raw::Transfer> build<fb::protocol::internal::response::Transfer>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Transfer& value);
template <> inline static
flatbuffers::Offset<fb::protocol::internal::response::raw::Whisper> build<fb::protocol::internal::response::Whisper>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Whisper& value);

template <typename T> inline static
std::vector<T> unpack(const flatbuffers::Vector<typename FlatBufferOffset<T>::type> *value);

} // end of namespace flatbuffers

namespace fb::protocol::internal {

enum class Service : int8_t
{
    Gateway = fb::protocol::internal::raw::Service::Service_Gateway,
    Login = fb::protocol::internal::raw::Service::Service_Login,
    Game = fb::protocol::internal::raw::Service::Service_Game,
};
enum class TransferResult : int8_t
{
    Success = fb::protocol::internal::raw::TransferResult::TransferResult_Success,
    Failed = fb::protocol::internal::raw::TransferResult::TransferResult_Failed,
    LoggedIn = fb::protocol::internal::raw::TransferResult::TransferResult_LoggedIn,
};

} // end of namespace fb::protocol::internal

namespace fb::protocol::db {

enum class FlatBufferProtocolType
{
    Position,
    Character,
    Item,
    Spell,
    ArticleSummary,
    Article,
};

class Position
{
public:
    static inline fb::protocol::db::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::FlatBufferProtocolType::Position;

public:
    uint16_t x = 0;
    uint16_t y = 0;

public:
    Position() = default;

    Position(const Position& x)
        : x(x.x), y(x.y)
    { }

    Position(uint16_t x, uint16_t y)
        : x(x), y(y)
    { }

    Position(const fb::protocol::db::raw::Position& raw)
        : x(raw.x()), y(raw.y())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::Position>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Position Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::raw::GetPosition(bytes);
        return Position(*raw);
    }
};
class Character
{
public:
    static inline fb::protocol::db::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::FlatBufferProtocolType::Character;

public:
    uint32_t id = 0;
    std::string name;
    std::string last_login;
    bool admin = false;
    uint16_t look = 0;
    uint16_t color = 0;
    uint16_t sex = 0;
    uint16_t nation = 0;
    std::optional<uint16_t> creature = std::nullopt;
    uint32_t map = 0;
    fb::protocol::db::Position position;
    uint8_t direction = 0;
    uint8_t state = 0;
    uint8_t class_type = 0;
    uint8_t promotion = 0;
    uint32_t exp = 0;
    uint32_t money = 0;
    uint32_t deposited_money = 0;
    std::optional<uint16_t> disguise = std::nullopt;
    uint32_t hp = 0;
    uint32_t base_hp = 0;
    uint32_t additional_hp = 0;
    uint32_t mp = 0;
    uint32_t base_mp = 0;
    uint32_t additional_mp = 0;
    std::optional<uint8_t> weapon_color = std::nullopt;
    std::optional<uint8_t> helmet_color = std::nullopt;
    std::optional<uint8_t> armor_color = std::nullopt;
    std::optional<uint8_t> shield_color = std::nullopt;
    std::optional<uint8_t> ring_left_color = std::nullopt;
    std::optional<uint8_t> ring_right_color = std::nullopt;
    std::optional<uint8_t> aux_top_color = std::nullopt;
    std::optional<uint8_t> aux_bot_color = std::nullopt;
    std::optional<uint32_t> clan = std::nullopt;

public:
    Character() = default;

    Character(const Character& x)
        : id(x.id), name(x.name), last_login(x.last_login), admin(x.admin), look(x.look), color(x.color), sex(x.sex), nation(x.nation), creature(x.creature), map(x.map), position(x.position), direction(x.direction), state(x.state), class_type(x.class_type), promotion(x.promotion), exp(x.exp), money(x.money), deposited_money(x.deposited_money), disguise(x.disguise), hp(x.hp), base_hp(x.base_hp), additional_hp(x.additional_hp), mp(x.mp), base_mp(x.base_mp), additional_mp(x.additional_mp), weapon_color(x.weapon_color), helmet_color(x.helmet_color), armor_color(x.armor_color), shield_color(x.shield_color), ring_left_color(x.ring_left_color), ring_right_color(x.ring_right_color), aux_top_color(x.aux_top_color), aux_bot_color(x.aux_bot_color), clan(x.clan)
    { }

    Character(uint32_t id, const std::string& name, const std::string& last_login, bool admin, uint16_t look, uint16_t color, uint16_t sex, uint16_t nation, const std::optional<uint16_t>& creature, uint32_t map, const fb::protocol::db::Position& position, uint8_t direction, uint8_t state, uint8_t class_type, uint8_t promotion, uint32_t exp, uint32_t money, uint32_t deposited_money, const std::optional<uint16_t>& disguise, uint32_t hp, uint32_t base_hp, uint32_t additional_hp, uint32_t mp, uint32_t base_mp, uint32_t additional_mp, const std::optional<uint8_t>& weapon_color, const std::optional<uint8_t>& helmet_color, const std::optional<uint8_t>& armor_color, const std::optional<uint8_t>& shield_color, const std::optional<uint8_t>& ring_left_color, const std::optional<uint8_t>& ring_right_color, const std::optional<uint8_t>& aux_top_color, const std::optional<uint8_t>& aux_bot_color, const std::optional<uint32_t>& clan)
        : id(id), name(name), last_login(last_login), admin(admin), look(look), color(color), sex(sex), nation(nation), creature(creature), map(map), position(position), direction(direction), state(state), class_type(class_type), promotion(promotion), exp(exp), money(money), deposited_money(deposited_money), disguise(disguise), hp(hp), base_hp(base_hp), additional_hp(additional_hp), mp(mp), base_mp(base_mp), additional_mp(additional_mp), weapon_color(weapon_color), helmet_color(helmet_color), armor_color(armor_color), shield_color(shield_color), ring_left_color(ring_left_color), ring_right_color(ring_right_color), aux_top_color(aux_top_color), aux_bot_color(aux_bot_color), clan(clan)
    { }

    Character(const fb::protocol::db::raw::Character& raw)
        : id(raw.id()), name(raw.name()->c_str()), last_login(raw.last_login()->c_str()), admin(raw.admin()), look(raw.look()), color(raw.color()), sex(raw.sex()), nation(raw.nation()), creature(raw.creature() != nullptr ? raw.creature()->value() : std::optional<uint16_t>()), map(raw.map()), position(*raw.position()), direction(raw.direction()), state(raw.state()), class_type(raw.class_type()), promotion(raw.promotion()), exp(raw.exp()), money(raw.money()), deposited_money(raw.deposited_money()), disguise(raw.disguise() != nullptr ? raw.disguise()->value() : std::optional<uint16_t>()), hp(raw.hp()), base_hp(raw.base_hp()), additional_hp(raw.additional_hp()), mp(raw.mp()), base_mp(raw.base_mp()), additional_mp(raw.additional_mp()), weapon_color(raw.weapon_color() != nullptr ? raw.weapon_color()->value() : std::optional<uint8_t>()), helmet_color(raw.helmet_color() != nullptr ? raw.helmet_color()->value() : std::optional<uint8_t>()), armor_color(raw.armor_color() != nullptr ? raw.armor_color()->value() : std::optional<uint8_t>()), shield_color(raw.shield_color() != nullptr ? raw.shield_color()->value() : std::optional<uint8_t>()), ring_left_color(raw.ring_left_color() != nullptr ? raw.ring_left_color()->value() : std::optional<uint8_t>()), ring_right_color(raw.ring_right_color() != nullptr ? raw.ring_right_color()->value() : std::optional<uint8_t>()), aux_top_color(raw.aux_top_color() != nullptr ? raw.aux_top_color()->value() : std::optional<uint8_t>()), aux_bot_color(raw.aux_bot_color() != nullptr ? raw.aux_bot_color()->value() : std::optional<uint8_t>()), clan(raw.clan() != nullptr ? raw.clan()->value() : std::optional<uint32_t>())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::Character>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Character Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::raw::GetCharacter(bytes);
        return Character(*raw);
    }
};
class Item
{
public:
    static inline fb::protocol::db::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::FlatBufferProtocolType::Item;

public:
    uint32_t user = 0;
    int16_t index = 0;
    int16_t parts = 0;
    int16_t deposited = 0;
    uint32_t model = 0;
    uint16_t count = 0;
    std::optional<uint32_t> durability = std::nullopt;
    std::optional<std::string> custom_name = std::nullopt;

public:
    Item() = default;

    Item(const Item& x)
        : user(x.user), index(x.index), parts(x.parts), deposited(x.deposited), model(x.model), count(x.count), durability(x.durability), custom_name(x.custom_name)
    { }

    Item(uint32_t user, int16_t index, int16_t parts, int16_t deposited, uint32_t model, uint16_t count, const std::optional<uint32_t>& durability, const std::optional<std::string>& custom_name)
        : user(user), index(index), parts(parts), deposited(deposited), model(model), count(count), durability(durability), custom_name(custom_name)
    { }

    Item(const fb::protocol::db::raw::Item& raw)
        : user(raw.user()), index(raw.index()), parts(raw.parts()), deposited(raw.deposited()), model(raw.model()), count(raw.count()), durability(raw.durability() != nullptr ? raw.durability()->value() : std::optional<uint32_t>()), custom_name(raw.custom_name() != nullptr ? raw.custom_name()->c_str() : std::optional<std::string>())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::Item>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Item Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::raw::GetItem(bytes);
        return Item(*raw);
    }
};
class Spell
{
public:
    static inline fb::protocol::db::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::FlatBufferProtocolType::Spell;

public:
    uint32_t user = 0;
    uint8_t slot = 0;
    uint32_t model = 0;

public:
    Spell() = default;

    Spell(const Spell& x)
        : user(x.user), slot(x.slot), model(x.model)
    { }

    Spell(uint32_t user, uint8_t slot, uint32_t model)
        : user(user), slot(slot), model(model)
    { }

    Spell(const fb::protocol::db::raw::Spell& raw)
        : user(raw.user()), slot(raw.slot()), model(raw.model())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::Spell>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Spell Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::raw::GetSpell(bytes);
        return Spell(*raw);
    }
};
class ArticleSummary
{
public:
    static inline fb::protocol::db::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::FlatBufferProtocolType::ArticleSummary;

public:
    uint32_t id = 0;
    uint32_t user = 0;
    std::string user_name;
    std::string title;
    std::string created_date;

public:
    ArticleSummary() = default;

    ArticleSummary(const ArticleSummary& x)
        : id(x.id), user(x.user), user_name(x.user_name), title(x.title), created_date(x.created_date)
    { }

    ArticleSummary(uint32_t id, uint32_t user, const std::string& user_name, const std::string& title, const std::string& created_date)
        : id(id), user(user), user_name(user_name), title(title), created_date(created_date)
    { }

    ArticleSummary(const fb::protocol::db::raw::ArticleSummary& raw)
        : id(raw.id()), user(raw.user()), user_name(raw.user_name()->c_str()), title(raw.title()->c_str()), created_date(raw.created_date()->c_str())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::ArticleSummary>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static ArticleSummary Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::raw::GetArticleSummary(bytes);
        return ArticleSummary(*raw);
    }
};
class Article
{
public:
    static inline fb::protocol::db::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::FlatBufferProtocolType::Article;

public:
    uint32_t id = 0;
    uint32_t user = 0;
    std::string user_name;
    std::string title;
    std::string contents;
    std::string created_date;

public:
    Article() = default;

    Article(const Article& x)
        : id(x.id), user(x.user), user_name(x.user_name), title(x.title), contents(x.contents), created_date(x.created_date)
    { }

    Article(uint32_t id, uint32_t user, const std::string& user_name, const std::string& title, const std::string& contents, const std::string& created_date)
        : id(id), user(user), user_name(user_name), title(title), contents(contents), created_date(created_date)
    { }

    Article(const fb::protocol::db::raw::Article& raw)
        : id(raw.id()), user(raw.user()), user_name(raw.user_name()->c_str()), title(raw.title()->c_str()), contents(raw.contents()->c_str()), created_date(raw.created_date()->c_str())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::Article>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Article Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::raw::GetArticle(bytes);
        return Article(*raw);
    }
};

} // end of namespace fb::protocol::db

namespace fb::protocol::db::request {

enum class FlatBufferProtocolType
{
    Account,
    ChangePw,
    InitCharacter,
    Authenticate,
    Login,
    MakeCharacter,
    ReserveName,
    Save,
    GetArticle,
    GetArticleList,
    WriteArticle,
    DeleteArticle,
};

class Account
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::Account;

public:
    uint32_t uid = 0;

public:
    Account() = default;

    Account(const Account& x)
        : uid(x.uid)
    { }

    Account(uint32_t uid)
        : uid(uid)
    { }

    Account(const fb::protocol::db::request::raw::Account& raw)
        : uid(raw.uid())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::request::Account>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Account Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetAccount(bytes);
        return Account(*raw);
    }
};
class ChangePw
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::ChangePw;

public:
    uint32_t uid = 0;
    std::string before;
    std::string after;
    uint32_t birthday = 0;

public:
    ChangePw() = default;

    ChangePw(const ChangePw& x)
        : uid(x.uid), before(x.before), after(x.after), birthday(x.birthday)
    { }

    ChangePw(uint32_t uid, const std::string& before, const std::string& after, uint32_t birthday)
        : uid(uid), before(before), after(after), birthday(birthday)
    { }

    ChangePw(const fb::protocol::db::request::raw::ChangePw& raw)
        : uid(raw.uid()), before(raw.before()->c_str()), after(raw.after()->c_str()), birthday(raw.birthday())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::request::ChangePw>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static ChangePw Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetChangePw(bytes);
        return ChangePw(*raw);
    }
};
class InitCharacter
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::InitCharacter;

public:
    uint32_t uid = 0;
    std::string name;
    std::string pw;
    uint32_t hp = 0;
    uint32_t mp = 0;
    uint16_t map = 0;
    uint16_t x = 0;
    uint16_t y = 0;
    bool admin = false;

public:
    InitCharacter() = default;

    InitCharacter(const InitCharacter& x)
        : uid(x.uid), name(x.name), pw(x.pw), hp(x.hp), mp(x.mp), map(x.map), x(x.x), y(x.y), admin(x.admin)
    { }

    InitCharacter(uint32_t uid, const std::string& name, const std::string& pw, uint32_t hp, uint32_t mp, uint16_t map, uint16_t x, uint16_t y, bool admin)
        : uid(uid), name(name), pw(pw), hp(hp), mp(mp), map(map), x(x), y(y), admin(admin)
    { }

    InitCharacter(const fb::protocol::db::request::raw::InitCharacter& raw)
        : uid(raw.uid()), name(raw.name()->c_str()), pw(raw.pw()->c_str()), hp(raw.hp()), mp(raw.mp()), map(raw.map()), x(raw.x()), y(raw.y()), admin(raw.admin())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::request::InitCharacter>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static InitCharacter Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetInitCharacter(bytes);
        return InitCharacter(*raw);
    }
};
class Authenticate
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::Authenticate;

public:
    uint32_t uid = 0;
    std::string pw;

public:
    Authenticate() = default;

    Authenticate(const Authenticate& x)
        : uid(x.uid), pw(x.pw)
    { }

    Authenticate(uint32_t uid, const std::string& pw)
        : uid(uid), pw(pw)
    { }

    Authenticate(const fb::protocol::db::request::raw::Authenticate& raw)
        : uid(raw.uid()), pw(raw.pw()->c_str())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::request::Authenticate>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Authenticate Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetAuthenticate(bytes);
        return Authenticate(*raw);
    }
};
class Login
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::Login;

public:
    uint32_t uid = 0;

public:
    Login() = default;

    Login(const Login& x)
        : uid(x.uid)
    { }

    Login(uint32_t uid)
        : uid(uid)
    { }

    Login(const fb::protocol::db::request::raw::Login& raw)
        : uid(raw.uid())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::request::Login>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Login Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetLogin(bytes);
        return Login(*raw);
    }
};
class MakeCharacter
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::MakeCharacter;

public:
    uint32_t uid = 0;
    uint16_t hair = 0;
    uint8_t sex = 0;
    uint8_t nation = 0;
    uint8_t creature = 0;

public:
    MakeCharacter() = default;

    MakeCharacter(const MakeCharacter& x)
        : uid(x.uid), hair(x.hair), sex(x.sex), nation(x.nation), creature(x.creature)
    { }

    MakeCharacter(uint32_t uid, uint16_t hair, uint8_t sex, uint8_t nation, uint8_t creature)
        : uid(uid), hair(hair), sex(sex), nation(nation), creature(creature)
    { }

    MakeCharacter(const fb::protocol::db::request::raw::MakeCharacter& raw)
        : uid(raw.uid()), hair(raw.hair()), sex(raw.sex()), nation(raw.nation()), creature(raw.creature())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::request::MakeCharacter>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static MakeCharacter Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetMakeCharacter(bytes);
        return MakeCharacter(*raw);
    }
};
class ReserveName
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::ReserveName;

public:
    std::string name;

public:
    ReserveName() = default;

    ReserveName(const ReserveName& x)
        : name(x.name)
    { }

    ReserveName(const std::string& name)
        : name(name)
    { }

    ReserveName(const fb::protocol::db::request::raw::ReserveName& raw)
        : name(raw.name()->c_str())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::request::ReserveName>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static ReserveName Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetReserveName(bytes);
        return ReserveName(*raw);
    }
};
class Save
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::Save;

public:
    fb::protocol::db::Character character;
    std::vector<fb::protocol::db::Item> items = {};
    std::vector<fb::protocol::db::Spell> spells = {};

public:
    Save() = default;

    Save(const Save& x)
        : character(x.character), items(x.items), spells(x.spells)
    { }

    Save(const fb::protocol::db::Character& character, std::vector<fb::protocol::db::Item> items, std::vector<fb::protocol::db::Spell> spells)
        : character(character), items(items), spells(spells)
    { }

    Save(const fb::protocol::db::request::raw::Save& raw)
        : character(*raw.character()), items(unpack<fb::protocol::db::Item>(raw.items())), spells(unpack<fb::protocol::db::Spell>(raw.spells()))
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::request::Save>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Save Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetSave(bytes);
        return Save(*raw);
    }
};
class GetArticle
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::GetArticle;

public:
    uint32_t section = 0;
    uint32_t article = 0;

public:
    GetArticle() = default;

    GetArticle(const GetArticle& x)
        : section(x.section), article(x.article)
    { }

    GetArticle(uint32_t section, uint32_t article)
        : section(section), article(article)
    { }

    GetArticle(const fb::protocol::db::request::raw::GetArticle& raw)
        : section(raw.section()), article(raw.article())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::request::GetArticle>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static GetArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetGetArticle(bytes);
        return GetArticle(*raw);
    }
};
class GetArticleList
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::GetArticleList;

public:
    uint32_t section = 0;
    uint32_t position = 0;

public:
    GetArticleList() = default;

    GetArticleList(const GetArticleList& x)
        : section(x.section), position(x.position)
    { }

    GetArticleList(uint32_t section, uint32_t position)
        : section(section), position(position)
    { }

    GetArticleList(const fb::protocol::db::request::raw::GetArticleList& raw)
        : section(raw.section()), position(raw.position())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::request::GetArticleList>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static GetArticleList Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetGetArticleList(bytes);
        return GetArticleList(*raw);
    }
};
class WriteArticle
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::WriteArticle;

public:
    uint32_t section = 0;
    uint32_t user = 0;
    std::string title;
    std::string contents;

public:
    WriteArticle() = default;

    WriteArticle(const WriteArticle& x)
        : section(x.section), user(x.user), title(x.title), contents(x.contents)
    { }

    WriteArticle(uint32_t section, uint32_t user, const std::string& title, const std::string& contents)
        : section(section), user(user), title(title), contents(contents)
    { }

    WriteArticle(const fb::protocol::db::request::raw::WriteArticle& raw)
        : section(raw.section()), user(raw.user()), title(raw.title()->c_str()), contents(raw.contents()->c_str())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::request::WriteArticle>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static WriteArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetWriteArticle(bytes);
        return WriteArticle(*raw);
    }
};
class DeleteArticle
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::DeleteArticle;

public:
    uint32_t id = 0;
    uint32_t user = 0;

public:
    DeleteArticle() = default;

    DeleteArticle(const DeleteArticle& x)
        : id(x.id), user(x.user)
    { }

    DeleteArticle(uint32_t id, uint32_t user)
        : id(id), user(user)
    { }

    DeleteArticle(const fb::protocol::db::request::raw::DeleteArticle& raw)
        : id(raw.id()), user(raw.user())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::request::DeleteArticle>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static DeleteArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetDeleteArticle(bytes);
        return DeleteArticle(*raw);
    }
};

} // end of namespace fb::protocol::db::request

namespace fb::protocol::db::response {

enum class FlatBufferProtocolType
{
    DeleteArticle,
    GetArticle,
    GetArticleList,
    WriteArticle,
    ChangePw,
    GetUid,
    InitCharacter,
    Authenticate,
    Login,
    MakeCharacter,
    ReserveName,
    Save,
};

class DeleteArticle
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::DeleteArticle;

public:
    int result = 0;

public:
    DeleteArticle() = default;

    DeleteArticle(const DeleteArticle& x)
        : result(x.result)
    { }

    DeleteArticle(int result)
        : result(result)
    { }

    DeleteArticle(const fb::protocol::db::response::raw::DeleteArticle& raw)
        : result(raw.result())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::response::DeleteArticle>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static DeleteArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetDeleteArticle(bytes);
        return DeleteArticle(*raw);
    }
};
class GetArticle
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::GetArticle;

public:
    fb::protocol::db::Article article;
    bool next = false;
    bool success = false;

public:
    GetArticle() = default;

    GetArticle(const GetArticle& x)
        : article(x.article), next(x.next), success(x.success)
    { }

    GetArticle(const fb::protocol::db::Article& article, bool next, bool success)
        : article(article), next(next), success(success)
    { }

    GetArticle(const fb::protocol::db::response::raw::GetArticle& raw)
        : article(*raw.article()), next(raw.next()), success(raw.success())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::response::GetArticle>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static GetArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetGetArticle(bytes);
        return GetArticle(*raw);
    }
};
class GetArticleList
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::GetArticleList;

public:
    std::vector<fb::protocol::db::ArticleSummary> summary_list = {};

public:
    GetArticleList() = default;

    GetArticleList(const GetArticleList& x)
        : summary_list(x.summary_list)
    { }

    GetArticleList(std::vector<fb::protocol::db::ArticleSummary> summary_list)
        : summary_list(summary_list)
    { }

    GetArticleList(const fb::protocol::db::response::raw::GetArticleList& raw)
        : summary_list(unpack<fb::protocol::db::ArticleSummary>(raw.summary_list()))
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::response::GetArticleList>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static GetArticleList Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetGetArticleList(bytes);
        return GetArticleList(*raw);
    }
};
class WriteArticle
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::WriteArticle;

public:
    bool success = false;

public:
    WriteArticle() = default;

    WriteArticle(const WriteArticle& x)
        : success(x.success)
    { }

    WriteArticle(bool success)
        : success(success)
    { }

    WriteArticle(const fb::protocol::db::response::raw::WriteArticle& raw)
        : success(raw.success())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::response::WriteArticle>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static WriteArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetWriteArticle(bytes);
        return WriteArticle(*raw);
    }
};
class ChangePw
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::ChangePw;

public:
    uint32_t error_code = 0;

public:
    ChangePw() = default;

    ChangePw(const ChangePw& x)
        : error_code(x.error_code)
    { }

    ChangePw(uint32_t error_code)
        : error_code(error_code)
    { }

    ChangePw(const fb::protocol::db::response::raw::ChangePw& raw)
        : error_code(raw.error_code())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::response::ChangePw>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static ChangePw Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetChangePw(bytes);
        return ChangePw(*raw);
    }
};
class GetUid
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::GetUid;

public:
    uint32_t uid = 0;
    bool success = false;

public:
    GetUid() = default;

    GetUid(const GetUid& x)
        : uid(x.uid), success(x.success)
    { }

    GetUid(uint32_t uid, bool success)
        : uid(uid), success(success)
    { }

    GetUid(const fb::protocol::db::response::raw::GetUid& raw)
        : uid(raw.uid()), success(raw.success())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::response::GetUid>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static GetUid Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetGetUid(bytes);
        return GetUid(*raw);
    }
};
class InitCharacter
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::InitCharacter;

public:
    bool success = false;

public:
    InitCharacter() = default;

    InitCharacter(const InitCharacter& x)
        : success(x.success)
    { }

    InitCharacter(bool success)
        : success(success)
    { }

    InitCharacter(const fb::protocol::db::response::raw::InitCharacter& raw)
        : success(raw.success())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::response::InitCharacter>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static InitCharacter Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetInitCharacter(bytes);
        return InitCharacter(*raw);
    }
};
class Authenticate
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::Authenticate;

public:
    uint32_t map = 0;
    uint32_t error_code = 0;

public:
    Authenticate() = default;

    Authenticate(const Authenticate& x)
        : map(x.map), error_code(x.error_code)
    { }

    Authenticate(uint32_t map, uint32_t error_code)
        : map(map), error_code(error_code)
    { }

    Authenticate(const fb::protocol::db::response::raw::Authenticate& raw)
        : map(raw.map()), error_code(raw.error_code())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::response::Authenticate>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Authenticate Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetAuthenticate(bytes);
        return Authenticate(*raw);
    }
};
class Login
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::Login;

public:
    fb::protocol::db::Character character;
    std::vector<fb::protocol::db::Item> items = {};
    std::vector<fb::protocol::db::Spell> spells = {};

public:
    Login() = default;

    Login(const Login& x)
        : character(x.character), items(x.items), spells(x.spells)
    { }

    Login(const fb::protocol::db::Character& character, std::vector<fb::protocol::db::Item> items, std::vector<fb::protocol::db::Spell> spells)
        : character(character), items(items), spells(spells)
    { }

    Login(const fb::protocol::db::response::raw::Login& raw)
        : character(*raw.character()), items(unpack<fb::protocol::db::Item>(raw.items())), spells(unpack<fb::protocol::db::Spell>(raw.spells()))
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::response::Login>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Login Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetLogin(bytes);
        return Login(*raw);
    }
};
class MakeCharacter
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::MakeCharacter;

public:
    bool success = false;

public:
    MakeCharacter() = default;

    MakeCharacter(const MakeCharacter& x)
        : success(x.success)
    { }

    MakeCharacter(bool success)
        : success(success)
    { }

    MakeCharacter(const fb::protocol::db::response::raw::MakeCharacter& raw)
        : success(raw.success())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::response::MakeCharacter>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static MakeCharacter Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetMakeCharacter(bytes);
        return MakeCharacter(*raw);
    }
};
class ReserveName
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::ReserveName;

public:
    uint32_t uid = 0;
    bool success = false;

public:
    ReserveName() = default;

    ReserveName(const ReserveName& x)
        : uid(x.uid), success(x.success)
    { }

    ReserveName(uint32_t uid, bool success)
        : uid(uid), success(success)
    { }

    ReserveName(const fb::protocol::db::response::raw::ReserveName& raw)
        : uid(raw.uid()), success(raw.success())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::response::ReserveName>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static ReserveName Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetReserveName(bytes);
        return ReserveName(*raw);
    }
};
class Save
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::Save;

public:
    bool success = false;

public:
    Save() = default;

    Save(const Save& x)
        : success(x.success)
    { }

    Save(bool success)
        : success(success)
    { }

    Save(const fb::protocol::db::response::raw::Save& raw)
        : success(raw.success())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::db::response::Save>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Save Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetSave(bytes);
        return Save(*raw);
    }
};

} // end of namespace fb::protocol::db::response

namespace fb::protocol::internal {

enum class FlatBufferProtocolType
{
};


} // end of namespace fb::protocol::internal

namespace fb::protocol::internal::request {

enum class FlatBufferProtocolType
{
    Login,
    Logout,
    Ping,
    Transfer,
    Whisper,
    KickOut,
};

class Login
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::Login;

public:
    uint32_t uid = 0;
    std::string name;
    uint16_t map = 0;

public:
    Login() = default;

    Login(const Login& x)
        : uid(x.uid), name(x.name), map(x.map)
    { }

    Login(uint32_t uid, const std::string& name, uint16_t map)
        : uid(uid), name(name), map(map)
    { }

    Login(const fb::protocol::internal::request::raw::Login& raw)
        : uid(raw.uid()), name(raw.name()->c_str()), map(raw.map())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::Login>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Login Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetLogin(bytes);
        return Login(*raw);
    }
};
class Logout
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::Logout;

public:
    std::string name;

public:
    Logout() = default;

    Logout(const Logout& x)
        : name(x.name)
    { }

    Logout(const std::string& name)
        : name(name)
    { }

    Logout(const fb::protocol::internal::request::raw::Logout& raw)
        : name(raw.name()->c_str())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::Logout>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Logout Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetLogout(bytes);
        return Logout(*raw);
    }
};
class Ping
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::Ping;

public:
    uint8_t id = 0;
    std::string name;
    fb::protocol::internal::Service service;
    std::string ip;
    uint16_t port = 0;

public:
    Ping() = default;

    Ping(const Ping& x)
        : id(x.id), name(x.name), service(x.service), ip(x.ip), port(x.port)
    { }

    Ping(uint8_t id, const std::string& name, fb::protocol::internal::Service service, const std::string& ip, uint16_t port)
        : id(id), name(name), service(service), ip(ip), port(port)
    { }

    Ping(const fb::protocol::internal::request::raw::Ping& raw)
        : id(raw.id()), name(raw.name()->c_str()), service((fb::protocol::internal::Service)raw.service()), ip(raw.ip()->c_str()), port(raw.port())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::Ping>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Ping Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetPing(bytes);
        return Ping(*raw);
    }
};
class Transfer
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::Transfer;

public:
    fb::protocol::internal::Service service;
    uint8_t id = 0;

public:
    Transfer() = default;

    Transfer(const Transfer& x)
        : service(x.service), id(x.id)
    { }

    Transfer(fb::protocol::internal::Service service, uint8_t id)
        : service(service), id(id)
    { }

    Transfer(const fb::protocol::internal::request::raw::Transfer& raw)
        : service((fb::protocol::internal::Service)raw.service()), id(raw.id())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::Transfer>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Transfer Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetTransfer(bytes);
        return Transfer(*raw);
    }
};
class Whisper
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::Whisper;

public:
    std::string from;
    std::string to;
    std::string message;

public:
    Whisper() = default;

    Whisper(const Whisper& x)
        : from(x.from), to(x.to), message(x.message)
    { }

    Whisper(const std::string& from, const std::string& to, const std::string& message)
        : from(from), to(to), message(message)
    { }

    Whisper(const fb::protocol::internal::request::raw::Whisper& raw)
        : from(raw.from()->c_str()), to(raw.to()->c_str()), message(raw.message()->c_str())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::Whisper>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Whisper Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetWhisper(bytes);
        return Whisper(*raw);
    }
};
class KickOut
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::KickOut;

public:
    uint32_t uid = 0;

public:
    KickOut() = default;

    KickOut(const KickOut& x)
        : uid(x.uid)
    { }

    KickOut(uint32_t uid)
        : uid(uid)
    { }

    KickOut(const fb::protocol::internal::request::raw::KickOut& raw)
        : uid(raw.uid())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::KickOut>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static KickOut Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetKickOut(bytes);
        return KickOut(*raw);
    }
};

} // end of namespace fb::protocol::internal::request

namespace fb::protocol::internal::response {

enum class FlatBufferProtocolType
{
    KickOut,
    Login,
    Logout,
    Pong,
    Transfer,
    Whisper,
};

class KickOut
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::KickOut;

public:
    uint32_t uid = 0;

public:
    KickOut() = default;

    KickOut(const KickOut& x)
        : uid(x.uid)
    { }

    KickOut(uint32_t uid)
        : uid(uid)
    { }

    KickOut(const fb::protocol::internal::response::raw::KickOut& raw)
        : uid(raw.uid())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::KickOut>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static KickOut Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetKickOut(bytes);
        return KickOut(*raw);
    }
};
class Login
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::Login;

public:
    bool success = false;
    bool logon = false;
    std::string ip;
    uint16_t port = 0;

public:
    Login() = default;

    Login(const Login& x)
        : success(x.success), logon(x.logon), ip(x.ip), port(x.port)
    { }

    Login(bool success, bool logon, const std::string& ip, uint16_t port)
        : success(success), logon(logon), ip(ip), port(port)
    { }

    Login(const fb::protocol::internal::response::raw::Login& raw)
        : success(raw.success()), logon(raw.logon()), ip(raw.ip()->c_str()), port(raw.port())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::Login>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Login Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetLogin(bytes);
        return Login(*raw);
    }
};
class Logout
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::Logout;

public:
    bool success = false;

public:
    Logout() = default;

    Logout(const Logout& x)
        : success(x.success)
    { }

    Logout(bool success)
        : success(success)
    { }

    Logout(const fb::protocol::internal::response::raw::Logout& raw)
        : success(raw.success())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::Logout>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Logout Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetLogout(bytes);
        return Logout(*raw);
    }
};
class Pong
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::Pong;


public:
    Pong() = default;

    Pong(const Pong& x)
    { }


    Pong(const fb::protocol::internal::response::raw::Pong& raw)
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::Pong>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Pong Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetPong(bytes);
        return Pong(*raw);
    }
};
class Transfer
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::Transfer;

public:
    fb::protocol::internal::TransferResult code;
    std::string ip;
    uint16_t port = 0;

public:
    Transfer() = default;

    Transfer(const Transfer& x)
        : code(x.code), ip(x.ip), port(x.port)
    { }

    Transfer(fb::protocol::internal::TransferResult code, const std::string& ip, uint16_t port)
        : code(code), ip(ip), port(port)
    { }

    Transfer(const fb::protocol::internal::response::raw::Transfer& raw)
        : code((fb::protocol::internal::TransferResult)raw.code()), ip(raw.ip()->c_str()), port(raw.port())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::Transfer>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Transfer Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetTransfer(bytes);
        return Transfer(*raw);
    }
};
class Whisper
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::Whisper;

public:
    bool success = false;
    std::string from;
    uint32_t to = 0;
    std::string message;

public:
    Whisper() = default;

    Whisper(const Whisper& x)
        : success(x.success), from(x.from), to(x.to), message(x.message)
    { }

    Whisper(bool success, const std::string& from, uint32_t to, const std::string& message)
        : success(success), from(from), to(to), message(message)
    { }

    Whisper(const fb::protocol::internal::response::raw::Whisper& raw)
        : success(raw.success()), from(raw.from()->c_str()), to(raw.to()), message(raw.message()->c_str())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::Whisper>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Whisper Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetWhisper(bytes);
        return Whisper(*raw);
    }
};

} // end of namespace fb::protocol::internal::response


namespace flatbuffers {

template <typename T> inline static
typename FlatBufferOffset<T>::type build(FlatBufferBuilder& builder, const T& value)
{
    if constexpr (is_optional<T>)
    {
        if (!value.has_value())
            return 0;
            
        if constexpr (std::is_same_v<T, std::optional<uint16_t>>)
        {
            return nullable::Createnullable_ushort(builder, value.value());
        }

        if constexpr (std::is_same_v<T, std::optional<uint8_t>>)
        {
            return nullable::Createnullable_ubyte(builder, value.value());
        }

        if constexpr (std::is_same_v<T, std::optional<uint32_t>>)
        {
            return nullable::Createnullable_uint(builder, value.value());
        }

        return build<typename T::value_type>(builder, value.value());
    }
    else if constexpr (is_vector<T>)
    {
        if constexpr (std::is_fundamental_v<typename T::value_type>)
        {
            return builder.CreateVector(value);
        }
        else
        {
            auto buffer = std::vector<typename FlatBufferOffset<typename T::value_type>::type>();
            for (auto& x : value)
            {
                buffer.push_back(build<typename T::value_type>(builder, x));
            }
            return builder.CreateVector(buffer);
        }
    }
    else if constexpr (std::is_enum_v<T>)
    {
        return static_cast<typename FlatBufferOffset<T>::type>(value);
    }
    else
    {
        return value;
    }
}

template <> 
flatbuffers::Offset<flatbuffers::String> build<std::string>(FlatBufferBuilder& builder, const std::string& value)
{
    return builder.CreateString(value);
}

template <>
flatbuffers::Offset<fb::protocol::db::raw::Position> build<fb::protocol::db::Position>(FlatBufferBuilder& builder, const fb::protocol::db::Position& value)
{
    return fb::protocol::db::raw::CreatePosition(builder,
            flatbuffers::build<uint16_t>(builder, value.x),
            flatbuffers::build<uint16_t>(builder, value.y));
}
template <>
flatbuffers::Offset<fb::protocol::db::raw::Character> build<fb::protocol::db::Character>(FlatBufferBuilder& builder, const fb::protocol::db::Character& value)
{
    return fb::protocol::db::raw::CreateCharacter(builder,
            flatbuffers::build<uint32_t>(builder, value.id),
            flatbuffers::build<std::string>(builder, value.name),
            flatbuffers::build<std::string>(builder, value.last_login),
            flatbuffers::build<bool>(builder, value.admin),
            flatbuffers::build<uint16_t>(builder, value.look),
            flatbuffers::build<uint16_t>(builder, value.color),
            flatbuffers::build<uint16_t>(builder, value.sex),
            flatbuffers::build<uint16_t>(builder, value.nation),
            flatbuffers::build<std::optional<uint16_t>>(builder, value.creature),
            flatbuffers::build<uint32_t>(builder, value.map),
            flatbuffers::build<fb::protocol::db::Position>(builder, value.position),
            flatbuffers::build<uint8_t>(builder, value.direction),
            flatbuffers::build<uint8_t>(builder, value.state),
            flatbuffers::build<uint8_t>(builder, value.class_type),
            flatbuffers::build<uint8_t>(builder, value.promotion),
            flatbuffers::build<uint32_t>(builder, value.exp),
            flatbuffers::build<uint32_t>(builder, value.money),
            flatbuffers::build<uint32_t>(builder, value.deposited_money),
            flatbuffers::build<std::optional<uint16_t>>(builder, value.disguise),
            flatbuffers::build<uint32_t>(builder, value.hp),
            flatbuffers::build<uint32_t>(builder, value.base_hp),
            flatbuffers::build<uint32_t>(builder, value.additional_hp),
            flatbuffers::build<uint32_t>(builder, value.mp),
            flatbuffers::build<uint32_t>(builder, value.base_mp),
            flatbuffers::build<uint32_t>(builder, value.additional_mp),
            flatbuffers::build<std::optional<uint8_t>>(builder, value.weapon_color),
            flatbuffers::build<std::optional<uint8_t>>(builder, value.helmet_color),
            flatbuffers::build<std::optional<uint8_t>>(builder, value.armor_color),
            flatbuffers::build<std::optional<uint8_t>>(builder, value.shield_color),
            flatbuffers::build<std::optional<uint8_t>>(builder, value.ring_left_color),
            flatbuffers::build<std::optional<uint8_t>>(builder, value.ring_right_color),
            flatbuffers::build<std::optional<uint8_t>>(builder, value.aux_top_color),
            flatbuffers::build<std::optional<uint8_t>>(builder, value.aux_bot_color),
            flatbuffers::build<std::optional<uint32_t>>(builder, value.clan));
}
template <>
flatbuffers::Offset<fb::protocol::db::raw::Item> build<fb::protocol::db::Item>(FlatBufferBuilder& builder, const fb::protocol::db::Item& value)
{
    return fb::protocol::db::raw::CreateItem(builder,
            flatbuffers::build<uint32_t>(builder, value.user),
            flatbuffers::build<int16_t>(builder, value.index),
            flatbuffers::build<int16_t>(builder, value.parts),
            flatbuffers::build<int16_t>(builder, value.deposited),
            flatbuffers::build<uint32_t>(builder, value.model),
            flatbuffers::build<uint16_t>(builder, value.count),
            flatbuffers::build<std::optional<uint32_t>>(builder, value.durability),
            flatbuffers::build<std::optional<std::string>>(builder, value.custom_name));
}
template <>
flatbuffers::Offset<fb::protocol::db::raw::Spell> build<fb::protocol::db::Spell>(FlatBufferBuilder& builder, const fb::protocol::db::Spell& value)
{
    return fb::protocol::db::raw::CreateSpell(builder,
            flatbuffers::build<uint32_t>(builder, value.user),
            flatbuffers::build<uint8_t>(builder, value.slot),
            flatbuffers::build<uint32_t>(builder, value.model));
}
template <>
flatbuffers::Offset<fb::protocol::db::raw::ArticleSummary> build<fb::protocol::db::ArticleSummary>(FlatBufferBuilder& builder, const fb::protocol::db::ArticleSummary& value)
{
    return fb::protocol::db::raw::CreateArticleSummary(builder,
            flatbuffers::build<uint32_t>(builder, value.id),
            flatbuffers::build<uint32_t>(builder, value.user),
            flatbuffers::build<std::string>(builder, value.user_name),
            flatbuffers::build<std::string>(builder, value.title),
            flatbuffers::build<std::string>(builder, value.created_date));
}
template <>
flatbuffers::Offset<fb::protocol::db::raw::Article> build<fb::protocol::db::Article>(FlatBufferBuilder& builder, const fb::protocol::db::Article& value)
{
    return fb::protocol::db::raw::CreateArticle(builder,
            flatbuffers::build<uint32_t>(builder, value.id),
            flatbuffers::build<uint32_t>(builder, value.user),
            flatbuffers::build<std::string>(builder, value.user_name),
            flatbuffers::build<std::string>(builder, value.title),
            flatbuffers::build<std::string>(builder, value.contents),
            flatbuffers::build<std::string>(builder, value.created_date));
}
template <>
flatbuffers::Offset<fb::protocol::db::request::raw::Account> build<fb::protocol::db::request::Account>(FlatBufferBuilder& builder, const fb::protocol::db::request::Account& value)
{
    return fb::protocol::db::request::raw::CreateAccount(builder,
            flatbuffers::build<uint32_t>(builder, value.uid));
}
template <>
flatbuffers::Offset<fb::protocol::db::request::raw::ChangePw> build<fb::protocol::db::request::ChangePw>(FlatBufferBuilder& builder, const fb::protocol::db::request::ChangePw& value)
{
    return fb::protocol::db::request::raw::CreateChangePw(builder,
            flatbuffers::build<uint32_t>(builder, value.uid),
            flatbuffers::build<std::string>(builder, value.before),
            flatbuffers::build<std::string>(builder, value.after),
            flatbuffers::build<uint32_t>(builder, value.birthday));
}
template <>
flatbuffers::Offset<fb::protocol::db::request::raw::InitCharacter> build<fb::protocol::db::request::InitCharacter>(FlatBufferBuilder& builder, const fb::protocol::db::request::InitCharacter& value)
{
    return fb::protocol::db::request::raw::CreateInitCharacter(builder,
            flatbuffers::build<uint32_t>(builder, value.uid),
            flatbuffers::build<std::string>(builder, value.name),
            flatbuffers::build<std::string>(builder, value.pw),
            flatbuffers::build<uint32_t>(builder, value.hp),
            flatbuffers::build<uint32_t>(builder, value.mp),
            flatbuffers::build<uint16_t>(builder, value.map),
            flatbuffers::build<uint16_t>(builder, value.x),
            flatbuffers::build<uint16_t>(builder, value.y),
            flatbuffers::build<bool>(builder, value.admin));
}
template <>
flatbuffers::Offset<fb::protocol::db::request::raw::Authenticate> build<fb::protocol::db::request::Authenticate>(FlatBufferBuilder& builder, const fb::protocol::db::request::Authenticate& value)
{
    return fb::protocol::db::request::raw::CreateAuthenticate(builder,
            flatbuffers::build<uint32_t>(builder, value.uid),
            flatbuffers::build<std::string>(builder, value.pw));
}
template <>
flatbuffers::Offset<fb::protocol::db::request::raw::Login> build<fb::protocol::db::request::Login>(FlatBufferBuilder& builder, const fb::protocol::db::request::Login& value)
{
    return fb::protocol::db::request::raw::CreateLogin(builder,
            flatbuffers::build<uint32_t>(builder, value.uid));
}
template <>
flatbuffers::Offset<fb::protocol::db::request::raw::MakeCharacter> build<fb::protocol::db::request::MakeCharacter>(FlatBufferBuilder& builder, const fb::protocol::db::request::MakeCharacter& value)
{
    return fb::protocol::db::request::raw::CreateMakeCharacter(builder,
            flatbuffers::build<uint32_t>(builder, value.uid),
            flatbuffers::build<uint16_t>(builder, value.hair),
            flatbuffers::build<uint8_t>(builder, value.sex),
            flatbuffers::build<uint8_t>(builder, value.nation),
            flatbuffers::build<uint8_t>(builder, value.creature));
}
template <>
flatbuffers::Offset<fb::protocol::db::request::raw::ReserveName> build<fb::protocol::db::request::ReserveName>(FlatBufferBuilder& builder, const fb::protocol::db::request::ReserveName& value)
{
    return fb::protocol::db::request::raw::CreateReserveName(builder,
            flatbuffers::build<std::string>(builder, value.name));
}
template <>
flatbuffers::Offset<fb::protocol::db::request::raw::Save> build<fb::protocol::db::request::Save>(FlatBufferBuilder& builder, const fb::protocol::db::request::Save& value)
{
    return fb::protocol::db::request::raw::CreateSave(builder,
            flatbuffers::build<fb::protocol::db::Character>(builder, value.character),
            flatbuffers::build<std::vector<fb::protocol::db::Item>>(builder, value.items),
            flatbuffers::build<std::vector<fb::protocol::db::Spell>>(builder, value.spells));
}
template <>
flatbuffers::Offset<fb::protocol::db::request::raw::GetArticle> build<fb::protocol::db::request::GetArticle>(FlatBufferBuilder& builder, const fb::protocol::db::request::GetArticle& value)
{
    return fb::protocol::db::request::raw::CreateGetArticle(builder,
            flatbuffers::build<uint32_t>(builder, value.section),
            flatbuffers::build<uint32_t>(builder, value.article));
}
template <>
flatbuffers::Offset<fb::protocol::db::request::raw::GetArticleList> build<fb::protocol::db::request::GetArticleList>(FlatBufferBuilder& builder, const fb::protocol::db::request::GetArticleList& value)
{
    return fb::protocol::db::request::raw::CreateGetArticleList(builder,
            flatbuffers::build<uint32_t>(builder, value.section),
            flatbuffers::build<uint32_t>(builder, value.position));
}
template <>
flatbuffers::Offset<fb::protocol::db::request::raw::WriteArticle> build<fb::protocol::db::request::WriteArticle>(FlatBufferBuilder& builder, const fb::protocol::db::request::WriteArticle& value)
{
    return fb::protocol::db::request::raw::CreateWriteArticle(builder,
            flatbuffers::build<uint32_t>(builder, value.section),
            flatbuffers::build<uint32_t>(builder, value.user),
            flatbuffers::build<std::string>(builder, value.title),
            flatbuffers::build<std::string>(builder, value.contents));
}
template <>
flatbuffers::Offset<fb::protocol::db::request::raw::DeleteArticle> build<fb::protocol::db::request::DeleteArticle>(FlatBufferBuilder& builder, const fb::protocol::db::request::DeleteArticle& value)
{
    return fb::protocol::db::request::raw::CreateDeleteArticle(builder,
            flatbuffers::build<uint32_t>(builder, value.id),
            flatbuffers::build<uint32_t>(builder, value.user));
}
template <>
flatbuffers::Offset<fb::protocol::db::response::raw::DeleteArticle> build<fb::protocol::db::response::DeleteArticle>(FlatBufferBuilder& builder, const fb::protocol::db::response::DeleteArticle& value)
{
    return fb::protocol::db::response::raw::CreateDeleteArticle(builder,
            flatbuffers::build<int>(builder, value.result));
}
template <>
flatbuffers::Offset<fb::protocol::db::response::raw::GetArticle> build<fb::protocol::db::response::GetArticle>(FlatBufferBuilder& builder, const fb::protocol::db::response::GetArticle& value)
{
    return fb::protocol::db::response::raw::CreateGetArticle(builder,
            flatbuffers::build<fb::protocol::db::Article>(builder, value.article),
            flatbuffers::build<bool>(builder, value.next),
            flatbuffers::build<bool>(builder, value.success));
}
template <>
flatbuffers::Offset<fb::protocol::db::response::raw::GetArticleList> build<fb::protocol::db::response::GetArticleList>(FlatBufferBuilder& builder, const fb::protocol::db::response::GetArticleList& value)
{
    return fb::protocol::db::response::raw::CreateGetArticleList(builder,
            flatbuffers::build<std::vector<fb::protocol::db::ArticleSummary>>(builder, value.summary_list));
}
template <>
flatbuffers::Offset<fb::protocol::db::response::raw::WriteArticle> build<fb::protocol::db::response::WriteArticle>(FlatBufferBuilder& builder, const fb::protocol::db::response::WriteArticle& value)
{
    return fb::protocol::db::response::raw::CreateWriteArticle(builder,
            flatbuffers::build<bool>(builder, value.success));
}
template <>
flatbuffers::Offset<fb::protocol::db::response::raw::ChangePw> build<fb::protocol::db::response::ChangePw>(FlatBufferBuilder& builder, const fb::protocol::db::response::ChangePw& value)
{
    return fb::protocol::db::response::raw::CreateChangePw(builder,
            flatbuffers::build<uint32_t>(builder, value.error_code));
}
template <>
flatbuffers::Offset<fb::protocol::db::response::raw::GetUid> build<fb::protocol::db::response::GetUid>(FlatBufferBuilder& builder, const fb::protocol::db::response::GetUid& value)
{
    return fb::protocol::db::response::raw::CreateGetUid(builder,
            flatbuffers::build<uint32_t>(builder, value.uid),
            flatbuffers::build<bool>(builder, value.success));
}
template <>
flatbuffers::Offset<fb::protocol::db::response::raw::InitCharacter> build<fb::protocol::db::response::InitCharacter>(FlatBufferBuilder& builder, const fb::protocol::db::response::InitCharacter& value)
{
    return fb::protocol::db::response::raw::CreateInitCharacter(builder,
            flatbuffers::build<bool>(builder, value.success));
}
template <>
flatbuffers::Offset<fb::protocol::db::response::raw::Authenticate> build<fb::protocol::db::response::Authenticate>(FlatBufferBuilder& builder, const fb::protocol::db::response::Authenticate& value)
{
    return fb::protocol::db::response::raw::CreateAuthenticate(builder,
            flatbuffers::build<uint32_t>(builder, value.map),
            flatbuffers::build<uint32_t>(builder, value.error_code));
}
template <>
flatbuffers::Offset<fb::protocol::db::response::raw::Login> build<fb::protocol::db::response::Login>(FlatBufferBuilder& builder, const fb::protocol::db::response::Login& value)
{
    return fb::protocol::db::response::raw::CreateLogin(builder,
            flatbuffers::build<fb::protocol::db::Character>(builder, value.character),
            flatbuffers::build<std::vector<fb::protocol::db::Item>>(builder, value.items),
            flatbuffers::build<std::vector<fb::protocol::db::Spell>>(builder, value.spells));
}
template <>
flatbuffers::Offset<fb::protocol::db::response::raw::MakeCharacter> build<fb::protocol::db::response::MakeCharacter>(FlatBufferBuilder& builder, const fb::protocol::db::response::MakeCharacter& value)
{
    return fb::protocol::db::response::raw::CreateMakeCharacter(builder,
            flatbuffers::build<bool>(builder, value.success));
}
template <>
flatbuffers::Offset<fb::protocol::db::response::raw::ReserveName> build<fb::protocol::db::response::ReserveName>(FlatBufferBuilder& builder, const fb::protocol::db::response::ReserveName& value)
{
    return fb::protocol::db::response::raw::CreateReserveName(builder,
            flatbuffers::build<uint32_t>(builder, value.uid),
            flatbuffers::build<bool>(builder, value.success));
}
template <>
flatbuffers::Offset<fb::protocol::db::response::raw::Save> build<fb::protocol::db::response::Save>(FlatBufferBuilder& builder, const fb::protocol::db::response::Save& value)
{
    return fb::protocol::db::response::raw::CreateSave(builder,
            flatbuffers::build<bool>(builder, value.success));
}
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::Login> build<fb::protocol::internal::request::Login>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Login& value)
{
    return fb::protocol::internal::request::raw::CreateLogin(builder,
            flatbuffers::build<uint32_t>(builder, value.uid),
            flatbuffers::build<std::string>(builder, value.name),
            flatbuffers::build<uint16_t>(builder, value.map));
}
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::Logout> build<fb::protocol::internal::request::Logout>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Logout& value)
{
    return fb::protocol::internal::request::raw::CreateLogout(builder,
            flatbuffers::build<std::string>(builder, value.name));
}
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::Ping> build<fb::protocol::internal::request::Ping>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Ping& value)
{
    return fb::protocol::internal::request::raw::CreatePing(builder,
            flatbuffers::build<uint8_t>(builder, value.id),
            flatbuffers::build<std::string>(builder, value.name),
            flatbuffers::build<fb::protocol::internal::Service>(builder, value.service),
            flatbuffers::build<std::string>(builder, value.ip),
            flatbuffers::build<uint16_t>(builder, value.port));
}
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::Transfer> build<fb::protocol::internal::request::Transfer>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Transfer& value)
{
    return fb::protocol::internal::request::raw::CreateTransfer(builder,
            flatbuffers::build<fb::protocol::internal::Service>(builder, value.service),
            flatbuffers::build<uint8_t>(builder, value.id));
}
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::Whisper> build<fb::protocol::internal::request::Whisper>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Whisper& value)
{
    return fb::protocol::internal::request::raw::CreateWhisper(builder,
            flatbuffers::build<std::string>(builder, value.from),
            flatbuffers::build<std::string>(builder, value.to),
            flatbuffers::build<std::string>(builder, value.message));
}
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::KickOut> build<fb::protocol::internal::request::KickOut>(FlatBufferBuilder& builder, const fb::protocol::internal::request::KickOut& value)
{
    return fb::protocol::internal::request::raw::CreateKickOut(builder,
            flatbuffers::build<uint32_t>(builder, value.uid));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::KickOut> build<fb::protocol::internal::response::KickOut>(FlatBufferBuilder& builder, const fb::protocol::internal::response::KickOut& value)
{
    return fb::protocol::internal::response::raw::CreateKickOut(builder,
            flatbuffers::build<uint32_t>(builder, value.uid));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::Login> build<fb::protocol::internal::response::Login>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Login& value)
{
    return fb::protocol::internal::response::raw::CreateLogin(builder,
            flatbuffers::build<bool>(builder, value.success),
            flatbuffers::build<bool>(builder, value.logon),
            flatbuffers::build<std::string>(builder, value.ip),
            flatbuffers::build<uint16_t>(builder, value.port));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::Logout> build<fb::protocol::internal::response::Logout>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Logout& value)
{
    return fb::protocol::internal::response::raw::CreateLogout(builder,
            flatbuffers::build<bool>(builder, value.success));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::Pong> build<fb::protocol::internal::response::Pong>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Pong& value)
{
    return fb::protocol::internal::response::raw::CreatePong(builder);
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::Transfer> build<fb::protocol::internal::response::Transfer>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Transfer& value)
{
    return fb::protocol::internal::response::raw::CreateTransfer(builder,
            flatbuffers::build<fb::protocol::internal::TransferResult>(builder, value.code),
            flatbuffers::build<std::string>(builder, value.ip),
            flatbuffers::build<uint16_t>(builder, value.port));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::Whisper> build<fb::protocol::internal::response::Whisper>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Whisper& value)
{
    return fb::protocol::internal::response::raw::CreateWhisper(builder,
            flatbuffers::build<bool>(builder, value.success),
            flatbuffers::build<std::string>(builder, value.from),
            flatbuffers::build<uint32_t>(builder, value.to),
            flatbuffers::build<std::string>(builder, value.message));
}

template <typename T> inline static
std::vector<T> unpack(const flatbuffers::Vector<typename FlatBufferOffset<T>::type> *value)
{
    auto result = std::vector<T>();
    for (int i = 0; i < value->size(); i++)
    {
        if constexpr (is_optional<T>)
        {
            if (value->Get(i) == nullptr)
            {
                result.push_back(std::nullopt);
            }
            else if constexpr (std::is_fundamental_v<typename T::value_type>)
            {
                result.push_back(value->Get(i)->value());
            }
            else
            {
                result.push_back(*value->Get(i));
            }
        }
        else
        {
            if constexpr (std::is_fundamental_v<T>)
            {
                result.push_back(value->Get(i));
            }
            else
            {
                result.push_back(*value->Get(i));
            }
        }
    }
    return result;
}

}

#endif