// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_FBPROTOCOLDBRESPONSECHANGEPW_FB_PROTOCOL_DB_RESPONSE_RAW_H_
#define FLATBUFFERS_GENERATED_FBPROTOCOLDBRESPONSECHANGEPW_FB_PROTOCOL_DB_RESPONSE_RAW_H_

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
namespace response {
namespace raw {

struct ChangePw;
struct ChangePwBuilder;

struct ChangePw FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ChangePwBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_RESULT = 4
  };
  int32_t result() const {
    return GetField<int32_t>(VT_RESULT, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int32_t>(verifier, VT_RESULT, 4) &&
           verifier.EndTable();
  }
};

struct ChangePwBuilder {
  typedef ChangePw Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_result(int32_t result) {
    fbb_.AddElement<int32_t>(ChangePw::VT_RESULT, result, 0);
  }
  explicit ChangePwBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ChangePw> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ChangePw>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<ChangePw> CreateChangePw(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    int32_t result = 0) {
  ChangePwBuilder builder_(_fbb);
  builder_.add_result(result);
  return builder_.Finish();
}

inline const fb::protocol::db::response::raw::ChangePw *GetChangePw(const void *buf) {
  return ::flatbuffers::GetRoot<fb::protocol::db::response::raw::ChangePw>(buf);
}

inline const fb::protocol::db::response::raw::ChangePw *GetSizePrefixedChangePw(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<fb::protocol::db::response::raw::ChangePw>(buf);
}

inline bool VerifyChangePwBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<fb::protocol::db::response::raw::ChangePw>(nullptr);
}

inline bool VerifySizePrefixedChangePwBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<fb::protocol::db::response::raw::ChangePw>(nullptr);
}

inline void FinishChangePwBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::db::response::raw::ChangePw> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedChangePwBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::db::response::raw::ChangePw> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace raw
}  // namespace response
}  // namespace db
}  // namespace protocol
}  // namespace fb

#endif  // FLATBUFFERS_GENERATED_FBPROTOCOLDBRESPONSECHANGEPW_FB_PROTOCOL_DB_RESPONSE_RAW_H_
