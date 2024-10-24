// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace fb.protocol.db.request.raw
{

using global::System;
using global::System.Collections.Generic;
using global::Google.FlatBuffers;

public struct GetArticleList : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static void ValidateVersion() { FlatBufferConstants.FLATBUFFERS_24_3_25(); }
  public static GetArticleList GetRootAsGetArticleList(ByteBuffer _bb) { return GetRootAsGetArticleList(_bb, new GetArticleList()); }
  public static GetArticleList GetRootAsGetArticleList(ByteBuffer _bb, GetArticleList obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public static bool VerifyGetArticleList(ByteBuffer _bb) {Google.FlatBuffers.Verifier verifier = new Google.FlatBuffers.Verifier(_bb); return verifier.VerifyBuffer("", false, GetArticleListVerify.Verify); }
  public void __init(int _i, ByteBuffer _bb) { __p = new Table(_i, _bb); }
  public GetArticleList __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public uint Section { get { int o = __p.__offset(4); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }
  public uint Position { get { int o = __p.__offset(6); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }

  public static Offset<fb.protocol.db.request.raw.GetArticleList> CreateGetArticleList(FlatBufferBuilder builder,
      uint section = 0,
      uint position = 0) {
    builder.StartTable(2);
    GetArticleList.AddPosition(builder, position);
    GetArticleList.AddSection(builder, section);
    return GetArticleList.EndGetArticleList(builder);
  }

  public static void StartGetArticleList(FlatBufferBuilder builder) { builder.StartTable(2); }
  public static void AddSection(FlatBufferBuilder builder, uint section) { builder.AddUint(0, section, 0); }
  public static void AddPosition(FlatBufferBuilder builder, uint position) { builder.AddUint(1, position, 0); }
  public static Offset<fb.protocol.db.request.raw.GetArticleList> EndGetArticleList(FlatBufferBuilder builder) {
    int o = builder.EndTable();
    return new Offset<fb.protocol.db.request.raw.GetArticleList>(o);
  }
  public static void FinishGetArticleListBuffer(FlatBufferBuilder builder, Offset<fb.protocol.db.request.raw.GetArticleList> offset) { builder.Finish(offset.Value); }
  public static void FinishSizePrefixedGetArticleListBuffer(FlatBufferBuilder builder, Offset<fb.protocol.db.request.raw.GetArticleList> offset) { builder.FinishSizePrefixed(offset.Value); }
}


static public class GetArticleListVerify
{
  static public bool Verify(Google.FlatBuffers.Verifier verifier, uint tablePos)
  {
    return verifier.VerifyTableStart(tablePos)
      && verifier.VerifyField(tablePos, 4 /*Section*/, 4 /*uint*/, 4, false)
      && verifier.VerifyField(tablePos, 6 /*Position*/, 4 /*uint*/, 4, false)
      && verifier.VerifyTableEnd(tablePos);
  }
}

}
