// clang-format off

#ifndef __FLATBUFFER_PROTOCOL_H__
#define __FLATBUFFER_PROTOCOL_H__

#include <string>
#include <optional>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.service_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.groupaction_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.position_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.character_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.item_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.spell_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.option_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.articlesummary_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.article_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.group_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.login_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.logout_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.ping_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.transfer_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.whisper_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.kickout_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.account_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.changepw_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.initcharacter_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.authenticate_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.init_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.makecharacter_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.reservename_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.save_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.getarticle_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.getarticlelist_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.writearticle_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.deletearticle_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.setoption_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.entergroup_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.leavegroup_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.kickout_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.login_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.logout_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.pong_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.transfer_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.whisper_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.deletearticle_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.getarticle_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.getarticlelist_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.writearticle_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.changepw_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.getuid_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.initcharacter_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.authenticate_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.init_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.makecharacter_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.reservename_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.save_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.setoption_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.getgroup_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.entergroup_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.leavegroup_generated.h>

namespace fb::protocol::internal
{
    class Position;
    class Character;
    class Item;
    class Spell;
    class Option;
    class ArticleSummary;
    class Article;
    class Group;
    enum class Service : int8_t;
    enum class GroupAction : int8_t;
} // end of namespace fb::protocol::internal
namespace fb::protocol::internal::request
{
    class Login;
    class Logout;
    class Ping;
    class Transfer;
    class Whisper;
    class KickOut;
    class Account;
    class ChangePw;
    class InitCharacter;
    class Authenticate;
    class Init;
    class MakeCharacter;
    class ReserveName;
    class Save;
    class GetArticle;
    class GetArticleList;
    class WriteArticle;
    class DeleteArticle;
    class SetOption;
    class EnterGroup;
    class LeaveGroup;
} // end of namespace fb::protocol::internal::request
namespace fb::protocol::internal::response
{
    class KickOut;
    class Login;
    class Logout;
    class Pong;
    class Transfer;
    class Whisper;
    class DeleteArticle;
    class GetArticle;
    class GetArticleList;
    class WriteArticle;
    class ChangePw;
    class GetUid;
    class InitCharacter;
    class Authenticate;
    class Init;
    class MakeCharacter;
    class ReserveName;
    class Save;
    class SetOption;
    class GetGroup;
    class EnterGroup;
    class LeaveGroup;
} // end of namespace fb::protocol::internal::response


namespace flatbuffers {

class option
{
public:
    using encoding_func_type = std::function<std::string(const std::string&)>;

private:
    encoding_func_type encoding_func;
    encoding_func_type decoding_func;

private:
    option() = default;

public:
    ~option() = default;

private:
    static option& get()
    {
        static std::once_flag flag;
        static std::unique_ptr<option> ist;

        std::call_once(flag, [] 
        {
            ist = std::unique_ptr<option>(new option());
        });
        return *ist;
    }

public:
    static void encoding(const encoding_func_type& fn)
    {
        auto& ist = get();
        ist.encoding_func = fn;
    }

public:
    static void decoding(const encoding_func_type& fn)
    {
        auto& ist = get();
        ist.decoding_func = fn;
    }

public:
    static std::string encode(const std::string& value)
    {
        auto& ist = get();
        if (ist.encoding_func)
        {
            return ist.encoding_func(value);
        }
        else
        {
            return value;
        }
    }

public:
    static std::string decode(const std::string& value)
    {
        auto& ist = get();
        if (ist.decoding_func)
        {
            return ist.decoding_func(value);
        }
        else
        {
            return value;
        }
    }
};

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
template <> struct FlatBufferOffset<fb::protocol::internal::Position> { typedef flatbuffers::Offset<fb::protocol::internal::raw::Position> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::Character> { typedef flatbuffers::Offset<fb::protocol::internal::raw::Character> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::Item> { typedef flatbuffers::Offset<fb::protocol::internal::raw::Item> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::Spell> { typedef flatbuffers::Offset<fb::protocol::internal::raw::Spell> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::Option> { typedef flatbuffers::Offset<fb::protocol::internal::raw::Option> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::ArticleSummary> { typedef flatbuffers::Offset<fb::protocol::internal::raw::ArticleSummary> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::Article> { typedef flatbuffers::Offset<fb::protocol::internal::raw::Article> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::Group> { typedef flatbuffers::Offset<fb::protocol::internal::raw::Group> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::Login> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::Login> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::Logout> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::Logout> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::Ping> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::Ping> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::Transfer> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::Transfer> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::Whisper> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::Whisper> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::KickOut> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::KickOut> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::Account> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::Account> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::ChangePw> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::ChangePw> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::InitCharacter> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::InitCharacter> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::Authenticate> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::Authenticate> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::Init> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::Init> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::MakeCharacter> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::MakeCharacter> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::ReserveName> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::ReserveName> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::Save> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::Save> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::GetArticle> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::GetArticle> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::GetArticleList> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::GetArticleList> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::WriteArticle> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::WriteArticle> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::DeleteArticle> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::DeleteArticle> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::SetOption> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::SetOption> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::EnterGroup> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::EnterGroup> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::request::LeaveGroup> { typedef flatbuffers::Offset<fb::protocol::internal::request::raw::LeaveGroup> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::KickOut> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::KickOut> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::Login> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::Login> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::Logout> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::Logout> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::Pong> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::Pong> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::Transfer> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::Transfer> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::Whisper> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::Whisper> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::DeleteArticle> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::DeleteArticle> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::GetArticle> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::GetArticle> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::GetArticleList> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::GetArticleList> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::WriteArticle> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::WriteArticle> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::ChangePw> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::ChangePw> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::GetUid> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::GetUid> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::InitCharacter> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::InitCharacter> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::Authenticate> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::Authenticate> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::Init> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::Init> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::MakeCharacter> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::MakeCharacter> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::ReserveName> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::ReserveName> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::Save> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::Save> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::SetOption> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::SetOption> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::GetGroup> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::GetGroup> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::EnterGroup> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::EnterGroup> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::response::LeaveGroup> { typedef flatbuffers::Offset<fb::protocol::internal::response::raw::LeaveGroup> type; };
template <> struct FlatBufferOffset<fb::protocol::internal::Service> { typedef fb::protocol::internal::raw::Service type; };
template <> struct FlatBufferOffset<fb::protocol::internal::GroupAction> { typedef fb::protocol::internal::raw::GroupAction type; };
template <typename T> struct FlatBufferOffset<std::optional<T>> { typedef typename FlatBufferOffset<T>::type type; };
template <typename T> struct FlatBufferOffset<std::vector<T>> { typedef flatbuffers::Offset<flatbuffers::Vector<typename FlatBufferOffset<T>::type>> type; };

template <typename T> inline static
typename FlatBufferOffset<T>::type build(FlatBufferBuilder& builder, const T& value);
template <>
flatbuffers::Offset<flatbuffers::String> build<std::string>(FlatBufferBuilder& builder, const std::string& value);
template <>
flatbuffers::Offset<fb::protocol::internal::raw::Position> build<fb::protocol::internal::Position>(FlatBufferBuilder& builder, const fb::protocol::internal::Position& value);
template <>
flatbuffers::Offset<fb::protocol::internal::raw::Character> build<fb::protocol::internal::Character>(FlatBufferBuilder& builder, const fb::protocol::internal::Character& value);
template <>
flatbuffers::Offset<fb::protocol::internal::raw::Item> build<fb::protocol::internal::Item>(FlatBufferBuilder& builder, const fb::protocol::internal::Item& value);
template <>
flatbuffers::Offset<fb::protocol::internal::raw::Spell> build<fb::protocol::internal::Spell>(FlatBufferBuilder& builder, const fb::protocol::internal::Spell& value);
template <>
flatbuffers::Offset<fb::protocol::internal::raw::Option> build<fb::protocol::internal::Option>(FlatBufferBuilder& builder, const fb::protocol::internal::Option& value);
template <>
flatbuffers::Offset<fb::protocol::internal::raw::ArticleSummary> build<fb::protocol::internal::ArticleSummary>(FlatBufferBuilder& builder, const fb::protocol::internal::ArticleSummary& value);
template <>
flatbuffers::Offset<fb::protocol::internal::raw::Article> build<fb::protocol::internal::Article>(FlatBufferBuilder& builder, const fb::protocol::internal::Article& value);
template <>
flatbuffers::Offset<fb::protocol::internal::raw::Group> build<fb::protocol::internal::Group>(FlatBufferBuilder& builder, const fb::protocol::internal::Group& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::Login> build<fb::protocol::internal::request::Login>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Login& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::Logout> build<fb::protocol::internal::request::Logout>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Logout& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::Ping> build<fb::protocol::internal::request::Ping>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Ping& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::Transfer> build<fb::protocol::internal::request::Transfer>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Transfer& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::Whisper> build<fb::protocol::internal::request::Whisper>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Whisper& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::KickOut> build<fb::protocol::internal::request::KickOut>(FlatBufferBuilder& builder, const fb::protocol::internal::request::KickOut& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::Account> build<fb::protocol::internal::request::Account>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Account& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::ChangePw> build<fb::protocol::internal::request::ChangePw>(FlatBufferBuilder& builder, const fb::protocol::internal::request::ChangePw& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::InitCharacter> build<fb::protocol::internal::request::InitCharacter>(FlatBufferBuilder& builder, const fb::protocol::internal::request::InitCharacter& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::Authenticate> build<fb::protocol::internal::request::Authenticate>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Authenticate& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::Init> build<fb::protocol::internal::request::Init>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Init& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::MakeCharacter> build<fb::protocol::internal::request::MakeCharacter>(FlatBufferBuilder& builder, const fb::protocol::internal::request::MakeCharacter& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::ReserveName> build<fb::protocol::internal::request::ReserveName>(FlatBufferBuilder& builder, const fb::protocol::internal::request::ReserveName& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::Save> build<fb::protocol::internal::request::Save>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Save& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::GetArticle> build<fb::protocol::internal::request::GetArticle>(FlatBufferBuilder& builder, const fb::protocol::internal::request::GetArticle& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::GetArticleList> build<fb::protocol::internal::request::GetArticleList>(FlatBufferBuilder& builder, const fb::protocol::internal::request::GetArticleList& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::WriteArticle> build<fb::protocol::internal::request::WriteArticle>(FlatBufferBuilder& builder, const fb::protocol::internal::request::WriteArticle& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::DeleteArticle> build<fb::protocol::internal::request::DeleteArticle>(FlatBufferBuilder& builder, const fb::protocol::internal::request::DeleteArticle& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::SetOption> build<fb::protocol::internal::request::SetOption>(FlatBufferBuilder& builder, const fb::protocol::internal::request::SetOption& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::EnterGroup> build<fb::protocol::internal::request::EnterGroup>(FlatBufferBuilder& builder, const fb::protocol::internal::request::EnterGroup& value);
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::LeaveGroup> build<fb::protocol::internal::request::LeaveGroup>(FlatBufferBuilder& builder, const fb::protocol::internal::request::LeaveGroup& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::KickOut> build<fb::protocol::internal::response::KickOut>(FlatBufferBuilder& builder, const fb::protocol::internal::response::KickOut& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::Login> build<fb::protocol::internal::response::Login>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Login& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::Logout> build<fb::protocol::internal::response::Logout>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Logout& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::Pong> build<fb::protocol::internal::response::Pong>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Pong& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::Transfer> build<fb::protocol::internal::response::Transfer>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Transfer& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::Whisper> build<fb::protocol::internal::response::Whisper>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Whisper& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::DeleteArticle> build<fb::protocol::internal::response::DeleteArticle>(FlatBufferBuilder& builder, const fb::protocol::internal::response::DeleteArticle& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::GetArticle> build<fb::protocol::internal::response::GetArticle>(FlatBufferBuilder& builder, const fb::protocol::internal::response::GetArticle& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::GetArticleList> build<fb::protocol::internal::response::GetArticleList>(FlatBufferBuilder& builder, const fb::protocol::internal::response::GetArticleList& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::WriteArticle> build<fb::protocol::internal::response::WriteArticle>(FlatBufferBuilder& builder, const fb::protocol::internal::response::WriteArticle& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::ChangePw> build<fb::protocol::internal::response::ChangePw>(FlatBufferBuilder& builder, const fb::protocol::internal::response::ChangePw& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::GetUid> build<fb::protocol::internal::response::GetUid>(FlatBufferBuilder& builder, const fb::protocol::internal::response::GetUid& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::InitCharacter> build<fb::protocol::internal::response::InitCharacter>(FlatBufferBuilder& builder, const fb::protocol::internal::response::InitCharacter& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::Authenticate> build<fb::protocol::internal::response::Authenticate>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Authenticate& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::Init> build<fb::protocol::internal::response::Init>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Init& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::MakeCharacter> build<fb::protocol::internal::response::MakeCharacter>(FlatBufferBuilder& builder, const fb::protocol::internal::response::MakeCharacter& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::ReserveName> build<fb::protocol::internal::response::ReserveName>(FlatBufferBuilder& builder, const fb::protocol::internal::response::ReserveName& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::Save> build<fb::protocol::internal::response::Save>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Save& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::SetOption> build<fb::protocol::internal::response::SetOption>(FlatBufferBuilder& builder, const fb::protocol::internal::response::SetOption& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::GetGroup> build<fb::protocol::internal::response::GetGroup>(FlatBufferBuilder& builder, const fb::protocol::internal::response::GetGroup& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::EnterGroup> build<fb::protocol::internal::response::EnterGroup>(FlatBufferBuilder& builder, const fb::protocol::internal::response::EnterGroup& value);
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::LeaveGroup> build<fb::protocol::internal::response::LeaveGroup>(FlatBufferBuilder& builder, const fb::protocol::internal::response::LeaveGroup& value);

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
enum class GroupAction : int8_t
{
    Create = fb::protocol::internal::raw::GroupAction::GroupAction_Create,
    Enter = fb::protocol::internal::raw::GroupAction::GroupAction_Enter,
    Leave = fb::protocol::internal::raw::GroupAction::GroupAction_Leave,
    Kick = fb::protocol::internal::raw::GroupAction::GroupAction_Kick,
    BreakUp = fb::protocol::internal::raw::GroupAction::GroupAction_BreakUp,
};

} // end of namespace fb::protocol::internal

namespace fb::protocol::internal {

enum class FlatBufferProtocolType
{
    Position,
    Character,
    Item,
    Spell,
    Option,
    ArticleSummary,
    Article,
    Group,
};

class Position
{
public:
    static inline fb::protocol::internal::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::FlatBufferProtocolType::Position;

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

    Position(const fb::protocol::internal::raw::Position& raw)
        : x(raw.x()), y(raw.y())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::Position>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Position Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::raw::GetPosition(bytes);
        return Position(*raw);
    }
};
class Character
{
public:
    static inline fb::protocol::internal::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::FlatBufferProtocolType::Character;

public:
    uint32_t id = 0;
    std::string name;
    std::string pw;
    bool admin = false;
    uint16_t look = 0;
    uint16_t color = 0;
    uint16_t sex = 0;
    uint16_t nation = 0;
    std::optional<uint16_t> creature = std::nullopt;
    uint32_t map = 0;
    fb::protocol::internal::Position position;
    uint8_t direction = 0;
    uint8_t state = 0;
    uint8_t class_type = 0;
    uint8_t promotion = 0;
    uint8_t level = 0;
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
    std::optional<uint32_t> group = std::nullopt;
    std::optional<uint32_t> clan = std::nullopt;
    std::string updated_date;

public:
    Character() = default;

    Character(const Character& x)
        : id(x.id), name(x.name), pw(x.pw), admin(x.admin), look(x.look), color(x.color), sex(x.sex), nation(x.nation), creature(x.creature), map(x.map), position(x.position), direction(x.direction), state(x.state), class_type(x.class_type), promotion(x.promotion), level(x.level), exp(x.exp), money(x.money), deposited_money(x.deposited_money), disguise(x.disguise), hp(x.hp), base_hp(x.base_hp), additional_hp(x.additional_hp), mp(x.mp), base_mp(x.base_mp), additional_mp(x.additional_mp), weapon_color(x.weapon_color), helmet_color(x.helmet_color), armor_color(x.armor_color), shield_color(x.shield_color), ring_left_color(x.ring_left_color), ring_right_color(x.ring_right_color), aux_top_color(x.aux_top_color), aux_bot_color(x.aux_bot_color), group(x.group), clan(x.clan), updated_date(x.updated_date)
    { }

    Character(uint32_t id, const std::string& name, const std::string& pw, bool admin, uint16_t look, uint16_t color, uint16_t sex, uint16_t nation, const std::optional<uint16_t>& creature, uint32_t map, const fb::protocol::internal::Position& position, uint8_t direction, uint8_t state, uint8_t class_type, uint8_t promotion, uint8_t level, uint32_t exp, uint32_t money, uint32_t deposited_money, const std::optional<uint16_t>& disguise, uint32_t hp, uint32_t base_hp, uint32_t additional_hp, uint32_t mp, uint32_t base_mp, uint32_t additional_mp, const std::optional<uint8_t>& weapon_color, const std::optional<uint8_t>& helmet_color, const std::optional<uint8_t>& armor_color, const std::optional<uint8_t>& shield_color, const std::optional<uint8_t>& ring_left_color, const std::optional<uint8_t>& ring_right_color, const std::optional<uint8_t>& aux_top_color, const std::optional<uint8_t>& aux_bot_color, const std::optional<uint32_t>& group, const std::optional<uint32_t>& clan, const std::string& updated_date)
        : id(id), name(name), pw(pw), admin(admin), look(look), color(color), sex(sex), nation(nation), creature(creature), map(map), position(position), direction(direction), state(state), class_type(class_type), promotion(promotion), level(level), exp(exp), money(money), deposited_money(deposited_money), disguise(disguise), hp(hp), base_hp(base_hp), additional_hp(additional_hp), mp(mp), base_mp(base_mp), additional_mp(additional_mp), weapon_color(weapon_color), helmet_color(helmet_color), armor_color(armor_color), shield_color(shield_color), ring_left_color(ring_left_color), ring_right_color(ring_right_color), aux_top_color(aux_top_color), aux_bot_color(aux_bot_color), group(group), clan(clan), updated_date(updated_date)
    { }

    Character(const fb::protocol::internal::raw::Character& raw)
        : id(raw.id()), name(flatbuffers::option::decode(raw.name()->c_str())), pw(flatbuffers::option::decode(raw.pw()->c_str())), admin(raw.admin()), look(raw.look()), color(raw.color()), sex(raw.sex()), nation(raw.nation()), creature(raw.creature() != nullptr ? raw.creature()->value() : std::optional<uint16_t>()), map(raw.map()), position(*raw.position()), direction(raw.direction()), state(raw.state()), class_type(raw.class_type()), promotion(raw.promotion()), level(raw.level()), exp(raw.exp()), money(raw.money()), deposited_money(raw.deposited_money()), disguise(raw.disguise() != nullptr ? raw.disguise()->value() : std::optional<uint16_t>()), hp(raw.hp()), base_hp(raw.base_hp()), additional_hp(raw.additional_hp()), mp(raw.mp()), base_mp(raw.base_mp()), additional_mp(raw.additional_mp()), weapon_color(raw.weapon_color() != nullptr ? raw.weapon_color()->value() : std::optional<uint8_t>()), helmet_color(raw.helmet_color() != nullptr ? raw.helmet_color()->value() : std::optional<uint8_t>()), armor_color(raw.armor_color() != nullptr ? raw.armor_color()->value() : std::optional<uint8_t>()), shield_color(raw.shield_color() != nullptr ? raw.shield_color()->value() : std::optional<uint8_t>()), ring_left_color(raw.ring_left_color() != nullptr ? raw.ring_left_color()->value() : std::optional<uint8_t>()), ring_right_color(raw.ring_right_color() != nullptr ? raw.ring_right_color()->value() : std::optional<uint8_t>()), aux_top_color(raw.aux_top_color() != nullptr ? raw.aux_top_color()->value() : std::optional<uint8_t>()), aux_bot_color(raw.aux_bot_color() != nullptr ? raw.aux_bot_color()->value() : std::optional<uint8_t>()), group(raw.group() != nullptr ? raw.group()->value() : std::optional<uint32_t>()), clan(raw.clan() != nullptr ? raw.clan()->value() : std::optional<uint32_t>()), updated_date(flatbuffers::option::decode(raw.updated_date()->c_str()))
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::Character>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Character Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::raw::GetCharacter(bytes);
        return Character(*raw);
    }
};
class Item
{
public:
    static inline fb::protocol::internal::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::FlatBufferProtocolType::Item;

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

    Item(const fb::protocol::internal::raw::Item& raw)
        : user(raw.user()), index(raw.index()), parts(raw.parts()), deposited(raw.deposited()), model(raw.model()), count(raw.count()), durability(raw.durability() != nullptr ? raw.durability()->value() : std::optional<uint32_t>()), custom_name(raw.custom_name() != nullptr ? flatbuffers::option::decode(raw.custom_name()->c_str()) : std::optional<std::string>())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::Item>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Item Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::raw::GetItem(bytes);
        return Item(*raw);
    }
};
class Spell
{
public:
    static inline fb::protocol::internal::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::FlatBufferProtocolType::Spell;

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

    Spell(const fb::protocol::internal::raw::Spell& raw)
        : user(raw.user()), slot(raw.slot()), model(raw.model())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::Spell>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Spell Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::raw::GetSpell(bytes);
        return Spell(*raw);
    }
};
class Option
{
public:
    static inline fb::protocol::internal::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::FlatBufferProtocolType::Option;

public:
    uint32_t uid = 0;
    bool whisper = false;
    bool group = false;
    bool roar = false;
    bool roar_worlds = false;
    bool magic_effect = false;
    bool weather_effect = false;
    bool fixed_move = false;
    bool trade = false;
    bool fast_move = false;
    bool effect_sound = false;
    bool pk_protect = false;

public:
    Option() = default;

    Option(const Option& x)
        : uid(x.uid), whisper(x.whisper), group(x.group), roar(x.roar), roar_worlds(x.roar_worlds), magic_effect(x.magic_effect), weather_effect(x.weather_effect), fixed_move(x.fixed_move), trade(x.trade), fast_move(x.fast_move), effect_sound(x.effect_sound), pk_protect(x.pk_protect)
    { }

    Option(uint32_t uid, bool whisper, bool group, bool roar, bool roar_worlds, bool magic_effect, bool weather_effect, bool fixed_move, bool trade, bool fast_move, bool effect_sound, bool pk_protect)
        : uid(uid), whisper(whisper), group(group), roar(roar), roar_worlds(roar_worlds), magic_effect(magic_effect), weather_effect(weather_effect), fixed_move(fixed_move), trade(trade), fast_move(fast_move), effect_sound(effect_sound), pk_protect(pk_protect)
    { }

    Option(const fb::protocol::internal::raw::Option& raw)
        : uid(raw.uid()), whisper(raw.whisper()), group(raw.group()), roar(raw.roar()), roar_worlds(raw.roar_worlds()), magic_effect(raw.magic_effect()), weather_effect(raw.weather_effect()), fixed_move(raw.fixed_move()), trade(raw.trade()), fast_move(raw.fast_move()), effect_sound(raw.effect_sound()), pk_protect(raw.pk_protect())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::Option>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Option Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::raw::GetOption(bytes);
        return Option(*raw);
    }
};
class ArticleSummary
{
public:
    static inline fb::protocol::internal::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::FlatBufferProtocolType::ArticleSummary;

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

    ArticleSummary(const fb::protocol::internal::raw::ArticleSummary& raw)
        : id(raw.id()), user(raw.user()), user_name(flatbuffers::option::decode(raw.user_name()->c_str())), title(flatbuffers::option::decode(raw.title()->c_str())), created_date(flatbuffers::option::decode(raw.created_date()->c_str()))
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::ArticleSummary>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static ArticleSummary Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::raw::GetArticleSummary(bytes);
        return ArticleSummary(*raw);
    }
};
class Article
{
public:
    static inline fb::protocol::internal::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::FlatBufferProtocolType::Article;

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

    Article(const fb::protocol::internal::raw::Article& raw)
        : id(raw.id()), user(raw.user()), user_name(flatbuffers::option::decode(raw.user_name()->c_str())), title(flatbuffers::option::decode(raw.title()->c_str())), contents(flatbuffers::option::decode(raw.contents()->c_str())), created_date(flatbuffers::option::decode(raw.created_date()->c_str()))
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::Article>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Article Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::raw::GetArticle(bytes);
        return Article(*raw);
    }
};
class Group
{
public:
    static inline fb::protocol::internal::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::FlatBufferProtocolType::Group;

public:
    uint32_t id = 0;
    std::string master;
    std::vector<std::string> members = {};

public:
    Group() = default;

    Group(const Group& x)
        : id(x.id), master(x.master), members(x.members)
    { }

    Group(uint32_t id, const std::string& master, std::vector<std::string> members)
        : id(id), master(master), members(members)
    { }

    Group(const fb::protocol::internal::raw::Group& raw)
        : id(raw.id()), master(flatbuffers::option::decode(raw.master()->c_str())), members(unpack<std::string>(raw.members()))
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::Group>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Group Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::raw::GetGroup(bytes);
        return Group(*raw);
    }
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
    Account,
    ChangePw,
    InitCharacter,
    Authenticate,
    Init,
    MakeCharacter,
    ReserveName,
    Save,
    GetArticle,
    GetArticleList,
    WriteArticle,
    DeleteArticle,
    SetOption,
    EnterGroup,
    LeaveGroup,
};

class Login
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::Login;

public:
    uint32_t uid = 0;
    std::string name;
    uint8_t host = 0;

public:
    Login() = default;

    Login(const Login& x)
        : uid(x.uid), name(x.name), host(x.host)
    { }

    Login(uint32_t uid, const std::string& name, uint8_t host)
        : uid(uid), name(name), host(host)
    { }

    Login(const fb::protocol::internal::request::raw::Login& raw)
        : uid(raw.uid()), name(flatbuffers::option::decode(raw.name()->c_str())), host(raw.host())
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
        : name(flatbuffers::option::decode(raw.name()->c_str()))
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
        : id(raw.id()), name(flatbuffers::option::decode(raw.name()->c_str())), service((fb::protocol::internal::Service)raw.service()), ip(flatbuffers::option::decode(raw.ip()->c_str())), port(raw.port())
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
    std::optional<std::string> name = std::nullopt;
    bool force_shutdown = false;

public:
    Transfer() = default;

    Transfer(const Transfer& x)
        : service(x.service), id(x.id), name(x.name), force_shutdown(x.force_shutdown)
    { }

    Transfer(fb::protocol::internal::Service service, uint8_t id, const std::optional<std::string>& name, bool force_shutdown)
        : service(service), id(id), name(name), force_shutdown(force_shutdown)
    { }

    Transfer(const fb::protocol::internal::request::raw::Transfer& raw)
        : service((fb::protocol::internal::Service)raw.service()), id(raw.id()), name(raw.name() != nullptr ? flatbuffers::option::decode(raw.name()->c_str()) : std::optional<std::string>()), force_shutdown(raw.force_shutdown())
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
        : from(flatbuffers::option::decode(raw.from()->c_str())), to(flatbuffers::option::decode(raw.to()->c_str())), message(flatbuffers::option::decode(raw.message()->c_str()))
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
class Account
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::Account;

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

    Account(const fb::protocol::internal::request::raw::Account& raw)
        : uid(raw.uid())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::Account>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Account Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetAccount(bytes);
        return Account(*raw);
    }
};
class ChangePw
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::ChangePw;

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

    ChangePw(const fb::protocol::internal::request::raw::ChangePw& raw)
        : uid(raw.uid()), before(flatbuffers::option::decode(raw.before()->c_str())), after(flatbuffers::option::decode(raw.after()->c_str())), birthday(raw.birthday())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::ChangePw>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static ChangePw Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetChangePw(bytes);
        return ChangePw(*raw);
    }
};
class InitCharacter
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::InitCharacter;

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

    InitCharacter(const fb::protocol::internal::request::raw::InitCharacter& raw)
        : uid(raw.uid()), name(flatbuffers::option::decode(raw.name()->c_str())), pw(flatbuffers::option::decode(raw.pw()->c_str())), hp(raw.hp()), mp(raw.mp()), map(raw.map()), x(raw.x()), y(raw.y()), admin(raw.admin())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::InitCharacter>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static InitCharacter Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetInitCharacter(bytes);
        return InitCharacter(*raw);
    }
};
class Authenticate
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::Authenticate;

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

    Authenticate(const fb::protocol::internal::request::raw::Authenticate& raw)
        : uid(raw.uid()), pw(flatbuffers::option::decode(raw.pw()->c_str()))
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::Authenticate>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Authenticate Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetAuthenticate(bytes);
        return Authenticate(*raw);
    }
};
class Init
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::Init;

public:
    uint32_t uid = 0;

public:
    Init() = default;

    Init(const Init& x)
        : uid(x.uid)
    { }

    Init(uint32_t uid)
        : uid(uid)
    { }

    Init(const fb::protocol::internal::request::raw::Init& raw)
        : uid(raw.uid())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::Init>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Init Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetInit(bytes);
        return Init(*raw);
    }
};
class MakeCharacter
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::MakeCharacter;

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

    MakeCharacter(const fb::protocol::internal::request::raw::MakeCharacter& raw)
        : uid(raw.uid()), hair(raw.hair()), sex(raw.sex()), nation(raw.nation()), creature(raw.creature())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::MakeCharacter>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static MakeCharacter Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetMakeCharacter(bytes);
        return MakeCharacter(*raw);
    }
};
class ReserveName
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::ReserveName;

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

    ReserveName(const fb::protocol::internal::request::raw::ReserveName& raw)
        : name(flatbuffers::option::decode(raw.name()->c_str()))
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::ReserveName>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static ReserveName Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetReserveName(bytes);
        return ReserveName(*raw);
    }
};
class Save
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::Save;

public:
    fb::protocol::internal::Character character;
    std::vector<fb::protocol::internal::Item> items = {};
    std::vector<fb::protocol::internal::Spell> spells = {};

public:
    Save() = default;

    Save(const Save& x)
        : character(x.character), items(x.items), spells(x.spells)
    { }

    Save(const fb::protocol::internal::Character& character, std::vector<fb::protocol::internal::Item> items, std::vector<fb::protocol::internal::Spell> spells)
        : character(character), items(items), spells(spells)
    { }

    Save(const fb::protocol::internal::request::raw::Save& raw)
        : character(*raw.character()), items(unpack<fb::protocol::internal::Item>(raw.items())), spells(unpack<fb::protocol::internal::Spell>(raw.spells()))
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::Save>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Save Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetSave(bytes);
        return Save(*raw);
    }
};
class GetArticle
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::GetArticle;

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

    GetArticle(const fb::protocol::internal::request::raw::GetArticle& raw)
        : section(raw.section()), article(raw.article())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::GetArticle>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static GetArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetGetArticle(bytes);
        return GetArticle(*raw);
    }
};
class GetArticleList
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::GetArticleList;

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

    GetArticleList(const fb::protocol::internal::request::raw::GetArticleList& raw)
        : section(raw.section()), position(raw.position())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::GetArticleList>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static GetArticleList Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetGetArticleList(bytes);
        return GetArticleList(*raw);
    }
};
class WriteArticle
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::WriteArticle;

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

    WriteArticle(const fb::protocol::internal::request::raw::WriteArticle& raw)
        : section(raw.section()), user(raw.user()), title(flatbuffers::option::decode(raw.title()->c_str())), contents(flatbuffers::option::decode(raw.contents()->c_str()))
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::WriteArticle>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static WriteArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetWriteArticle(bytes);
        return WriteArticle(*raw);
    }
};
class DeleteArticle
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::DeleteArticle;

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

    DeleteArticle(const fb::protocol::internal::request::raw::DeleteArticle& raw)
        : id(raw.id()), user(raw.user())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::DeleteArticle>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static DeleteArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetDeleteArticle(bytes);
        return DeleteArticle(*raw);
    }
};
class SetOption
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::SetOption;

public:
    uint32_t user = 0;
    uint8_t type = 0;
    bool enabled = false;

public:
    SetOption() = default;

    SetOption(const SetOption& x)
        : user(x.user), type(x.type), enabled(x.enabled)
    { }

    SetOption(uint32_t user, uint8_t type, bool enabled)
        : user(user), type(type), enabled(enabled)
    { }

    SetOption(const fb::protocol::internal::request::raw::SetOption& raw)
        : user(raw.user()), type(raw.type()), enabled(raw.enabled())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::SetOption>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static SetOption Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetSetOption(bytes);
        return SetOption(*raw);
    }
};
class EnterGroup
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::EnterGroup;

public:
    uint32_t master = 0;
    std::string member;

public:
    EnterGroup() = default;

    EnterGroup(const EnterGroup& x)
        : master(x.master), member(x.member)
    { }

    EnterGroup(uint32_t master, const std::string& member)
        : master(master), member(member)
    { }

    EnterGroup(const fb::protocol::internal::request::raw::EnterGroup& raw)
        : master(raw.master()), member(flatbuffers::option::decode(raw.member()->c_str()))
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::EnterGroup>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static EnterGroup Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetEnterGroup(bytes);
        return EnterGroup(*raw);
    }
};
class LeaveGroup
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::LeaveGroup;

public:
    std::string member;

public:
    LeaveGroup() = default;

    LeaveGroup(const LeaveGroup& x)
        : member(x.member)
    { }

    LeaveGroup(const std::string& member)
        : member(member)
    { }

    LeaveGroup(const fb::protocol::internal::request::raw::LeaveGroup& raw)
        : member(flatbuffers::option::decode(raw.member()->c_str()))
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::request::LeaveGroup>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static LeaveGroup Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetLeaveGroup(bytes);
        return LeaveGroup(*raw);
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
    DeleteArticle,
    GetArticle,
    GetArticleList,
    WriteArticle,
    ChangePw,
    GetUid,
    InitCharacter,
    Authenticate,
    Init,
    MakeCharacter,
    ReserveName,
    Save,
    SetOption,
    GetGroup,
    EnterGroup,
    LeaveGroup,
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
    uint32_t error = 0;
    bool logon = false;
    std::string ip;
    uint16_t port = 0;

public:
    Login() = default;

    Login(const Login& x)
        : error(x.error), logon(x.logon), ip(x.ip), port(x.port)
    { }

    Login(uint32_t error, bool logon, const std::string& ip, uint16_t port)
        : error(error), logon(logon), ip(ip), port(port)
    { }

    Login(const fb::protocol::internal::response::raw::Login& raw)
        : error(raw.error()), logon(raw.logon()), ip(flatbuffers::option::decode(raw.ip()->c_str())), port(raw.port())
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
    uint32_t error = 0;
    std::string ip;
    uint16_t port = 0;

public:
    Transfer() = default;

    Transfer(const Transfer& x)
        : error(x.error), ip(x.ip), port(x.port)
    { }

    Transfer(uint32_t error, const std::string& ip, uint16_t port)
        : error(error), ip(ip), port(port)
    { }

    Transfer(const fb::protocol::internal::response::raw::Transfer& raw)
        : error(raw.error()), ip(flatbuffers::option::decode(raw.ip()->c_str())), port(raw.port())
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
    std::string from;
    std::string to;
    std::string message;
    uint32_t host = 0;
    uint32_t error = 0;

public:
    Whisper() = default;

    Whisper(const Whisper& x)
        : from(x.from), to(x.to), message(x.message), host(x.host), error(x.error)
    { }

    Whisper(const std::string& from, const std::string& to, const std::string& message, uint32_t host, uint32_t error)
        : from(from), to(to), message(message), host(host), error(error)
    { }

    Whisper(const fb::protocol::internal::response::raw::Whisper& raw)
        : from(flatbuffers::option::decode(raw.from()->c_str())), to(flatbuffers::option::decode(raw.to()->c_str())), message(flatbuffers::option::decode(raw.message()->c_str())), host(raw.host()), error(raw.error())
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
class DeleteArticle
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::DeleteArticle;

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

    DeleteArticle(const fb::protocol::internal::response::raw::DeleteArticle& raw)
        : result(raw.result())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::DeleteArticle>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static DeleteArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetDeleteArticle(bytes);
        return DeleteArticle(*raw);
    }
};
class GetArticle
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::GetArticle;

public:
    fb::protocol::internal::Article article;
    bool next = false;
    bool success = false;

public:
    GetArticle() = default;

    GetArticle(const GetArticle& x)
        : article(x.article), next(x.next), success(x.success)
    { }

    GetArticle(const fb::protocol::internal::Article& article, bool next, bool success)
        : article(article), next(next), success(success)
    { }

    GetArticle(const fb::protocol::internal::response::raw::GetArticle& raw)
        : article(*raw.article()), next(raw.next()), success(raw.success())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::GetArticle>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static GetArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetGetArticle(bytes);
        return GetArticle(*raw);
    }
};
class GetArticleList
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::GetArticleList;

public:
    std::vector<fb::protocol::internal::ArticleSummary> summary_list = {};

public:
    GetArticleList() = default;

    GetArticleList(const GetArticleList& x)
        : summary_list(x.summary_list)
    { }

    GetArticleList(std::vector<fb::protocol::internal::ArticleSummary> summary_list)
        : summary_list(summary_list)
    { }

    GetArticleList(const fb::protocol::internal::response::raw::GetArticleList& raw)
        : summary_list(unpack<fb::protocol::internal::ArticleSummary>(raw.summary_list()))
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::GetArticleList>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static GetArticleList Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetGetArticleList(bytes);
        return GetArticleList(*raw);
    }
};
class WriteArticle
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::WriteArticle;

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

    WriteArticle(const fb::protocol::internal::response::raw::WriteArticle& raw)
        : success(raw.success())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::WriteArticle>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static WriteArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetWriteArticle(bytes);
        return WriteArticle(*raw);
    }
};
class ChangePw
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::ChangePw;

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

    ChangePw(const fb::protocol::internal::response::raw::ChangePw& raw)
        : error_code(raw.error_code())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::ChangePw>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static ChangePw Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetChangePw(bytes);
        return ChangePw(*raw);
    }
};
class GetUid
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::GetUid;

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

    GetUid(const fb::protocol::internal::response::raw::GetUid& raw)
        : uid(raw.uid()), success(raw.success())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::GetUid>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static GetUid Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetGetUid(bytes);
        return GetUid(*raw);
    }
};
class InitCharacter
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::InitCharacter;

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

    InitCharacter(const fb::protocol::internal::response::raw::InitCharacter& raw)
        : success(raw.success())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::InitCharacter>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static InitCharacter Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetInitCharacter(bytes);
        return InitCharacter(*raw);
    }
};
class Authenticate
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::Authenticate;

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

    Authenticate(const fb::protocol::internal::response::raw::Authenticate& raw)
        : map(raw.map()), error_code(raw.error_code())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::Authenticate>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Authenticate Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetAuthenticate(bytes);
        return Authenticate(*raw);
    }
};
class Init
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::Init;

public:
    fb::protocol::internal::Character character;
    std::vector<fb::protocol::internal::Item> items = {};
    std::vector<fb::protocol::internal::Spell> spells = {};
    fb::protocol::internal::Option option;

public:
    Init() = default;

    Init(const Init& x)
        : character(x.character), items(x.items), spells(x.spells), option(x.option)
    { }

    Init(const fb::protocol::internal::Character& character, std::vector<fb::protocol::internal::Item> items, std::vector<fb::protocol::internal::Spell> spells, const fb::protocol::internal::Option& option)
        : character(character), items(items), spells(spells), option(option)
    { }

    Init(const fb::protocol::internal::response::raw::Init& raw)
        : character(*raw.character()), items(unpack<fb::protocol::internal::Item>(raw.items())), spells(unpack<fb::protocol::internal::Spell>(raw.spells())), option(*raw.option())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::Init>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Init Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetInit(bytes);
        return Init(*raw);
    }
};
class MakeCharacter
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::MakeCharacter;

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

    MakeCharacter(const fb::protocol::internal::response::raw::MakeCharacter& raw)
        : success(raw.success())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::MakeCharacter>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static MakeCharacter Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetMakeCharacter(bytes);
        return MakeCharacter(*raw);
    }
};
class ReserveName
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::ReserveName;

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

    ReserveName(const fb::protocol::internal::response::raw::ReserveName& raw)
        : uid(raw.uid()), success(raw.success())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::ReserveName>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static ReserveName Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetReserveName(bytes);
        return ReserveName(*raw);
    }
};
class Save
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::Save;

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

    Save(const fb::protocol::internal::response::raw::Save& raw)
        : success(raw.success())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::Save>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static Save Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetSave(bytes);
        return Save(*raw);
    }
};
class SetOption
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::SetOption;

public:
    bool success = false;

public:
    SetOption() = default;

    SetOption(const SetOption& x)
        : success(x.success)
    { }

    SetOption(bool success)
        : success(success)
    { }

    SetOption(const fb::protocol::internal::response::raw::SetOption& raw)
        : success(raw.success())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::SetOption>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static SetOption Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetSetOption(bytes);
        return SetOption(*raw);
    }
};
class GetGroup
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::GetGroup;

public:
    fb::protocol::internal::Group group;
    uint32_t error = 0;

public:
    GetGroup() = default;

    GetGroup(const GetGroup& x)
        : group(x.group), error(x.error)
    { }

    GetGroup(const fb::protocol::internal::Group& group, uint32_t error)
        : group(group), error(error)
    { }

    GetGroup(const fb::protocol::internal::response::raw::GetGroup& raw)
        : group(*raw.group()), error(raw.error())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::GetGroup>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static GetGroup Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetGetGroup(bytes);
        return GetGroup(*raw);
    }
};
class EnterGroup
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::EnterGroup;

public:
    fb::protocol::internal::Group group;
    std::string member;
    fb::protocol::internal::GroupAction action;
    uint32_t host = 0;
    uint32_t error = 0;

public:
    EnterGroup() = default;

    EnterGroup(const EnterGroup& x)
        : group(x.group), member(x.member), action(x.action), host(x.host), error(x.error)
    { }

    EnterGroup(const fb::protocol::internal::Group& group, const std::string& member, fb::protocol::internal::GroupAction action, uint32_t host, uint32_t error)
        : group(group), member(member), action(action), host(host), error(error)
    { }

    EnterGroup(const fb::protocol::internal::response::raw::EnterGroup& raw)
        : group(*raw.group()), member(flatbuffers::option::decode(raw.member()->c_str())), action((fb::protocol::internal::GroupAction)raw.action()), host(raw.host()), error(raw.error())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::EnterGroup>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static EnterGroup Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetEnterGroup(bytes);
        return EnterGroup(*raw);
    }
};
class LeaveGroup
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::LeaveGroup;

public:
    fb::protocol::internal::Group group;
    std::string member;
    fb::protocol::internal::GroupAction action;
    uint32_t host = 0;
    uint32_t error = 0;

public:
    LeaveGroup() = default;

    LeaveGroup(const LeaveGroup& x)
        : group(x.group), member(x.member), action(x.action), host(x.host), error(x.error)
    { }

    LeaveGroup(const fb::protocol::internal::Group& group, const std::string& member, fb::protocol::internal::GroupAction action, uint32_t host, uint32_t error)
        : group(group), member(member), action(action), host(host), error(error)
    { }

    LeaveGroup(const fb::protocol::internal::response::raw::LeaveGroup& raw)
        : group(*raw.group()), member(flatbuffers::option::decode(raw.member()->c_str())), action((fb::protocol::internal::GroupAction)raw.action()), host(raw.host()), error(raw.error())
    { }

public:
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(build<fb::protocol::internal::response::LeaveGroup>(builder, *this));
        auto buffer = std::vector<uint8_t>(builder.GetSize());
        std::memcpy(buffer.data(), builder.GetBufferPointer(), builder.GetSize());
        return buffer;
    }

    static LeaveGroup Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetLeaveGroup(bytes);
        return LeaveGroup(*raw);
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
    return builder.CreateString(flatbuffers::option::encode(value));
}

template <>
flatbuffers::Offset<fb::protocol::internal::raw::Position> build<fb::protocol::internal::Position>(FlatBufferBuilder& builder, const fb::protocol::internal::Position& value)
{
    return fb::protocol::internal::raw::CreatePosition(builder,
            flatbuffers::build<uint16_t>(builder, value.x),
            flatbuffers::build<uint16_t>(builder, value.y));
}
template <>
flatbuffers::Offset<fb::protocol::internal::raw::Character> build<fb::protocol::internal::Character>(FlatBufferBuilder& builder, const fb::protocol::internal::Character& value)
{
    return fb::protocol::internal::raw::CreateCharacter(builder,
            flatbuffers::build<uint32_t>(builder, value.id),
            flatbuffers::build<std::string>(builder, value.name),
            flatbuffers::build<std::string>(builder, value.pw),
            flatbuffers::build<bool>(builder, value.admin),
            flatbuffers::build<uint16_t>(builder, value.look),
            flatbuffers::build<uint16_t>(builder, value.color),
            flatbuffers::build<uint16_t>(builder, value.sex),
            flatbuffers::build<uint16_t>(builder, value.nation),
            flatbuffers::build<std::optional<uint16_t>>(builder, value.creature),
            flatbuffers::build<uint32_t>(builder, value.map),
            flatbuffers::build<fb::protocol::internal::Position>(builder, value.position),
            flatbuffers::build<uint8_t>(builder, value.direction),
            flatbuffers::build<uint8_t>(builder, value.state),
            flatbuffers::build<uint8_t>(builder, value.class_type),
            flatbuffers::build<uint8_t>(builder, value.promotion),
            flatbuffers::build<uint8_t>(builder, value.level),
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
            flatbuffers::build<std::optional<uint32_t>>(builder, value.group),
            flatbuffers::build<std::optional<uint32_t>>(builder, value.clan),
            flatbuffers::build<std::string>(builder, value.updated_date));
}
template <>
flatbuffers::Offset<fb::protocol::internal::raw::Item> build<fb::protocol::internal::Item>(FlatBufferBuilder& builder, const fb::protocol::internal::Item& value)
{
    return fb::protocol::internal::raw::CreateItem(builder,
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
flatbuffers::Offset<fb::protocol::internal::raw::Spell> build<fb::protocol::internal::Spell>(FlatBufferBuilder& builder, const fb::protocol::internal::Spell& value)
{
    return fb::protocol::internal::raw::CreateSpell(builder,
            flatbuffers::build<uint32_t>(builder, value.user),
            flatbuffers::build<uint8_t>(builder, value.slot),
            flatbuffers::build<uint32_t>(builder, value.model));
}
template <>
flatbuffers::Offset<fb::protocol::internal::raw::Option> build<fb::protocol::internal::Option>(FlatBufferBuilder& builder, const fb::protocol::internal::Option& value)
{
    return fb::protocol::internal::raw::CreateOption(builder,
            flatbuffers::build<uint32_t>(builder, value.uid),
            flatbuffers::build<bool>(builder, value.whisper),
            flatbuffers::build<bool>(builder, value.group),
            flatbuffers::build<bool>(builder, value.roar),
            flatbuffers::build<bool>(builder, value.roar_worlds),
            flatbuffers::build<bool>(builder, value.magic_effect),
            flatbuffers::build<bool>(builder, value.weather_effect),
            flatbuffers::build<bool>(builder, value.fixed_move),
            flatbuffers::build<bool>(builder, value.trade),
            flatbuffers::build<bool>(builder, value.fast_move),
            flatbuffers::build<bool>(builder, value.effect_sound),
            flatbuffers::build<bool>(builder, value.pk_protect));
}
template <>
flatbuffers::Offset<fb::protocol::internal::raw::ArticleSummary> build<fb::protocol::internal::ArticleSummary>(FlatBufferBuilder& builder, const fb::protocol::internal::ArticleSummary& value)
{
    return fb::protocol::internal::raw::CreateArticleSummary(builder,
            flatbuffers::build<uint32_t>(builder, value.id),
            flatbuffers::build<uint32_t>(builder, value.user),
            flatbuffers::build<std::string>(builder, value.user_name),
            flatbuffers::build<std::string>(builder, value.title),
            flatbuffers::build<std::string>(builder, value.created_date));
}
template <>
flatbuffers::Offset<fb::protocol::internal::raw::Article> build<fb::protocol::internal::Article>(FlatBufferBuilder& builder, const fb::protocol::internal::Article& value)
{
    return fb::protocol::internal::raw::CreateArticle(builder,
            flatbuffers::build<uint32_t>(builder, value.id),
            flatbuffers::build<uint32_t>(builder, value.user),
            flatbuffers::build<std::string>(builder, value.user_name),
            flatbuffers::build<std::string>(builder, value.title),
            flatbuffers::build<std::string>(builder, value.contents),
            flatbuffers::build<std::string>(builder, value.created_date));
}
template <>
flatbuffers::Offset<fb::protocol::internal::raw::Group> build<fb::protocol::internal::Group>(FlatBufferBuilder& builder, const fb::protocol::internal::Group& value)
{
    return fb::protocol::internal::raw::CreateGroup(builder,
            flatbuffers::build<uint32_t>(builder, value.id),
            flatbuffers::build<std::string>(builder, value.master),
            flatbuffers::build<std::vector<std::string>>(builder, value.members));
}
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::Login> build<fb::protocol::internal::request::Login>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Login& value)
{
    return fb::protocol::internal::request::raw::CreateLogin(builder,
            flatbuffers::build<uint32_t>(builder, value.uid),
            flatbuffers::build<std::string>(builder, value.name),
            flatbuffers::build<uint8_t>(builder, value.host));
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
            flatbuffers::build<uint8_t>(builder, value.id),
            flatbuffers::build<std::optional<std::string>>(builder, value.name),
            flatbuffers::build<bool>(builder, value.force_shutdown));
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
flatbuffers::Offset<fb::protocol::internal::request::raw::Account> build<fb::protocol::internal::request::Account>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Account& value)
{
    return fb::protocol::internal::request::raw::CreateAccount(builder,
            flatbuffers::build<uint32_t>(builder, value.uid));
}
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::ChangePw> build<fb::protocol::internal::request::ChangePw>(FlatBufferBuilder& builder, const fb::protocol::internal::request::ChangePw& value)
{
    return fb::protocol::internal::request::raw::CreateChangePw(builder,
            flatbuffers::build<uint32_t>(builder, value.uid),
            flatbuffers::build<std::string>(builder, value.before),
            flatbuffers::build<std::string>(builder, value.after),
            flatbuffers::build<uint32_t>(builder, value.birthday));
}
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::InitCharacter> build<fb::protocol::internal::request::InitCharacter>(FlatBufferBuilder& builder, const fb::protocol::internal::request::InitCharacter& value)
{
    return fb::protocol::internal::request::raw::CreateInitCharacter(builder,
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
flatbuffers::Offset<fb::protocol::internal::request::raw::Authenticate> build<fb::protocol::internal::request::Authenticate>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Authenticate& value)
{
    return fb::protocol::internal::request::raw::CreateAuthenticate(builder,
            flatbuffers::build<uint32_t>(builder, value.uid),
            flatbuffers::build<std::string>(builder, value.pw));
}
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::Init> build<fb::protocol::internal::request::Init>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Init& value)
{
    return fb::protocol::internal::request::raw::CreateInit(builder,
            flatbuffers::build<uint32_t>(builder, value.uid));
}
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::MakeCharacter> build<fb::protocol::internal::request::MakeCharacter>(FlatBufferBuilder& builder, const fb::protocol::internal::request::MakeCharacter& value)
{
    return fb::protocol::internal::request::raw::CreateMakeCharacter(builder,
            flatbuffers::build<uint32_t>(builder, value.uid),
            flatbuffers::build<uint16_t>(builder, value.hair),
            flatbuffers::build<uint8_t>(builder, value.sex),
            flatbuffers::build<uint8_t>(builder, value.nation),
            flatbuffers::build<uint8_t>(builder, value.creature));
}
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::ReserveName> build<fb::protocol::internal::request::ReserveName>(FlatBufferBuilder& builder, const fb::protocol::internal::request::ReserveName& value)
{
    return fb::protocol::internal::request::raw::CreateReserveName(builder,
            flatbuffers::build<std::string>(builder, value.name));
}
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::Save> build<fb::protocol::internal::request::Save>(FlatBufferBuilder& builder, const fb::protocol::internal::request::Save& value)
{
    return fb::protocol::internal::request::raw::CreateSave(builder,
            flatbuffers::build<fb::protocol::internal::Character>(builder, value.character),
            flatbuffers::build<std::vector<fb::protocol::internal::Item>>(builder, value.items),
            flatbuffers::build<std::vector<fb::protocol::internal::Spell>>(builder, value.spells));
}
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::GetArticle> build<fb::protocol::internal::request::GetArticle>(FlatBufferBuilder& builder, const fb::protocol::internal::request::GetArticle& value)
{
    return fb::protocol::internal::request::raw::CreateGetArticle(builder,
            flatbuffers::build<uint32_t>(builder, value.section),
            flatbuffers::build<uint32_t>(builder, value.article));
}
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::GetArticleList> build<fb::protocol::internal::request::GetArticleList>(FlatBufferBuilder& builder, const fb::protocol::internal::request::GetArticleList& value)
{
    return fb::protocol::internal::request::raw::CreateGetArticleList(builder,
            flatbuffers::build<uint32_t>(builder, value.section),
            flatbuffers::build<uint32_t>(builder, value.position));
}
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::WriteArticle> build<fb::protocol::internal::request::WriteArticle>(FlatBufferBuilder& builder, const fb::protocol::internal::request::WriteArticle& value)
{
    return fb::protocol::internal::request::raw::CreateWriteArticle(builder,
            flatbuffers::build<uint32_t>(builder, value.section),
            flatbuffers::build<uint32_t>(builder, value.user),
            flatbuffers::build<std::string>(builder, value.title),
            flatbuffers::build<std::string>(builder, value.contents));
}
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::DeleteArticle> build<fb::protocol::internal::request::DeleteArticle>(FlatBufferBuilder& builder, const fb::protocol::internal::request::DeleteArticle& value)
{
    return fb::protocol::internal::request::raw::CreateDeleteArticle(builder,
            flatbuffers::build<uint32_t>(builder, value.id),
            flatbuffers::build<uint32_t>(builder, value.user));
}
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::SetOption> build<fb::protocol::internal::request::SetOption>(FlatBufferBuilder& builder, const fb::protocol::internal::request::SetOption& value)
{
    return fb::protocol::internal::request::raw::CreateSetOption(builder,
            flatbuffers::build<uint32_t>(builder, value.user),
            flatbuffers::build<uint8_t>(builder, value.type),
            flatbuffers::build<bool>(builder, value.enabled));
}
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::EnterGroup> build<fb::protocol::internal::request::EnterGroup>(FlatBufferBuilder& builder, const fb::protocol::internal::request::EnterGroup& value)
{
    return fb::protocol::internal::request::raw::CreateEnterGroup(builder,
            flatbuffers::build<uint32_t>(builder, value.master),
            flatbuffers::build<std::string>(builder, value.member));
}
template <>
flatbuffers::Offset<fb::protocol::internal::request::raw::LeaveGroup> build<fb::protocol::internal::request::LeaveGroup>(FlatBufferBuilder& builder, const fb::protocol::internal::request::LeaveGroup& value)
{
    return fb::protocol::internal::request::raw::CreateLeaveGroup(builder,
            flatbuffers::build<std::string>(builder, value.member));
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
            flatbuffers::build<uint32_t>(builder, value.error),
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
            flatbuffers::build<uint32_t>(builder, value.error),
            flatbuffers::build<std::string>(builder, value.ip),
            flatbuffers::build<uint16_t>(builder, value.port));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::Whisper> build<fb::protocol::internal::response::Whisper>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Whisper& value)
{
    return fb::protocol::internal::response::raw::CreateWhisper(builder,
            flatbuffers::build<std::string>(builder, value.from),
            flatbuffers::build<std::string>(builder, value.to),
            flatbuffers::build<std::string>(builder, value.message),
            flatbuffers::build<uint32_t>(builder, value.host),
            flatbuffers::build<uint32_t>(builder, value.error));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::DeleteArticle> build<fb::protocol::internal::response::DeleteArticle>(FlatBufferBuilder& builder, const fb::protocol::internal::response::DeleteArticle& value)
{
    return fb::protocol::internal::response::raw::CreateDeleteArticle(builder,
            flatbuffers::build<int>(builder, value.result));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::GetArticle> build<fb::protocol::internal::response::GetArticle>(FlatBufferBuilder& builder, const fb::protocol::internal::response::GetArticle& value)
{
    return fb::protocol::internal::response::raw::CreateGetArticle(builder,
            flatbuffers::build<fb::protocol::internal::Article>(builder, value.article),
            flatbuffers::build<bool>(builder, value.next),
            flatbuffers::build<bool>(builder, value.success));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::GetArticleList> build<fb::protocol::internal::response::GetArticleList>(FlatBufferBuilder& builder, const fb::protocol::internal::response::GetArticleList& value)
{
    return fb::protocol::internal::response::raw::CreateGetArticleList(builder,
            flatbuffers::build<std::vector<fb::protocol::internal::ArticleSummary>>(builder, value.summary_list));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::WriteArticle> build<fb::protocol::internal::response::WriteArticle>(FlatBufferBuilder& builder, const fb::protocol::internal::response::WriteArticle& value)
{
    return fb::protocol::internal::response::raw::CreateWriteArticle(builder,
            flatbuffers::build<bool>(builder, value.success));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::ChangePw> build<fb::protocol::internal::response::ChangePw>(FlatBufferBuilder& builder, const fb::protocol::internal::response::ChangePw& value)
{
    return fb::protocol::internal::response::raw::CreateChangePw(builder,
            flatbuffers::build<uint32_t>(builder, value.error_code));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::GetUid> build<fb::protocol::internal::response::GetUid>(FlatBufferBuilder& builder, const fb::protocol::internal::response::GetUid& value)
{
    return fb::protocol::internal::response::raw::CreateGetUid(builder,
            flatbuffers::build<uint32_t>(builder, value.uid),
            flatbuffers::build<bool>(builder, value.success));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::InitCharacter> build<fb::protocol::internal::response::InitCharacter>(FlatBufferBuilder& builder, const fb::protocol::internal::response::InitCharacter& value)
{
    return fb::protocol::internal::response::raw::CreateInitCharacter(builder,
            flatbuffers::build<bool>(builder, value.success));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::Authenticate> build<fb::protocol::internal::response::Authenticate>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Authenticate& value)
{
    return fb::protocol::internal::response::raw::CreateAuthenticate(builder,
            flatbuffers::build<uint32_t>(builder, value.map),
            flatbuffers::build<uint32_t>(builder, value.error_code));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::Init> build<fb::protocol::internal::response::Init>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Init& value)
{
    return fb::protocol::internal::response::raw::CreateInit(builder,
            flatbuffers::build<fb::protocol::internal::Character>(builder, value.character),
            flatbuffers::build<std::vector<fb::protocol::internal::Item>>(builder, value.items),
            flatbuffers::build<std::vector<fb::protocol::internal::Spell>>(builder, value.spells),
            flatbuffers::build<fb::protocol::internal::Option>(builder, value.option));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::MakeCharacter> build<fb::protocol::internal::response::MakeCharacter>(FlatBufferBuilder& builder, const fb::protocol::internal::response::MakeCharacter& value)
{
    return fb::protocol::internal::response::raw::CreateMakeCharacter(builder,
            flatbuffers::build<bool>(builder, value.success));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::ReserveName> build<fb::protocol::internal::response::ReserveName>(FlatBufferBuilder& builder, const fb::protocol::internal::response::ReserveName& value)
{
    return fb::protocol::internal::response::raw::CreateReserveName(builder,
            flatbuffers::build<uint32_t>(builder, value.uid),
            flatbuffers::build<bool>(builder, value.success));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::Save> build<fb::protocol::internal::response::Save>(FlatBufferBuilder& builder, const fb::protocol::internal::response::Save& value)
{
    return fb::protocol::internal::response::raw::CreateSave(builder,
            flatbuffers::build<bool>(builder, value.success));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::SetOption> build<fb::protocol::internal::response::SetOption>(FlatBufferBuilder& builder, const fb::protocol::internal::response::SetOption& value)
{
    return fb::protocol::internal::response::raw::CreateSetOption(builder,
            flatbuffers::build<bool>(builder, value.success));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::GetGroup> build<fb::protocol::internal::response::GetGroup>(FlatBufferBuilder& builder, const fb::protocol::internal::response::GetGroup& value)
{
    return fb::protocol::internal::response::raw::CreateGetGroup(builder,
            flatbuffers::build<fb::protocol::internal::Group>(builder, value.group),
            flatbuffers::build<uint32_t>(builder, value.error));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::EnterGroup> build<fb::protocol::internal::response::EnterGroup>(FlatBufferBuilder& builder, const fb::protocol::internal::response::EnterGroup& value)
{
    return fb::protocol::internal::response::raw::CreateEnterGroup(builder,
            flatbuffers::build<fb::protocol::internal::Group>(builder, value.group),
            flatbuffers::build<std::string>(builder, value.member),
            flatbuffers::build<fb::protocol::internal::GroupAction>(builder, value.action),
            flatbuffers::build<uint32_t>(builder, value.host),
            flatbuffers::build<uint32_t>(builder, value.error));
}
template <>
flatbuffers::Offset<fb::protocol::internal::response::raw::LeaveGroup> build<fb::protocol::internal::response::LeaveGroup>(FlatBufferBuilder& builder, const fb::protocol::internal::response::LeaveGroup& value)
{
    return fb::protocol::internal::response::raw::CreateLeaveGroup(builder,
            flatbuffers::build<fb::protocol::internal::Group>(builder, value.group),
            flatbuffers::build<std::string>(builder, value.member),
            flatbuffers::build<fb::protocol::internal::GroupAction>(builder, value.action),
            flatbuffers::build<uint32_t>(builder, value.host),
            flatbuffers::build<uint32_t>(builder, value.error));
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
            else if constexpr (std::is_same_v<T, std::string>)
            {
                result.push_back(flatbuffers::option::decode(value->Get(i)->c_str()));
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