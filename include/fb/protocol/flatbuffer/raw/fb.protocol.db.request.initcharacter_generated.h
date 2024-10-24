// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_FBPROTOCOLDBREQUESTINITCHARACTER_FB_PROTOCOL_DB_REQUEST_RAW_H_
#define FLATBUFFERS_GENERATED_FBPROTOCOLDBREQUESTINITCHARACTER_FB_PROTOCOL_DB_REQUEST_RAW_H_

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
namespace raw {

struct InitCharacter;
struct InitCharacterBuilder;

struct InitCharacter FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef InitCharacterBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_UID = 4,
    VT_NAME = 6,
    VT_PW = 8,
    VT_HP = 10,
    VT_MP = 12,
    VT_MAP = 14,
    VT_X = 16,
    VT_Y = 18,
    VT_ADMIN = 20
  };
  uint32_t uid() const {
    return GetField<uint32_t>(VT_UID, 0);
  }
  const ::flatbuffers::String *name() const {
    return GetPointer<const ::flatbuffers::String *>(VT_NAME);
  }
  const ::flatbuffers::String *pw() const {
    return GetPointer<const ::flatbuffers::String *>(VT_PW);
  }
  uint32_t hp() const {
    return GetField<uint32_t>(VT_HP, 0);
  }
  uint32_t mp() const {
    return GetField<uint32_t>(VT_MP, 0);
  }
  uint16_t map() const {
    return GetField<uint16_t>(VT_MAP, 0);
  }
  uint16_t x() const {
    return GetField<uint16_t>(VT_X, 0);
  }
  uint16_t y() const {
    return GetField<uint16_t>(VT_Y, 0);
  }
  bool admin() const {
    return GetField<uint8_t>(VT_ADMIN, 0) != 0;
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_UID, 4) &&
           VerifyOffset(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyOffset(verifier, VT_PW) &&
           verifier.VerifyString(pw()) &&
           VerifyField<uint32_t>(verifier, VT_HP, 4) &&
           VerifyField<uint32_t>(verifier, VT_MP, 4) &&
           VerifyField<uint16_t>(verifier, VT_MAP, 2) &&
           VerifyField<uint16_t>(verifier, VT_X, 2) &&
           VerifyField<uint16_t>(verifier, VT_Y, 2) &&
           VerifyField<uint8_t>(verifier, VT_ADMIN, 1) &&
           verifier.EndTable();
  }
};

struct InitCharacterBuilder {
  typedef InitCharacter Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_uid(uint32_t uid) {
    fbb_.AddElement<uint32_t>(InitCharacter::VT_UID, uid, 0);
  }
  void add_name(::flatbuffers::Offset<::flatbuffers::String> name) {
    fbb_.AddOffset(InitCharacter::VT_NAME, name);
  }
  void add_pw(::flatbuffers::Offset<::flatbuffers::String> pw) {
    fbb_.AddOffset(InitCharacter::VT_PW, pw);
  }
  void add_hp(uint32_t hp) {
    fbb_.AddElement<uint32_t>(InitCharacter::VT_HP, hp, 0);
  }
  void add_mp(uint32_t mp) {
    fbb_.AddElement<uint32_t>(InitCharacter::VT_MP, mp, 0);
  }
  void add_map(uint16_t map) {
    fbb_.AddElement<uint16_t>(InitCharacter::VT_MAP, map, 0);
  }
  void add_x(uint16_t x) {
    fbb_.AddElement<uint16_t>(InitCharacter::VT_X, x, 0);
  }
  void add_y(uint16_t y) {
    fbb_.AddElement<uint16_t>(InitCharacter::VT_Y, y, 0);
  }
  void add_admin(bool admin) {
    fbb_.AddElement<uint8_t>(InitCharacter::VT_ADMIN, static_cast<uint8_t>(admin), 0);
  }
  explicit InitCharacterBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<InitCharacter> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<InitCharacter>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<InitCharacter> CreateInitCharacter(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t uid = 0,
    ::flatbuffers::Offset<::flatbuffers::String> name = 0,
    ::flatbuffers::Offset<::flatbuffers::String> pw = 0,
    uint32_t hp = 0,
    uint32_t mp = 0,
    uint16_t map = 0,
    uint16_t x = 0,
    uint16_t y = 0,
    bool admin = false) {
  InitCharacterBuilder builder_(_fbb);
  builder_.add_mp(mp);
  builder_.add_hp(hp);
  builder_.add_pw(pw);
  builder_.add_name(name);
  builder_.add_uid(uid);
  builder_.add_y(y);
  builder_.add_x(x);
  builder_.add_map(map);
  builder_.add_admin(admin);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<InitCharacter> CreateInitCharacterDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t uid = 0,
    const char *name = nullptr,
    const char *pw = nullptr,
    uint32_t hp = 0,
    uint32_t mp = 0,
    uint16_t map = 0,
    uint16_t x = 0,
    uint16_t y = 0,
    bool admin = false) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  auto pw__ = pw ? _fbb.CreateString(pw) : 0;
  return fb::protocol::db::request::raw::CreateInitCharacter(
      _fbb,
      uid,
      name__,
      pw__,
      hp,
      mp,
      map,
      x,
      y,
      admin);
}

inline const fb::protocol::db::request::raw::InitCharacter *GetInitCharacter(const void *buf) {
  return ::flatbuffers::GetRoot<fb::protocol::db::request::raw::InitCharacter>(buf);
}

inline const fb::protocol::db::request::raw::InitCharacter *GetSizePrefixedInitCharacter(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<fb::protocol::db::request::raw::InitCharacter>(buf);
}

inline bool VerifyInitCharacterBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<fb::protocol::db::request::raw::InitCharacter>(nullptr);
}

inline bool VerifySizePrefixedInitCharacterBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<fb::protocol::db::request::raw::InitCharacter>(nullptr);
}

inline void FinishInitCharacterBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::db::request::raw::InitCharacter> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedInitCharacterBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fb::protocol::db::request::raw::InitCharacter> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace raw
}  // namespace request
}  // namespace db
}  // namespace protocol
}  // namespace fb

#endif  // FLATBUFFERS_GENERATED_FBPROTOCOLDBREQUESTINITCHARACTER_FB_PROTOCOL_DB_REQUEST_RAW_H_
