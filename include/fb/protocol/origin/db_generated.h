// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_DB_FB_PROTOCOL_DB_ORIGIN_H_
#define FLATBUFFERS_GENERATED_DB_FB_PROTOCOL_DB_ORIGIN_H_

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
namespace origin {

struct nullable_uint;
struct nullable_uintBuilder;

struct nullable_uint FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef nullable_uintBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VALUE = 4
  };
  uint32_t value() const {
    return GetField<uint32_t>(VT_VALUE, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_VALUE, 4) &&
           verifier.EndTable();
  }
};

struct nullable_uintBuilder {
  typedef nullable_uint Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_value(uint32_t value) {
    fbb_.AddElement<uint32_t>(nullable_uint::VT_VALUE, value, 0);
  }
  explicit nullable_uintBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<nullable_uint> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<nullable_uint>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<nullable_uint> Createnullable_uint(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t value = 0) {
  nullable_uintBuilder builder_(_fbb);
  builder_.add_value(value);
  return builder_.Finish();
}

}  // namespace origin
}  // namespace db
}  // namespace protocol
}  // namespace fb

#endif  // FLATBUFFERS_GENERATED_DB_FB_PROTOCOL_DB_ORIGIN_H_
