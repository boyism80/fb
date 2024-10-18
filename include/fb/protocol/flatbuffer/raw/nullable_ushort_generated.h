// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_NULLABLEUSHORT_NULLABLE_H_
#define FLATBUFFERS_GENERATED_NULLABLEUSHORT_NULLABLE_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

namespace nullable {

struct nullable_ushort;
struct nullable_ushortBuilder;

struct nullable_ushort FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef nullable_ushortBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VALUE = 4
  };
  uint16_t value() const {
    return GetField<uint16_t>(VT_VALUE, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint16_t>(verifier, VT_VALUE, 2) &&
           verifier.EndTable();
  }
};

struct nullable_ushortBuilder {
  typedef nullable_ushort Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_value(uint16_t value) {
    fbb_.AddElement<uint16_t>(nullable_ushort::VT_VALUE, value, 0);
  }
  explicit nullable_ushortBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<nullable_ushort> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<nullable_ushort>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<nullable_ushort> Createnullable_ushort(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint16_t value = 0) {
  nullable_ushortBuilder builder_(_fbb);
  builder_.add_value(value);
  return builder_.Finish();
}

}  // namespace nullable

#endif  // FLATBUFFERS_GENERATED_NULLABLEUSHORT_NULLABLE_H_