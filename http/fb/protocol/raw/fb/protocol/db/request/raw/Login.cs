// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace fb.protocol.db.request.raw
{

using global::System;
using global::System.Collections.Generic;
using global::Google.FlatBuffers;

public struct Login : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static void ValidateVersion() { FlatBufferConstants.FLATBUFFERS_24_3_25(); }
  public static Login GetRootAsLogin(ByteBuffer _bb) { return GetRootAsLogin(_bb, new Login()); }
  public static Login GetRootAsLogin(ByteBuffer _bb, Login obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public static bool VerifyLogin(ByteBuffer _bb) {Google.FlatBuffers.Verifier verifier = new Google.FlatBuffers.Verifier(_bb); return verifier.VerifyBuffer("", false, LoginVerify.Verify); }
  public void __init(int _i, ByteBuffer _bb) { __p = new Table(_i, _bb); }
  public Login __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public uint Uid { get { int o = __p.__offset(4); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }

  public static Offset<fb.protocol.db.request.raw.Login> CreateLogin(FlatBufferBuilder builder,
      uint uid = 0) {
    builder.StartTable(1);
    Login.AddUid(builder, uid);
    return Login.EndLogin(builder);
  }

  public static void StartLogin(FlatBufferBuilder builder) { builder.StartTable(1); }
  public static void AddUid(FlatBufferBuilder builder, uint uid) { builder.AddUint(0, uid, 0); }
  public static Offset<fb.protocol.db.request.raw.Login> EndLogin(FlatBufferBuilder builder) {
    int o = builder.EndTable();
    return new Offset<fb.protocol.db.request.raw.Login>(o);
  }
  public static void FinishLoginBuffer(FlatBufferBuilder builder, Offset<fb.protocol.db.request.raw.Login> offset) { builder.Finish(offset.Value); }
  public static void FinishSizePrefixedLoginBuffer(FlatBufferBuilder builder, Offset<fb.protocol.db.request.raw.Login> offset) { builder.FinishSizePrefixed(offset.Value); }
}


static public class LoginVerify
{
  static public bool Verify(Google.FlatBuffers.Verifier verifier, uint tablePos)
  {
    return verifier.VerifyTableStart(tablePos)
      && verifier.VerifyField(tablePos, 4 /*Uid*/, 4 /*uint*/, 4, false)
      && verifier.VerifyTableEnd(tablePos);
  }
}

}