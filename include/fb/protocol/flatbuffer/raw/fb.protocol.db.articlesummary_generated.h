// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_FBPROTOCOLDBARTICLESUMMARY_FB_PROTOCOL_DB_RAW_H_
#define FLATBUFFERS_GENERATED_FBPROTOCOLDBARTICLESUMMARY_FB_PROTOCOL_DB_RAW_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

namespace fb {
namespace protocol {
namespace db {
namespace raw {

struct ArticleSummary;
struct ArticleSummaryBuilder;

struct ArticleSummary FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ArticleSummaryBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ID = 4,
    VT_USER = 6,
    VT_USER_NAME = 8,
    VT_TITLE = 10,
    VT_CREATED_DATE = 12
  };
  uint32_t id() const {
    return GetField<uint32_t>(VT_ID, 0);
  }
  uint32_t user() const {
    return GetField<uint32_t>(VT_USER, 0);
  }
  const ::flatbuffers::String *user_name() const {
    return GetPointer<const ::flatbuffers::String *>(VT_USER_NAME);
  }
  const ::flatbuffers::String *title() const {
    return GetPointer<const ::flatbuffers::String *>(VT_TITLE);
  }
  const ::flatbuffers::String *created_date() const {
    return GetPointer<const ::flatbuffers::String *>(VT_CREATED_DATE);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_ID, 4) &&
           VerifyField<uint32_t>(verifier, VT_USER, 4) &&
           VerifyOffset(verifier, VT_USER_NAME) &&
           verifier.VerifyString(user_name()) &&
           VerifyOffset(verifier, VT_TITLE) &&
           verifier.VerifyString(title()) &&
           VerifyOffset(verifier, VT_CREATED_DATE) &&
           verifier.VerifyString(created_date()) &&
           verifier.EndTable();
  }
};

struct ArticleSummaryBuilder {
  typedef ArticleSummary Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_id(uint32_t id) {
    fbb_.AddElement<uint32_t>(ArticleSummary::VT_ID, id, 0);
  }
  void add_user(uint32_t user) {
    fbb_.AddElement<uint32_t>(ArticleSummary::VT_USER, user, 0);
  }
  void add_user_name(::flatbuffers::Offset<::flatbuffers::String> user_name) {
    fbb_.AddOffset(ArticleSummary::VT_USER_NAME, user_name);
  }
  void add_title(::flatbuffers::Offset<::flatbuffers::String> title) {
    fbb_.AddOffset(ArticleSummary::VT_TITLE, title);
  }
  void add_created_date(::flatbuffers::Offset<::flatbuffers::String> created_date) {
    fbb_.AddOffset(ArticleSummary::VT_CREATED_DATE, created_date);
  }
  explicit ArticleSummaryBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ArticleSummary> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ArticleSummary>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<ArticleSummary> CreateArticleSummary(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t id = 0,
    uint32_t user = 0,
    ::flatbuffers::Offset<::flatbuffers::String> user_name = 0,
    ::flatbuffers::Offset<::flatbuffers::String> title = 0,
    ::flatbuffers::Offset<::flatbuffers::String> created_date = 0) {
  ArticleSummaryBuilder builder_(_fbb);
  builder_.add_created_date(created_date);
  builder_.add_title(title);
  builder_.add_user_name(user_name);
  builder_.add_user(user);
  builder_.add_id(id);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<ArticleSummary> CreateArticleSummaryDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t id = 0,
    uint32_t user = 0,
    const char *user_name = nullptr,
    const char *title = nullptr,
    const char *created_date = nullptr) {
  auto user_name__ = user_name ? _fbb.CreateString(user_name) : 0;
  auto title__ = title ? _fbb.CreateString(title) : 0;
  auto created_date__ = created_date ? _fbb.CreateString(created_date) : 0;
  return fb::protocol::db::raw::CreateArticleSummary(
      _fbb,
      id,
      user,
      user_name__,
      title__,
      created_date__);
}

inline const fb::protocol::db::raw::ArticleSummary *GetArticleSummary(const void *buf) {
  return ::flatbuffers::GetRoot<fb::protocol::db::raw::ArticleSummary>(buf);
}

inline const fb::protocol::db::raw::ArticleSummary *GetSizePrefixedArticleSummary(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<fb::protocol::db::raw::ArticleSummary>(buf);
}

inline bool VerifyArticleSummaryBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<fb::protocol::db::raw::ArticleSummary>(nullptr);
}

inline bool VerifySizePrefixedArticleSummaryBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<fb::protocol::db::raw::ArticleSummary>(nullptr);
}

inline void FinishArticleSummaryBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::db::raw::ArticleSummary> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedArticleSummaryBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::db::raw::ArticleSummary> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace raw
}  // namespace db
}  // namespace protocol
}  // namespace fb

#endif  // FLATBUFFERS_GENERATED_FBPROTOCOLDBARTICLESUMMARY_FB_PROTOCOL_DB_RAW_H_
