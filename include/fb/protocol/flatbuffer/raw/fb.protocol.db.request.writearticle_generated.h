// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_FBPROTOCOLDBREQUESTWRITEARTICLE_FB_PROTOCOL_DB_REQUEST_RAW_H_
#define FLATBUFFERS_GENERATED_FBPROTOCOLDBREQUESTWRITEARTICLE_FB_PROTOCOL_DB_REQUEST_RAW_H_

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
namespace request {
namespace raw {

struct WriteArticle;
struct WriteArticleBuilder;

struct WriteArticle FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef WriteArticleBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SECTION = 4,
    VT_USER = 6,
    VT_TITLE = 8,
    VT_CONTENTS = 10
  };
  uint32_t section() const {
    return GetField<uint32_t>(VT_SECTION, 0);
  }
  uint32_t user() const {
    return GetField<uint32_t>(VT_USER, 0);
  }
  const ::flatbuffers::String *title() const {
    return GetPointer<const ::flatbuffers::String *>(VT_TITLE);
  }
  const ::flatbuffers::String *contents() const {
    return GetPointer<const ::flatbuffers::String *>(VT_CONTENTS);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_SECTION, 4) &&
           VerifyField<uint32_t>(verifier, VT_USER, 4) &&
           VerifyOffset(verifier, VT_TITLE) &&
           verifier.VerifyString(title()) &&
           VerifyOffset(verifier, VT_CONTENTS) &&
           verifier.VerifyString(contents()) &&
           verifier.EndTable();
  }
};

struct WriteArticleBuilder {
  typedef WriteArticle Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_section(uint32_t section) {
    fbb_.AddElement<uint32_t>(WriteArticle::VT_SECTION, section, 0);
  }
  void add_user(uint32_t user) {
    fbb_.AddElement<uint32_t>(WriteArticle::VT_USER, user, 0);
  }
  void add_title(::flatbuffers::Offset<::flatbuffers::String> title) {
    fbb_.AddOffset(WriteArticle::VT_TITLE, title);
  }
  void add_contents(::flatbuffers::Offset<::flatbuffers::String> contents) {
    fbb_.AddOffset(WriteArticle::VT_CONTENTS, contents);
  }
  explicit WriteArticleBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<WriteArticle> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<WriteArticle>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<WriteArticle> CreateWriteArticle(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t section = 0,
    uint32_t user = 0,
    ::flatbuffers::Offset<::flatbuffers::String> title = 0,
    ::flatbuffers::Offset<::flatbuffers::String> contents = 0) {
  WriteArticleBuilder builder_(_fbb);
  builder_.add_contents(contents);
  builder_.add_title(title);
  builder_.add_user(user);
  builder_.add_section(section);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<WriteArticle> CreateWriteArticleDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t section = 0,
    uint32_t user = 0,
    const char *title = nullptr,
    const char *contents = nullptr) {
  auto title__ = title ? _fbb.CreateString(title) : 0;
  auto contents__ = contents ? _fbb.CreateString(contents) : 0;
  return fb::protocol::db::request::raw::CreateWriteArticle(
      _fbb,
      section,
      user,
      title__,
      contents__);
}

inline const fb::protocol::db::request::raw::WriteArticle *GetWriteArticle(const void *buf) {
  return ::flatbuffers::GetRoot<fb::protocol::db::request::raw::WriteArticle>(buf);
}

inline const fb::protocol::db::request::raw::WriteArticle *GetSizePrefixedWriteArticle(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<fb::protocol::db::request::raw::WriteArticle>(buf);
}

inline bool VerifyWriteArticleBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<fb::protocol::db::request::raw::WriteArticle>(nullptr);
}

inline bool VerifySizePrefixedWriteArticleBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<fb::protocol::db::request::raw::WriteArticle>(nullptr);
}

inline void FinishWriteArticleBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::db::request::raw::WriteArticle> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedWriteArticleBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::db::request::raw::WriteArticle> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace raw
}  // namespace request
}  // namespace db
}  // namespace protocol
}  // namespace fb

#endif  // FLATBUFFERS_GENERATED_FBPROTOCOLDBREQUESTWRITEARTICLE_FB_PROTOCOL_DB_REQUEST_RAW_H_
