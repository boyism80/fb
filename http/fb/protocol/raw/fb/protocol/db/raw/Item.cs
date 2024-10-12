// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace fb.protocol.db.raw
{

using global::System;
using global::System.Collections.Generic;
using global::Google.FlatBuffers;

public struct Item : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static void ValidateVersion() { FlatBufferConstants.FLATBUFFERS_24_3_25(); }
  public static Item GetRootAsItem(ByteBuffer _bb) { return GetRootAsItem(_bb, new Item()); }
  public static Item GetRootAsItem(ByteBuffer _bb, Item obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public static bool VerifyItem(ByteBuffer _bb) {Google.FlatBuffers.Verifier verifier = new Google.FlatBuffers.Verifier(_bb); return verifier.VerifyBuffer("", false, ItemVerify.Verify); }
  public void __init(int _i, ByteBuffer _bb) { __p = new Table(_i, _bb); }
  public Item __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public uint User { get { int o = __p.__offset(4); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }
  public short Index { get { int o = __p.__offset(6); return o != 0 ? __p.bb.GetShort(o + __p.bb_pos) : (short)0; } }
  public short Parts { get { int o = __p.__offset(8); return o != 0 ? __p.bb.GetShort(o + __p.bb_pos) : (short)0; } }
  public short Deposited { get { int o = __p.__offset(10); return o != 0 ? __p.bb.GetShort(o + __p.bb_pos) : (short)0; } }
  public uint Model { get { int o = __p.__offset(12); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }
  public ushort Count { get { int o = __p.__offset(14); return o != 0 ? __p.bb.GetUshort(o + __p.bb_pos) : (ushort)0; } }
  public nullable.nullable_uint? Durability { get { int o = __p.__offset(16); return o != 0 ? (nullable.nullable_uint?)(new nullable.nullable_uint()).__assign(__p.__indirect(o + __p.bb_pos), __p.bb) : null; } }
  public string CustomName { get { int o = __p.__offset(18); return o != 0 ? __p.__string(o + __p.bb_pos) : null; } }
#if ENABLE_SPAN_T
  public Span<byte> GetCustomNameBytes() { return __p.__vector_as_span<byte>(18, 1); }
#else
  public ArraySegment<byte>? GetCustomNameBytes() { return __p.__vector_as_arraysegment(18); }
#endif
  public byte[] GetCustomNameArray() { return __p.__vector_as_array<byte>(18); }

  public static Offset<fb.protocol.db.raw.Item> CreateItem(FlatBufferBuilder builder,
      uint user = 0,
      short index = 0,
      short parts = 0,
      short deposited = 0,
      uint model = 0,
      ushort count = 0,
      Offset<nullable.nullable_uint> durabilityOffset = default(Offset<nullable.nullable_uint>),
      StringOffset custom_nameOffset = default(StringOffset)) {
    builder.StartTable(8);
    Item.AddCustomName(builder, custom_nameOffset);
    Item.AddDurability(builder, durabilityOffset);
    Item.AddModel(builder, model);
    Item.AddUser(builder, user);
    Item.AddCount(builder, count);
    Item.AddDeposited(builder, deposited);
    Item.AddParts(builder, parts);
    Item.AddIndex(builder, index);
    return Item.EndItem(builder);
  }

  public static void StartItem(FlatBufferBuilder builder) { builder.StartTable(8); }
  public static void AddUser(FlatBufferBuilder builder, uint user) { builder.AddUint(0, user, 0); }
  public static void AddIndex(FlatBufferBuilder builder, short index) { builder.AddShort(1, index, 0); }
  public static void AddParts(FlatBufferBuilder builder, short parts) { builder.AddShort(2, parts, 0); }
  public static void AddDeposited(FlatBufferBuilder builder, short deposited) { builder.AddShort(3, deposited, 0); }
  public static void AddModel(FlatBufferBuilder builder, uint model) { builder.AddUint(4, model, 0); }
  public static void AddCount(FlatBufferBuilder builder, ushort count) { builder.AddUshort(5, count, 0); }
  public static void AddDurability(FlatBufferBuilder builder, Offset<nullable.nullable_uint> durabilityOffset) { builder.AddOffset(6, durabilityOffset.Value, 0); }
  public static void AddCustomName(FlatBufferBuilder builder, StringOffset customNameOffset) { builder.AddOffset(7, customNameOffset.Value, 0); }
  public static Offset<fb.protocol.db.raw.Item> EndItem(FlatBufferBuilder builder) {
    int o = builder.EndTable();
    return new Offset<fb.protocol.db.raw.Item>(o);
  }
  public static void FinishItemBuffer(FlatBufferBuilder builder, Offset<fb.protocol.db.raw.Item> offset) { builder.Finish(offset.Value); }
  public static void FinishSizePrefixedItemBuffer(FlatBufferBuilder builder, Offset<fb.protocol.db.raw.Item> offset) { builder.FinishSizePrefixed(offset.Value); }
}


static public class ItemVerify
{
  static public bool Verify(Google.FlatBuffers.Verifier verifier, uint tablePos)
  {
    return verifier.VerifyTableStart(tablePos)
      && verifier.VerifyField(tablePos, 4 /*User*/, 4 /*uint*/, 4, false)
      && verifier.VerifyField(tablePos, 6 /*Index*/, 2 /*short*/, 2, false)
      && verifier.VerifyField(tablePos, 8 /*Parts*/, 2 /*short*/, 2, false)
      && verifier.VerifyField(tablePos, 10 /*Deposited*/, 2 /*short*/, 2, false)
      && verifier.VerifyField(tablePos, 12 /*Model*/, 4 /*uint*/, 4, false)
      && verifier.VerifyField(tablePos, 14 /*Count*/, 2 /*ushort*/, 2, false)
      && verifier.VerifyTable(tablePos, 16 /*Durability*/, nullable.nullable_uintVerify.Verify, false)
      && verifier.VerifyString(tablePos, 18 /*CustomName*/, false)
      && verifier.VerifyTableEnd(tablePos);
  }
}

}
