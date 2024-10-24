// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_FBPROTOCOLINTERNALREQUESTLOGOUT_FB_PROTOCOL_INTERNAL_REQUEST_RAW_H_
#define FLATBUFFERS_GENERATED_FBPROTOCOLINTERNALREQUESTLOGOUT_FB_PROTOCOL_INTERNAL_REQUEST_RAW_H_

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
namespace raw {

struct Logout;
struct LogoutBuilder;

struct Logout FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef LogoutBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4
  };
  const ::flatbuffers::String *name() const {
    return GetPointer<const ::flatbuffers::String *>(VT_NAME);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           verifier.EndTable();
  }
};

struct LogoutBuilder {
  typedef Logout Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_name(::flatbuffers::Offset<::flatbuffers::String> name) {
    fbb_.AddOffset(Logout::VT_NAME, name);
  }
  explicit LogoutBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Logout> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Logout>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Logout> CreateLogout(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> name = 0) {
  LogoutBuilder builder_(_fbb);
  builder_.add_name(name);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<Logout> CreateLogoutDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  return fb::protocol::internal::request::raw::CreateLogout(
      _fbb,
      name__);
}

inline const fb::protocol::internal::request::raw::Logout *GetLogout(const void *buf) {
  return ::flatbuffers::GetRoot<fb::protocol::internal::request::raw::Logout>(buf);
}

inline const fb::protocol::internal::request::raw::Logout *GetSizePrefixedLogout(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<fb::protocol::internal::request::raw::Logout>(buf);
}

inline bool VerifyLogoutBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<fb::protocol::internal::request::raw::Logout>(nullptr);
}

inline bool VerifySizePrefixedLogoutBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<fb::protocol::internal::request::raw::Logout>(nullptr);
}

inline void FinishLogoutBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::internal::request::raw::Logout> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedLogoutBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::internal::request::raw::Logout> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace raw
}  // namespace request
}  // namespace internal
}  // namespace protocol
}  // namespace fb

#endif  // FLATBUFFERS_GENERATED_FBPROTOCOLINTERNALREQUESTLOGOUT_FB_PROTOCOL_INTERNAL_REQUEST_RAW_H_
