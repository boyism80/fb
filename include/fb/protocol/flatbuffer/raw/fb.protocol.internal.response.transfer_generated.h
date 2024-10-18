// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_FBPROTOCOLINTERNALRESPONSETRANSFER_FB_PROTOCOL_INTERNAL_RESPONSE_RAW_H_
#define FLATBUFFERS_GENERATED_FBPROTOCOLINTERNALRESPONSETRANSFER_FB_PROTOCOL_INTERNAL_RESPONSE_RAW_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

#include "fb.protocol.internal.transferresult_generated.h"

namespace fb {
namespace protocol {
namespace internal {
namespace response {
namespace raw {

struct Transfer;
struct TransferBuilder;

struct Transfer FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef TransferBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_CODE = 4,
    VT_IP = 6,
    VT_PORT = 8
  };
  fb::protocol::internal::raw::TransferResult code() const {
    return static_cast<fb::protocol::internal::raw::TransferResult>(GetField<int8_t>(VT_CODE, 0));
  }
  const ::flatbuffers::String *ip() const {
    return GetPointer<const ::flatbuffers::String *>(VT_IP);
  }
  uint16_t port() const {
    return GetField<uint16_t>(VT_PORT, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, VT_CODE, 1) &&
           VerifyOffset(verifier, VT_IP) &&
           verifier.VerifyString(ip()) &&
           VerifyField<uint16_t>(verifier, VT_PORT, 2) &&
           verifier.EndTable();
  }
};

struct TransferBuilder {
  typedef Transfer Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_code(fb::protocol::internal::raw::TransferResult code) {
    fbb_.AddElement<int8_t>(Transfer::VT_CODE, static_cast<int8_t>(code), 0);
  }
  void add_ip(::flatbuffers::Offset<::flatbuffers::String> ip) {
    fbb_.AddOffset(Transfer::VT_IP, ip);
  }
  void add_port(uint16_t port) {
    fbb_.AddElement<uint16_t>(Transfer::VT_PORT, port, 0);
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
    fb::protocol::internal::raw::TransferResult code = fb::protocol::internal::raw::TransferResult_Success,
    ::flatbuffers::Offset<::flatbuffers::String> ip = 0,
    uint16_t port = 0) {
  TransferBuilder builder_(_fbb);
  builder_.add_ip(ip);
  builder_.add_port(port);
  builder_.add_code(code);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<Transfer> CreateTransferDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    fb::protocol::internal::raw::TransferResult code = fb::protocol::internal::raw::TransferResult_Success,
    const char *ip = nullptr,
    uint16_t port = 0) {
  auto ip__ = ip ? _fbb.CreateString(ip) : 0;
  return fb::protocol::internal::response::raw::CreateTransfer(
      _fbb,
      code,
      ip__,
      port);
}

inline const fb::protocol::internal::response::raw::Transfer *GetTransfer(const void *buf) {
  return ::flatbuffers::GetRoot<fb::protocol::internal::response::raw::Transfer>(buf);
}

inline const fb::protocol::internal::response::raw::Transfer *GetSizePrefixedTransfer(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<fb::protocol::internal::response::raw::Transfer>(buf);
}

inline bool VerifyTransferBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<fb::protocol::internal::response::raw::Transfer>(nullptr);
}

inline bool VerifySizePrefixedTransferBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<fb::protocol::internal::response::raw::Transfer>(nullptr);
}

inline void FinishTransferBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::internal::response::raw::Transfer> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedTransferBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::internal::response::raw::Transfer> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace raw
}  // namespace response
}  // namespace internal
}  // namespace protocol
}  // namespace fb

#endif  // FLATBUFFERS_GENERATED_FBPROTOCOLINTERNALRESPONSETRANSFER_FB_PROTOCOL_INTERNAL_RESPONSE_RAW_H_