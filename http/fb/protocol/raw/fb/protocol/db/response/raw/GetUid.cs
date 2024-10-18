// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace fb.protocol.db.response.raw
{

using global::System;
using global::System.Collections.Generic;
using global::Google.FlatBuffers;

public struct GetUid : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static void ValidateVersion() { FlatBufferConstants.FLATBUFFERS_24_3_25(); }
  public static GetUid GetRootAsGetUid(ByteBuffer _bb) { return GetRootAsGetUid(_bb, new GetUid()); }
  public static GetUid GetRootAsGetUid(ByteBuffer _bb, GetUid obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public static bool VerifyGetUid(ByteBuffer _bb) {Google.FlatBuffers.Verifier verifier = new Google.FlatBuffers.Verifier(_bb); return verifier.VerifyBuffer("", false, GetUidVerify.Verify); }
  public void __init(int _i, ByteBuffer _bb) { __p = new Table(_i, _bb); }
  public GetUid __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public uint Uid { get { int o = __p.__offset(4); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }
  public bool Success { get { int o = __p.__offset(6); return o != 0 ? 0!=__p.bb.Get(o + __p.bb_pos) : (bool)false; } }

  public static Offset<fb.protocol.db.response.raw.GetUid> CreateGetUid(FlatBufferBuilder builder,
      uint uid = 0,
      bool success = false) {
    builder.StartTable(2);
    GetUid.AddUid(builder, uid);
    GetUid.AddSuccess(builder, success);
    return GetUid.EndGetUid(builder);
  }

  public static void StartGetUid(FlatBufferBuilder builder) { builder.StartTable(2); }
  public static void AddUid(FlatBufferBuilder builder, uint uid) { builder.AddUint(0, uid, 0); }
  public static void AddSuccess(FlatBufferBuilder builder, bool success) { builder.AddBool(1, success, false); }
  public static Offset<fb.protocol.db.response.raw.GetUid> EndGetUid(FlatBufferBuilder builder) {
    int o = builder.EndTable();
    return new Offset<fb.protocol.db.response.raw.GetUid>(o);
  }
  public static void FinishGetUidBuffer(FlatBufferBuilder builder, Offset<fb.protocol.db.response.raw.GetUid> offset) { builder.Finish(offset.Value); }
  public static void FinishSizePrefixedGetUidBuffer(FlatBufferBuilder builder, Offset<fb.protocol.db.response.raw.GetUid> offset) { builder.FinishSizePrefixed(offset.Value); }
}


static public class GetUidVerify
{
  static public bool Verify(Google.FlatBuffers.Verifier verifier, uint tablePos)
  {
    return verifier.VerifyTableStart(tablePos)
      && verifier.VerifyField(tablePos, 4 /*Uid*/, 4 /*uint*/, 4, false)
      && verifier.VerifyField(tablePos, 6 /*Success*/, 1 /*bool*/, 1, false)
      && verifier.VerifyTableEnd(tablePos);
  }
}

}