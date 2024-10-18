// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_FBPROTOCOLDBRESPONSEGETARTICLE_FB_PROTOCOL_DB_RESPONSE_RAW_H_
#define FLATBUFFERS_GENERATED_FBPROTOCOLDBRESPONSEGETARTICLE_FB_PROTOCOL_DB_RESPONSE_RAW_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

#include "fb.protocol.db.article_generated.h"

namespace fb {
namespace protocol {
namespace db {
namespace response {
namespace raw {

struct GetArticle;
struct GetArticleBuilder;

struct GetArticle FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef GetArticleBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ARTICLE = 4,
    VT_NEXT = 6,
    VT_SUCCESS = 8
  };
  const fb::protocol::db::raw::Article *article() const {
    return GetPointer<const fb::protocol::db::raw::Article *>(VT_ARTICLE);
  }
  bool next() const {
    return GetField<uint8_t>(VT_NEXT, 0) != 0;
  }
  bool success() const {
    return GetField<uint8_t>(VT_SUCCESS, 0) != 0;
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_ARTICLE) &&
           verifier.VerifyTable(article()) &&
           VerifyField<uint8_t>(verifier, VT_NEXT, 1) &&
           VerifyField<uint8_t>(verifier, VT_SUCCESS, 1) &&
           verifier.EndTable();
  }
};

struct GetArticleBuilder {
  typedef GetArticle Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_article(::flatbuffers::Offset<fb::protocol::db::raw::Article> article) {
    fbb_.AddOffset(GetArticle::VT_ARTICLE, article);
  }
  void add_next(bool next) {
    fbb_.AddElement<uint8_t>(GetArticle::VT_NEXT, static_cast<uint8_t>(next), 0);
  }
  void add_success(bool success) {
    fbb_.AddElement<uint8_t>(GetArticle::VT_SUCCESS, static_cast<uint8_t>(success), 0);
  }
  explicit GetArticleBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<GetArticle> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<GetArticle>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<GetArticle> CreateGetArticle(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<fb::protocol::db::raw::Article> article = 0,
    bool next = false,
    bool success = false) {
  GetArticleBuilder builder_(_fbb);
  builder_.add_article(article);
  builder_.add_success(success);
  builder_.add_next(next);
  return builder_.Finish();
}

inline const fb::protocol::db::response::raw::GetArticle *GetGetArticle(const void *buf) {
  return ::flatbuffers::GetRoot<fb::protocol::db::response::raw::GetArticle>(buf);
}

inline const fb::protocol::db::response::raw::GetArticle *GetSizePrefixedGetArticle(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<fb::protocol::db::response::raw::GetArticle>(buf);
}

inline bool VerifyGetArticleBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<fb::protocol::db::response::raw::GetArticle>(nullptr);
}

inline bool VerifySizePrefixedGetArticleBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<fb::protocol::db::response::raw::GetArticle>(nullptr);
}

inline void FinishGetArticleBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::db::response::raw::GetArticle> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedGetArticleBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::db::response::raw::GetArticle> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace raw
}  // namespace response
}  // namespace db
}  // namespace protocol
}  // namespace fb

#endif  // FLATBUFFERS_GENERATED_FBPROTOCOLDBRESPONSEGETARTICLE_FB_PROTOCOL_DB_RESPONSE_RAW_H_