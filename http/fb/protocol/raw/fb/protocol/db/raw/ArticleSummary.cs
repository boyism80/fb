// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace fb.protocol.db.raw
{

using global::System;
using global::System.Collections.Generic;
using global::Google.FlatBuffers;

public struct ArticleSummary : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static void ValidateVersion() { FlatBufferConstants.FLATBUFFERS_24_3_25(); }
  public static ArticleSummary GetRootAsArticleSummary(ByteBuffer _bb) { return GetRootAsArticleSummary(_bb, new ArticleSummary()); }
  public static ArticleSummary GetRootAsArticleSummary(ByteBuffer _bb, ArticleSummary obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public static bool VerifyArticleSummary(ByteBuffer _bb) {Google.FlatBuffers.Verifier verifier = new Google.FlatBuffers.Verifier(_bb); return verifier.VerifyBuffer("", false, ArticleSummaryVerify.Verify); }
  public void __init(int _i, ByteBuffer _bb) { __p = new Table(_i, _bb); }
  public ArticleSummary __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public uint Id { get { int o = __p.__offset(4); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }
  public uint User { get { int o = __p.__offset(6); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }
  public string UserName { get { int o = __p.__offset(8); return o != 0 ? __p.__string(o + __p.bb_pos) : null; } }
#if ENABLE_SPAN_T
  public Span<byte> GetUserNameBytes() { return __p.__vector_as_span<byte>(8, 1); }
#else
  public ArraySegment<byte>? GetUserNameBytes() { return __p.__vector_as_arraysegment(8); }
#endif
  public byte[] GetUserNameArray() { return __p.__vector_as_array<byte>(8); }
  public string Title { get { int o = __p.__offset(10); return o != 0 ? __p.__string(o + __p.bb_pos) : null; } }
#if ENABLE_SPAN_T
  public Span<byte> GetTitleBytes() { return __p.__vector_as_span<byte>(10, 1); }
#else
  public ArraySegment<byte>? GetTitleBytes() { return __p.__vector_as_arraysegment(10); }
#endif
  public byte[] GetTitleArray() { return __p.__vector_as_array<byte>(10); }
  public string CreatedDate { get { int o = __p.__offset(12); return o != 0 ? __p.__string(o + __p.bb_pos) : null; } }
#if ENABLE_SPAN_T
  public Span<byte> GetCreatedDateBytes() { return __p.__vector_as_span<byte>(12, 1); }
#else
  public ArraySegment<byte>? GetCreatedDateBytes() { return __p.__vector_as_arraysegment(12); }
#endif
  public byte[] GetCreatedDateArray() { return __p.__vector_as_array<byte>(12); }

  public static Offset<fb.protocol.db.raw.ArticleSummary> CreateArticleSummary(FlatBufferBuilder builder,
      uint id = 0,
      uint user = 0,
      StringOffset user_nameOffset = default(StringOffset),
      StringOffset titleOffset = default(StringOffset),
      StringOffset created_dateOffset = default(StringOffset)) {
    builder.StartTable(5);
    ArticleSummary.AddCreatedDate(builder, created_dateOffset);
    ArticleSummary.AddTitle(builder, titleOffset);
    ArticleSummary.AddUserName(builder, user_nameOffset);
    ArticleSummary.AddUser(builder, user);
    ArticleSummary.AddId(builder, id);
    return ArticleSummary.EndArticleSummary(builder);
  }

  public static void StartArticleSummary(FlatBufferBuilder builder) { builder.StartTable(5); }
  public static void AddId(FlatBufferBuilder builder, uint id) { builder.AddUint(0, id, 0); }
  public static void AddUser(FlatBufferBuilder builder, uint user) { builder.AddUint(1, user, 0); }
  public static void AddUserName(FlatBufferBuilder builder, StringOffset userNameOffset) { builder.AddOffset(2, userNameOffset.Value, 0); }
  public static void AddTitle(FlatBufferBuilder builder, StringOffset titleOffset) { builder.AddOffset(3, titleOffset.Value, 0); }
  public static void AddCreatedDate(FlatBufferBuilder builder, StringOffset createdDateOffset) { builder.AddOffset(4, createdDateOffset.Value, 0); }
  public static Offset<fb.protocol.db.raw.ArticleSummary> EndArticleSummary(FlatBufferBuilder builder) {
    int o = builder.EndTable();
    return new Offset<fb.protocol.db.raw.ArticleSummary>(o);
  }
  public static void FinishArticleSummaryBuffer(FlatBufferBuilder builder, Offset<fb.protocol.db.raw.ArticleSummary> offset) { builder.Finish(offset.Value); }
  public static void FinishSizePrefixedArticleSummaryBuffer(FlatBufferBuilder builder, Offset<fb.protocol.db.raw.ArticleSummary> offset) { builder.FinishSizePrefixed(offset.Value); }
}


static public class ArticleSummaryVerify
{
  static public bool Verify(Google.FlatBuffers.Verifier verifier, uint tablePos)
  {
    return verifier.VerifyTableStart(tablePos)
      && verifier.VerifyField(tablePos, 4 /*Id*/, 4 /*uint*/, 4, false)
      && verifier.VerifyField(tablePos, 6 /*User*/, 4 /*uint*/, 4, false)
      && verifier.VerifyString(tablePos, 8 /*UserName*/, false)
      && verifier.VerifyString(tablePos, 10 /*Title*/, false)
      && verifier.VerifyString(tablePos, 12 /*CreatedDate*/, false)
      && verifier.VerifyTableEnd(tablePos);
  }
}

}