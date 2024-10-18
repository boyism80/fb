// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace fb.protocol.db.request.raw
{

using global::System;
using global::System.Collections.Generic;
using global::Google.FlatBuffers;

public struct Save : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static void ValidateVersion() { FlatBufferConstants.FLATBUFFERS_24_3_25(); }
  public static Save GetRootAsSave(ByteBuffer _bb) { return GetRootAsSave(_bb, new Save()); }
  public static Save GetRootAsSave(ByteBuffer _bb, Save obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public static bool VerifySave(ByteBuffer _bb) {Google.FlatBuffers.Verifier verifier = new Google.FlatBuffers.Verifier(_bb); return verifier.VerifyBuffer("", false, SaveVerify.Verify); }
  public void __init(int _i, ByteBuffer _bb) { __p = new Table(_i, _bb); }
  public Save __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public fb.protocol.db.raw.Character? Character { get { int o = __p.__offset(4); return o != 0 ? (fb.protocol.db.raw.Character?)(new fb.protocol.db.raw.Character()).__assign(__p.__indirect(o + __p.bb_pos), __p.bb) : null; } }
  public fb.protocol.db.raw.Item? Items(int j) { int o = __p.__offset(6); return o != 0 ? (fb.protocol.db.raw.Item?)(new fb.protocol.db.raw.Item()).__assign(__p.__indirect(__p.__vector(o) + j * 4), __p.bb) : null; }
  public int ItemsLength { get { int o = __p.__offset(6); return o != 0 ? __p.__vector_len(o) : 0; } }
  public fb.protocol.db.raw.Spell? Spells(int j) { int o = __p.__offset(8); return o != 0 ? (fb.protocol.db.raw.Spell?)(new fb.protocol.db.raw.Spell()).__assign(__p.__indirect(__p.__vector(o) + j * 4), __p.bb) : null; }
  public int SpellsLength { get { int o = __p.__offset(8); return o != 0 ? __p.__vector_len(o) : 0; } }

  public static Offset<fb.protocol.db.request.raw.Save> CreateSave(FlatBufferBuilder builder,
      Offset<fb.protocol.db.raw.Character> characterOffset = default(Offset<fb.protocol.db.raw.Character>),
      VectorOffset itemsOffset = default(VectorOffset),
      VectorOffset spellsOffset = default(VectorOffset)) {
    builder.StartTable(3);
    Save.AddSpells(builder, spellsOffset);
    Save.AddItems(builder, itemsOffset);
    Save.AddCharacter(builder, characterOffset);
    return Save.EndSave(builder);
  }

  public static void StartSave(FlatBufferBuilder builder) { builder.StartTable(3); }
  public static void AddCharacter(FlatBufferBuilder builder, Offset<fb.protocol.db.raw.Character> characterOffset) { builder.AddOffset(0, characterOffset.Value, 0); }
  public static void AddItems(FlatBufferBuilder builder, VectorOffset itemsOffset) { builder.AddOffset(1, itemsOffset.Value, 0); }
  public static VectorOffset CreateItemsVector(FlatBufferBuilder builder, Offset<fb.protocol.db.raw.Item>[] data) { builder.StartVector(4, data.Length, 4); for (int i = data.Length - 1; i >= 0; i--) builder.AddOffset(data[i].Value); return builder.EndVector(); }
  public static VectorOffset CreateItemsVectorBlock(FlatBufferBuilder builder, Offset<fb.protocol.db.raw.Item>[] data) { builder.StartVector(4, data.Length, 4); builder.Add(data); return builder.EndVector(); }
  public static VectorOffset CreateItemsVectorBlock(FlatBufferBuilder builder, ArraySegment<Offset<fb.protocol.db.raw.Item>> data) { builder.StartVector(4, data.Count, 4); builder.Add(data); return builder.EndVector(); }
  public static VectorOffset CreateItemsVectorBlock(FlatBufferBuilder builder, IntPtr dataPtr, int sizeInBytes) { builder.StartVector(1, sizeInBytes, 1); builder.Add<Offset<fb.protocol.db.raw.Item>>(dataPtr, sizeInBytes); return builder.EndVector(); }
  public static void StartItemsVector(FlatBufferBuilder builder, int numElems) { builder.StartVector(4, numElems, 4); }
  public static void AddSpells(FlatBufferBuilder builder, VectorOffset spellsOffset) { builder.AddOffset(2, spellsOffset.Value, 0); }
  public static VectorOffset CreateSpellsVector(FlatBufferBuilder builder, Offset<fb.protocol.db.raw.Spell>[] data) { builder.StartVector(4, data.Length, 4); for (int i = data.Length - 1; i >= 0; i--) builder.AddOffset(data[i].Value); return builder.EndVector(); }
  public static VectorOffset CreateSpellsVectorBlock(FlatBufferBuilder builder, Offset<fb.protocol.db.raw.Spell>[] data) { builder.StartVector(4, data.Length, 4); builder.Add(data); return builder.EndVector(); }
  public static VectorOffset CreateSpellsVectorBlock(FlatBufferBuilder builder, ArraySegment<Offset<fb.protocol.db.raw.Spell>> data) { builder.StartVector(4, data.Count, 4); builder.Add(data); return builder.EndVector(); }
  public static VectorOffset CreateSpellsVectorBlock(FlatBufferBuilder builder, IntPtr dataPtr, int sizeInBytes) { builder.StartVector(1, sizeInBytes, 1); builder.Add<Offset<fb.protocol.db.raw.Spell>>(dataPtr, sizeInBytes); return builder.EndVector(); }
  public static void StartSpellsVector(FlatBufferBuilder builder, int numElems) { builder.StartVector(4, numElems, 4); }
  public static Offset<fb.protocol.db.request.raw.Save> EndSave(FlatBufferBuilder builder) {
    int o = builder.EndTable();
    return new Offset<fb.protocol.db.request.raw.Save>(o);
  }
  public static void FinishSaveBuffer(FlatBufferBuilder builder, Offset<fb.protocol.db.request.raw.Save> offset) { builder.Finish(offset.Value); }
  public static void FinishSizePrefixedSaveBuffer(FlatBufferBuilder builder, Offset<fb.protocol.db.request.raw.Save> offset) { builder.FinishSizePrefixed(offset.Value); }
}


static public class SaveVerify
{
  static public bool Verify(Google.FlatBuffers.Verifier verifier, uint tablePos)
  {
    return verifier.VerifyTableStart(tablePos)
      && verifier.VerifyTable(tablePos, 4 /*Character*/, fb.protocol.db.raw.CharacterVerify.Verify, false)
      && verifier.VerifyVectorOfTables(tablePos, 6 /*Items*/, fb.protocol.db.raw.ItemVerify.Verify, false)
      && verifier.VerifyVectorOfTables(tablePos, 8 /*Spells*/, fb.protocol.db.raw.SpellVerify.Verify, false)
      && verifier.VerifyTableEnd(tablePos);
  }
}

}