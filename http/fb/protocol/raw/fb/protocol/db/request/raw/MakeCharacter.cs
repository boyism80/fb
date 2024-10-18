// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace fb.protocol.db.request.raw
{

using global::System;
using global::System.Collections.Generic;
using global::Google.FlatBuffers;

public struct MakeCharacter : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static void ValidateVersion() { FlatBufferConstants.FLATBUFFERS_24_3_25(); }
  public static MakeCharacter GetRootAsMakeCharacter(ByteBuffer _bb) { return GetRootAsMakeCharacter(_bb, new MakeCharacter()); }
  public static MakeCharacter GetRootAsMakeCharacter(ByteBuffer _bb, MakeCharacter obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public static bool VerifyMakeCharacter(ByteBuffer _bb) {Google.FlatBuffers.Verifier verifier = new Google.FlatBuffers.Verifier(_bb); return verifier.VerifyBuffer("", false, MakeCharacterVerify.Verify); }
  public void __init(int _i, ByteBuffer _bb) { __p = new Table(_i, _bb); }
  public MakeCharacter __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public uint Uid { get { int o = __p.__offset(4); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }
  public ushort Hair { get { int o = __p.__offset(6); return o != 0 ? __p.bb.GetUshort(o + __p.bb_pos) : (ushort)0; } }
  public byte Sex { get { int o = __p.__offset(8); return o != 0 ? __p.bb.Get(o + __p.bb_pos) : (byte)0; } }
  public byte Nation { get { int o = __p.__offset(10); return o != 0 ? __p.bb.Get(o + __p.bb_pos) : (byte)0; } }
  public byte Creature { get { int o = __p.__offset(12); return o != 0 ? __p.bb.Get(o + __p.bb_pos) : (byte)0; } }

  public static Offset<fb.protocol.db.request.raw.MakeCharacter> CreateMakeCharacter(FlatBufferBuilder builder,
      uint uid = 0,
      ushort hair = 0,
      byte sex = 0,
      byte nation = 0,
      byte creature = 0) {
    builder.StartTable(5);
    MakeCharacter.AddUid(builder, uid);
    MakeCharacter.AddHair(builder, hair);
    MakeCharacter.AddCreature(builder, creature);
    MakeCharacter.AddNation(builder, nation);
    MakeCharacter.AddSex(builder, sex);
    return MakeCharacter.EndMakeCharacter(builder);
  }

  public static void StartMakeCharacter(FlatBufferBuilder builder) { builder.StartTable(5); }
  public static void AddUid(FlatBufferBuilder builder, uint uid) { builder.AddUint(0, uid, 0); }
  public static void AddHair(FlatBufferBuilder builder, ushort hair) { builder.AddUshort(1, hair, 0); }
  public static void AddSex(FlatBufferBuilder builder, byte sex) { builder.AddByte(2, sex, 0); }
  public static void AddNation(FlatBufferBuilder builder, byte nation) { builder.AddByte(3, nation, 0); }
  public static void AddCreature(FlatBufferBuilder builder, byte creature) { builder.AddByte(4, creature, 0); }
  public static Offset<fb.protocol.db.request.raw.MakeCharacter> EndMakeCharacter(FlatBufferBuilder builder) {
    int o = builder.EndTable();
    return new Offset<fb.protocol.db.request.raw.MakeCharacter>(o);
  }
  public static void FinishMakeCharacterBuffer(FlatBufferBuilder builder, Offset<fb.protocol.db.request.raw.MakeCharacter> offset) { builder.Finish(offset.Value); }
  public static void FinishSizePrefixedMakeCharacterBuffer(FlatBufferBuilder builder, Offset<fb.protocol.db.request.raw.MakeCharacter> offset) { builder.FinishSizePrefixed(offset.Value); }
}


static public class MakeCharacterVerify
{
  static public bool Verify(Google.FlatBuffers.Verifier verifier, uint tablePos)
  {
    return verifier.VerifyTableStart(tablePos)
      && verifier.VerifyField(tablePos, 4 /*Uid*/, 4 /*uint*/, 4, false)
      && verifier.VerifyField(tablePos, 6 /*Hair*/, 2 /*ushort*/, 2, false)
      && verifier.VerifyField(tablePos, 8 /*Sex*/, 1 /*byte*/, 1, false)
      && verifier.VerifyField(tablePos, 10 /*Nation*/, 1 /*byte*/, 1, false)
      && verifier.VerifyField(tablePos, 12 /*Creature*/, 1 /*byte*/, 1, false)
      && verifier.VerifyTableEnd(tablePos);
  }
}

}