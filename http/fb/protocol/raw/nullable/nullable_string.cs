// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace nullable
{

using global::System;
using global::System.Collections.Generic;
using global::Google.FlatBuffers;

public struct nullable_string : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static void ValidateVersion() { FlatBufferConstants.FLATBUFFERS_24_3_25(); }
  public static nullable_string GetRootAsnullable_string(ByteBuffer _bb) { return GetRootAsnullable_string(_bb, new nullable_string()); }
  public static nullable_string GetRootAsnullable_string(ByteBuffer _bb, nullable_string obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __p = new Table(_i, _bb); }
  public nullable_string __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public string Value { get { int o = __p.__offset(4); return o != 0 ? __p.__string(o + __p.bb_pos) : null; } }
#if ENABLE_SPAN_T
  public Span<byte> GetValueBytes() { return __p.__vector_as_span<byte>(4, 1); }
#else
  public ArraySegment<byte>? GetValueBytes() { return __p.__vector_as_arraysegment(4); }
#endif
  public byte[] GetValueArray() { return __p.__vector_as_array<byte>(4); }

  public static Offset<nullable.nullable_string> Createnullable_string(FlatBufferBuilder builder,
      StringOffset valueOffset = default(StringOffset)) {
    builder.StartTable(1);
    nullable_string.AddValue(builder, valueOffset);
    return nullable_string.Endnullable_string(builder);
  }

  public static void Startnullable_string(FlatBufferBuilder builder) { builder.StartTable(1); }
  public static void AddValue(FlatBufferBuilder builder, StringOffset valueOffset) { builder.AddOffset(0, valueOffset.Value, 0); }
  public static Offset<nullable.nullable_string> Endnullable_string(FlatBufferBuilder builder) {
    int o = builder.EndTable();
    return new Offset<nullable.nullable_string>(o);
  }
}


static public class nullable_stringVerify
{
  static public bool Verify(Google.FlatBuffers.Verifier verifier, uint tablePos)
  {
    return verifier.VerifyTableStart(tablePos)
      && verifier.VerifyString(tablePos, 4 /*Value*/, false)
      && verifier.VerifyTableEnd(tablePos);
  }
}

}