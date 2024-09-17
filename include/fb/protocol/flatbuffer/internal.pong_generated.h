// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_INTERNALPONG_FB_GAME_FLATBUFFER_INTER_H_
#define FLATBUFFERS_GENERATED_INTERNALPONG_FB_GAME_FLATBUFFER_INTER_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

namespace fb {
namespace game {
namespace flatbuffer {
namespace inter {

struct Pong;
struct PongBuilder;

struct Pong FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef PongBuilder Builder;
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           verifier.EndTable();
  }
};

struct PongBuilder {
  typedef Pong Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  explicit PongBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Pong> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Pong>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Pong> CreatePong(
    ::flatbuffers::FlatBufferBuilder &_fbb) {
  PongBuilder builder_(_fbb);
  return builder_.Finish();
}

inline const fb::game::flatbuffer::inter::Pong *GetPong(const void *buf) {
  return ::flatbuffers::GetRoot<fb::game::flatbuffer::inter::Pong>(buf);
}

inline const fb::game::flatbuffer::inter::Pong *GetSizePrefixedPong(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<fb::game::flatbuffer::inter::Pong>(buf);
}

inline bool VerifyPongBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<fb::game::flatbuffer::inter::Pong>(nullptr);
}

inline bool VerifySizePrefixedPongBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<fb::game::flatbuffer::inter::Pong>(nullptr);
}

inline void FinishPongBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::game::flatbuffer::inter::Pong> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedPongBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::game::flatbuffer::inter::Pong> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace inter
}  // namespace flatbuffer
}  // namespace game
}  // namespace fb

#endif  // FLATBUFFERS_GENERATED_INTERNALPONG_FB_GAME_FLATBUFFER_INTER_H_
