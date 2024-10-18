// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_FBPROTOCOLDBREQUESTLOGIN_FB_PROTOCOL_DB_REQUEST_RAW_H_
#define FLATBUFFERS_GENERATED_FBPROTOCOLDBREQUESTLOGIN_FB_PROTOCOL_DB_REQUEST_RAW_H_

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

struct Login;
struct LoginBuilder;

struct Login FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef LoginBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_UID = 4
  };
  uint32_t uid() const {
    return GetField<uint32_t>(VT_UID, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_UID, 4) &&
           verifier.EndTable();
  }
};

struct LoginBuilder {
  typedef Login Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_uid(uint32_t uid) {
    fbb_.AddElement<uint32_t>(Login::VT_UID, uid, 0);
  }
  explicit LoginBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Login> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Login>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Login> CreateLogin(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t uid = 0) {
  LoginBuilder builder_(_fbb);
  builder_.add_uid(uid);
  return builder_.Finish();
}

inline const fb::protocol::db::request::raw::Login *GetLogin(const void *buf) {
  return ::flatbuffers::GetRoot<fb::protocol::db::request::raw::Login>(buf);
}

inline const fb::protocol::db::request::raw::Login *GetSizePrefixedLogin(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<fb::protocol::db::request::raw::Login>(buf);
}

inline bool VerifyLoginBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<fb::protocol::db::request::raw::Login>(nullptr);
}

inline bool VerifySizePrefixedLoginBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<fb::protocol::db::request::raw::Login>(nullptr);
}

inline void FinishLoginBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::db::request::raw::Login> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedLoginBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::db::request::raw::Login> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace raw
}  // namespace request
}  // namespace db
}  // namespace protocol
}  // namespace fb

#endif  // FLATBUFFERS_GENERATED_FBPROTOCOLDBREQUESTLOGIN_FB_PROTOCOL_DB_REQUEST_RAW_H_