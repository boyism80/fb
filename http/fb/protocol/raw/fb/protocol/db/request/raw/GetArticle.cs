// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace fb.protocol.db.request.raw
{

using global::System;
using global::System.Collections.Generic;
using global::Google.FlatBuffers;

public struct GetArticle : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static void ValidateVersion() { FlatBufferConstants.FLATBUFFERS_24_3_25(); }
  public static GetArticle GetRootAsGetArticle(ByteBuffer _bb) { return GetRootAsGetArticle(_bb, new GetArticle()); }
  public static GetArticle GetRootAsGetArticle(ByteBuffer _bb, GetArticle obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public static bool VerifyGetArticle(ByteBuffer _bb) {Google.FlatBuffers.Verifier verifier = new Google.FlatBuffers.Verifier(_bb); return verifier.VerifyBuffer("", false, GetArticleVerify.Verify); }
  public void __init(int _i, ByteBuffer _bb) { __p = new Table(_i, _bb); }
  public GetArticle __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public uint Section { get { int o = __p.__offset(4); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }
  public uint Article { get { int o = __p.__offset(6); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }

  public static Offset<fb.protocol.db.request.raw.GetArticle> CreateGetArticle(FlatBufferBuilder builder,
      uint section = 0,
      uint article = 0) {
    builder.StartTable(2);
    GetArticle.AddArticle(builder, article);
    GetArticle.AddSection(builder, section);
    return GetArticle.EndGetArticle(builder);
  }

  public static void StartGetArticle(FlatBufferBuilder builder) { builder.StartTable(2); }
  public static void AddSection(FlatBufferBuilder builder, uint section) { builder.AddUint(0, section, 0); }
  public static void AddArticle(FlatBufferBuilder builder, uint article) { builder.AddUint(1, article, 0); }
  public static Offset<fb.protocol.db.request.raw.GetArticle> EndGetArticle(FlatBufferBuilder builder) {
    int o = builder.EndTable();
    return new Offset<fb.protocol.db.request.raw.GetArticle>(o);
  }
  public static void FinishGetArticleBuffer(FlatBufferBuilder builder, Offset<fb.protocol.db.request.raw.GetArticle> offset) { builder.Finish(offset.Value); }
  public static void FinishSizePrefixedGetArticleBuffer(FlatBufferBuilder builder, Offset<fb.protocol.db.request.raw.GetArticle> offset) { builder.FinishSizePrefixed(offset.Value); }
}


static public class GetArticleVerify
{
  static public bool Verify(Google.FlatBuffers.Verifier verifier, uint tablePos)
  {
    return verifier.VerifyTableStart(tablePos)
      && verifier.VerifyField(tablePos, 4 /*Section*/, 4 /*uint*/, 4, false)
      && verifier.VerifyField(tablePos, 6 /*Article*/, 4 /*uint*/, 4, false)
      && verifier.VerifyTableEnd(tablePos);
  }
}

}
