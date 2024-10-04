// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_DBREQUESTREVERSENAME_FB_PROTOCOL_DB_REQUEST_ORIGIN_H_
#define FLATBUFFERS_GENERATED_DBREQUESTREVERSENAME_FB_PROTOCOL_DB_REQUEST_ORIGIN_H_

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
namespace origin {

struct ReserveName;
struct ReserveNameBuilder;

struct ReserveName FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ReserveNameBuilder Builder;
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

struct ReserveNameBuilder {
  typedef ReserveName Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_name(::flatbuffers::Offset<::flatbuffers::String> name) {
    fbb_.AddOffset(ReserveName::VT_NAME, name);
  }
  explicit ReserveNameBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ReserveName> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ReserveName>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<ReserveName> CreateReserveName(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> name = 0) {
  ReserveNameBuilder builder_(_fbb);
  builder_.add_name(name);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<ReserveName> CreateReserveNameDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  return fb::protocol::db::request::origin::CreateReserveName(
      _fbb,
      name__);
}

inline const fb::protocol::db::request::origin::ReserveName *GetReserveName(const void *buf) {
  return ::flatbuffers::GetRoot<fb::protocol::db::request::origin::ReserveName>(buf);
}

inline const fb::protocol::db::request::origin::ReserveName *GetSizePrefixedReserveName(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<fb::protocol::db::request::origin::ReserveName>(buf);
}

inline bool VerifyReserveNameBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<fb::protocol::db::request::origin::ReserveName>(nullptr);
}

inline bool VerifySizePrefixedReserveNameBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<fb::protocol::db::request::origin::ReserveName>(nullptr);
}

inline void FinishReserveNameBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::db::request::origin::ReserveName> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedReserveNameBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::db::request::origin::ReserveName> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace origin
}  // namespace request
}  // namespace db
}  // namespace protocol
}  // namespace fb

#endif  // FLATBUFFERS_GENERATED_DBREQUESTREVERSENAME_FB_PROTOCOL_DB_REQUEST_ORIGIN_H_
