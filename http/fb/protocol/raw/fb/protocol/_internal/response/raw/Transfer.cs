// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace fb.protocol._internal.response.raw
{

using global::System;
using global::System.Collections.Generic;
using global::Google.FlatBuffers;

public struct Transfer : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static void ValidateVersion() { FlatBufferConstants.FLATBUFFERS_24_3_25(); }
  public static Transfer GetRootAsTransfer(ByteBuffer _bb) { return GetRootAsTransfer(_bb, new Transfer()); }
  public static Transfer GetRootAsTransfer(ByteBuffer _bb, Transfer obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public static bool VerifyTransfer(ByteBuffer _bb) {Google.FlatBuffers.Verifier verifier = new Google.FlatBuffers.Verifier(_bb); return verifier.VerifyBuffer("", false, TransferVerify.Verify); }
  public void __init(int _i, ByteBuffer _bb) { __p = new Table(_i, _bb); }
  public Transfer __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public fb.protocol._internal.raw.TransferResult Code { get { int o = __p.__offset(4); return o != 0 ? (fb.protocol._internal.raw.TransferResult)__p.bb.GetSbyte(o + __p.bb_pos) : fb.protocol._internal.raw.TransferResult.Success; } }
  public string Ip { get { int o = __p.__offset(6); return o != 0 ? __p.__string(o + __p.bb_pos) : null; } }
#if ENABLE_SPAN_T
  public Span<byte> GetIpBytes() { return __p.__vector_as_span<byte>(6, 1); }
#else
  public ArraySegment<byte>? GetIpBytes() { return __p.__vector_as_arraysegment(6); }
#endif
  public byte[] GetIpArray() { return __p.__vector_as_array<byte>(6); }
  public ushort Port { get { int o = __p.__offset(8); return o != 0 ? __p.bb.GetUshort(o + __p.bb_pos) : (ushort)0; } }

  public static Offset<fb.protocol._internal.response.raw.Transfer> CreateTransfer(FlatBufferBuilder builder,
      fb.protocol._internal.raw.TransferResult code = fb.protocol._internal.raw.TransferResult.Success,
      StringOffset ipOffset = default(StringOffset),
      ushort port = 0) {
    builder.StartTable(3);
    Transfer.AddIp(builder, ipOffset);
    Transfer.AddPort(builder, port);
    Transfer.AddCode(builder, code);
    return Transfer.EndTransfer(builder);
  }

  public static void StartTransfer(FlatBufferBuilder builder) { builder.StartTable(3); }
  public static void AddCode(FlatBufferBuilder builder, fb.protocol._internal.raw.TransferResult code) { builder.AddSbyte(0, (sbyte)code, 0); }
  public static void AddIp(FlatBufferBuilder builder, StringOffset ipOffset) { builder.AddOffset(1, ipOffset.Value, 0); }
  public static void AddPort(FlatBufferBuilder builder, ushort port) { builder.AddUshort(2, port, 0); }
  public static Offset<fb.protocol._internal.response.raw.Transfer> EndTransfer(FlatBufferBuilder builder) {
    int o = builder.EndTable();
    return new Offset<fb.protocol._internal.response.raw.Transfer>(o);
  }
  public static void FinishTransferBuffer(FlatBufferBuilder builder, Offset<fb.protocol._internal.response.raw.Transfer> offset) { builder.Finish(offset.Value); }
  public static void FinishSizePrefixedTransferBuffer(FlatBufferBuilder builder, Offset<fb.protocol._internal.response.raw.Transfer> offset) { builder.FinishSizePrefixed(offset.Value); }
}


static public class TransferVerify
{
  static public bool Verify(Google.FlatBuffers.Verifier verifier, uint tablePos)
  {
    return verifier.VerifyTableStart(tablePos)
      && verifier.VerifyField(tablePos, 4 /*Code*/, 1 /*fb.protocol._internal.raw.TransferResult*/, 1, false)
      && verifier.VerifyString(tablePos, 6 /*Ip*/, false)
      && verifier.VerifyField(tablePos, 8 /*Port*/, 2 /*ushort*/, 2, false)
      && verifier.VerifyTableEnd(tablePos);
  }
}

}
