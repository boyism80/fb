// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_FBPROTOCOLDBITEM_FB_PROTOCOL_DB_RAW_H_
#define FLATBUFFERS_GENERATED_FBPROTOCOLDBITEM_FB_PROTOCOL_DB_RAW_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

#include "nullable_uint_generated.h"

namespace fb {
namespace protocol {
namespace db {
namespace raw {

struct Item;
struct ItemBuilder;

struct Item FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ItemBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_USER = 4,
    VT_INDEX = 6,
    VT_PARTS = 8,
    VT_DEPOSITED = 10,
    VT_MODEL = 12,
    VT_COUNT = 14,
    VT_DURABILITY = 16,
    VT_CUSTOM_NAME = 18
  };
  uint32_t user() const {
    return GetField<uint32_t>(VT_USER, 0);
  }
  int16_t index() const {
    return GetField<int16_t>(VT_INDEX, 0);
  }
  int16_t parts() const {
    return GetField<int16_t>(VT_PARTS, 0);
  }
  int16_t deposited() const {
    return GetField<int16_t>(VT_DEPOSITED, 0);
  }
  uint32_t model() const {
    return GetField<uint32_t>(VT_MODEL, 0);
  }
  uint16_t count() const {
    return GetField<uint16_t>(VT_COUNT, 0);
  }
  const nullable::nullable_uint *durability() const {
    return GetPointer<const nullable::nullable_uint *>(VT_DURABILITY);
  }
  const ::flatbuffers::String *custom_name() const {
    return GetPointer<const ::flatbuffers::String *>(VT_CUSTOM_NAME);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_USER, 4) &&
           VerifyField<int16_t>(verifier, VT_INDEX, 2) &&
           VerifyField<int16_t>(verifier, VT_PARTS, 2) &&
           VerifyField<int16_t>(verifier, VT_DEPOSITED, 2) &&
           VerifyField<uint32_t>(verifier, VT_MODEL, 4) &&
           VerifyField<uint16_t>(verifier, VT_COUNT, 2) &&
           VerifyOffset(verifier, VT_DURABILITY) &&
           verifier.VerifyTable(durability()) &&
           VerifyOffset(verifier, VT_CUSTOM_NAME) &&
           verifier.VerifyString(custom_name()) &&
           verifier.EndTable();
  }
};

struct ItemBuilder {
  typedef Item Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_user(uint32_t user) {
    fbb_.AddElement<uint32_t>(Item::VT_USER, user, 0);
  }
  void add_index(int16_t index) {
    fbb_.AddElement<int16_t>(Item::VT_INDEX, index, 0);
  }
  void add_parts(int16_t parts) {
    fbb_.AddElement<int16_t>(Item::VT_PARTS, parts, 0);
  }
  void add_deposited(int16_t deposited) {
    fbb_.AddElement<int16_t>(Item::VT_DEPOSITED, deposited, 0);
  }
  void add_model(uint32_t model) {
    fbb_.AddElement<uint32_t>(Item::VT_MODEL, model, 0);
  }
  void add_count(uint16_t count) {
    fbb_.AddElement<uint16_t>(Item::VT_COUNT, count, 0);
  }
  void add_durability(::flatbuffers::Offset<nullable::nullable_uint> durability) {
    fbb_.AddOffset(Item::VT_DURABILITY, durability);
  }
  void add_custom_name(::flatbuffers::Offset<::flatbuffers::String> custom_name) {
    fbb_.AddOffset(Item::VT_CUSTOM_NAME, custom_name);
  }
  explicit ItemBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Item> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Item>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Item> CreateItem(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t user = 0,
    int16_t index = 0,
    int16_t parts = 0,
    int16_t deposited = 0,
    uint32_t model = 0,
    uint16_t count = 0,
    ::flatbuffers::Offset<nullable::nullable_uint> durability = 0,
    ::flatbuffers::Offset<::flatbuffers::String> custom_name = 0) {
  ItemBuilder builder_(_fbb);
  builder_.add_custom_name(custom_name);
  builder_.add_durability(durability);
  builder_.add_model(model);
  builder_.add_user(user);
  builder_.add_count(count);
  builder_.add_deposited(deposited);
  builder_.add_parts(parts);
  builder_.add_index(index);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<Item> CreateItemDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t user = 0,
    int16_t index = 0,
    int16_t parts = 0,
    int16_t deposited = 0,
    uint32_t model = 0,
    uint16_t count = 0,
    ::flatbuffers::Offset<nullable::nullable_uint> durability = 0,
    const char *custom_name = nullptr) {
  auto custom_name__ = custom_name ? _fbb.CreateString(custom_name) : 0;
  return fb::protocol::db::raw::CreateItem(
      _fbb,
      user,
      index,
      parts,
      deposited,
      model,
      count,
      durability,
      custom_name__);
}

inline const fb::protocol::db::raw::Item *GetItem(const void *buf) {
  return ::flatbuffers::GetRoot<fb::protocol::db::raw::Item>(buf);
}

inline const fb::protocol::db::raw::Item *GetSizePrefixedItem(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<fb::protocol::db::raw::Item>(buf);
}

inline bool VerifyItemBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<fb::protocol::db::raw::Item>(nullptr);
}

inline bool VerifySizePrefixedItemBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<fb::protocol::db::raw::Item>(nullptr);
}

inline void FinishItemBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::db::raw::Item> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedItemBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::db::raw::Item> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace raw
}  // namespace db
}  // namespace protocol
}  // namespace fb

#endif  // FLATBUFFERS_GENERATED_FBPROTOCOLDBITEM_FB_PROTOCOL_DB_RAW_H_
