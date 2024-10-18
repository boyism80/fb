// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace fb.protocol.db.raw
{

using global::System;
using global::System.Collections.Generic;
using global::Google.FlatBuffers;

public struct Character : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static void ValidateVersion() { FlatBufferConstants.FLATBUFFERS_24_3_25(); }
  public static Character GetRootAsCharacter(ByteBuffer _bb) { return GetRootAsCharacter(_bb, new Character()); }
  public static Character GetRootAsCharacter(ByteBuffer _bb, Character obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public static bool VerifyCharacter(ByteBuffer _bb) {Google.FlatBuffers.Verifier verifier = new Google.FlatBuffers.Verifier(_bb); return verifier.VerifyBuffer("", false, CharacterVerify.Verify); }
  public void __init(int _i, ByteBuffer _bb) { __p = new Table(_i, _bb); }
  public Character __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public uint Id { get { int o = __p.__offset(4); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }
  public string Name { get { int o = __p.__offset(6); return o != 0 ? __p.__string(o + __p.bb_pos) : null; } }
#if ENABLE_SPAN_T
  public Span<byte> GetNameBytes() { return __p.__vector_as_span<byte>(6, 1); }
#else
  public ArraySegment<byte>? GetNameBytes() { return __p.__vector_as_arraysegment(6); }
#endif
  public byte[] GetNameArray() { return __p.__vector_as_array<byte>(6); }
  public string LastLogin { get { int o = __p.__offset(8); return o != 0 ? __p.__string(o + __p.bb_pos) : null; } }
#if ENABLE_SPAN_T
  public Span<byte> GetLastLoginBytes() { return __p.__vector_as_span<byte>(8, 1); }
#else
  public ArraySegment<byte>? GetLastLoginBytes() { return __p.__vector_as_arraysegment(8); }
#endif
  public byte[] GetLastLoginArray() { return __p.__vector_as_array<byte>(8); }
  public bool Admin { get { int o = __p.__offset(10); return o != 0 ? 0!=__p.bb.Get(o + __p.bb_pos) : (bool)false; } }
  public ushort Look { get { int o = __p.__offset(12); return o != 0 ? __p.bb.GetUshort(o + __p.bb_pos) : (ushort)0; } }
  public ushort Color { get { int o = __p.__offset(14); return o != 0 ? __p.bb.GetUshort(o + __p.bb_pos) : (ushort)0; } }
  public ushort Sex { get { int o = __p.__offset(16); return o != 0 ? __p.bb.GetUshort(o + __p.bb_pos) : (ushort)0; } }
  public ushort Nation { get { int o = __p.__offset(18); return o != 0 ? __p.bb.GetUshort(o + __p.bb_pos) : (ushort)0; } }
  public nullable.nullable_ushort? Creature { get { int o = __p.__offset(20); return o != 0 ? (nullable.nullable_ushort?)(new nullable.nullable_ushort()).__assign(__p.__indirect(o + __p.bb_pos), __p.bb) : null; } }
  public uint Map { get { int o = __p.__offset(22); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }
  public fb.protocol.db.raw.Position? Position { get { int o = __p.__offset(24); return o != 0 ? (fb.protocol.db.raw.Position?)(new fb.protocol.db.raw.Position()).__assign(__p.__indirect(o + __p.bb_pos), __p.bb) : null; } }
  public byte Direction { get { int o = __p.__offset(26); return o != 0 ? __p.bb.Get(o + __p.bb_pos) : (byte)0; } }
  public byte State { get { int o = __p.__offset(28); return o != 0 ? __p.bb.Get(o + __p.bb_pos) : (byte)0; } }
  public byte ClassType { get { int o = __p.__offset(30); return o != 0 ? __p.bb.Get(o + __p.bb_pos) : (byte)0; } }
  public byte Promotion { get { int o = __p.__offset(32); return o != 0 ? __p.bb.Get(o + __p.bb_pos) : (byte)0; } }
  public uint Exp { get { int o = __p.__offset(34); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }
  public uint Money { get { int o = __p.__offset(36); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }
  public uint DepositedMoney { get { int o = __p.__offset(38); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }
  public nullable.nullable_ushort? Disguise { get { int o = __p.__offset(40); return o != 0 ? (nullable.nullable_ushort?)(new nullable.nullable_ushort()).__assign(__p.__indirect(o + __p.bb_pos), __p.bb) : null; } }
  public uint Hp { get { int o = __p.__offset(42); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }
  public uint BaseHp { get { int o = __p.__offset(44); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }
  public uint AdditionalHp { get { int o = __p.__offset(46); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }
  public uint Mp { get { int o = __p.__offset(48); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }
  public uint BaseMp { get { int o = __p.__offset(50); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }
  public uint AdditionalMp { get { int o = __p.__offset(52); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }
  public nullable.nullable_ubyte? WeaponColor { get { int o = __p.__offset(54); return o != 0 ? (nullable.nullable_ubyte?)(new nullable.nullable_ubyte()).__assign(__p.__indirect(o + __p.bb_pos), __p.bb) : null; } }
  public nullable.nullable_ubyte? HelmetColor { get { int o = __p.__offset(56); return o != 0 ? (nullable.nullable_ubyte?)(new nullable.nullable_ubyte()).__assign(__p.__indirect(o + __p.bb_pos), __p.bb) : null; } }
  public nullable.nullable_ubyte? ArmorColor { get { int o = __p.__offset(58); return o != 0 ? (nullable.nullable_ubyte?)(new nullable.nullable_ubyte()).__assign(__p.__indirect(o + __p.bb_pos), __p.bb) : null; } }
  public nullable.nullable_ubyte? ShieldColor { get { int o = __p.__offset(60); return o != 0 ? (nullable.nullable_ubyte?)(new nullable.nullable_ubyte()).__assign(__p.__indirect(o + __p.bb_pos), __p.bb) : null; } }
  public nullable.nullable_ubyte? RingLeftColor { get { int o = __p.__offset(62); return o != 0 ? (nullable.nullable_ubyte?)(new nullable.nullable_ubyte()).__assign(__p.__indirect(o + __p.bb_pos), __p.bb) : null; } }
  public nullable.nullable_ubyte? RingRightColor { get { int o = __p.__offset(64); return o != 0 ? (nullable.nullable_ubyte?)(new nullable.nullable_ubyte()).__assign(__p.__indirect(o + __p.bb_pos), __p.bb) : null; } }
  public nullable.nullable_ubyte? AuxTopColor { get { int o = __p.__offset(66); return o != 0 ? (nullable.nullable_ubyte?)(new nullable.nullable_ubyte()).__assign(__p.__indirect(o + __p.bb_pos), __p.bb) : null; } }
  public nullable.nullable_ubyte? AuxBotColor { get { int o = __p.__offset(68); return o != 0 ? (nullable.nullable_ubyte?)(new nullable.nullable_ubyte()).__assign(__p.__indirect(o + __p.bb_pos), __p.bb) : null; } }
  public nullable.nullable_uint? Clan { get { int o = __p.__offset(70); return o != 0 ? (nullable.nullable_uint?)(new nullable.nullable_uint()).__assign(__p.__indirect(o + __p.bb_pos), __p.bb) : null; } }

  public static Offset<fb.protocol.db.raw.Character> CreateCharacter(FlatBufferBuilder builder,
      uint id = 0,
      StringOffset nameOffset = default(StringOffset),
      StringOffset last_loginOffset = default(StringOffset),
      bool admin = false,
      ushort look = 0,
      ushort color = 0,
      ushort sex = 0,
      ushort nation = 0,
      Offset<nullable.nullable_ushort> creatureOffset = default(Offset<nullable.nullable_ushort>),
      uint map = 0,
      Offset<fb.protocol.db.raw.Position> positionOffset = default(Offset<fb.protocol.db.raw.Position>),
      byte direction = 0,
      byte state = 0,
      byte class_type = 0,
      byte promotion = 0,
      uint exp = 0,
      uint money = 0,
      uint deposited_money = 0,
      Offset<nullable.nullable_ushort> disguiseOffset = default(Offset<nullable.nullable_ushort>),
      uint hp = 0,
      uint base_hp = 0,
      uint additional_hp = 0,
      uint mp = 0,
      uint base_mp = 0,
      uint additional_mp = 0,
      Offset<nullable.nullable_ubyte> weapon_colorOffset = default(Offset<nullable.nullable_ubyte>),
      Offset<nullable.nullable_ubyte> helmet_colorOffset = default(Offset<nullable.nullable_ubyte>),
      Offset<nullable.nullable_ubyte> armor_colorOffset = default(Offset<nullable.nullable_ubyte>),
      Offset<nullable.nullable_ubyte> shield_colorOffset = default(Offset<nullable.nullable_ubyte>),
      Offset<nullable.nullable_ubyte> ring_left_colorOffset = default(Offset<nullable.nullable_ubyte>),
      Offset<nullable.nullable_ubyte> ring_right_colorOffset = default(Offset<nullable.nullable_ubyte>),
      Offset<nullable.nullable_ubyte> aux_top_colorOffset = default(Offset<nullable.nullable_ubyte>),
      Offset<nullable.nullable_ubyte> aux_bot_colorOffset = default(Offset<nullable.nullable_ubyte>),
      Offset<nullable.nullable_uint> clanOffset = default(Offset<nullable.nullable_uint>)) {
    builder.StartTable(34);
    Character.AddClan(builder, clanOffset);
    Character.AddAuxBotColor(builder, aux_bot_colorOffset);
    Character.AddAuxTopColor(builder, aux_top_colorOffset);
    Character.AddRingRightColor(builder, ring_right_colorOffset);
    Character.AddRingLeftColor(builder, ring_left_colorOffset);
    Character.AddShieldColor(builder, shield_colorOffset);
    Character.AddArmorColor(builder, armor_colorOffset);
    Character.AddHelmetColor(builder, helmet_colorOffset);
    Character.AddWeaponColor(builder, weapon_colorOffset);
    Character.AddAdditionalMp(builder, additional_mp);
    Character.AddBaseMp(builder, base_mp);
    Character.AddMp(builder, mp);
    Character.AddAdditionalHp(builder, additional_hp);
    Character.AddBaseHp(builder, base_hp);
    Character.AddHp(builder, hp);
    Character.AddDisguise(builder, disguiseOffset);
    Character.AddDepositedMoney(builder, deposited_money);
    Character.AddMoney(builder, money);
    Character.AddExp(builder, exp);
    Character.AddPosition(builder, positionOffset);
    Character.AddMap(builder, map);
    Character.AddCreature(builder, creatureOffset);
    Character.AddLastLogin(builder, last_loginOffset);
    Character.AddName(builder, nameOffset);
    Character.AddId(builder, id);
    Character.AddNation(builder, nation);
    Character.AddSex(builder, sex);
    Character.AddColor(builder, color);
    Character.AddLook(builder, look);
    Character.AddPromotion(builder, promotion);
    Character.AddClassType(builder, class_type);
    Character.AddState(builder, state);
    Character.AddDirection(builder, direction);
    Character.AddAdmin(builder, admin);
    return Character.EndCharacter(builder);
  }

  public static void StartCharacter(FlatBufferBuilder builder) { builder.StartTable(34); }
  public static void AddId(FlatBufferBuilder builder, uint id) { builder.AddUint(0, id, 0); }
  public static void AddName(FlatBufferBuilder builder, StringOffset nameOffset) { builder.AddOffset(1, nameOffset.Value, 0); }
  public static void AddLastLogin(FlatBufferBuilder builder, StringOffset lastLoginOffset) { builder.AddOffset(2, lastLoginOffset.Value, 0); }
  public static void AddAdmin(FlatBufferBuilder builder, bool admin) { builder.AddBool(3, admin, false); }
  public static void AddLook(FlatBufferBuilder builder, ushort look) { builder.AddUshort(4, look, 0); }
  public static void AddColor(FlatBufferBuilder builder, ushort color) { builder.AddUshort(5, color, 0); }
  public static void AddSex(FlatBufferBuilder builder, ushort sex) { builder.AddUshort(6, sex, 0); }
  public static void AddNation(FlatBufferBuilder builder, ushort nation) { builder.AddUshort(7, nation, 0); }
  public static void AddCreature(FlatBufferBuilder builder, Offset<nullable.nullable_ushort> creatureOffset) { builder.AddOffset(8, creatureOffset.Value, 0); }
  public static void AddMap(FlatBufferBuilder builder, uint map) { builder.AddUint(9, map, 0); }
  public static void AddPosition(FlatBufferBuilder builder, Offset<fb.protocol.db.raw.Position> positionOffset) { builder.AddOffset(10, positionOffset.Value, 0); }
  public static void AddDirection(FlatBufferBuilder builder, byte direction) { builder.AddByte(11, direction, 0); }
  public static void AddState(FlatBufferBuilder builder, byte state) { builder.AddByte(12, state, 0); }
  public static void AddClassType(FlatBufferBuilder builder, byte classType) { builder.AddByte(13, classType, 0); }
  public static void AddPromotion(FlatBufferBuilder builder, byte promotion) { builder.AddByte(14, promotion, 0); }
  public static void AddExp(FlatBufferBuilder builder, uint exp) { builder.AddUint(15, exp, 0); }
  public static void AddMoney(FlatBufferBuilder builder, uint money) { builder.AddUint(16, money, 0); }
  public static void AddDepositedMoney(FlatBufferBuilder builder, uint depositedMoney) { builder.AddUint(17, depositedMoney, 0); }
  public static void AddDisguise(FlatBufferBuilder builder, Offset<nullable.nullable_ushort> disguiseOffset) { builder.AddOffset(18, disguiseOffset.Value, 0); }
  public static void AddHp(FlatBufferBuilder builder, uint hp) { builder.AddUint(19, hp, 0); }
  public static void AddBaseHp(FlatBufferBuilder builder, uint baseHp) { builder.AddUint(20, baseHp, 0); }
  public static void AddAdditionalHp(FlatBufferBuilder builder, uint additionalHp) { builder.AddUint(21, additionalHp, 0); }
  public static void AddMp(FlatBufferBuilder builder, uint mp) { builder.AddUint(22, mp, 0); }
  public static void AddBaseMp(FlatBufferBuilder builder, uint baseMp) { builder.AddUint(23, baseMp, 0); }
  public static void AddAdditionalMp(FlatBufferBuilder builder, uint additionalMp) { builder.AddUint(24, additionalMp, 0); }
  public static void AddWeaponColor(FlatBufferBuilder builder, Offset<nullable.nullable_ubyte> weaponColorOffset) { builder.AddOffset(25, weaponColorOffset.Value, 0); }
  public static void AddHelmetColor(FlatBufferBuilder builder, Offset<nullable.nullable_ubyte> helmetColorOffset) { builder.AddOffset(26, helmetColorOffset.Value, 0); }
  public static void AddArmorColor(FlatBufferBuilder builder, Offset<nullable.nullable_ubyte> armorColorOffset) { builder.AddOffset(27, armorColorOffset.Value, 0); }
  public static void AddShieldColor(FlatBufferBuilder builder, Offset<nullable.nullable_ubyte> shieldColorOffset) { builder.AddOffset(28, shieldColorOffset.Value, 0); }
  public static void AddRingLeftColor(FlatBufferBuilder builder, Offset<nullable.nullable_ubyte> ringLeftColorOffset) { builder.AddOffset(29, ringLeftColorOffset.Value, 0); }
  public static void AddRingRightColor(FlatBufferBuilder builder, Offset<nullable.nullable_ubyte> ringRightColorOffset) { builder.AddOffset(30, ringRightColorOffset.Value, 0); }
  public static void AddAuxTopColor(FlatBufferBuilder builder, Offset<nullable.nullable_ubyte> auxTopColorOffset) { builder.AddOffset(31, auxTopColorOffset.Value, 0); }
  public static void AddAuxBotColor(FlatBufferBuilder builder, Offset<nullable.nullable_ubyte> auxBotColorOffset) { builder.AddOffset(32, auxBotColorOffset.Value, 0); }
  public static void AddClan(FlatBufferBuilder builder, Offset<nullable.nullable_uint> clanOffset) { builder.AddOffset(33, clanOffset.Value, 0); }
  public static Offset<fb.protocol.db.raw.Character> EndCharacter(FlatBufferBuilder builder) {
    int o = builder.EndTable();
    return new Offset<fb.protocol.db.raw.Character>(o);
  }
  public static void FinishCharacterBuffer(FlatBufferBuilder builder, Offset<fb.protocol.db.raw.Character> offset) { builder.Finish(offset.Value); }
  public static void FinishSizePrefixedCharacterBuffer(FlatBufferBuilder builder, Offset<fb.protocol.db.raw.Character> offset) { builder.FinishSizePrefixed(offset.Value); }
}


static public class CharacterVerify
{
  static public bool Verify(Google.FlatBuffers.Verifier verifier, uint tablePos)
  {
    return verifier.VerifyTableStart(tablePos)
      && verifier.VerifyField(tablePos, 4 /*Id*/, 4 /*uint*/, 4, false)
      && verifier.VerifyString(tablePos, 6 /*Name*/, false)
      && verifier.VerifyString(tablePos, 8 /*LastLogin*/, false)
      && verifier.VerifyField(tablePos, 10 /*Admin*/, 1 /*bool*/, 1, false)
      && verifier.VerifyField(tablePos, 12 /*Look*/, 2 /*ushort*/, 2, false)
      && verifier.VerifyField(tablePos, 14 /*Color*/, 2 /*ushort*/, 2, false)
      && verifier.VerifyField(tablePos, 16 /*Sex*/, 2 /*ushort*/, 2, false)
      && verifier.VerifyField(tablePos, 18 /*Nation*/, 2 /*ushort*/, 2, false)
      && verifier.VerifyTable(tablePos, 20 /*Creature*/, nullable.nullable_ushortVerify.Verify, false)
      && verifier.VerifyField(tablePos, 22 /*Map*/, 4 /*uint*/, 4, false)
      && verifier.VerifyTable(tablePos, 24 /*Position*/, fb.protocol.db.raw.PositionVerify.Verify, false)
      && verifier.VerifyField(tablePos, 26 /*Direction*/, 1 /*byte*/, 1, false)
      && verifier.VerifyField(tablePos, 28 /*State*/, 1 /*byte*/, 1, false)
      && verifier.VerifyField(tablePos, 30 /*ClassType*/, 1 /*byte*/, 1, false)
      && verifier.VerifyField(tablePos, 32 /*Promotion*/, 1 /*byte*/, 1, false)
      && verifier.VerifyField(tablePos, 34 /*Exp*/, 4 /*uint*/, 4, false)
      && verifier.VerifyField(tablePos, 36 /*Money*/, 4 /*uint*/, 4, false)
      && verifier.VerifyField(tablePos, 38 /*DepositedMoney*/, 4 /*uint*/, 4, false)
      && verifier.VerifyTable(tablePos, 40 /*Disguise*/, nullable.nullable_ushortVerify.Verify, false)
      && verifier.VerifyField(tablePos, 42 /*Hp*/, 4 /*uint*/, 4, false)
      && verifier.VerifyField(tablePos, 44 /*BaseHp*/, 4 /*uint*/, 4, false)
      && verifier.VerifyField(tablePos, 46 /*AdditionalHp*/, 4 /*uint*/, 4, false)
      && verifier.VerifyField(tablePos, 48 /*Mp*/, 4 /*uint*/, 4, false)
      && verifier.VerifyField(tablePos, 50 /*BaseMp*/, 4 /*uint*/, 4, false)
      && verifier.VerifyField(tablePos, 52 /*AdditionalMp*/, 4 /*uint*/, 4, false)
      && verifier.VerifyTable(tablePos, 54 /*WeaponColor*/, nullable.nullable_ubyteVerify.Verify, false)
      && verifier.VerifyTable(tablePos, 56 /*HelmetColor*/, nullable.nullable_ubyteVerify.Verify, false)
      && verifier.VerifyTable(tablePos, 58 /*ArmorColor*/, nullable.nullable_ubyteVerify.Verify, false)
      && verifier.VerifyTable(tablePos, 60 /*ShieldColor*/, nullable.nullable_ubyteVerify.Verify, false)
      && verifier.VerifyTable(tablePos, 62 /*RingLeftColor*/, nullable.nullable_ubyteVerify.Verify, false)
      && verifier.VerifyTable(tablePos, 64 /*RingRightColor*/, nullable.nullable_ubyteVerify.Verify, false)
      && verifier.VerifyTable(tablePos, 66 /*AuxTopColor*/, nullable.nullable_ubyteVerify.Verify, false)
      && verifier.VerifyTable(tablePos, 68 /*AuxBotColor*/, nullable.nullable_ubyteVerify.Verify, false)
      && verifier.VerifyTable(tablePos, 70 /*Clan*/, nullable.nullable_uintVerify.Verify, false)
      && verifier.VerifyTableEnd(tablePos);
  }
}

}