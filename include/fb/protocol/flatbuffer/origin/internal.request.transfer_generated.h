// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_INTERNALREQUESTTRANSFER_FB_PROTOCOL_INTERNAL_REQUEST_ORIGIN_H_
#define FLATBUFFERS_GENERATED_INTERNALREQUESTTRANSFER_FB_PROTOCOL_INTERNAL_REQUEST_ORIGIN_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

#include "internal_generated.h"

namespace fb {
namespace protocol {
namespace internal {
namespace request {
namespace origin {

struct Transfer;
struct TransferBuilder;

struct Transfer FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef TransferBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SERVICE = 4,
    VT_ID = 6
  };
  fb::protocol::internal::origin::Service service() const {
    return static_cast<fb::protocol::internal::origin::Service>(GetField<int8_t>(VT_SERVICE, 0));
  }
  uint8_t id() const {
    return GetField<uint8_t>(VT_ID, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, VT_SERVICE, 1) &&
           VerifyField<uint8_t>(verifier, VT_ID, 1) &&
           verifier.EndTable();
  }
};

struct TransferBuilder {
  typedef Transfer Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_service(fb::protocol::internal::origin::Service service) {
    fbb_.AddElement<int8_t>(Transfer::VT_SERVICE, static_cast<int8_t>(service), 0);
  }
  void add_id(uint8_t id) {
    fbb_.AddElement<uint8_t>(Transfer::VT_ID, id, 0);
  }
  explicit TransferBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Transfer> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Transfer>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Transfer> CreateTransfer(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    fb::protocol::internal::origin::Service service = fb::protocol::internal::origin::Service_Gateway,
    uint8_t id = 0) {
  TransferBuilder builder_(_fbb);
  builder_.add_id(id);
  builder_.add_service(service);
  return builder_.Finish();
}

inline const fb::protocol::internal::request::origin::Transfer *GetTransfer(const void *buf) {
  return ::flatbuffers::GetRoot<fb::protocol::internal::request::origin::Transfer>(buf);
}

inline const fb::protocol::internal::request::origin::Transfer *GetSizePrefixedTransfer(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<fb::protocol::internal::request::origin::Transfer>(buf);
}

inline bool VerifyTransferBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<fb::protocol::internal::request::origin::Transfer>(nullptr);
}

inline bool VerifySizePrefixedTransferBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<fb::protocol::internal::request::origin::Transfer>(nullptr);
}

inline void FinishTransferBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::internal::request::origin::Transfer> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedTransferBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::internal::request::origin::Transfer> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace origin
}  // namespace request
}  // namespace internal
}  // namespace protocol
}  // namespace fb

#endif  // FLATBUFFERS_GENERATED_INTERNALREQUESTTRANSFER_FB_PROTOCOL_INTERNAL_REQUEST_ORIGIN_H_
