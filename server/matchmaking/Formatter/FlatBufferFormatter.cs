using Google.FlatBuffers;
using Newtonsoft.Json;
using Request = fb.protocol.matchmaking.request;
using Response = fb.protocol.matchmaking.response;

namespace Matchmaking.Formatter;

public class FlatBufferInputFormatter : Http.Formatter.FlatBufferInputFormatter
{
    protected override IFlatBufferEx GetProtocol(BinaryReader reader)
    {
        var protocolType = (Request.FlatBufferProtocolType)reader.ReadInt32();
        var size = reader.ReadInt32();
        var bytes = reader.ReadBytes(size);

        var type = Request.FlatBufferProtocolRouter.GetProtocolType(protocolType);
        return Activator.CreateInstance(type, bytes) as IFlatBufferEx;
    }

    protected override string OnLog(IFlatBufferEx protocol)
    {
        var protocolType = (Request.FlatBufferProtocolType)protocol.ProtocolType;
        return $"Request {protocolType} < {JsonConvert.SerializeObject(protocol)}";
    }
}

public class FlatBufferOutputFormatter : Http.Formatter.FlatBufferOutputFormatter
{
    protected override string OnLog(IFlatBufferEx protocol)
    {
        var protocolType = (Response.FlatBufferProtocolType)protocol.ProtocolType;
        return $"Response {protocolType} > {JsonConvert.SerializeObject(protocol)}";
    }
}
