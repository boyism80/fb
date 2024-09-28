// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_INTERNALREQUESTLOGIN_FB_PROTOCOL_INTERNAL_REQUEST_ORIGIN_H_
#define FLATBUFFERS_GENERATED_INTERNALREQUESTLOGIN_FB_PROTOCOL_INTERNAL_REQUEST_ORIGIN_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

namespace fb {
namespace protocol {
namespace internal {
namespace request {
namespace origin {

struct Login;
struct LoginBuilder;

struct Login FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef LoginBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_UID = 4,
    VT_NAME = 6,
    VT_MAP = 8
  };
  uint32_t uid() const {
    return GetField<uint32_t>(VT_UID, 0);
  }
  const ::flatbuffers::String *name() const {
    return GetPointer<const ::flatbuffers::String *>(VT_NAME);
  }
  uint16_t map() const {
    return GetField<uint16_t>(VT_MAP, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_UID, 4) &&
           VerifyOffset(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyField<uint16_t>(verifier, VT_MAP, 2) &&
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
  void add_name(::flatbuffers::Offset<::flatbuffers::String> name) {
    fbb_.AddOffset(Login::VT_NAME, name);
  }
  void add_map(uint16_t map) {
    fbb_.AddElement<uint16_t>(Login::VT_MAP, map, 0);
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
    uint32_t uid = 0,
    ::flatbuffers::Offset<::flatbuffers::String> name = 0,
    uint16_t map = 0) {
  LoginBuilder builder_(_fbb);
  builder_.add_name(name);
  builder_.add_uid(uid);
  builder_.add_map(map);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<Login> CreateLoginDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t uid = 0,
    const char *name = nullptr,
    uint16_t map = 0) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  return fb::protocol::internal::request::origin::CreateLogin(
      _fbb,
      uid,
      name__,
      map);
}

inline const fb::protocol::internal::request::origin::Login *GetLogin(const void *buf) {
  return ::flatbuffers::GetRoot<fb::protocol::internal::request::origin::Login>(buf);
}

inline const fb::protocol::internal::request::origin::Login *GetSizePrefixedLogin(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<fb::protocol::internal::request::origin::Login>(buf);
}

inline bool VerifyLoginBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<fb::protocol::internal::request::origin::Login>(nullptr);
}

inline bool VerifySizePrefixedLoginBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<fb::protocol::internal::request::origin::Login>(nullptr);
}

inline void FinishLoginBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::internal::request::origin::Login> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedLoginBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::internal::request::origin::Login> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace origin
}  // namespace request
}  // namespace internal
}  // namespace protocol
}  // namespace fb

#endif  // FLATBUFFERS_GENERATED_INTERNALREQUESTLOGIN_FB_PROTOCOL_INTERNAL_REQUEST_ORIGIN_H_
