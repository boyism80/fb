// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace fb.game.flatbuffer
{

using global::System;
using global::System.Collections.Generic;
using global::Google.FlatBuffers;

public struct ping : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static void ValidateVersion() { FlatBufferConstants.FLATBUFFERS_24_3_25(); }
  public static ping GetRootAsping(ByteBuffer _bb) { return GetRootAsping(_bb, new ping()); }
  public static ping GetRootAsping(ByteBuffer _bb, ping obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public static bool Verifyping(ByteBuffer _bb) {Google.FlatBuffers.Verifier verifier = new Google.FlatBuffers.Verifier(_bb); return verifier.VerifyBuffer("", false, pingVerify.Verify); }
  public void __init(int _i, ByteBuffer _bb) { __p = new Table(_i, _bb); }
  public ping __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }


  public static void Startping(FlatBufferBuilder builder) { builder.StartTable(0); }
  public static Offset<fb.game.flatbuffer.ping> Endping(FlatBufferBuilder builder) {
    int o = builder.EndTable();
    return new Offset<fb.game.flatbuffer.ping>(o);
  }
  public static void FinishpingBuffer(FlatBufferBuilder builder, Offset<fb.game.flatbuffer.ping> offset) { builder.Finish(offset.Value); }
  public static void FinishSizePrefixedpingBuffer(FlatBufferBuilder builder, Offset<fb.game.flatbuffer.ping> offset) { builder.FinishSizePrefixed(offset.Value); }
}


static public class pingVerify
{
  static public bool Verify(Google.FlatBuffers.Verifier verifier, uint tablePos)
  {
    return verifier.VerifyTableStart(tablePos)
      && verifier.VerifyTableEnd(tablePos);
  }
}

}
