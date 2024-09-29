// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_INTERNALRESPONSEWHISPER_FB_PROTOCOL_INTERNAL_RESPONSE_ORIGIN_H_
#define FLATBUFFERS_GENERATED_INTERNALRESPONSEWHISPER_FB_PROTOCOL_INTERNAL_RESPONSE_ORIGIN_H_

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
namespace response {
namespace origin {

struct Whisper;
struct WhisperBuilder;

struct Whisper FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef WhisperBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SUCCESS = 4,
    VT_FROM = 6,
    VT_TO = 8,
    VT_MESSAGE = 10
  };
  bool success() const {
    return GetField<uint8_t>(VT_SUCCESS, 0) != 0;
  }
  const ::flatbuffers::String *from() const {
    return GetPointer<const ::flatbuffers::String *>(VT_FROM);
  }
  uint32_t to() const {
    return GetField<uint32_t>(VT_TO, 0);
  }
  const ::flatbuffers::String *message() const {
    return GetPointer<const ::flatbuffers::String *>(VT_MESSAGE);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_SUCCESS, 1) &&
           VerifyOffset(verifier, VT_FROM) &&
           verifier.VerifyString(from()) &&
           VerifyField<uint32_t>(verifier, VT_TO, 4) &&
           VerifyOffset(verifier, VT_MESSAGE) &&
           verifier.VerifyString(message()) &&
           verifier.EndTable();
  }
};

struct WhisperBuilder {
  typedef Whisper Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_success(bool success) {
    fbb_.AddElement<uint8_t>(Whisper::VT_SUCCESS, static_cast<uint8_t>(success), 0);
  }
  void add_from(::flatbuffers::Offset<::flatbuffers::String> from) {
    fbb_.AddOffset(Whisper::VT_FROM, from);
  }
  void add_to(uint32_t to) {
    fbb_.AddElement<uint32_t>(Whisper::VT_TO, to, 0);
  }
  void add_message(::flatbuffers::Offset<::flatbuffers::String> message) {
    fbb_.AddOffset(Whisper::VT_MESSAGE, message);
  }
  explicit WhisperBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Whisper> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Whisper>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Whisper> CreateWhisper(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    bool success = false,
    ::flatbuffers::Offset<::flatbuffers::String> from = 0,
    uint32_t to = 0,
    ::flatbuffers::Offset<::flatbuffers::String> message = 0) {
  WhisperBuilder builder_(_fbb);
  builder_.add_message(message);
  builder_.add_to(to);
  builder_.add_from(from);
  builder_.add_success(success);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<Whisper> CreateWhisperDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    bool success = false,
    const char *from = nullptr,
    uint32_t to = 0,
    const char *message = nullptr) {
  auto from__ = from ? _fbb.CreateString(from) : 0;
  auto message__ = message ? _fbb.CreateString(message) : 0;
  return fb::protocol::internal::response::origin::CreateWhisper(
      _fbb,
      success,
      from__,
      to,
      message__);
}

inline const fb::protocol::internal::response::origin::Whisper *GetWhisper(const void *buf) {
  return ::flatbuffers::GetRoot<fb::protocol::internal::response::origin::Whisper>(buf);
}

inline const fb::protocol::internal::response::origin::Whisper *GetSizePrefixedWhisper(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<fb::protocol::internal::response::origin::Whisper>(buf);
}

inline bool VerifyWhisperBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<fb::protocol::internal::response::origin::Whisper>(nullptr);
}

inline bool VerifySizePrefixedWhisperBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<fb::protocol::internal::response::origin::Whisper>(nullptr);
}

inline void FinishWhisperBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::internal::response::origin::Whisper> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedWhisperBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::internal::response::origin::Whisper> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace origin
}  // namespace response
}  // namespace internal
}  // namespace protocol
}  // namespace fb

#endif  // FLATBUFFERS_GENERATED_INTERNALRESPONSEWHISPER_FB_PROTOCOL_INTERNAL_RESPONSE_ORIGIN_H_
